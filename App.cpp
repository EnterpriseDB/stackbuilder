/////////////////////////////////////////////////////////////////////////////
// Name:        App.cpp
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.cpp,v 1.1 2007/02/19 09:57:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/msw/registry.h>

// Application headers
#include "App.h"
#include "AppList.h"

App::App(AppList *applist) 
{ 
	m_applist = applist; 
	sequence = 0; 
	download = false; 
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

void App::SelectForDownload(bool select)
{
	// If this item doesn't have a checkbox image, we cannot select it.
	if (!m_tree || m_tree->GetItemImage(m_treeitem) >= 2)
		return;

	// Deselect item if required.
	if (!select)
	{
		download = false;
		m_tree->SetItemImage(m_treeitem, 0);
		return;
	}
	else
	{
		download = true;
		m_tree->SetItemImage(m_treeitem, 1);

		// Select all dependencies
		for (unsigned int x=0; x < dependencies.Count(); x++)
		{
			for (unsigned int y=0; y < m_applist->Count(); y++)
			{
				if (m_applist->GetItem(y)->id == dependencies[x])
					m_applist->GetItem(y)->SelectForDownload(true);
			}
		}
	}
}