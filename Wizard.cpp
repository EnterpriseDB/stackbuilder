/////////////////////////////////////////////////////////////////////////////
// Name:        Wizard.cpp
// Purpose:     The StackBuilder Wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Wizard.cpp,v 1.5 2007/03/29 15:08:53 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "Wizard.h"
#include "AppList.h"
#include "IntroductionPage.h"
#include "AppSelectionPage.h"
#include "MirrorList.h"
#include "MirrorSelectionPage.h"
#include "DownloadPage.h"
#include "InstallationPage.h"
#include "CompletionPage.h"

Wizard::Wizard(wxFrame *frame, wxBitmap bitmap, const wxString &applicationListUrl, const wxString &mirrorListUrl)
	: wxWizard(frame, wxID_ANY, _("PostgreSQL & EnterpriseDB Stack Builder"),
	bitmap, wxDefaultPosition,
      wxDEFAULT_DIALOG_STYLE)
{
	// Create the application & mirror lists
	m_applist = new AppList(applicationListUrl);
	m_mirrorlist = new MirrorList(mirrorListUrl);

    // Add the pages
    m_page1 = new IntroductionPage(this, m_applist);
	m_page2 = new AppSelectionPage(this, m_applist, m_mirrorlist);
	m_page3 = new MirrorSelectionPage(this, m_mirrorlist);
	m_page4 = new DownloadPage(this, m_applist, m_mirrorlist);
	m_page5 = new InstallationPage(this, m_applist);
	m_page6 = new CompletionPage(this);

	// Join 'em all up
	wxWizardPageSimple::Chain(m_page1, m_page2);
	wxWizardPageSimple::Chain(m_page2, m_page3);
	wxWizardPageSimple::Chain(m_page3, m_page4);
	wxWizardPageSimple::Chain(m_page4, m_page5);
	wxWizardPageSimple::Chain(m_page5, m_page6);

    // Allow the wizard to size itself around the pages
    GetPageAreaSizer()->Add(m_page1);
	SetBorder(0);
}
