/////////////////////////////////////////////////////////////////////////////
// Name:        InstallationPage.h
// Purpose:     Installation page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: InstallationPage.cpp,v 1.2 2007/02/20 10:52:04 dpage Exp $
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

    mainSizer->Add(
        new wxStaticText(this, wxID_ANY,
                         _("Please click the \"Finish\" button to start the installations.")),
        0, wxALL, 5
	);

	mainSizer->Add(0, 20);

    mainSizer->Add(
        new wxStaticText(this, wxID_ANY,
						 _("Note: You must allow all installations to run to completion. If you are\n prompted to restart the computer, click \"No\" or \"Restart Later\" and\nmanually restart you computer when all the installation have finished.")),
        0, wxALL, 5
	);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}
