/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.h
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.h,v 1.5 2007/05/01 11:17:09 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _STACKBUILDER_H
#define _STACKBUILDER_H

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// App headers
#include "Config.h"

class Wizard;

class StackBuilder : public wxApp
{
public:
    virtual bool OnInit();

private:
	void OnWizardCancelled(wxWizardEvent &evt);
	void OnWizardFinished(wxWizardEvent &evt);
    void initializeLocale(wxChar *argv0, const wxString &lang);

    Wizard *wizard;

	DECLARE_EVENT_TABLE()
};

#endif