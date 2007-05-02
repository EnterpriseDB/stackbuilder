/////////////////////////////////////////////////////////////////////////////
// Name:        App.cpp
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.cpp,v 1.12 2007/05/02 12:44:43 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/progdlg.h>
#include <wx/stream.h>
#include <wx/timer.h>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/wfstream.h>
#include <wx/msw/registry.h>

// Application headers
#include "App.h"
#include "AppList.h"
#include "MD5.h"
#include "Mirror.h"
#include "Server.h"
#include "ProxyDialog.h"

App::App(AppList *applist, Server *server) 
{ 
	m_applist = applist; 
    m_server = server;
	sequence = 0; 
	download = false; 
	isDependency = false;
    downloaded = false;
    installed = false;
	m_tree = NULL; 
};

bool App::IsValid() 
{ 
	return (!id.IsEmpty() && 
		    !name.IsEmpty() && 
			!version.IsEmpty() && 
			!category.IsEmpty() && 
			!format.IsEmpty() && 
			!checksum.IsEmpty() && 
			!mirrorpath.IsEmpty() &&
            !versionkey.IsEmpty()); 
}

bool App::IsInstalled()
{

	// If the regkey for this app id exists, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\") + versionkey.BeforeLast('\\'));

	if (!key->Exists() || !key->HasValue(versionkey.AfterLast('\\')))
		return false;

	return true;
}

bool App::IsVersionInstalled()
{
	// If the regkey for this app id exists AND it contains our version number, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\") + versionkey.BeforeLast('\\'));

	if (!key->Exists() || !key->HasValue(versionkey.AfterLast('\\')))
		return false;

	wxString ver;
	key->QueryValue(versionkey.AfterLast('\\'), ver);
	if (ver != version)
		return false;

	return true;
}

bool App::WorksWithDB()
{
    wxString tmpversion;

    if (!m_server)
        return true;

    switch (m_server->serverType)
    {
        case SVR_POSTGRESQL:
            tmpversion = pgversion;
            break;

        case SVR_ENTERPRISEDB:
            tmpversion = edbversion;
            break;

        default:
            tmpversion = wxEmptyString;
    }

	if (tmpversion.Trim() == wxEmptyString)
		return true;

	if (tmpversion.Trim() == wxString::Format(wxT("%d.%d"), m_server->majorVer, m_server->minorVer))
		return true;

	return false;
}

wxString App::GetInstalledVersion()
{
	// If the regkey for this app id exists AND it contains our version number, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\") + versionkey.BeforeLast('\\'));

	if (!key->Exists() || !key->HasValue(versionkey.AfterLast('\\')))
		return wxEmptyString;

	wxString ver;
	key->QueryValue(versionkey.AfterLast('\\'), ver);
	return ver;
}

void App::SelectForDownload(bool select, bool isdep)
{
	// If this item doesn't have a checkbox image, we cannot select it.
	if (!m_tree || m_tree->GetItemImage(m_treeitem) >= 2)
		return;

	// Deselect item if required.
	if (!select)
	{
		download = false;
		isDependency = false;
		m_tree->SetItemImage(m_treeitem, 0);
		return;
	}
	else
	{
        // If this item is already selected, bail out to avoid
        // the possiblity of hitting an infinite loop.
        if (download)
            return;

		download = true;
		if (isdep)
			isDependency = true;

		m_tree->SetItemImage(m_treeitem, 1);

		// Select all dependencies
		for (unsigned int x=0; x < dependencies.Count(); x++)
		{
			for (unsigned int y=0; y < m_applist->Count(); y++)
			{
				if (m_applist->GetItem(y)->id == dependencies[x])
					m_applist->GetItem(y)->SelectForDownload(true, true);
			}
		}
	}
}

