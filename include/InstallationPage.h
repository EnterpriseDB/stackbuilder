/////////////////////////////////////////////////////////////////////////////
// Name:        InstallationPage.h
// Purpose:     Installation page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: InstallationPage.h,v 1.3 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _INSTALLATIONPAGE_H
#define _INSTALLATIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class AppList;

class InstallationPage : public wxWizardPageSimple
{
public:
    InstallationPage(wxWizard *parent, AppList *applist);
    void OnWizardPageChanging(wxWizardEvent& event);

private:
    AppList *m_applist;

	DECLARE_EVENT_TABLE()
};

#endif