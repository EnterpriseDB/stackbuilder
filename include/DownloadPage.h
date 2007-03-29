/////////////////////////////////////////////////////////////////////////////
// Name:        DownloadPage.h
// Purpose:     Download page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: DownloadPage.h,v 1.4 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _DOWNLOADPAGE_H
#define _DOWNLOADPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/button.h>

class AppList;
class MirrorList;

class DownloadPage : public wxWizardPageSimple
{
public:
    DownloadPage(wxWizard *parent, AppList *applist, MirrorList *mirrorlist);
    virtual void OnWizardPageChanging(wxWizardEvent& event);

private:
    void OnBrowse(wxCommandEvent& WXUNUSED(event));

    wxTextCtrl *m_path;
    wxButton *m_browse;
    wxString m_downloadPath; 
    AppList *m_applist;
    MirrorList *m_mirrorlist;

    DECLARE_EVENT_TABLE()
};

#endif