// Figure out what order to download and install
int App::RankDependencies(int rank, unsigned int depth)
{
    // Only recurse if we've not gone too deep
    if (depth < m_applist->Count())
    {
	    // Iterate through the dependencies, setting the sequence as required
	    for (unsigned int i=0; i<dependencies.GetCount(); i++)
	    {
            App *dep = m_applist->GetItem(dependencies[i]); 
		    if (!dep->IsSelectedForDownload() || dep->sequence > 0)
			    continue;

            depth++;
		    rank = dep->RankDependencies(rank, depth);
	    }
    }

    depth--;

    // The sequence might already be set if we've gone through a 
    // circular dependency. In that case, don't reset it.
    if (!sequence)
    {
	    sequence = rank;
	    return sequence + 1;
    }
    else
        return rank;
}

bool App::Download(const wxString& downloadPath, const Mirror *mirror)
{
    GetFilename(downloadPath);

    // If this file has already been downloaded, don't bother getting it again.
    // GetFilename would have set this flag if it found a matching filename and
    // the checksum was correct.
    if (downloaded)
        return true;

    wxProgressDialog *pd = new wxProgressDialog(wxString::Format(_("Downloading %s"), wxFileName(mirrorpath).GetFullName()),
                                                _("Connecting to server..."),
                                                100,
                                                NULL, 
                                                wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME);
    pd->Show();

    wxURL url(wxString::Format(wxT("%s://%s%s%s/%s"), 
              mirror->protocol, 
              mirror->hostname, 
              (mirror->port == 0 ? wxEmptyString : wxString::Format(wxT(":%d"), mirror->port)), 
              mirror->rootpath, 
              mirrorpath));

    url.SetProxy(ProxyDialog::GetProxy(mirror->protocol));

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
		wxLogError(wxString::Format(_("Failed to open %s\n\nError: %s"), url.BuildURI(), msg));
        pd->Show(false);
        delete pd;
        return false;
	}

    wxInputStream *ip = url.GetInputStream();

	if (!ip || !ip->IsOk())
	{
		wxLogError(wxString::Format(_("Failed to open %s\n\nError: The URL specified could not be opened."), url.BuildURI()));
        pd->Show(false);
        if (ip)
            delete ip;
        delete pd;
		return false;
	}

    wxFFileOutputStream *op = new wxFFileOutputStream(file.GetFullPath());

    size_t total = ip->GetSize();
    size_t downloaded = 0, chunk = 0;
    long count = 0;
    unsigned short buffer[4097];

    bool abort = false;
    wxString msg;

    wxStopWatch sw;
    long updateTime = 100;
    long speed = 0;

    do
    {
        ip->Read(buffer, 4096);
        chunk = ip->LastRead();
        op->Write(buffer, chunk);
        downloaded += chunk;
        count++;

        if (sw.Time() >= updateTime)
        {
            // Calculate the download speed
            if (updateTime >= 1000)
                speed = round((downloaded/1024) / (updateTime/1000));

            // Set the next time to update the progress dialog
            updateTime += 100;

            if (total)
                msg = wxString::Format(_("Downloaded %d of %d KB (%d KB/Sec)"), downloaded/1024, total/1024, speed);
            else
                msg = wxString::Format(_("Downloaded %d KB (%d KB/Sec)"), downloaded/1024, speed);

            if (!pd->Pulse(msg, &abort))
            {
                op->Close();
                wxRemoveFile(file.GetFullPath());
                delete ip;
                delete op;
                pd->Show(false);
                delete pd;
                return false;
            }
        count = 0;
        }
    } while (!ip->Eof() && ip->LastRead() != 0);

    op->Close();
    delete ip;
    delete op;
    pd->Show(false);
    delete pd;

    // Having downloaded the file, now verify the checksum
    wxString tmpsum;
    
    {
        wxBusyInfo wait(wxString::Format(_("Verifying checksum for: %s"), file.GetFullName()));
        tmpsum = md5sum(file.GetFullPath());
    }

    if (tmpsum.Lower() == checksum.Lower())
    {
        downloaded = true;
        return true;
    }
    else
    {
        downloaded = false;
        wxLogError(_("Checksum verification failed for: %s! Deleting file..."), file.GetFullName());
        wxRemoveFile(file.GetFullPath());
        return false;
    }

    return true;
}

