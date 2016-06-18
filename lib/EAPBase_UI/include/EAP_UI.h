﻿/*
    Copyright 2015-2016 Amebis
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

#include <wx/icon.h>
#include <wx/statbmp.h>
#include <Windows.h>


///
/// EAP configuration dialog
///
template <class _Tcfg, class _wxT> class wxEAPConfigDialog;

///
/// EAP credentials dialog
///
class wxEAPCredentialsDialog;

///
/// EAP dialog banner
///
class wxEAPBannerPanel;

///
/// Base template for credentials configuration panel
///
template <class _Tcfg, class _Tcred, class _Tpanel> class wxEAPCredentialsConfigPanel;

///
/// Base template for all credential panels
///
template <class _Tbase, class _Tcred> class wxCredentialsPanel;

///
/// Password credentials panel
///
class wxPasswordCredentialsPanel;

///
/// Sets icon from resource
///
inline bool wxSetIconFromResource(wxStaticBitmap *bmp, wxIcon &icon, HINSTANCE hinst, PCWSTR pszName);

#pragma once

#include <wx/msw/winundef.h> // Fixes `CreateDialog` name collision
#include "../res/wxEAP_UI.h"

#include "../../EAPBase/include/Config.h"
#include "../../EAPBase/include/Credentials.h"

#include <WinStd/Common.h>
#include <WinStd/Cred.h>
#include <WinStd/Win.h>

#include <wx/log.h>

#include <CommCtrl.h>

#include <list>
#include <memory>


template <class _Tmeth, class _wxT>
class wxEAPConfigDialog : public wxEAPConfigDialogBase
{
public:
    ///
    /// Configuration provider data type
    ///
    typedef eap::config_provider<_Tmeth> _Tprov;

    ///
    /// Configuration data type
    ///
    typedef eap::config_providers<_Tprov> config_type;

    ///
    /// This data type
    ///
    typedef wxEAPConfigDialog<_Tmeth, _wxT> _T;

public:
    ///
    /// Constructs a configuration dialog
    ///
    wxEAPConfigDialog(config_type &cfg, wxWindow* parent) :
        m_cfg(cfg),
        wxEAPConfigDialogBase(parent)
    {
        // Set extra style here, as wxFormBuilder overrides all default flags.
        this->SetExtraStyle(this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);

        for (std::list<_Tprov>::iterator provider = m_cfg.m_providers.begin(), provider_end = m_cfg.m_providers.end(); provider != provider_end; ++provider) {
            bool is_single = provider->m_methods.size() == 1;
            std::list<_Tmeth>::size_type count = 0;
            std::list<_Tmeth>::iterator method = provider->m_methods.begin(), method_end = provider->m_methods.end();
            for (; method != method_end; ++method, count++)
                m_providers->AddPage(
                    new _wxT(
                        provider->m_methods.front(),
                        provider->m_id.c_str(),
                        m_providers),
                    is_single ? provider->m_id : winstd::tstring_printf(_T("%s (%u)"), provider->m_id.c_str(), count));
        }

        this->Layout();
        this->GetSizer()->Fit(this);

        m_buttonsOK->SetDefault();
    }


protected:
    /// \cond internal
    virtual void OnInitDialog(wxInitDialogEvent& event)
    {
        // Forward the event to child panels.
        for (wxWindowList::compatibility_iterator provider = m_providers->GetChildren().GetFirst(); provider; provider = provider->GetNext()) {
            _wxT *prov = wxDynamicCast(provider->GetData(), _wxT);
            if (prov)
                prov->GetEventHandler()->ProcessEvent(event);
        }
    }
    /// \endcond


protected:
    config_type &m_cfg;  ///< EAP providers configuration
};


class wxEAPCredentialsDialog : public wxEAPCredentialsDialogBase
{
public:
    ///
    /// Constructs a credential dialog
    ///
    wxEAPCredentialsDialog(wxWindow* parent);

    ///
    /// Adds panels to the dialog
    ///
    void AddContents(wxPanel **contents, size_t content_count);

protected:
    /// \cond internal
    virtual void OnInitDialog(wxInitDialogEvent& event);
    /// \endcond
};


class wxEAPBannerPanel : public wxEAPBannerPanelBase
{
public:
    ///
    /// Constructs a banner pannel and set the title text to product name
    ///
    wxEAPBannerPanel(wxWindow* parent);

protected:
    /// \cond internal
    virtual bool AcceptsFocusFromKeyboard() const { return false; }
    /// \endcond
};


template <class _Tcfg, class _Tcred, class _Tpanel>
class wxEAPCredentialsConfigPanel : public wxEAPCredentialsConfigPanelBase
{
public:
    ///
    /// Constructs a credential configuration panel
    ///
    /// \param[inout] cfg            Configuration data
    /// \param[in]    pszCredTarget  Target name of credentials in Windows Credential Manager. Can be further decorated to create final target name.
    /// \param[in]    parent         Parent window
    ///
    wxEAPCredentialsConfigPanel(_Tcfg &cfg, LPCTSTR pszCredTarget, wxWindow *parent) :
        m_cfg(cfg),
        m_target(pszCredTarget),
        m_cred(m_cfg.m_module),
        wxEAPCredentialsConfigPanelBase(parent)
    {
        // Load and set icon.
        if (m_shell32.load(_T("shell32.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE))
            wxSetIconFromResource(m_credentials_icon, m_icon, m_shell32, MAKEINTRESOURCE(48));
    }

protected:
    /// \cond internal

    virtual bool TransferDataToWindow()
    {
        wxCHECK(wxEAPCredentialsConfigPanelBase::TransferDataToWindow(), false);

        if (!m_cfg.m_use_preshared) {
            m_own->SetValue(true);
        } else {
            m_preshared->SetValue(true);
            m_cred = m_cfg.m_preshared;
        }

        return true;
    }


    virtual bool TransferDataFromWindow()
    {
        if (m_own->GetValue()) {
            m_cfg.m_use_preshared = false;
        } else {
            m_cfg.m_use_preshared = true;
            m_cfg.m_preshared = m_cred;
        }

        return wxEAPCredentialsConfigPanelBase::TransferDataFromWindow();
    }


    virtual void OnUpdateUI(wxUpdateUIEvent& event)
    {
        UNREFERENCED_PARAMETER(event);
        DWORD dwResult;

        bool has_own;
        std::unique_ptr<CREDENTIAL, winstd::CredFree_delete<CREDENTIAL> > cred;
        if (CredRead(m_cred.target_name(m_target.c_str()).c_str(), CRED_TYPE_GENERIC, 0, (PCREDENTIAL*)&cred)) {
            m_identity_own->SetValue(cred->UserName && cred->UserName[0] != 0 ? cred->UserName : _("<blank>"));
            has_own = true;
        } else if ((dwResult = GetLastError()) == ERROR_NOT_FOUND) {
            m_identity_own->Clear();
            has_own = false;
        } else {
            m_identity_own->SetValue(wxString::Format(_("<error %u>"), dwResult));
            has_own = true;
        }

        if (m_own->GetValue()) {
            m_identity_own      ->Enable(true);
            m_set_own           ->Enable(true);
            m_clear_own         ->Enable(has_own);
            m_identity_preshared->Enable(false);
            m_identity_preshared->SetValue(wxEmptyString);
            m_set_preshared     ->Enable(false);
        } else {
            m_identity_own      ->Enable(false);
            m_set_own           ->Enable(false);
            m_clear_own         ->Enable(false);
            m_identity_preshared->Enable(true);
            m_identity_preshared->SetValue(!m_cred.empty() ? m_cred.m_identity : _("<blank>"));
            m_set_preshared     ->Enable(true);
        }
    }


    virtual void OnSetOwn(wxCommandEvent& event)
    {
        UNREFERENCED_PARAMETER(event);

        wxEAPCredentialsDialog dlg(this);

        _Tpanel *panel = new _Tpanel(m_cred, m_target.c_str(), &dlg, true);

        dlg.AddContents((wxPanel**)&panel, 1);
        dlg.ShowModal();
    }


    virtual void OnClearOwn(wxCommandEvent& event)
    {
        UNREFERENCED_PARAMETER(event);

        if (!CredDelete(m_cred.target_name(m_target.c_str()).c_str(), CRED_TYPE_GENERIC, 0))
            wxLogError(_("Deleting credentials failed (error %u)."), GetLastError());
    }


    virtual void OnSetPreshared(wxCommandEvent& event)
    {
        UNREFERENCED_PARAMETER(event);

        wxEAPCredentialsDialog dlg(this);

        _Tpanel *panel = new _Tpanel(m_cred, _T(""), &dlg, true);

        dlg.AddContents((wxPanel**)&panel, 1);
        dlg.ShowModal();
    }

    /// \endcond

protected:
    _Tcfg &m_cfg;               ///< EAP configuration
    winstd::library m_shell32;  ///< shell32.dll resource library reference
    wxIcon m_icon;              ///< Panel icon
    winstd::tstring m_target;   ///< Credential Manager target

private:
    _Tcred m_cred;              ///< Temporary credential data
};


template <class _Tbase, class _Tcred>
class wxCredentialsPanel : public _Tbase
{
public:
    ///
    /// Constructs a credentials panel
    ///
    /// \param[inout] cred           Credentials data
    /// \param[in]    pszCredTarget  Target name of credentials in Windows Credential Manager. Can be further decorated to create final target name.
    /// \param[in]    parent         Parent window
    /// \param[in]    is_config      Is this panel used to pre-enter credentials? When \c true, the "Remember" checkbox is always selected and disabled.
    ///
    wxCredentialsPanel(_Tcred &cred, LPCTSTR pszCredTarget, wxWindow* parent, bool is_config = false) :
        m_cred(cred),
        m_target(pszCredTarget),
        _Tbase(parent)
    {
        if (m_target.empty() || is_config) {
            // No Credential Manager, or user is setting credentials via configuration UI.
            // => Pointless if not stored to Credential Manager
            m_remember->SetValue(true);
            m_remember->Enable(false);
        }
    }

protected:
    /// \cond internal

    virtual bool TransferDataToWindow()
    {
        wxCHECK(_Tbase::TransferDataToWindow(), false);

        if (!m_target.empty()) {
            // Read credentials from Credential Manager
            EAP_ERROR *pEapError;
            if (m_cred.retrieve(m_target.c_str(), &pEapError)) {
                m_remember->SetValue(true);
            } else if (pEapError) {
                if (pEapError->dwWinError != ERROR_NOT_FOUND)
                    wxLogError(winstd::tstring_printf(_("Error reading credentials from Credential Manager: %ls (error %u)"), pEapError->pRootCauseString, pEapError->dwWinError).c_str());
                m_cred.m_module.free_error_memory(pEapError);
            } else
                wxLogError(_("Reading credentials failed."));
        }

        return true;
    }


    virtual bool TransferDataFromWindow()
    {
        if (!m_target.empty()) {
            // Write credentials to credential manager.
            if (m_remember->GetValue()) {
                EAP_ERROR *pEapError;
                if (!m_cred.store(m_target.c_str(), &pEapError)) {
                    if (pEapError) {
                        wxLogError(winstd::tstring_printf(_("Error writing credentials to Credential Manager: %ls (error %u)"), pEapError->pRootCauseString, pEapError->dwWinError).c_str());
                        m_cred.m_module.free_error_memory(pEapError);
                    } else
                        wxLogError(_("Writing credentials failed."));
                }
            }
        }

        return _Tbase::TransferDataFromWindow();
    }

    /// \endcond

protected:
    _Tcred &m_cred;             ///< Password credentials
    winstd::tstring m_target;   ///< Credential Manager target
};


class wxPasswordCredentialsPanel : public wxCredentialsPanel<wxPasswordCredentialsPanelBase, eap::credentials_pass>
{
public:
    ///
    /// Constructs a password credentials panel
    ///
    /// \param[inout] cred           Credentials data
    /// \param[in]    pszCredTarget  Target name of credentials in Windows Credential Manager. Can be further decorated to create final target name.
    /// \param[in]    parent         Parent window
    /// \param[in]    is_config      Is this panel used to pre-enter credentials? When \c true, the "Remember" checkbox is always selected and disabled.
    ///
    wxPasswordCredentialsPanel(eap::credentials_pass &cred, LPCTSTR pszCredTarget, wxWindow* parent, bool is_config = false);

protected:
    /// \cond internal
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    /// \endcond

protected:
    winstd::library m_shell32;  ///< shell32.dll resource library reference
    wxIcon m_icon;              ///< Panel icon

private:
    static const wxStringCharType *s_dummy_password;
};


inline bool wxSetIconFromResource(wxStaticBitmap *bmp, wxIcon &icon, HINSTANCE hinst, PCWSTR pszName)
{
    wxASSERT(bmp);

    HICON hIcon;
    if (SUCCEEDED(LoadIconWithScaleDown(hinst, pszName, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), &hIcon))) {
        icon.CreateFromHICON(hIcon);
        bmp->SetIcon(icon);
        return true;
    } else
        return false;
}