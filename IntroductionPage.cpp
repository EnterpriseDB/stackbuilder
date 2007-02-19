/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.cpp,v 1.1 2007/02/19 09:57:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/utils.h>
#include <wx/wizard.h>
#include <wx/msw/registry.h>

// Application headers
#include "IntroductionPage.h"
#include "AppSelectionPage.h"

IntroductionPage::IntroductionPage(wxWizard *parent, AppList *applist) 
	: wxWizardPageSimple(parent)
{
	m_applist = applist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Welcome to the PostgreSQL && EnterpriseDB Stack Builder!"));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL | wxALIGN_CENTER, 5);

	mainSizer->Add(0, 20);

	st = new wxStaticText(this, wxID_ANY, _("This wizard will help you install additional software to complement your PostgreSQL or EnterpriseDB installation."));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 20);

    st = new wxStaticText(this, wxID_ANY, _("To begin, please select the PostgreSQL or EnterpriseDB installation you are installing software for from the list below. If you are installing software to use with a remote server, please select the <remote server> option."));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 20);

    m_installation = new wxComboBox(this, wxID_ANY, _("<remote server>"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
    unsigned int item = m_installation->Append(_("<remote server>"));
	m_installation->SetClientObject(item, (wxClientData *)NULL);
	FindPgServers();
	FindEdbServers();

	mainSizer->Add(m_installation, 0, wxALL | wxALIGN_CENTER, 5);
    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool IntroductionPage::TransferDataFromWindow()
{
	if (m_installation->GetValue().IsEmpty())
	{
		wxLogError(_("You must select an installation option before you continue."));
		return false;
	}

	m_applist->SetTree(((AppSelectionPage *)GetNext())->GetTreeCtrl());

	// Get the app list, parse it and build the tree
	bool retval;
	{
		wxWindowDisabler disableAll;
		wxBusyInfo info(_("Downloading application list..."));
		wxTheApp->Yield();
		retval = m_applist->LoadAppList((ServerData *)m_installation->GetClientObject(m_installation->GetSelection()));
	}

	if (!retval)
		return false;

	retval = m_applist->PopulateTreeCtrl();

	if (!retval)
		return false;

    return true;
}

bool IntroductionPage::FindPgServers()
{
	bool success = false;

	// Add local servers.
	wxRegKey *rootKey = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Services"));

	if (rootKey->Exists())
	{
		wxString svcName, temp;
		long cookie = 0;
		long port = 0;
		bool flag = false;

		flag = rootKey->GetFirstKey(svcName, cookie);

		while (flag != false)
		{
            wxString keyName, guid;
			ServerData *data = new ServerData();

			// Get the service data
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Services\\%s"), svcName);
		    wxRegKey *svcKey = new wxRegKey(keyName);
		    svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &data->port);

			// Get the version number from installation record
			svcKey->QueryValue(wxT("Product Code"), guid);
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Installations\\%s"), guid);
		    wxRegKey *instKey = new wxRegKey(keyName);
			instKey->QueryValue(wxT("Version"), temp);
			temp.BeforeFirst('.').ToLong(&data->majorVer);
			temp.AfterFirst('.').ToLong(&data->minorVer);

			// Build the user description
			temp.Printf(_("%s on port %d"), data->description, data->port);
 
			// Add the item
		    m_installation->Append(temp, data);
			success = true;

			// Get the next one...
			flag = rootKey->GetNextKey(svcName, cookie);
		}
	}

	return success;
}

// Note: The following function is currently the same as FindPgServers() just
//       using different registry keys until the EDB installer starts writing 
//       it's install data to the registry.
bool IntroductionPage::FindEdbServers()
{
	bool success = false;

	// Add local servers.
	wxRegKey *rootKey = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Services"));

	if (rootKey->Exists())
	{
		wxString svcName, temp;
		long cookie = 0;
		long port = 0;
		bool flag = false;

		flag = rootKey->GetFirstKey(svcName, cookie);

		while (flag != false)
		{
            wxString keyName, guid;
			ServerData *data = new ServerData();

			// Get the service data
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Services\\%s"), svcName);
		    wxRegKey *svcKey = new wxRegKey(keyName);
		    svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &data->port);

			// Get the version number from installation record
			svcKey->QueryValue(wxT("Product Code"), guid);
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Installations\\%s"), guid);
		    wxRegKey *instKey = new wxRegKey(keyName);
			instKey->QueryValue(wxT("Version"), temp);
			temp.BeforeFirst('.').ToLong(&data->majorVer);
			temp.AfterFirst('.').ToLong(&data->minorVer);

			// Build the user description
			temp.Printf(_("%s on port %d"), data->description, data->port);
 
			// Add the item
		    m_installation->Append(temp, data);
			success = true;

			// Get the next one...
			flag = rootKey->GetNextKey(svcName, cookie);
		}
	}

	return success;
}