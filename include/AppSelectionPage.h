/////////////////////////////////////////////////////////////////////////////
// Name:        AppSelectionPage.h
// Purpose:     Application selection page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: AppSelectionPage.h,v 1.5 2008/08/14 15:54:08 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _APPSELECTIONPAGE_H
#define _APPSELECTIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>

class AppList;
class MirrorList;

class AppTreeCtrl : public wxTreeCtrl
{
public:
    AppTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);

private:
    void OnLeftClick(wxMouseEvent &evt);

    DECLARE_EVENT_TABLE()
};

class AppSelectionPage : public wxWizardPageSimple
{
public:
    AppSelectionPage(wxWizard *parent, AppList *applist, MirrorList *mirrorlist, wxWizardPageSimple *mirrorpage, wxWizardPageSimple *downloadpage);
    virtual void OnWizardPageChanging(wxWizardEvent& event);
    wxTreeCtrl *GetTreeCtrl() { return m_apptree; };
    void ReChain();

private:
    void OnTreeItemSelected(wxTreeEvent &evt);
    void OnTreeItemActivated(wxTreeEvent &evt);

    AppTreeCtrl *m_apptree;
    wxImageList *m_treeimages;
    wxTextCtrl *m_description;
    AppList *m_applist;
    MirrorList *m_mirrorlist;
    wxWizardPageSimple *m_mirrorpage, *m_downloadpage;

    DECLARE_EVENT_TABLE()
};

#endif

