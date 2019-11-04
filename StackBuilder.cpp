/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.cpp
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.cpp,v 1.18 2011/11/28 19:09:57 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/dir.h>

// Application headers
#include "Wizard.h"
#include "images/background.xpm"

IMPLEMENT_APP(StackBuilder)

BEGIN_EVENT_TABLE(StackBuilder, wxApp)
    EVT_WIZARD_CANCEL(wxID_ANY,   StackBuilder::OnWizardCancelled)
    EVT_WIZARD_FINISHED(wxID_ANY, StackBuilder::OnWizardFinished)
END_EVENT_TABLE()

// This is declared an extern in StackBuilder.h
wxString downloadCounterUrl;
wxString g_certificateBundle;

// The Application!
bool StackBuilder::OnInit()
{
    wxString mirrorListUrl;
    wxString applicationListUrl;
    wxString language;

    SetAppName(_("Stack Builder"));

    // Command line options
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
#if wxCHECK_VERSION(3, 0, 0)
        {wxCMD_LINE_SWITCH, "h", "help", _("show this help message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        {wxCMD_LINE_OPTION, "m", "mirror-list", _("download the mirror list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, "a", "application-list", _("download the application list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, "d", "download-counter", _("use the download counter at the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, "l", "language", _("use the specified language in the UI"), wxCMD_LINE_VAL_STRING},
#else
        {wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), _("show this help message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        {wxCMD_LINE_OPTION, wxT("m"), wxT("mirror-list"), _("download the mirror list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("a"), wxT("application-list"), _("download the application list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("d"), wxT("download-counter"), _("use the download counter at the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("l"), wxT("language"), _("use the specified language in the UI"), wxCMD_LINE_VAL_STRING},
#endif

#ifndef __WIN32__
#if wxCHECK_VERSION(3, 0, 0)
        {wxCMD_LINE_OPTION, "c", "ca-bundle", _("user certificate for https support from the specified URL"), wxCMD_LINE_VAL_STRING},
#else
        {wxCMD_LINE_OPTION, wxT("c"), wxT("ca-bundle"), _("user certificate for https support from the specified URL"), wxCMD_LINE_VAL_STRING},
#endif
#endif
        {wxCMD_LINE_NONE}
    };

    wxCmdLineParser cmdParser(cmdLineDesc, argc, argv);
    if (cmdParser.Parse() != 0)
        return false;

    if (!cmdParser.Found(wxT("m"), &mirrorListUrl))
        mirrorListUrl = DEFAULT_MIRROR_LIST_URL;

    if (!cmdParser.Found(wxT("a"), &applicationListUrl))
        applicationListUrl = DEFAULT_APPLICATION_LIST_URL;

    // postgresql.org no longer uses the counter, so we default to not trying
    // to use any, unless the user overrides on the command line.
    if (!cmdParser.Found(wxT("d"), &downloadCounterUrl))
        downloadCounterUrl = wxEmptyString;

    if (!cmdParser.Found(wxT("l"), &language))
        language = wxEmptyString;

#ifndef __WIN32__
    if (!cmdParser.Found(wxT("c"), &g_certificateBundle))
        g_certificateBundle = wxEmptyString;
#endif

    // Hack for the PostgreSQL installer - it might ask for the default language
    if (language == wxT("DEFAULT"))
        language = wxEmptyString;

    // Initialize our locale and load the language catalog...
    wxString executablePath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
    initializeLocale(executablePath, language);

    // We need to run as root on Unix in order to ensure that
    // the instalers will run with appropriate privileges. On
    // Mac and Windows, installers can elevate themselves.
#ifdef __WXGTK__
    if(geteuid() != 0)
    {
	wxLogError(_("Stack Builder requires superuser privileges. Please execute Stack Builder using sudo, or from the root user account."));
        return false;
    }
#endif
#ifdef __WXMSW__
    BOOL isAdmin;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    isAdmin = AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup);
    if(isAdmin)
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &isAdmin))
        {
            isAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    if(!isAdmin)
    {
	wxLogError(_("Stack Builder requires administrator privileges. Please execute Stack Builder by right-clicking the shortcut and using the 'Run as Administrator' option, or from an Administrator account on Windows XP or 2003."));
	return false;
    }
#endif

    // Create and run the wizard
    wxBitmap bitmap = wxBitmap(background_xpm);
	wxString title = wxString::Format(wxT("%s %s"), _("Stack Builder"), STACKBUILDER_VERSION);
    wizard = new Wizard(NULL, bitmap, applicationListUrl, mirrorListUrl, title);
    bool retval = wizard->RunWizard(wizard->GetFirstPage());

    return retval;
}

void StackBuilder::OnWizardCancelled(wxWizardEvent &evt)
{
    if (wxMessageBox(_("Are you sure you want to close the Stack Builder wizard?"), _("Exit wizard?"), wxICON_QUESTION | wxYES_NO) == wxNO)
    {
        evt.Veto();
        return;
    }
    wizard->Destroy();
    this->Exit();
}

void StackBuilder::OnWizardFinished(wxWizardEvent &evt)
{
    wizard->SetReturnCode(0);
    wizard->Destroy();
    this->Exit();
}

void StackBuilder::initializeLocale(const wxString &appPath, const wxString &lang)
{
    wxString i18nPath;

    if(wxDir::Exists(appPath + wxT("/i18n")))
        i18nPath = appPath + wxT("/i18n");
    else if(wxDir::Exists(appPath + wxT("/../StackBuilder/i18n")))
        i18nPath = appPath + wxT("/../StackBuilder/i18n");
    else if(wxDir::Exists(appPath + wxT("/../StackBuilder/share/i18n")))
        i18nPath = appPath + wxT("/../StackBuilder/share/i18n");
    else if(wxDir::Exists(appPath + wxT("/../i18n")))
        i18nPath = appPath + wxT("/../i18n");
    else if(wxDir::Exists(appPath + wxT("/../share/i18n")))
        i18nPath = appPath + wxT("/../share/i18n");
    else if(wxDir::Exists(appPath + wxT("/../Resources/i18n")))
        i18nPath = appPath + wxT("/../Resources/i18n");

    wxLocale *locale = new wxLocale();
    locale->AddCatalogLookupPathPrefix(i18nPath);

    const wxLanguageInfo *langInfo;

    if (!lang.IsEmpty())
        langInfo = wxLocale::FindLanguageInfo(lang);
    else
        langInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);

    if (!langInfo)
        return;

    if(locale->Init(langInfo->Language), wxLOCALE_LOAD_DEFAULT)
        locale->AddCatalog(wxT("StackBuilder"));
}
