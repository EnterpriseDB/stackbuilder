/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.cpp,v 1.10 2008/06/25 12:15:38 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/utils.h>
#include <wx/wizard.h>

#ifdef __WXMSW__
#include <wx/msw/registry.h>
#endif

// Application headers
#include "IntroductionPage.h"
#include "AppSelectionPage.h"
#include "AppList.h"
#include "Server.h"
#include "ProxyDialog.h"

const int BTN_PROXIES=1002;

BEGIN_EVENT_TABLE(IntroductionPage, wxWizardPageSimple)
    EVT_BUTTON(BTN_PROXIES,            IntroductionPage::OnProxies)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, IntroductionPage::OnWizardPageChanging)
END_EVENT_TABLE()

IntroductionPage::IntroductionPage(wxWizard *parent, AppList *applist) 
    : wxWizardPageSimple(parent)
{
    m_applist = applist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(0, 10);

    wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Welcome to the PostgreSQL && EnterpriseDB Stack Builder!"));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxALIGN_CENTER | wxFIXED_MINSIZE, 5);

    st = new wxStaticText(this, wxID_ANY, _("This wizard will help you install additional software to complement your PostgreSQL or EnterpriseDB installation."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 40);

    st = new wxStaticText(this, wxID_ANY, _("To begin, please select the installation you are installing software for from the list below. If you are installing software to use with a remote server, please select the <remote server> option."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL| wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 60);

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
    if (m_installation->GetSelection() == wxNOT_FOUND)
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
#ifdef __WXMSW__
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
            keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Services\\%s"), svcName.c_str());
            wxRegKey *svcKey = new wxRegKey(keyName);
			if (svcKey->HasValue(wxT("Display Name")))
                svcKey->QueryValue(wxT("Display Name"), data->description);
			else
				data->description = _("Unknown server");
			if (svcKey->HasValue(wxT("Port")))
                svcKey->QueryValue(wxT("Port"), &data->port);
			else
				data->port = 0;
			if (svcKey->HasValue(wxT("Data Directory")))
                svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
			if (svcKey->HasValue(wxT("Database Superuser")))
                svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
			if (svcKey->HasValue(wxT("Service Account")))
                svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
			if (svcKey->HasValue(wxT("Encoding")))
                svcKey->QueryValue(wxT("Encoding"), data->encoding);
			if (svcKey->HasValue(wxT("Locale")))
                svcKey->QueryValue(wxT("Locale"), data->locale);

            // Get the version number from installation record
			if (svcKey->HasValue(wxT("Product Code")))
                svcKey->QueryValue(wxT("Product Code"), guid);

			if (!guid.IsEmpty())
			{
                keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Installations\\%s"), guid.c_str());
                wxRegKey *instKey = new wxRegKey(keyName);
				if (instKey->HasValue(wxT("Version")))
				{
                    instKey->QueryValue(wxT("Version"), data->serverVersion);
                    data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);
                    data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
				}
				else
				{
					data->majorVer = 0;
					data->minorVer = 0;
				}
				if (instKey->HasValue(wxT("Base Directory")))
                    instKey->QueryValue(wxT("Base Directory"), data->installationPath);
			}

            // Build the user description
            temp.Printf(_("%s on port %d"), data->description.c_str(), data->port);
 
            // Add the item
            m_installation->Append(temp, data);
            success = true;

            // Get the next one...
            flag = rootKey->GetNextKey(svcName, cookie);
        }
    }

    return success;
#else
    // TODO: Fix for *nix
    return true;
#endif
}

// Note: The following function is currently the same as FindPgServers() just
//       using different registry keys until the EDB installer starts writing 
//       it's install data to the registry.
bool IntroductionPage::FindEdbServers()
{
#ifdef __WXMSW__
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
            data->serverType = SVR_POSTGRESQL;

            // Get the service data
            data->serviceId = svcName;
            keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Services\\%s"), svcName.c_str());
            wxRegKey *svcKey = new wxRegKey(keyName);
			if (svcKey->HasValue(wxT("Display Name")))
                svcKey->QueryValue(wxT("Display Name"), data->description);
			else
				data->description = _("Unknown server");
			if (svcKey->HasValue(wxT("Port")))
                svcKey->QueryValue(wxT("Port"), &data->port);
			else
				data->port = 0;
			if (svcKey->HasValue(wxT("Data Directory")))
                svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
			if (svcKey->HasValue(wxT("Database Superuser")))
                svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
			if (svcKey->HasValue(wxT("Service Account")))
                svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
			if (svcKey->HasValue(wxT("Encoding")))
                svcKey->QueryValue(wxT("Encoding"), data->encoding);
			if (svcKey->HasValue(wxT("Locale")))
                svcKey->QueryValue(wxT("Locale"), data->locale);

            // Get the version number from installation record
			if (svcKey->HasValue(wxT("Product Code")))
                svcKey->QueryValue(wxT("Product Code"), guid);

			if (!guid.IsEmpty())
			{
                keyName.Printf(wxT("HKEY_LOCAL_MACHINE\\Software\\EnterpriseDB\\Installations\\%s"), guid.c_str());
                wxRegKey *instKey = new wxRegKey(keyName);
				if (instKey->HasValue(wxT("Version")))
				{
                    instKey->QueryValue(wxT("Version"), data->serverVersion);
                    data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);
                    data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
				}
				else
				{
					data->majorVer = 0;
					data->minorVer = 0;
				}
				if (instKey->HasValue(wxT("Base Directory")))
                    instKey->QueryValue(wxT("Base Directory"), data->installationPath);
			}

            // Build the user description
            temp.Printf(_("%s on port %d"), data->description.c_str(), data->port);
 
            // Add the item
            m_installation->Append(temp, data);
            success = true;

            // Get the next one...
            flag = rootKey->GetNextKey(svcName, cookie);
        }
    }

    return success;
#else
    // TODO: Fix for *nix 
    return true;
#endif
}

void IntroductionPage::OnProxies(wxCommandEvent& WXUNUSED(event))
{
    ProxyDialog *pd = new ProxyDialog(this, _("Proxy servers"));
    pd->ShowModal();
    delete pd;
}
