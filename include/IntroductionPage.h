/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.h,v 1.5 2008/08/14 15:54:08 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _INTRODUCTIONPAGE_H
#define _INTRODUCTIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/button.h>

class AppList;

class IntroductionPage : public wxWizardPageSimple
{
public:
    IntroductionPage(wxWizard *parent, AppList *applist);
    virtual void OnWizardPageChanging(wxWizardEvent& event);

private:
    bool FindPgServers();
    bool FindEdbServers();
    void OnProxies(wxCommandEvent& event);

    wxComboBox *m_installation;
    wxButton *m_proxies;
    AppList *m_applist;

    DECLARE_EVENT_TABLE()
};

#endif