void App::GetFilename(const wxString& downloadPath)
{
    wxFileName svrFile(mirrorpath);

    file = downloadPath + wxT("/") + svrFile.GetFullName();

    int ver = 1;

    while(file.FileExists())
    {
        // Check the file to see if it's checksum matches ours. If
        // if does, keep the filename and set the 'Downloaded' flag
        wxString tmpsum;
        
        {
            wxBusyInfo wait(wxString::Format(_("Checking existing file: %s"), file.GetFullName()));
            tmpsum = md5sum(file.GetFullPath());
        }

        if (tmpsum.Lower() == checksum.Lower())
        {
            downloaded = true;
            return;
        }

        // OK, the file is no good, so generate an incremented filename.
        file = downloadPath + wxT("/") + svrFile.GetName() + wxString::Format(wxT("-%d."), ver) + svrFile.GetExt();
        ver++;
    }

    downloaded = false;
}

bool App::Install()
{
    // Have we already installed this package in this session?
    if (installed)
        return true;

    wxString cmd;

    // MSI or EXE?
    if (format.Lower() == wxT("msi"))
        cmd = wxT("msiexec /i \"") + file.GetFullPath() + wxT("\"");
    else
        cmd = wxT("\"") + file.GetFullPath() + wxT("\"");

    // Install or upgrade?
    if (IsInstalled())
    {
        if (!upgradeoptions.IsEmpty())
            cmd += wxT(" ") + SubstituteFlags(upgradeoptions);
    }
    else
    {
        if (!installoptions.IsEmpty())
            cmd += wxT(" ") + SubstituteFlags(installoptions);
    }

    // Now run the installation
    long retval = wxExecute(cmd, wxEXEC_SYNC);

    if (retval == 0) // Installed OK
    {
        installed = true;
        return true;
    }
    else
    {
        int response = wxMessageBox(wxString::Format(_("The installation of %s returned an error.\n\n Click on the OK button to ignore this error and continue with any remaining installations, or click Cancel to abort the remaining installations.\n\nNote that ignoring this error may result in failure of any later installations that depend on this one."), this->name), 
                                                     _("Installation error"), 
                                                     wxOK | wxCANCEL | wxICON_EXCLAMATION);

        if (response == wxCANCEL) // USer cancelled the installations
            return false;
        else // User skipped this installation
        {
            m_applist->IncrementErrorCount();
            installed = true;
            return true;
        }
    }
}

wxString App::SubstituteFlags(const wxString &options)
{
    wxString retval = options;

    // Do we have any server options?
    if (!m_server)
    {
        retval.Replace(wxT("$LOCAL"), wxT("0"));
        retval.Replace(wxT("$PATH"), wxEmptyString);
        retval.Replace(wxT("$DATA"), wxEmptyString);
        retval.Replace(wxT("$VERSION"), wxEmptyString);
        retval.Replace(wxT("$PORT"), wxEmptyString);
        retval.Replace(wxT("$SERVICE"), wxEmptyString);
        retval.Replace(wxT("$ACCOUNT"), wxEmptyString);
        retval.Replace(wxT("$SUPER"), wxEmptyString);
        retval.Replace(wxT("$ENCODING"), wxEmptyString);
        retval.Replace(wxT("$LOCALE"), wxEmptyString);
    }
    else
    {
        retval.Replace(wxT("$LOCAL"), wxT("1"));
        retval.Replace(wxT("$PATH"), m_server->installationPath);
        retval.Replace(wxT("$DATA"), m_server->dataDirectory);
        retval.Replace(wxT("$VERSION"), m_server->serverVersion);
        retval.Replace(wxT("$PORT"), wxString::Format(wxT("%ld"), m_server->port));
        retval.Replace(wxT("$SERVICE"), m_server->serviceId);
        retval.Replace(wxT("$ACCOUNT"), m_server->serviceAccount);
        retval.Replace(wxT("$SUPER"), m_server->superuserName);
        retval.Replace(wxT("$ENCODING"), m_server->encoding);
        retval.Replace(wxT("$LOCALE"), m_server->locale);
    }

    return retval;
}