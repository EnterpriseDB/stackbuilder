/////////////////////////////////////////////////////////////////////////////
// Name:        CompletionPage.h
// Purpose:     Completion page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: CompletionPage.h,v 1.3 2010/06/03 10:45:11 sachin Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _COMPLETIONPAGE_H
#define _COMPLETIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class CompletionPage : public wxWizardPageSimple
{
public:
    CompletionPage(wxWizard *parent);
    void ShowErrorWarning(const int errors);
    void SetPageText(bool installationSkipped);
    void DisableBackButton();

private:
    wxStaticText *stTitle;
    wxStaticText *stBody;
    wxStaticText *stStatus;
};

#endif

