/////////////////////////////////////////////////////////////////////////////
// Name:        InstallationPage.h
// Purpose:     Installation page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: InstallationPage.cpp,v 1.12 2010/06/03 10:45:11 sachin Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "InstallationPage.h"
#include "CompletionPage.h"
#include "AppList.h"

BEGIN_EVENT_TABLE(InstallationPage, wxWizardPageSimple)
    EVT_CHECKBOX(wxID_ANY,          InstallationPage::OnSkipInstallationPressed)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY,        InstallationPage::OnWizardPageChanging)
END_EVENT_TABLE()

InstallationPage::InstallationPage(wxWizard *parent, AppList *applist)
    : wxWizardPageSimple(parent)
{
    skipInstallation = false;

    m_applist = applist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(0, 10);

    wxStaticText *st = new wxStaticText(this, wxID_ANY, _("All the installation files have now been successfully downloaded."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 40);

    st = new wxStaticText(this, wxID_ANY, _("Please click the \"Next\" button to start the installations."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 40);

    st = new wxStaticText(this, wxID_ANY, _("Note: You must allow all installations to run to completion. If you are prompted to restart the computer, click \"No\" or \"Restart Later\" and manually restart your computer when all the installation have finished."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 80);

    m_skipInstallation = new wxCheckBox(this, -1, _("Skip Installation"), wxDefaultPosition, wxDefaultSize);
    mainSizer->Add(m_skipInstallation, 0, wxALL | wxALIGN_LEFT, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void InstallationPage::OnSkipInstallationPressed(wxCommandEvent& WXUNUSED(event))
{
    if (m_skipInstallation->IsChecked())
    {
       //Skip the installation
        skipInstallation = true;
    }
    else
    {
       //Continue with the installation
       skipInstallation = false;
    }
}

void InstallationPage::OnWizardPageChanging(wxWizardEvent& event)
{
    // If we're going backwards, just bail out
    if (!event.GetDirection())
        return;

    ((CompletionPage *)GetNext())->SetPageText(skipInstallation);

    if (!skipInstallation)
    {
        if (!m_applist->InstallApps())
        {
            event.Veto();
            return;
        }
        ((CompletionPage *)GetNext())->ShowErrorWarning(m_applist->GetErrorCount());
    }

    ((CompletionPage *)GetNext())->DisableBackButton();
}

