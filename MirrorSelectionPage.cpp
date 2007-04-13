/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorSelectionPage.h
// Purpose:     Mirror selection page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorSelectionPage.cpp,v 1.4 2007/04/13 11:20:47 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "MirrorSelectionPage.h"
#include "AppList.h"
#include "MirrorList.h"
#include "DownloadPage.h"

#include "images/bullet.xpm"
#include "images/mirror.xpm"

BEGIN_EVENT_TABLE(MirrorSelectionPage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY,		MirrorSelectionPage::OnWizardPageChanging)
END_EVENT_TABLE()

MirrorSelectionPage::MirrorSelectionPage(wxWizard *parent, AppList *applist, MirrorList *mirrorlist) 
	: wxWizardPageSimple(parent)
{
    m_applist = applist;
	m_mirrorlist = mirrorlist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Please select a mirror site to download from."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

    m_mirrortree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	m_treeimages = new wxImageList(16, 16, true, 1);
	m_treeimages->Add(wxIcon(bullet_xpm));
    m_treeimages->Add(wxIcon(mirror_xpm));
	m_mirrortree->SetImageList(m_treeimages);
	mainSizer->Add(m_mirrortree, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void MirrorSelectionPage::OnWizardPageChanging(wxWizardEvent& event)
{
	// If we're going backwards, clear the tree view
	if (!event.GetDirection())
	{
		m_mirrorlist->DeleteAllItems();
		m_mirrortree->DeleteAllItems();
		return;
	}

    wxTreeItemId id = m_mirrortree->GetSelection();
	if (!id || m_mirrortree->GetItemImage(id) != 1)
	{
		wxLogError(_("You must select a mirror before you continue."));
		event.Veto();
		return;
	}

    m_mirrorlist->SetSelectedMirror((Mirror *)m_mirrortree->GetItemData(m_mirrortree->GetSelection()));

    // Stuff the summary in the next page
    ((DownloadPage *)GetNext())->SetSummary(m_applist->GetSummary());
}