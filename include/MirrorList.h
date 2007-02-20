/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorList.h
// Purpose:     Maintains the list of mirrors
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorList.h,v 1.2 2007/02/20 10:52:04 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _MIRRORLIST_H
#define _MIRRORLIST_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/treectrl.h>
#include <wx/xml/xml.h>

// Application headers
#include "Mirror.h"

WX_DECLARE_OBJARRAY(Mirror, MirrorArray);

class MirrorList
{
public:
	MirrorList(const wxString &mirrorListUrl) { m_mirrorListUrl = mirrorListUrl; };
	bool LoadMirrorList();
	bool PopulateTreeCtrl();
	void SetTree(wxTreeCtrl *tree) { m_treectrl = tree; };
	void DeleteAllItems();

private:
	MirrorArray m_mirrors;
	wxString m_mirrorListUrl;
	wxTreeCtrl *m_treectrl;
};

#endif