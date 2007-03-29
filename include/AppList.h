/////////////////////////////////////////////////////////////////////////////
// Name:        AppList.h
// Purpose:     Maintains the list of applications
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: AppList.h,v 1.6 2007/03/29 15:08:53 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _APPLIST_H
#define _APPLIST_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/treectrl.h>
#include <wx/xml/xml.h>

class App;
class Mirror;
class Server;

WX_DECLARE_OBJARRAY(App, AppArray);

class AppList
{
public:
	AppList(const wxString &applicationListUrl) { m_applicationListUrl = applicationListUrl; errors = 0; };
	bool LoadAppList();
	bool PopulateTreeCtrl();
    bool HaveDownloads();
	App *GetItem(unsigned int index) { return &m_apps[index]; };
    App *GetItem(const wxString &appid);
	size_t Count() { return m_apps.Count(); };
	void SetTree(wxTreeCtrl *tree) { m_treectrl = tree; };
	void SetServer(Server *server) { m_server = server; };
	void RankDownloads();
	void DeleteAllItems();
    bool DownloadFiles(const wxString& downloadPath, const Mirror *mirror);
    bool InstallApps();
    void IncrementErrorCount() { errors++; };
    int GetErrorCount() { return errors; };

private:
	AppArray m_apps;
	wxString m_applicationListUrl;
	Server *m_server;
	wxTreeCtrl *m_treectrl;
    int errors;
};

#endif
