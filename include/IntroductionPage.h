/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.h,v 1.2 2007/02/20 10:52:04 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _INTRODUCTIONPAGE_H
#define _INTRODUCTIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "AppList.h"

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