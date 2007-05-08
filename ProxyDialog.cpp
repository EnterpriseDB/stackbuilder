/////////////////////////////////////////////////////////////////////////////
// Name:        ProxyDialog.cpp
// Purpose:     Proxy server configuration dialog
// Author:      Dave Page
// Created:     2007-05-02
// RCS-ID:      $Id: ProxyDialog.cpp,v 1.2 2007/05/08 08:04:38 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/msw/registry.h>

// Application headers
#include "ProxyDialog.h"

const int TXT_HTTP_PORT=1002;
const int TXT_FTP_PORT=1003;

BEGIN_EVENT_TABLE(ProxyDialog, wxDialog)
    EVT_BUTTON(wxID_OK,         ProxyDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, 	ProxyDialog::OnCancel)
END_EVENT_TABLE()

ProxyDialog::ProxyDialog(wxWindow *parent, const wxString& title)
: wxDialog(parent, wxID_ANY, title)
{
    // Get the proxy settings
	wxRegKey *key = new wxRegKey(wxT("HKEY_CURRENT_USER\\Software\\PostgreSQL\\StackBuilder\\"));

    wxString http_host, http_port, ftp_host, ftp_port;

	if (key->Exists() && key->HasValue(wxT("HTTP proxy host")))
		key->QueryValue(wxT("HTTP proxy host"), http_host);

	if (key->Exists() && key->HasValue(wxT("HTTP proxy port")))
		key->QueryValue(wxT("HTTP proxy port"), http_port);

	if (key->Exists() && key->HasValue(wxT("FTP proxy host")))
		key->QueryValue(wxT("FTP proxy host"), ftp_host);

	if (key->Exists() && key->HasValue(wxT("FTP proxy port")))
		key->QueryValue(wxT("FTP proxy port"), ftp_port);

    delete key;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    wxGridSizer *gridSizer = new wxFlexGridSizer(2, 4, 5, 5);

    wxStaticText *st = new wxStaticText(this, wxID_ANY, _("HTTP proxy"));
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL);
    m_http_host = new wxTextCtrl(this, wxID_ANY, http_host, wxDefaultPosition, wxSize(200, -1));
	gridSizer->Add(m_http_host, 0, wxALIGN_CENTER_VERTICAL);
    st = new wxStaticText(this, wxID_ANY, _("Port"));
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL);
    m_http_port = new wxTextCtrl(this, wxID_ANY, http_port, wxDefaultPosition, wxSize(40, -1));
	gridSizer->Add(m_http_port, 0, wxALIGN_CENTER_VERTICAL);

    st = new wxStaticText(this, wxID_ANY, _("FTP proxy"));
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL);
    m_ftp_host = new wxTextCtrl(this, wxID_ANY, ftp_host, wxDefaultPosition, wxSize(200, -1));
	gridSizer->Add(m_ftp_host, 0, wxALIGN_CENTER_VERTICAL);
    st = new wxStaticText(this, wxID_ANY, _("Port"));
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL);
    m_ftp_port = new wxTextCtrl(this, wxID_ANY, ftp_port, wxDefaultPosition, wxSize(40, -1));
	gridSizer->Add(m_ftp_port, 0, wxALIGN_CENTER_VERTICAL);

    mainSizer->Add(gridSizer, 0, wxALL, 10);

    wxSizer *buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);

    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void ProxyDialog::OnOK(wxCommandEvent& event)
{
    // Check the ports are valid
    if (!m_http_port->IsEmpty() && !m_http_port->GetValue().IsNumber())
    {
        wxLogError(_("Invalid HTTP port specified."));
        m_http_port->SetFocus();
        return;
    }

    if (!m_ftp_port->IsEmpty() && !m_ftp_port->GetValue().IsNumber())
    {
        wxLogError(_("Invalid FTP port specified."));
        m_ftp_port->SetFocus();
        return;
    }

    // Check that both host and port are specified
    if ((!m_http_host->IsEmpty() && m_http_port->IsEmpty()) || (m_http_host->IsEmpty() && !m_http_port->IsEmpty()))
    {
        wxLogError(_("Both the proxy server and port must be specified."));
        m_http_host->SetFocus();
        return;
    }

    if ((!m_ftp_host->IsEmpty() && m_ftp_port->IsEmpty()) || (m_ftp_host->IsEmpty() && !m_ftp_port->IsEmpty()))
    {
        wxLogError(_("Both the proxy server and port must be specified."));
        m_ftp_host->SetFocus();
        return;
    }

    // Store the settings
    wxRegKey *key = new wxRegKey(wxT("HKEY_CURRENT_USER\\Software\\PostgreSQL\\StackBuilder\\"));

    key->SetValue(wxT("HTTP proxy host"), m_http_host->GetValue());
    key->SetValue(wxT("HTTP proxy port"), m_http_port->GetValue());
    key->SetValue(wxT("FTP proxy host"), m_ftp_host->GetValue());
    key->SetValue(wxT("FTP proxy port"), m_ftp_port->GetValue());

    delete key;

    this->EndModal(wxOK);
}

void ProxyDialog::OnCancel(wxCommandEvent& event)
{
    this->EndModal(wxCANCEL);
}

wxString ProxyDialog::GetProxy(const wxString &protocol)
{
    // Get the proxy settings
	wxRegKey *key = new wxRegKey(wxT("HKEY_CURRENT_USER\\Software\\PostgreSQL\\StackBuilder\\"));

    wxString host, port;

    if (protocol.Lower() == wxT("ftp"))
    {
        if (key->Exists() && key->HasValue(wxT("FTP proxy host")))
	        key->QueryValue(wxT("FTP proxy host"), host);

        if (key->Exists() && key->HasValue(wxT("FTP proxy port")))
	        key->QueryValue(wxT("FTP proxy port"), port);
    }
    else
    {
	    if (key->Exists() && key->HasValue(wxT("HTTP proxy host")))
		    key->QueryValue(wxT("HTTP proxy host"), host);

	    if (key->Exists() && key->HasValue(wxT("HTTP proxy port")))
		    key->QueryValue(wxT("HTTP proxy port"), port);
    }

    if (!host.IsEmpty() && !port.IsEmpty() && port.IsNumber())
        return host + wxT(":") + port;

    return wxEmptyString;
}
