/////////////////////////////////////////////////////////////////////////////
// Name:        InstallationPage.h
// Purpose:     Installation page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: InstallationPage.cpp,v 1.3 2007/03/23 21:19:07 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "InstallationPage.h"

InstallationPage::InstallationPage(wxWizard *parent) 
	: wxWizardPageSimple(parent)
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("All the installation files have now been successfully downloaded."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 20);

	st = new wxStaticText(this, wxID_ANY, _("Please click the \"Finish\" button to start the installations."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 20);

    st = new wxStaticText(this, wxID_ANY, _("Note: You must allow all installations to run to completion. If you are prompted to restart the computer, click \"No\" or \"Restart Later\" and manually restart your computer when all the installation have finished."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 20);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}
