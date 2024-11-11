/////////////////////////////////////////////////////////////////////////////
// Name:        IntroductionPage.h
// Purpose:     Introduction page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: IntroductionPage.cpp,v 1.28 2011/11/28 18:53:49 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/utils.h>
#include <wx/wizard.h>
#include <wx/wfstream.h>

#ifdef __WXMSW__
#include <wx/msw/registry.h>
#endif

// Application headers
#include "IntroductionPage.h"
#include "AppSelectionPage.h"
#include "AppList.h"
#include "Server.h"
#include "ProxyDialog.h"
#include "Registry.h"

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

    wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Welcome to Stack Builder!"));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxALIGN_CENTER | wxFIXED_MINSIZE, 5);

    st = new wxStaticText(this, wxID_ANY, _("This wizard will help you install additional software to complement your PostgreSQL or EnterpriseDB Postgres Plus installation."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL | wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 60);

    st = new wxStaticText(this, wxID_ANY, _("To begin, please select the installation you are installing software for from the list below. Your computer must be connected to the Internet before proceeding."));
    st->Wrap(400);
    mainSizer->Add(st, 0, wxALL| wxFIXED_MINSIZE, 5);

    mainSizer->SetItemMinSize(st, 400, 65);

    m_installation = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);

    // Add a dummy server to the remote server selection item.
    Server *dummyServer = new Server();
    dummyServer->port = 5432;
    dummyServer->superuserName = wxT("postgres");
    dummyServer->serverType = SVR_POSTGRESQL;

    unsigned int item = m_installation->Append(_("<remote server>"));
    m_installation->SetClientObject(item, dummyServer);
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

    if (!m_applist->Count())
    {
        wxLogError(_("There are currently no applications available for your platform."));
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
    bool flag = false;
    long cookie = 0;
    long port = 0;
    wxString temp;

#ifdef __WXMSW__
    // Add local servers.
    pgRegKey::PGREGWOWMODE wowMode = pgRegKey::PGREG_WOW_DEFAULT;
    wxString strPgArch;

    if (::wxIsPlatform64Bit())
        wowMode = pgRegKey::PGREG_WOW32;

    pgRegKey *rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\PostgreSQL\\Services"), pgRegKey::PGREG_READ, wowMode);

    if (rootKey == NULL && ::wxIsPlatform64Bit())
    {
        wowMode = pgRegKey::PGREG_WOW64;
        rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\PostgreSQL\\Services"), pgRegKey::PGREG_READ, wowMode);
        strPgArch = wxT(" (x64)");
    }

    while (rootKey != NULL)
    {
        long cookie = 0;
        wxString svcName;
        pgRegKey *svcKey = NULL;

        flag = rootKey->GetFirstKey(svcKey, cookie);

        while (flag != false)
        {
            wxString guid;
            DWORD tmpPort = 0;

            Server *data = new Server();

            if (wowMode == pgRegKey::PGREG_WOW64)
                data->platform = wxT("windows-x64");
            else
                data->platform = wxT("windows");
            data->serverType = SVR_POSTGRESQL;

            svcName = svcKey->GetKeyName();

            // Get the service data
            data->serviceId = svcName;

            data->description = _("Unknown server");
            svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &tmpPort);
            data->port = (long)tmpPort;
            svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
            svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
            svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
            svcKey->QueryValue(wxT("Encoding"), data->encoding);
            svcKey->QueryValue(wxT("Locale"), data->locale);

            // Get the version number from installation record
            svcKey->QueryValue(wxT("Product Code"), guid);

            if (!guid.IsEmpty())
            {
                wxString keyName;
                keyName.Printf(wxT("Software\\PostgreSQL\\Installations\\%s"), guid.c_str());

                pgRegKey *instKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, keyName, pgRegKey::PGREG_READ, wowMode);

                data->majorVer = 0;
                data->minorVer = 0;

                if (instKey != NULL)
                {
                    if (instKey->HasValue(wxT("Version")))
                    {
                        instKey->QueryValue(wxT("Version"), data->serverVersion);
                        data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);

                        if (data->majorVer < 10)
                            data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
                    }

                    if (instKey->HasValue(wxT("Base Directory")))
                        instKey->QueryValue(wxT("Base Directory"), data->installationPath);

                    delete instKey;
                }
            }

            // Build the user description
            temp.Printf(_("%s%s on port %d"), data->description.c_str(), strPgArch.c_str(), data->port);

			// Add the item, if it looks sane
			if (data->port != 0 && data->dataDirectory != wxEmptyString && data->superuserName != wxEmptyString)
			{
				m_installation->Append(temp, data);
				success = true;
			}

            delete svcKey;
            svcKey = NULL;
            // Get the next one...
            flag = rootKey->GetNextKey(svcKey, cookie);
        }

        delete rootKey;
        rootKey = NULL;

        if (strPgArch == wxEmptyString && ::wxIsPlatform64Bit())
        {
            wowMode = pgRegKey::PGREG_WOW64;
            rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\PostgreSQL\\Services"), pgRegKey::PGREG_READ, wowMode);
            strPgArch = wxT(" (x64)");
        }
    }

    return success;
