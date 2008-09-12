/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.cpp
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.cpp,v 1.12 2008/09/12 10:47:00 dpage Exp $
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
        {wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), _("show this help message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        {wxCMD_LINE_OPTION, wxT("m"), wxT("mirror-list"), _("download the mirror list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("a"), wxT("application-list"), _("download the application list from the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("d"), wxT("download-counter"), _("use the download counter at the specified URL"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_OPTION, wxT("l"), wxT("language"), _("use the specified language in the UI"), wxCMD_LINE_VAL_STRING},
        {wxCMD_LINE_NONE}
    };

    wxCmdLineParser cmdParser(cmdLineDesc, argc, argv);
    if (cmdParser.Parse() != 0) 
        return false;

    if (!cmdParser.Found(wxT("m"), &mirrorListUrl))
        mirrorListUrl = DEFAULT_MIRROR_LIST_URL;

    if (!cmdParser.Found(wxT("a"), &applicationListUrl))
        applicationListUrl = DEFAULT_APPLICATION_LIST_URL;
    
    // If we're using an alternate application list, we don't want to log anything
    // unless an alternate download counter is specified as well
    if (!cmdParser.Found(wxT("d"), &downloadCounterUrl))
    {
        if (applicationListUrl == DEFAULT_APPLICATION_LIST_URL)
            downloadCounterUrl = DEFAULT_DOWNLOAD_COUNTER_URL;
        else
            downloadCounterUrl = wxEmptyString;
    }

    if (!cmdParser.Found(wxT("l"), &language))
        language = wxEmptyString;

    // Hack for the PostgreSQL installer - it might ask for the default language
    if (language == wxT("DEFAULT"))
        language = wxEmptyString;

    // Initialize our locale and load the language catalog...
    initializeLocale(argv[0], language);

    // We need to run as root on Unix in order to ensure that
    // the instalers will run with appropriate privileges. On
    // Mac and Windows, installers can elevate themselves.
#ifdef __WXGTK__
    if(geteuid() != 0)
    {
        wxLogError(_("This application must be run as the superuser."));
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

void StackBuilder::initializeLocale(wxChar *argv0, const wxString &lang)
{
    wxString appPath = wxPathOnly(argv0);
    wxString i18nPath;

    // Figure out where the pgadmin3 language catalog is located
    if( appPath.IsEmpty())
        appPath = wxT(".");

    if(wxDir::Exists(appPath + wxT("/i18n")))
        i18nPath = appPath + wxT("/i18n");
    else if(wxDir::Exists(appPath + wxT("/../StackBuilder/i18n")))
        i18nPath = appPath + wxT("/../StackBuilder/i18n");
    else if(wxDir::Exists(appPath + wxT("/../i18n")))
        i18nPath = appPath + wxT("/../i18n");
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

