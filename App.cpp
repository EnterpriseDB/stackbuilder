/////////////////////////////////////////////////////////////////////////////
// Name:        App.cpp
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.cpp,v 1.5 2007/03/24 20:58:04 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/progdlg.h>
#include <wx/stream.h>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/wfstream.h>
#include <wx/msw/registry.h>

// Application headers
#include "App.h"
#include "AppList.h"
#include "MD5.h"
#include "Mirror.h"

App::App(AppList *applist) 
{ 
	m_applist = applist; 
	sequence = 0; 
	download = false; 
	isDependency = false;
    downloaded = false;
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
			!mirrorpath.IsEmpty()); 
}

bool App::IsInstalled()
{
	// If the regkey for this app id exists, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Stack Builder\\Applications"));

	if (!key->Exists() || !key->HasValue(id.c_str()))
		return false;

	return true;
}

bool App::IsVersionInstalled()
{
	// If the regkey for this app id exists AND it contains our version number, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Stack Builder\\Applications\\"));

	if (!key->Exists() || !key->HasValue(id))
		return false;

	wxString ver;
	key->QueryValue(id, ver);
	if (ver != version)
		return false;

	return true;
}

bool App::WorksWithDB(ServerData *server)
{
	if (dbversion.Trim() == wxEmptyString || !server)
		return true;

	if (dbversion.Trim() == wxString::Format(wxT("%d.%d"), server->majorVer, server->minorVer))
		return true;

	return false;
}

wxString App::GetInstalledVersion()
{
	// If the regkey for this app id exists AND it contains our version number, it's installed.
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Stack Builder\\Applications\\"));

	if (!key->Exists() || !key->HasValue(id))
		return wxEmptyString;

	wxString ver;
	key->QueryValue(id, ver);
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
int App::RankDependencies(int rank)
{
	// Iterate through the dependencies, setting the sequence as required
	for (unsigned int i=0; i<dependencies.GetCount(); i++)
	{
		if (!m_applist->GetItem(i)->IsSelectedForDownload() || m_applist->GetItem(i)->sequence > 0)
			continue;

		rank = m_applist->GetItem(dependencies[i])->RankDependencies(rank);
	}
	sequence = rank;
	return sequence + 1;
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
              mirror->path, 
              mirrorpath));

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

    op->PutC(ip->GetC());
    downloaded++;

    while(!ip->Eof() && ip->LastRead() != 0)
    {
        ip->Read(buffer, 4096);
        chunk = ip->LastRead();
        op->Write(buffer, chunk);
        downloaded += chunk;
        count++;

        if (count == 4)
        {
            if (total)
                msg = wxString::Format(_("Downloaded %d of %d KB"), downloaded/1024, total/1024);
            else
                msg = wxString::Format(_("Downloaded %d KB"), downloaded/1024);

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
    }
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
