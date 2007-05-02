/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorList.cpp
// Purpose:     Maintains the list of mirrors
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorList.cpp,v 1.5 2007/05/02 12:44:43 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/xml/xml.h>

// Application headers
#include "Mirror.h"
#include "MirrorList.h"
#include "ProxyDialog.h"

// Define the MirrorArray type
WX_DEFINE_OBJARRAY(MirrorArray);

bool MirrorList::LoadMirrorList()
{
    wxURL url(m_mirrorListUrl);
    url.SetProxy(ProxyDialog::GetProxy(wxT("http")));
	wxURLError err = url.GetError();
    if (err != wxURL_NOERR)
	{
		wxString msg;
		switch (err)
		{
			case wxURL_SNTXERR:
				msg = _("Could not parse the URL.");
				break;
			case wxURL_NOPROTO:
				msg = _("Unsupported protocol specified.");
				break;
			case wxURL_NOHOST:
				msg = _("No hostname specified in URL.");
				break;
			case wxURL_NOPATH:
				msg = _("No path specified in URL.");
				break;
			case wxURL_CONNERR:
				msg = _("A connection error occurred.");
				break;
			case wxURL_PROTOERR:
				msg = _("A protocol error occurred.");
				break;
		}
		wxLogError(wxString::Format(_("Failed to open the mirror list: %s\n\nError: %s"), m_mirrorListUrl, msg));
        return false;
	}

    wxInputStream *ip = url.GetInputStream();

	if (!ip || !ip->IsOk())
	{
		wxLogError(wxString::Format(_("Failed to open the mirror list: %s\n\nError: The URL specified could not be opened."), m_mirrorListUrl));
		return false;
	}

	wxXmlDocument xml;
	if (!xml.Load(*ip))
	{
		wxLogError(wxString::Format(_("Failed to parse the mirror list: %s"), m_mirrorListUrl));
		return false;
	}

	// Clear the applist
	m_mirrors.Clear();

	// Iterate through the applications and build the list
	wxXmlNode *mirror, *properties;
	mirror = xml.GetRoot()->GetChildren();

	while (mirror) 
	{
		if (mirror->GetName() == wxT("mirror")) 
		{
			Mirror *newMirror = new Mirror();
			properties = mirror->GetChildren();

			while (properties)
			{
				if (properties->GetName() == wxT("country"))
					newMirror->country = properties->GetNodeContent();
				else if (properties->GetName() == wxT("protocol"))
					newMirror->protocol = properties->GetNodeContent();
				else if (properties->GetName() == wxT("hostname"))
					newMirror->hostname = properties->GetNodeContent();
				else if (properties->GetName() == wxT("path"))
					newMirror->rootpath = properties->GetNodeContent();
				else if (properties->GetName() == wxT("port"))
					properties->GetNodeContent().ToLong(&newMirror->port);

				properties = properties->GetNext();
			}


            // Cleanup the path
            if (newMirror->rootpath.EndsWith(wxT("/")))
                newMirror->rootpath = newMirror->rootpath.Left(newMirror->rootpath.Length() -1);

			if (newMirror->IsValid())
				m_mirrors.Add(newMirror);
		}

		mirror = mirror->GetNext();
	}

	return true;
}

bool MirrorList::PopulateTreeCtrl()
{
	wxTreeItemIdValue cookie;
	wxTreeItemId node, root, country, mirror;
	bool found;

	root = m_treectrl->AddRoot(_("Countries"), 0);

	for (unsigned int i=0; i<m_mirrors.GetCount(); i++)
	{
		found = false;

		node = m_treectrl->GetFirstChild(root, cookie);

		while (node)
		{
			if (m_treectrl->GetItemText(node) == m_mirrors[i].country)
			{
				country = node;
				found = true;
				break;
			}
			node = m_treectrl->GetNextChild(root, cookie);
		}

		if (!found)
			country = m_treectrl->AppendItem(root, m_mirrors[i].country, 0);

        wxString label = m_mirrors[i].hostname + wxT(" (") + m_mirrors[i].protocol.Upper() + wxT(")");
	    mirror = m_treectrl->AppendItem(country, label, 1, -1, &m_mirrors[i]);
		m_mirrors[i].m_treeitem = mirror;

		m_treectrl->SortChildren(country);
	}

	m_treectrl->SortChildren(root);
	m_treectrl->Expand(root);

	return true;
}

// Clear the list, but unlink the tree items first.
void MirrorList::DeleteAllItems()
{
	for (unsigned int i=0; i<m_mirrors.GetCount(); i++)
	{
		m_treectrl->SetItemData(m_mirrors[i].m_treeitem, NULL);
		m_mirrors[i].m_treeitem = 0;
	}
	m_mirrors.Clear();
}
