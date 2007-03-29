/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.h
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.h,v 1.3 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _STACKBUILDER_H
#define _STACKBUILDER_H

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class Wizard;

#define DEFAULT_MIRROR_LIST_URL wxT("http://www.postgresql.org/mirrors.xml")
#define DEFAULT_APPLICATION_LIST_URL wxT("http://www.postgresql.org/files/stackbuilder/applications.xml")

class StackBuilder : public wxApp
{
public:
    virtual bool OnInit();

private:
	void OnWizardCancelled(wxWizardEvent &evt);
	void OnWizardFinished(wxWizardEvent &evt);

    Wizard *wizard;

	DECLARE_EVENT_TABLE()
};

#endif