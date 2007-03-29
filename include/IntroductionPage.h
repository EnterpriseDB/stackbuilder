/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.h,v 1.3 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _INTRODUCTIONPAGE_H
#define _INTRODUCTIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class AppList;

class IntroductionPage : public wxWizardPageSimple
{
public:
    IntroductionPage(wxWizard *parent, AppList *applist);
    virtual void OnWizardPageChanging(wxWizardEvent& event);

private:
	bool FindPgServers();
	bool FindEdbServers();

    wxComboBox *m_installation;
	AppList *m_applist;

	DECLARE_EVENT_TABLE()
};

#endif