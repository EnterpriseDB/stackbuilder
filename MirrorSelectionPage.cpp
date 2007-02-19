/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorSelectionPage.h
// Purpose:     Mirror selection page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorSelectionPage.cpp,v 1.1 2007/02/19 09:57:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

// Application headers
#include "MirrorSelectionPage.h"
#include "images/bullet.xpm"

MirrorSelectionPage::MirrorSelectionPage(wxWizard *parent) 
	: wxWizardPageSimple(parent)
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Please select a mirror site to download from."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

    m_mirrortree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	m_treeimages = new wxImageList(16, 16, true, 1);
	m_treeimages->Add(wxIcon(bullet_xpm));
	m_mirrortree->SetImageList(m_treeimages);
	mainSizer->Add(m_mirrortree, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool MirrorSelectionPage::TransferDataFromWindow()
{
    return true;
}
