/////////////////////////////////////////////////////////////////////////////
// Name:        AppList.cpp
// Purpose:     Maintains the list of applications
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: AppList.cpp,v 1.24 2010/06/03 10:45:11 sachin Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

// Application headers
#include "App.h"
#include "AppList.h"
#include "Mirror.h"
#include "ProxyDialog.h"
#include "CurlHandler.h"

class Server;

// Define the AppArray type
WX_DEFINE_OBJARRAY(AppArray);

bool AppList::LoadAppList()
{
    wxString readBuffer = wxEmptyString;

    // Fetch the application list using libcurl
    CurlHandler curlObj;
    curlObj.SetApplicationUrl(m_applicationListUrl);
    bool bIsSuccess = curlObj.ParseApplicationList();
    if (!bIsSuccess)
        return false;

    readBuffer = curlObj.GetApplicationList();

    wxStringInputStream ip(readBuffer);
    wxXmlDocument xml;
    bool xmlLoaded = false;
    {
        wxLogNull noLog;
        xmlLoaded = xml.Load(ip);
    }

    if (!xmlLoaded)
    {
        wxLogError(wxString::Format(_("Failed to load application list: %s"), m_applicationListUrl.c_str()));
        return false;
    }

    // Clear the applist
    m_apps.Clear();

    // Iterate through the applications and build the list
    wxXmlNode *application, *properties;
    application = xml.GetRoot()->GetChildren();

    while (application)
    {
        if (application->GetName() == wxT("application"))
        {
            App *newApplication = new App(this, m_server);
            properties = application->GetChildren();

            while (properties)
            {
                if (properties->GetName() == wxT("id"))
                    newApplication->id = properties->GetNodeContent();
                else if (properties->GetName() == wxT("platform"))
                    newApplication->platform = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("secondaryplatform"))
                    newApplication->secondaryplatform = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("name"))
                    newApplication->name = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("description"))
                    newApplication->description = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("version"))
                    newApplication->version = properties->GetNodeContent();
                else if (properties->GetName() == wxT("category"))
                    newApplication->category = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("pgversion"))
                    newApplication->pgversion = properties->GetNodeContent();
                else if (properties->GetName() == wxT("edbversion"))
                    newApplication->edbversion = properties->GetNodeContent();
                else if (properties->GetName() == wxT("format"))
                    newApplication->format = properties->GetNodeContent();
                else if (properties->GetName() == wxT("installoptions"))
                    newApplication->installoptions = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("upgradeoptions"))
                    newApplication->upgradeoptions = DeHTMLise(properties->GetNodeContent());
                else if (properties->GetName() == wxT("checksum"))
                    newApplication->checksum = properties->GetNodeContent();
                else if (properties->GetName() == wxT("mirrorpath"))
                    newApplication->mirrorpath = properties->GetNodeContent();
                else if (properties->GetName() == wxT("dependency"))
                    newApplication->dependencies.Add(DeHTMLise(properties->GetNodeContent()));
                else if (properties->GetName() == wxT("versionkey"))
                    newApplication->versionkey = properties->GetNodeContent();
                else if (properties->GetName() == wxT("alturl"))
                    newApplication->alturl = properties->GetNodeContent();

                properties = properties->GetNext();
            }

            // Cleanup the mirrorpath
            if (newApplication->mirrorpath.StartsWith(wxT("/")))
                newApplication->mirrorpath = newApplication->mirrorpath.Right(newApplication->mirrorpath.Length() -1);

            if (newApplication->IsValid() && newApplication->WorksWithDB() && newApplication->WorksWithPlatform())
                m_apps.Add(newApplication);
        }
        application = application->GetNext();
    }

    return true;
}

wxString AppList::DeHTMLise(const wxString &string)
{
    wxString ret = string;

    ret.Replace(wxT("&quot;"), wxT("\""));
    ret.Replace(wxT("&lt;"), wxT("<"));
    ret.Replace(wxT("&gt;"), wxT(">"));
    ret.Replace(wxT("&ldquo;"), wxT("\""));
    ret.Replace(wxT("&rdquo;"), wxT("\""));
    ret.Replace(wxT("&#039;"), wxT("'"));

    return ret;
}

wxTreeItemId AppList::FindCategory(wxTreeItemId parent, wxString &category)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId node;

    node = m_treectrl->GetFirstChild(parent, cookie);

    while (node)
    {
	if (m_treectrl->GetItemText(node) == category)
	    return node;
        node = m_treectrl->GetNextChild(parent, cookie);
    }
    return node;
}