#else
    if (wxFile::Exists(REGISTRY_FILE))
    {
        wxString version, locale;
        long cookie;

        wxFileInputStream fst(REGISTRY_FILE);
        wxFileConfig *cnf = new wxFileConfig(fst);

        cnf->SetPath(wxT("/PostgreSQL"));
        flag = cnf->GetFirstGroup(version, cookie);
        while (flag)
        {
            // If there is no Version entry, this is probably an uninstalled server
            if (cnf->Read(version + wxT("/Version"), wxEmptyString) != wxEmptyString)
            {
                Server *data = new Server();
                data->serverType = SVR_POSTGRESQL;
                data->platform = STACKBUILDER_PLATFORM;

                // Server version
                data->serverVersion = version;
                data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);

                if (data->majorVer < 10)
                    data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
                else
                    data->minorVer = 0;

                // And the rest of the data
                data->description = cnf->Read(version + wxT("/Description"), _("Unknown server"));
                data->port = cnf->Read(version + wxT("/Port"), 0L);
                data->dataDirectory = cnf->Read(version + wxT("/DataDirectory"), wxEmptyString);
                data->installationPath = cnf->Read(version + wxT("/InstallationDirectory"), wxEmptyString);
                data->superuserName = cnf->Read(version + wxT("/Superuser"), wxEmptyString);
                data->serviceAccount = cnf->Read(version + wxT("/Superuser"), wxEmptyString); // We use the Superuser entry for both on *nix

                // Separate the locale and encoding if possible
                locale = cnf->Read(version + wxT("/Locale"), wxEmptyString);

                if (locale.Find('.') == wxNOT_FOUND)
                {
                    data->locale = locale;
                }
                else
                {
                    data->locale = locale.BeforeFirst('.');
                    data->encoding = locale.AfterFirst('.');
                }

                // Build the user description
                temp.Printf(_("%s on port %ld"), data->description.c_str(), data->port);

                // Add the item, if it looks sane
				if (data->port != 0 && data->dataDirectory != wxEmptyString && data->superuserName != wxEmptyString)
				{
                    m_installation->Append(temp, data);
                    success = true;
				}
            }

            flag = cnf->GetNextGroup(version, cookie);
        }

        delete cnf;
    }

    return success;
#endif
}


