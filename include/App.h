/////////////////////////////////////////////////////////////////////////////
// Name:        App.h
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.h,v 1.17 2010/06/18 09:21:15 sachin Exp $
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
    bool IsRequired();
    bool IsDownloaded() { return downloaded; }
    bool WorksWithDB();
    bool WorksWithPlatform();
    wxString GetInstalledVersion();
    void SelectForDownload(bool select, bool isdep);
    bool IsSelectedForDownload() { return download; };
    bool IsSelectedAsDependency() { return isDependency; };
    int RankDependencies(int rank, unsigned int depth = 0);
    bool Download(const wxString& downloadPath, const Mirror *mirror);
    bool Install();

    wxString id;
    wxString platform;
    wxString secondaryplatform;
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
    wxString alturl;

    wxTreeItemId m_treeitem;
    wxTreeCtrl *m_tree;

    int sequence;

private:
    bool CheckFilename(const wxString& downloadPath);
    wxString SubstituteFlags(const wxString &options);
#ifdef __WXMAC__
    wxString GetBundleExecutable(const wxString &bundle);
#endif
#ifndef __WXMSW__
    int ExecProcess(const wxString &cmd);
#endif

    bool download, isDependency, downloaded, installed;
    wxFileName file;
    AppList *m_applist;
    Server *m_server;
};

#endif

