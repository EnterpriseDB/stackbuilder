/////////////////////////////////////////////////////////////////////////////
// Name:        StackBuilder.cpp
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: StackBuilder.cpp,v 1.2 2007/02/19 14:22:23 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/cmdline.h>

// Application headers
#include "Wizard.h"
#include "images/background.xpm"

IMPLEMENT_APP(StackBuilder)

BEGIN_EVENT_TABLE(StackBuilder, wxApp)
    EVT_WIZARD_CANCEL(wxID_ANY,   StackBuilder::OnWizardCancelled)
    EVT_WIZARD_FINISHED(wxID_ANY, StackBuilder::OnWizardFinished)
END_EVENT_TABLE()

// The Application!
bool StackBuilder::OnInit()
{
	wxString mirrorListUrl;
	wxString applicationListUrl;

	SetAppName(_("Stack Builder"));

	// Command line options
	static const wxCmdLineEntryDesc cmdLineDesc[] = 
	{
		{wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), _("show this help message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{wxCMD_LINE_OPTION, wxT("m"), wxT("mirror-list"), _("download the mirror list from the specified URL"), wxCMD_LINE_VAL_STRING},
		{wxCMD_LINE_OPTION, wxT("a"), wxT("application-list"), _("download the application list from the specified URL"), wxCMD_LINE_VAL_STRING},
		{wxCMD_LINE_NONE}
	};

	wxCmdLineParser cmdParser(cmdLineDesc, argc, argv);
	if (cmdParser.Parse() != 0) 
		return false;

	if (!cmdParser.Found(wxT("m"), &mirrorListUrl))
		mirrorListUrl = DEFAULT_MIRROR_LIST_URL;

	if (!cmdParser.Found(wxT("a"), &applicationListUrl))
		applicationListUrl = DEFAULT_APPLICATION_LIST_URL;

	// Create and run the wizard
	wxBitmap bitmap = wxBitmap(background_xpm);
    Wizard wizard(NULL, bitmap, applicationListUrl, mirrorListUrl);
	bool retval = wizard.RunWizard(wizard.GetFirstPage()); 

	return retval;
}

void StackBuilder::OnWizardCancelled(wxWizardEvent &evt)
{
	if (wxMessageBox(_("Are you sure you want to close the Stack Builder wizard?"), _("Exit wizard?"), wxICON_QUESTION | wxYES_NO) == wxNO)
	{
		evt.Veto();
		return;
	}
	this->Exit();
}

void StackBuilder::OnWizardFinished(wxWizardEvent &evt)
{
	wxMessageBox(_("The wizard finished!!"));
	this->Exit();
}