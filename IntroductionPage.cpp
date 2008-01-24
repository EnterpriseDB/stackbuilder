/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.cpp,v 1.7 2008/01/24 14:38:25 h-saito Exp $
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
#include "AppList.h"
#include "Server.h"
#include "ProxyDialog.h"

const int BTN_PROXIES=1002;

BEGIN_EVENT_TABLE(IntroductionPage, wxWizardPageSimple)
    EVT_BUTTON(BTN_PROXIES,                 IntroductionPage::OnProxies)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY,		IntroductionPage::OnWizardPageChanging)
END_EVENT_TABLE()

IntroductionPage::IntroductionPage(wxWizard *parent, AppList *applist) 
	: wxWizardPageSimple(parent)
{
	m_applist = applist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Welcome to the PostgreSQL && EnterpriseDB Stack Builder!"));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL | wxALIGN_CENTER | wxFIXED_MINSIZE, 5);

//	mainSizer->Add(0, 20);

	st = new wxStaticText(this, wxID_ANY, _("This wizard will help you install additional software to complement your PostgreSQL or EnterpriseDB installation."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

	mainSizer->SetItemMinSize(st, 400, 40);
//	mainSizer->Add(0, 20);

    st = new wxStaticText(this, wxID_ANY, _("To begin, please select the installation you are installing software for from the list below. If you are installing software to use with a remote server, please select the <remote server> option."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL| wxFIXED_MINSIZE, 5);

	mainSizer->SetItemMinSize(st, 400, 60);
//	mainSizer->Add(0, 20);

    m_installation = new wxComboBox(this, wxID_ANY, _("<remote server>"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
    unsigned int item = m_installation->Append(_("<remote server>"));
	m_installation->SetClientObject(item, (wxClientData *)NULL);
	FindPgServers();
	FindEdbServers();

	mainSizer->Add(m_installation, 0, wxALL | wxALIGN_CENTER, 5);

    mainSizer->AddStretchSpacer();

    // Add the Proxy config button
    m_proxies = new wxButton(this, BTN_PROXIES, _("Proxy servers"));
	mainSizer->Add(m_proxies, 0, wxALL | wxALIGN_RIGHT, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void IntroductionPage::OnWizardPageChanging(wxWizardEvent& event)
{
	if (m_installation->GetValue().IsEmpty())
	{
		wxLogError(_("You must select an installation option before you continue."));
		event.Veto();
		return;
	}

	m_applist->SetTree(((AppSelectionPage *)GetNext())->GetTreeCtrl());
    m_applist->SetServer((Server *)m_installation->GetClientObject(m_installation->GetSelection()));

	// Get the app list, parse it and build the tree
	bool retval;
	{
		wxWindowDisabler disableAll;
		wxBusyInfo info(_("Downloading application list..."));
		wxTheApp->Yield();
		retval = m_applist->LoadAppList();
	}

	if (!retval)
	{
		event.Veto();
		return;
	}

	retval = m_applist->PopulateTreeCtrl();

	if (!retval)
	{
		return;
		event.Veto();
	}

    return;
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
			Server *data = new Server();
            data->serverType = SVR_POSTGRESQL;

			// Get the service data
            data->serviceId = svcName;
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Services\\%s"), svcName);
		    wxRegKey *svcKey = new wxRegKey(keyName);
		    svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &data->port);
            svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
            svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
            svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
            svcKey->QueryValue(wxT("Encoding"), data->encoding);
            svcKey->QueryValue(wxT("Locale"), data->locale);

			// Get the version number from installation record
			svcKey->QueryValue(wxT("Product Code"), guid);
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Installations\\%s"), guid);
		    wxRegKey *instKey = new wxRegKey(keyName);
			instKey->QueryValue(wxT("Version"), data->serverVersion);
			data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);
			data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
            instKey->QueryValue(wxT("Base Directory"), data->installationPath);

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
			Server *data = new Server();
            data->serverType = SVR_ENTERPRISEDB;

			// Get the service data
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Services\\%s"), svcName);
		    wxRegKey *svcKey = new wxRegKey(keyName);
		    svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &data->port);
            svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
            svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
            svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
            svcKey->QueryValue(wxT("Encoding"), data->encoding);
            svcKey->QueryValue(wxT("Locale"), data->locale);

			// Get the version number from installation record
			svcKey->QueryValue(wxT("Product Code"), guid);
			keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Installations\\%s"), guid);
		    wxRegKey *instKey = new wxRegKey(keyName);
			instKey->QueryValue(wxT("Version"), data->serverVersion);
			data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);
			data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
            instKey->QueryValue(wxT("Base Directory"), data->installationPath);

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

void IntroductionPage::OnProxies(wxCommandEvent& WXUNUSED(event))
{
    ProxyDialog *pd = new ProxyDialog(this, _("Proxy servers"));
    pd->ShowModal();
    delete pd;
}