bool AppList::PopulateTreeCtrl()
{
    wxTreeItemId node, root, category, application, parentNode;

    root = m_treectrl->AddRoot(_("Categories"), 3);

    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        parentNode = root;

        wxStringTokenizer tokens(m_apps[i].category, wxT("\\"));
        while (tokens.HasMoreTokens())
        {
            wxString ctg = tokens.GetNextToken();
            node = FindCategory(parentNode, ctg);
            if (!node)
                node =  m_treectrl->AppendItem(parentNode, ctg, 3);
            parentNode = node;
        }
        category = node;

        // We used to disable installed apps here (by using image #2), but that
        // may not be the best strategy as it prevents us from reinstalling apps.
        if (m_apps[i].IsVersionInstalled())
            application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s (installed)"), m_apps[i].name.c_str(), m_apps[i].version.c_str()), 0, -1, &m_apps[i]);
        else if (m_apps[i].IsInstalled())
            application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s (v%s installed)"), m_apps[i].name.c_str(), m_apps[i].version.c_str(), m_apps[i].GetInstalledVersion().c_str()), 0, -1, &m_apps[i]);
        else
            application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s"), m_apps[i].name.c_str(), m_apps[i].version.c_str()), 0, -1, &m_apps[i]);

        m_apps[i].SelectForDownload(false, false);
        m_apps[i].m_tree = m_treectrl;
        m_apps[i].m_treeitem = application;

        m_treectrl->SortChildren(category);
    }

    m_treectrl->SortChildren(root);
    m_treectrl->Expand(root);

    return true;
}

// Check to see if any downloads have been selected.
bool AppList::HaveDownloads()
{
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        if (m_apps[i].IsSelectedForDownload() == true)
            return true;
    }
    return false;
}

// Check to see if any downloads are coming from mirrors.
bool AppList::UsingMirrors()
{
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        if (m_apps[i].IsSelectedForDownload() == true && m_apps[i].alturl == wxEmptyString)
            return true;
    }
    return false;
}

// Figure out what order to download and install
void AppList::RankDownloads()
{
    int rank = 1;

    // Clear any existing rankings
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
        m_apps[i].sequence = 0;

    // For each app in the list, if it's selected for download,
    // scan to the bottom of the dependency tree and rank the
    // downloads from the bottom level back up. If a package has
    // already been ranked, we can ignore it as we must have
    // ranked it and it's dependencies already.
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        // If the app isn't selected for download, or has already
        // been ranked, ignore it.
        if (!m_apps[i].IsSelectedForDownload() || m_apps[i].sequence > 0)
            continue;

        rank = m_apps[i].RankDependencies(rank, 0);
    }
}

// Clear the list, but unlink the tree items first.
void AppList::DeleteAllItems()
{
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        m_treectrl->SetItemData(m_apps[i].m_treeitem, NULL);
        m_apps[i].m_treeitem = 0;
    }
    m_apps.Clear();
}


App *AppList::GetItem(const wxString &appid)
{
    for (unsigned int i=0; i<m_apps.GetCount(); i++)
    {
        if (m_apps[i].id == appid)
            return &m_apps[i];
    }
    return NULL;
}

bool AppList::DownloadFiles(const wxString& downloadPath, const Mirror *mirror)
{
    unsigned int x = 1;

    // Loop round once for every app. For each loop, search the
    // applist for a matching download.
    while(x <= m_apps.GetCount())
    {
        for (unsigned int i=0; i<m_apps.GetCount(); i++)
        {
            if (m_apps[i].sequence == x)
            {
                if (!m_apps[i].Download(downloadPath, mirror))
                    return false;
            }
        }
        x++;
    }

    return true;
}

bool AppList::InstallApps()
{
    unsigned int x = 1;

    // Loop round once for every app. For each loop, search the
    // applist for a matching download.
    while(x <= m_apps.GetCount())
    {
        for (unsigned int i=0; i<m_apps.GetCount(); i++)
        {
            if (m_apps[i].sequence == x)
            {
                if (!m_apps[i].Install())
                    return false;
            }
        }
        x++;
    }

    return true;
}

wxArrayString AppList::GetSummary()
{
    wxArrayString summary;
    unsigned int x = 1;

    // Loop round once for every app. For each loop, search the
    // applist for a matching download.
    while(x <= m_apps.GetCount())
    {
        for (unsigned int i=0; i<m_apps.GetCount(); i++)
        {
            if (m_apps[i].sequence == x)
            {
                summary.Add(m_apps[i].name + wxT(" v") + m_apps[i].version);
            }
        }
        x++;
    }

    return summary;
}
