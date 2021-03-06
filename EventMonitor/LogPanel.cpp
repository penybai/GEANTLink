/*
    Copyright 2015-2020 Amebis
    Copyright 2016 GÉANT

    This file is part of GÉANTLink.

    GÉANTLink is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GÉANTLink is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GÉANTLink. If not, see <http://www.gnu.org/licenses/>.
*/

#include "PCH.h"


//////////////////////////////////////////////////////////////////////////
// wxEventMonitorLogPanel
//////////////////////////////////////////////////////////////////////////

wxEventMonitorLogPanel::wxEventMonitorLogPanel(wxWindow* parent) : wxEventMonitorLogPanelBase(parent)
{
    // Set focus.
    m_log->SetFocus();
}


//////////////////////////////////////////////////////////////////////////
// wxPersistentEventMonitorLogPanel
//////////////////////////////////////////////////////////////////////////

wxPersistentEventMonitorLogPanel::wxPersistentEventMonitorLogPanel(wxEventMonitorLogPanel *wnd) : wxPersistentWindow<wxEventMonitorLogPanel>(wnd)
{
}


wxString wxPersistentEventMonitorLogPanel::GetKind() const
{
    return wxT(wxPERSIST_TLW_KIND);
}


void wxPersistentEventMonitorLogPanel::Save() const
{
    const wxEventMonitorLogPanel * const wnd = static_cast<const wxEventMonitorLogPanel*>(GetWindow());

    wxPersistentETWListCtrl(wnd->m_log).Save();
}


bool wxPersistentEventMonitorLogPanel::Restore()
{
    wxEventMonitorLogPanel * const wnd = static_cast<wxEventMonitorLogPanel*>(GetWindow());

    wxPersistentETWListCtrl(wnd->m_log).Restore();

    return true;
}
