/////////////////////////////////////////////////////////////////////////////
// Name:        AppList.cpp
// Purpose:     Maintains the list of applications
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: AppList.cpp,v 1.11 2007/05/02 13:45:28 dpage Exp $
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

// Application headers
#include "App.h"
#include "AppList.h"
#include "Mirror.h"
#include "ProxyDialog.h"

class Server;

// Define the AppArray type
WX_DEFINE_OBJARRAY(AppArray);

bool AppList::LoadAppList()
{
    wxURL url(m_applicationListUrl);
    url.SetProxy(ProxyDialog::GetProxy(wxT("http")));
	wxURLError err = url.GetError();
    if (err != wxURL_NOERR)
	{
		wxString msg;
		switch (err)
		{
			case wxURL_SNTXERR:
				msg = _("Could not parse the URL.");
				break;
			case wxURL_NOPROTO:
				msg = _("Unsupported protocol specified.");
				break;
			case wxURL_NOHOST:
				msg = _("No hostname specified in URL.");
				break;
			case wxURL_NOPATH:
				msg = _("No path specified in URL.");
				break;
			case wxURL_CONNERR:
				msg = _("A connection error occurred.");
				break;
			case wxURL_PROTOERR:
				msg = _("A protocol error occurred.");
				break;
		}
		wxLogError(wxString::Format(_("Failed to open the application list: %s\n\nError: %s"), m_applicationListUrl, msg));
        return false;
	}

    wxInputStream *ip = url.GetInputStream();

	if (!ip || !ip->IsOk())
	{
		wxLogError(wxString::Format(_("Failed to open the application list: %s\n\nError: The URL specified could not be opened."), m_applicationListUrl));
		return false;
	}

	wxXmlDocument xml;
	if (!xml.Load(*ip))
	{
		wxLogError(wxString::Format(_("Failed to parse the application list: %s"), m_applicationListUrl));
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
				else if (properties->GetName() == wxT("name"))
					newApplication->name = properties->GetNodeContent();
				else if (properties->GetName() == wxT("description"))
					newApplication->description = properties->GetNodeContent();
				else if (properties->GetName() == wxT("version"))
					newApplication->version = properties->GetNodeContent();
				else if (properties->GetName() == wxT("category"))
					newApplication->category = properties->GetNodeContent();
				else if (properties->GetName() == wxT("pgversion"))
					newApplication->pgversion = properties->GetNodeContent();
				else if (properties->GetName() == wxT("edbversion"))
					newApplication->edbversion = properties->GetNodeContent();
				else if (properties->GetName() == wxT("format"))
					newApplication->format = properties->GetNodeContent();
				else if (properties->GetName() == wxT("installoptions"))
					newApplication->installoptions = properties->GetNodeContent();
				else if (properties->GetName() == wxT("upgradeoptions"))
					newApplication->upgradeoptions = properties->GetNodeContent();
				else if (properties->GetName() == wxT("checksum"))
					newApplication->checksum = properties->GetNodeContent();
				else if (properties->GetName() == wxT("mirrorpath"))
					newApplication->mirrorpath = properties->GetNodeContent();
				else if (properties->GetName() == wxT("dependency"))
					newApplication->dependencies.Add(properties->GetNodeContent());
				else if (properties->GetName() == wxT("versionkey"))
					newApplication->versionkey = properties->GetNodeContent();
				else if (properties->GetName() == wxT("alturl"))
					newApplication->alturl = properties->GetNodeContent();

				properties = properties->GetNext();
			}

            // Cleanup the mirrorpath
            if (newApplication->mirrorpath.StartsWith(wxT("/")))
                newApplication->mirrorpath = newApplication->mirrorpath.Right(newApplication->mirrorpath.Length() -1);

			if (newApplication->IsValid() && newApplication->WorksWithDB())
				m_apps.Add(newApplication);
		}
		application = application->GetNext();
	}

	return true;
}

bool AppList::PopulateTreeCtrl()
{
	wxTreeItemIdValue cookie;
	wxTreeItemId node, root, category, application;
	bool found;

	root = m_treectrl->AddRoot(_("Categories"), 3);

	for (unsigned int i=0; i<m_apps.GetCount(); i++)
	{
		found = false;

		node = m_treectrl->GetFirstChild(root, cookie);

		while (node)
		{
			if (m_treectrl->GetItemText(node) == m_apps[i].category)
			{
				category = node;
				found = true;
				break;
			}
			node = m_treectrl->GetNextChild(root, cookie);
		}

		if (!found)
			category = m_treectrl->AppendItem(root, m_apps[i].category, 3);

		if (m_apps[i].IsVersionInstalled())
		    application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s (installed)"), m_apps[i].name, m_apps[i].version), 2, -1, &m_apps[i]);
		else if (m_apps[i].IsInstalled())
		    application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s (v%s installed)"), m_apps[i].name, m_apps[i].version, m_apps[i].GetInstalledVersion()), 0, -1, &m_apps[i]);
		else
			application = m_treectrl->AppendItem(category, wxString::Format(_("%s v%s"), m_apps[i].name, m_apps[i].version), 0, -1, &m_apps[i]);

		m_apps[i].SelectForDownload(false, false);
		m_apps[i].m_tree = m_treectrl;
        m_apps[i].m_treeitem = application;

        m_treectrl->SortChildren(category);
	}

    m_treectrl->SortChildren(root);
	m_treectrl->ExpandAll();

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