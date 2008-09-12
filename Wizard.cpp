/////////////////////////////////////////////////////////////////////////////
// Name:        Wizard.cpp
// Purpose:     The StackBuilder Wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Wizard.cpp,v 1.13 2008/09/12 10:47:00 dpage Exp $
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

#include "images/StackBuilder.xpm"

Wizard::Wizard(wxFrame *frame, wxBitmap bitmap, const wxString &applicationListUrl, const wxString &mirrorListUrl, const wxString &title)
    : wxWizard(frame, wxID_ANY, title,
    bitmap, wxDefaultPosition,
      wxDEFAULT_DIALOG_STYLE)
{
    // Set the app icon
    SetIcon(wxIcon(StackBuilder_xpm));

    // Create the application & mirror lists
    m_applist = new AppList(applicationListUrl);
    m_mirrorlist = new MirrorList(mirrorListUrl);

    // Add the pages. Do this in reverse order, so we can 
    // pass pointers to later pages to earlier ones.
    m_page6 = new CompletionPage(this);
    m_page5 = new InstallationPage(this, m_applist);
    m_page4 = new DownloadPage(this, m_applist, m_mirrorlist);
    m_page3 = new MirrorSelectionPage(this, m_applist, m_mirrorlist);
    m_page2 = new AppSelectionPage(this, m_applist, m_mirrorlist, m_page3, m_page4);
    m_page1 = new IntroductionPage(this, m_applist);

    // Join 'em all up
    wxWizardPageSimple::Chain(m_page1, m_page2);
    wxWizardPageSimple::Chain(m_page2, m_page3);
    wxWizardPageSimple::Chain(m_page3, m_page4);
    wxWizardPageSimple::Chain(m_page4, m_page5);
    wxWizardPageSimple::Chain(m_page5, m_page6);

    // Allow the wizard to size itself around the pages
    /*
     * Since a problem is in correspondence of Japanese font size, it is made fixed 
     * size. However, in the future version of wxWidgets, an original code is desirable. 
     * GetPageAreaSizer()->Add(m_page1);
     */
    GetPageAreaSizer()->Add(420, 0);

    SetBorder(0);
}

