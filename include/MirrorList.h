/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorList.h
// Purpose:     Maintains the list of mirrors
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorList.h,v 1.4 2007/03/29 11:39:40 dpage Exp $
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

class Mirror;

WX_DECLARE_OBJARRAY(Mirror, MirrorArray);

class MirrorList
{
public:
	MirrorList(const wxString &mirrorListUrl) { m_mirrorListUrl = mirrorListUrl; };
	bool LoadMirrorList();
	bool PopulateTreeCtrl();
	void SetTree(wxTreeCtrl *tree) { m_treectrl = tree; };
	void DeleteAllItems();
    void SetSelectedMirror(Mirror *mirror) { m_selectedMirror = mirror; };
    Mirror *GetSelectedMirror() { return m_selectedMirror; };

private:
	MirrorArray m_mirrors;
	wxString m_mirrorListUrl;
	wxTreeCtrl *m_treectrl;
    Mirror *m_selectedMirror;
};

#endif