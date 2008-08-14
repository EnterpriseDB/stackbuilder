/////////////////////////////////////////////////////////////////////////////
// Name:        CompletionPage.h
// Purpose:     Completion page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: CompletionPage.h,v 1.2 2008/08/14 15:54:08 dpage Exp $
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

private:
    wxStaticText *stStatus;
};

#endif