// Note: The following function is currently the same as FindPgServers() just
//       using different registry keys until the EDB installer starts writing
//       it's install data to the registry.
bool IntroductionPage::FindEdbServers()
{
    bool success = false;
    bool flag = false;
    long cookie = 0;
    long port = 0;
    wxString temp;

#ifdef __WXMSW__
    // Add local servers.
    pgRegKey::PGREGWOWMODE wowMode = pgRegKey::PGREG_WOW_DEFAULT;
    wxString strPgArch;

    if (::wxIsPlatform64Bit())
    {
        strPgArch = wxT(" (x86)");
        wowMode = pgRegKey::PGREG_WOW32;
    }
    pgRegKey *rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\EnterpriseDB\\Services"), pgRegKey::PGREG_READ, wowMode);

    if (rootKey == NULL && wowMode == pgRegKey::PGREG_WOW32)
    {
        wowMode = pgRegKey::PGREG_WOW64;
        rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\EnterpriseDB\\Services"), pgRegKey::PGREG_READ, wowMode);
        strPgArch = wxT(" (x64)");
    }

    while (rootKey != NULL)
    {
        long cookie = 0;
        wxString svcName;
        pgRegKey *svcKey = NULL;

        flag = rootKey->GetFirstKey(svcKey, cookie);

        while (flag != false)
        {
            wxString guid;
            DWORD tmpPort = 0;

            Server *data = new Server();

            if (wowMode == pgRegKey::PGREG_WOW64)
                data->platform = wxT("windows-x64");
            else
                data->platform = wxT("windows");
            data->serverType = SVR_ENTERPRISEDB;

            svcName = svcKey->GetKeyName();

            // Get the service data
            data->serviceId = svcName;

            data->description = _("Unknown server");
            svcKey->QueryValue(wxT("Display Name"), data->description);
            svcKey->QueryValue(wxT("Port"), &tmpPort);
            data->port = (long)tmpPort;
            svcKey->QueryValue(wxT("Data Directory"), data->dataDirectory);
            svcKey->QueryValue(wxT("Database Superuser"), data->superuserName);
            svcKey->QueryValue(wxT("Service Account"), data->serviceAccount);
            svcKey->QueryValue(wxT("Encoding"), data->encoding);
            svcKey->QueryValue(wxT("Locale"), data->locale);

            // Get the version number from installation record
            svcKey->QueryValue(wxT("Product Code"), guid);

            if (!guid.IsEmpty())
            {
                wxString keyName;
                keyName.Printf(wxT("Software\\EnterpriseDB\\Installations\\%s"), guid.c_str());

                pgRegKey *instKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, keyName, pgRegKey::PGREG_READ, wowMode);

                data->majorVer = 0;
                data->minorVer = 0;

                if (instKey != NULL)
                {
                    if (instKey->HasValue(wxT("Version")))
                    {
                        instKey->QueryValue(wxT("Version"), data->serverVersion);
                        data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);

                        if (data->majorVer < 10)
                            data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);
                    }

                    if (instKey->HasValue(wxT("Base Directory")))
                        instKey->QueryValue(wxT("Base Directory"), data->installationPath);

                    delete instKey;
                }
            }

            // Build the user description
            temp.Printf(_("%s%s on port %d"), data->description.c_str(), strPgArch.c_str(), data->port);

			// Add the item, if it looks sane
			if (data->port != 0 && data->dataDirectory != wxEmptyString && data->superuserName != wxEmptyString)
			{
				m_installation->Append(temp, data);
				success = true;
			}

            delete svcKey;
            svcKey = NULL;
            // Get the next one...
            flag = rootKey->GetNextKey(svcKey, cookie);
        }

        delete rootKey;
        rootKey = NULL;

        if (wowMode == pgRegKey::PGREG_WOW32)
        {
            wowMode = pgRegKey::PGREG_WOW64;
            rootKey = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, wxT("Software\\EnterpriseDB\\Services"), pgRegKey::PGREG_READ, wowMode);
            strPgArch = wxT(" (x64)");
        }
    }

    return success;
#else
    if (wxFile::Exists(REGISTRY_FILE))
    {
        wxString version, locale;
        long cookie;

        wxFileInputStream fst(REGISTRY_FILE);
        wxFileConfig *cnf = new wxFileConfig(fst);

        cnf->SetPath(wxT("/EnterpriseDB"));
        flag = cnf->GetFirstGroup(version, cookie);
        while (flag)
        {
            // If there is no Version entry, this is probably an uninstalled server
            if (cnf->Read(version + wxT("/Version"), wxEmptyString) != wxEmptyString)
            {
                Server *data = new Server();
                data->serverType = SVR_ENTERPRISEDB;
                data->platform = STACKBUILDER_PLATFORM;

                // Server version
                data->majorVer = 0;
                data->minorVer = 0;
                data->serverVersion = version;
                data->serverVersion.BeforeFirst('.').ToLong(&data->majorVer);
                if ( data->majorVer < 10 )
                    data->serverVersion.AfterFirst('.').ToLong(&data->minorVer);

                // And the rest of the data
                data->description = cnf->Read(version + wxT("/Description"), _("Unknown server"));
                data->port = cnf->Read(version + wxT("/Port"), 0L);
                data->dataDirectory = cnf->Read(version + wxT("/DataDirectory"), wxEmptyString);
                data->installationPath = cnf->Read(version + wxT("/InstallationDirectory"), wxEmptyString);
                data->superuserName = cnf->Read(version + wxT("/Superuser"), wxEmptyString);
                data->serviceAccount = cnf->Read(version + wxT("/Superuser"), wxEmptyString); // We use the Superuser entry for both on *nix

                // Separate the locale and encoding if possible
                locale = cnf->Read(version + wxT("/Locale"), wxEmptyString);

                if (locale.Find('.') == wxNOT_FOUND)
                {
                    data->locale = locale;
                }
                else
                {
                    data->locale = locale.BeforeFirst('.');
                    data->encoding = locale.AfterFirst('.');
                }

                // Build the user description
                temp.Printf(_("%s on port %d"), data->description.c_str(), data->port);

                // Add the item, if it looks sane
				if (data->port != 0 && data->dataDirectory != wxEmptyString && data->superuserName != wxEmptyString)
				{
                    m_installation->Append(temp, data);
                    success = true;
				}
            }

            flag = cnf->GetNextGroup(version, cookie);
        }

        delete cnf;
    }

    return success;
#endif
}

void IntroductionPage::OnProxies(wxCommandEvent& WXUNUSED(event))
{
    ProxyDialog *pd = new ProxyDialog(this, _("Proxy servers"));
    pd->CenterOnParent();
    pd->ShowModal();
    delete pd;
}

