/////////////////////////////////////////////////////////////////////////////
// Name:        CompletionPage.h
// Purpose:     CompletionPage page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: CompletionPage.cpp,v 1.3 2008/01/24 14:38:25 h-saito Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "CompletionPage.h"

CompletionPage::CompletionPage(wxWizard *parent)
	: wxWizardPageSimple(parent)
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Installation complete!"));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL | wxALIGN_CENTER | wxFIXED_MINSIZE, 5);

//	mainSizer->Add(0, 20);

    st = new wxStaticText(this, wxID_ANY, _("Installation of the packages you selected has finished. The downloaded files have been retained to allow future installations or upgrades (some packages require the original installation files when being upgraded)."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

	mainSizer->SetItemMinSize(st, 400, 60);
//	mainSizer->Add(0, 10);

    stStatus = new wxStaticText(this, wxID_ANY, wxEmptyString);
    mainSizer->Add(stStatus, 0, wxALL | wxFIXED_MINSIZE, 5);

//	mainSizer->Add(0, 20);

    st = new wxStaticText(this, wxID_ANY, _("You may run this wizard again at any time to add to or upgrade the software in your stack. If you wish to remove any software, please use the Add/Remove Programs Control Panel applet."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

	mainSizer->SetItemMinSize(st, 400, 60);
//	mainSizer->Add(0, 20);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void CompletionPage::ShowErrorWarning(const int errors)
{
    if (errors)
    {
        stStatus->SetLabel(wxString::Format(_("%d installations were skipped - you may attempt to manually install those packages using the downloaded files."), errors));
        stStatus->Wrap(400);
    }
}
