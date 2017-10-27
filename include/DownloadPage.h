/////////////////////////////////////////////////////////////////////////////
// Name:        DownloadPage.h
// Purpose:     Download page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: DownloadPage.h,v 1.7 2008/08/14 15:54:08 dpage Exp $
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
    void SetSummary(const wxArrayString &s) { m_summary->Clear(); m_summary->InsertItems(s, 0); }

private:
    void OnBrowse(wxCommandEvent& WXUNUSED(event));

    wxTextCtrl *m_path;
    wxListBox *m_summary;
    wxButton *m_browse;
    AppList *m_applist;
    MirrorList *m_mirrorlist;

    DECLARE_EVENT_TABLE()
};

#endif

