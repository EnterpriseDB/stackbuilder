/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.h
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.h,v 1.10 2010/06/03 19:43:35 sachin Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _STACKBUILDER_H
#define _STACKBUILDER_H

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/stdpaths.h>

// App headers
#include "Config.h"

// Keep this as a global, rather than passing it through lots of places
extern wxString downloadCounterUrl;
extern wxString g_certificateBundle;

class Wizard;

class StackBuilder : public wxApp
{
public:
    virtual bool OnInit();
private:
    void OnWizardCancelled(wxWizardEvent &evt);
    void OnWizardFinished(wxWizardEvent &evt);
    void initializeLocale(const wxString &appPath, const wxString &lang);

    Wizard *wizard;

    DECLARE_EVENT_TABLE()
};

#endif

