/////////////////////////////////////////////////////////////////////////////
// Name:        App.h
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.h,v 1.8 2007/04/03 15:25:28 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _APP_H
#define _APP_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/treectrl.h>

class Mirror;
class AppList;
class Server;

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

class App : public wxTreeItemData
{
public:
	App(AppList *applist, Server *server);
	bool IsValid();
	bool IsInstalled();
	bool IsVersionInstalled();
    bool WorksWithDB();
	wxString GetInstalledVersion();
	void SelectForDownload(bool select, bool isdep);
	bool IsSelectedForDownload() { return download; };
	bool IsSelectedAsDependency() { return isDependency; };
	int RankDependencies(int rank, unsigned int depth = 0);
    bool Download(const wxString& downloadPath, const Mirror *mirror);
    bool Install();

	wxString id;
	wxString name;
	wxString description;
	wxString version;
	wxString category;
	wxString pgversion;
	wxString edbversion;
	wxString format;
	wxString installoptions;
	wxString upgradeoptions;
	wxString checksum;
	wxString mirrorpath;
	wxArrayString dependencies;
    wxString versionkey;

	wxTreeItemId m_treeitem;
	wxTreeCtrl *m_tree;

	int sequence;

private:
    void GetFilename(const wxString& downloadPath);
    wxString SubstituteFlags(const wxString &options);

	bool download, isDependency, downloaded, installed;
    wxFileName file;
	AppList *m_applist;
    Server *m_server;
};

#endif