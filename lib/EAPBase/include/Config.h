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

#include <sal.h>

namespace eap
{
    ///
    /// Base class for configuration storage
    ///
    class config;

    ///
    /// Base template for method configuration storage
    ///
    template <class _Tcred> class config_method;

    ///
    /// Provider configuration
    ///
    template <class _Tmeth> class config_provider;

    ///
    /// Providers configuration
    ///
    template <class _Tprov> class config_providers;
}

namespace eapserial
{
    ///
    /// Packs a method configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[in]    val     Configuration to pack
    ///
    template <class _Tcred> inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_method<_Tcred> &val);

    ///
    /// Returns packed size of a method configuration
    ///
    /// \param[in] val  Configuration to pack
    ///
    /// \returns Size of data when packed (in bytes)
    ///
    template <class _Tcred> inline size_t get_pk_size(const eap::config_method<_Tcred> &val);

    ///
    /// Unpacks a method configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[out]   val     Configuration to unpack to
    ///
    template <class _Tcred> inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_method<_Tcred> &val);

    ///
    /// Packs a provider configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[in]    val     Configuration to pack
    ///
    template <class _Tmeth> inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_provider<_Tmeth> &val);

    ///
    /// Returns packed size of a provider configuration
    ///
    /// \param[in] val  Configuration to pack
    ///
    /// \returns Size of data when packed (in bytes)
    ///
    template <class _Tmeth> inline size_t get_pk_size(const eap::config_provider<_Tmeth> &val);

    ///
    /// Unpacks a provider configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[out]   val     Configuration to unpack to
    ///
    template <class _Tmeth> inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_provider<_Tmeth> &val);

    ///
    /// Packs a providers configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[in]    val     Configuration to pack
    ///
    template <class _Tprov> inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_providers<_Tprov> &val);

    ///
    /// Returns packed size of a providers configuration
    ///
    /// \param[in] val  Configuration to pack
    ///
    /// \returns Size of data when packed (in bytes)
    ///
    template <class _Tprov> inline size_t get_pk_size(const eap::config_providers<_Tprov> &val);

    ///
    /// Unpacks a providers configuration
    ///
    /// \param[inout] cursor  Memory cursor
    /// \param[out]   val     Configuration to unpack to
    ///
    template <class _Tprov> inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_providers<_Tprov> &val);
}

#pragma once

#include "Module.h"
#include "EAPSerial.h"
#include "EAPXML.h"

#include <WinStd/COM.h>
#include <WinStd/Common.h>

#include <Windows.h>
#include <eaptypes.h> // Must include after <Windows.h>
#include <tchar.h>

#include <string>
#include <list>


namespace eap
{
    class config
    {
    public:
        ///
        /// Constructs configuration
        ///
        /// \param[in] mod  Reference of the EAP module to use for global services
        ///
        config(_In_ module &mod);

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        config(_In_ const config &other);

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        config(_Inout_ config &&other);

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        /// \returns Reference to this object
        ///
        config& operator=(_In_ const config &other);

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        /// \returns Reference to this object
        ///
        config& operator=(_Inout_ config &&other);

        ///
        /// Clones this configuration
        ///
        /// \returns Pointer to cloned configuration
        ///
        virtual config* clone() const = 0;

        /// \name XML configuration management
        /// @{

        ///
        /// Save configuration to XML document
        ///
        /// \param[in]  pDoc         XML document
        /// \param[in]  pConfigRoot  Suggested root element for saving configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool save(_In_ IXMLDOMDocument *pDoc, _In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError) const = 0;

        ///
        /// Load configuration from XML document
        ///
        /// \param[in]  pConfigRoot  Root element for loading configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool load(_In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError) = 0;

        /// @}

    public:
        module &m_module;   ///< Reference of the EAP module
    };


    template <class _Tcred>
    class config_method : public config
    {
    public:
        ///
        /// Constructs configuration
        ///
        /// \param[in] mod  Reference of the EAP module to use for global services
        ///
        config_method(_In_ module &mod) :
            m_allow_save(true),
            m_use_preshared(false),
            m_preshared(mod),
            config(mod)
        {
        }


        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        config_method(_In_ const config_method<_Tcred> &other) :
            m_allow_save(other.m_allow_save),
            m_anonymous_identity(other.m_anonymous_identity),
            m_use_preshared(other.m_use_preshared),
            m_preshared(other.m_preshared),
            config(other)
        {
        }


        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        config_method(_Inout_ config_method<_Tcred> &&other) :
            m_allow_save(std::move(other.m_allow_save)),
            m_anonymous_identity(std::move(other.m_anonymous_identity)),
            m_use_preshared(std::move(other.m_use_preshared)),
            m_preshared(std::move(other.m_preshared)),
            config(std::move(other))
        {
        }


        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        /// \returns Reference to this object
        ///
        config_method& operator=(_In_ const config_method<_Tcred> &other)
        {
            if (this != &other) {
                (config&)*this       = other;
                m_allow_save         = other.m_allow_save;
                m_anonymous_identity = other.m_anonymous_identity;
                m_use_preshared      = other.m_use_preshared;
                m_preshared          = other.m_preshared;
            }

            return *this;
        }


        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        /// \returns Reference to this object
        ///
        config_method& operator=(_Inout_ config_method<_Tcred> &&other)
        {
            if (this != &other) {
                (config&&)*this      = std::move(other);
                m_allow_save         = std::move(other.m_allow_save);
                m_anonymous_identity = std::move(other.m_anonymous_identity);
                m_use_preshared      = std::move(other.m_use_preshared);
                m_preshared          = std::move(other.m_preshared);
            }

            return *this;
        }


        /// \name XML configuration management
        /// @{

        ///
        /// Save configuration to XML document
        ///
        /// \param[in]  pDoc         XML document
        /// \param[in]  pConfigRoot  Suggested root element for saving configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool save(_In_ IXMLDOMDocument *pDoc, _In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError) const
        {
            const winstd::bstr bstrNamespace(L"urn:ietf:params:xml:ns:yang:ietf-eap-metadata");
            DWORD dwResult;

            // <ClientSideCredential>
            winstd::com_obj<IXMLDOMElement> pXmlElClientSideCredential;
            if ((dwResult = eapxml::create_element(pDoc, pConfigRoot, winstd::bstr(L"eap-metadata:ClientSideCredential"), winstd::bstr(L"ClientSideCredential"), bstrNamespace, &pXmlElClientSideCredential)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <ClientSideCredential> element."), NULL);
                return false;
            }

            // <ClientSideCredential>/<allow-save>
            if ((dwResult = eapxml::put_element_value(pDoc, pXmlElClientSideCredential, winstd::bstr(L"allow-save"), bstrNamespace, m_allow_save)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <allow-save> element."), NULL);
                return false;
            }

            // <ClientSideCredential>/<AnonymousIdentity>
            if (!m_anonymous_identity.empty())
                if ((dwResult = eapxml::put_element_value(pDoc, pXmlElClientSideCredential, winstd::bstr(L"AnonymousIdentity"), bstrNamespace, winstd::bstr(m_anonymous_identity))) != ERROR_SUCCESS) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <AnonymousIdentity> element."), NULL);
                    return false;
                }

            if (m_use_preshared)
                if (!m_preshared.save(pDoc, pXmlElClientSideCredential, ppEapError))
                    return false;

            return true;
        }


        ///
        /// Load configuration from XML document
        ///
        /// \param[in]  pConfigRoot  Root element for loading configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool load(_In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError)
        {
            m_allow_save = true;
            m_use_preshared = false;
            m_preshared.clear();
            m_anonymous_identity.clear();

            // <ClientSideCredential>
            winstd::com_obj<IXMLDOMElement> pXmlElClientSideCredential;
            if (eapxml::select_element(pConfigRoot, winstd::bstr(L"eap-metadata:ClientSideCredential"), &pXmlElClientSideCredential) == ERROR_SUCCESS) {
                // <allow-save>
                eapxml::get_element_value(pXmlElClientSideCredential, winstd::bstr(L"eap-metadata:allow-save"), &m_allow_save);

                // <AnonymousIdentity>
                eapxml::get_element_value(pXmlElClientSideCredential, winstd::bstr(L"eap-metadata:AnonymousIdentity"), m_anonymous_identity);

                if (!m_preshared.load(pXmlElClientSideCredential, ppEapError)) {
                    // This is not really an error - merely an indication pre-shared credentials are unavailable.
                    if (*ppEapError) {
                        m_module.free_error_memory(*ppEapError);
                        *ppEapError = NULL;
                    }
                } else
                    m_use_preshared = true;
            }

            return true;
        }

        /// @}

        ///
        /// Returns EAP method type of this configuration
        ///
        /// \returns One of `eap::type_t` constants.
        ///
        virtual type_t get_method_id() const = 0;

    public:
        bool m_allow_save;                  ///< Are credentials allowed to be saved to Windows Credential Manager?
        std::wstring m_anonymous_identity;  ///< Anonymous identity
        bool m_use_preshared;               ///< Does configuration use pre-shared credentials?
        _Tcred m_preshared;                 ///< Pre-shared credentials
    };


    template <class _Tmeth>
    class config_provider : public config
    {
    public:
        ///
        /// Constructs configuration
        ///
        /// \param[in] mod  Reference of the EAP module to use for global services
        ///
        config_provider(_In_ module &mod) : config(mod)
        {
        }

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        config_provider(_In_ const config_provider &other) :
            m_id(other.m_id),
            m_lbl_alt_credential(other.m_lbl_alt_credential),
            m_lbl_alt_identity(other.m_lbl_alt_identity),
            m_lbl_alt_password(other.m_lbl_alt_password),
            m_methods(other.m_methods),
            config(other)
        {
        }

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        config_provider(_Inout_ config_provider &&other) :
            m_id(std::move(other.m_id)),
            m_lbl_alt_credential(std::move(other.m_lbl_alt_credential)),
            m_lbl_alt_identity(std::move(other.m_lbl_alt_identity)),
            m_lbl_alt_password(std::move(other.m_lbl_alt_password)),
            m_methods(std::move(other.m_methods)),
            config(std::move(other))
        {
        }

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        /// \returns Reference to this object
        ///
        config_provider& operator=(_In_ const config_provider &other)
        {
            if (this != &other) {
                (config&)*this       = other;
                m_id                 = other.m_id;
                m_lbl_alt_credential = other.m_lbl_alt_credential;
                m_lbl_alt_identity   = other.m_lbl_alt_identity;
                m_lbl_alt_password   = other.m_lbl_alt_password;
                m_methods            = other.m_methods;
            }

            return *this;
        }

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        /// \returns Reference to this object
        ///
        config_provider& operator=(_Inout_ config_provider &&other)
        {
            if (this != &other) {
                (config&&)*this      = std::move(other);
                m_id                 = std::move(other.m_id);
                m_lbl_alt_credential = std::move(other.m_lbl_alt_credential);
                m_lbl_alt_identity   = std::move(other.m_lbl_alt_identity);
                m_lbl_alt_password   = std::move(other.m_lbl_alt_password);
                m_methods            = std::move(other.m_methods);
            }

            return *this;
        }

        ///
        /// Clones configuration
        ///
        /// \returns Pointer to cloned configuration
        ///
        virtual config* clone() const { return new config_provider<_Tmeth>(*this); }

        /// \name XML configuration management
        /// @{

        ///
        /// Save configuration to XML document
        ///
        /// \param[in]  pDoc         XML document
        /// \param[in]  pConfigRoot  Suggested root element for saving configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool save(_In_ IXMLDOMDocument *pDoc, _In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError) const
        {
            const winstd::bstr bstrNamespace(L"urn:ietf:params:xml:ns:yang:ietf-eap-metadata");
            DWORD dwResult;
            HRESULT hr;

            // <ID>
            if (!m_id.empty())
                if ((dwResult = eapxml::put_element_value(pDoc, pConfigRoot, winstd::bstr(L"ID"), bstrNamespace, winstd::bstr(m_id))) != ERROR_SUCCESS) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <ID> element."), NULL);
                    return false;
                }

            // <ProviderInfo>
            winstd::com_obj<IXMLDOMElement> pXmlElProviderInfo;
            if ((dwResult = eapxml::create_element(pDoc, pConfigRoot, winstd::bstr(L"eap-metadata:ProviderInfo"), winstd::bstr(L"ProviderInfo"), bstrNamespace, &pXmlElProviderInfo)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <ProviderInfo> element."), NULL);
                return false;
            }

            // <ProviderInfo>/<CredentialPrompt>
            if (!m_lbl_alt_credential.empty())
                if ((dwResult = eapxml::put_element_value(pDoc, pXmlElProviderInfo, winstd::bstr(L"CredentialPrompt"), bstrNamespace, winstd::bstr(m_lbl_alt_credential))) != ERROR_SUCCESS) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <CredentialPrompt> element."), NULL);
                    return false;
                }

            // <ProviderInfo>/<UserNameLabel>
            if (!m_lbl_alt_identity.empty())
                if ((dwResult = eapxml::put_element_value(pDoc, pXmlElProviderInfo, winstd::bstr(L"UserNameLabel"), bstrNamespace, winstd::bstr(m_lbl_alt_identity))) != ERROR_SUCCESS) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <UserNameLabel> element."), NULL);
                    return false;
                }

            // <ProviderInfo>/<PasswordLabel>
            if (!m_lbl_alt_password.empty())
                if ((dwResult = eapxml::put_element_value(pDoc, pXmlElProviderInfo, winstd::bstr(L"PasswordLabel"), bstrNamespace, winstd::bstr(m_lbl_alt_password))) != ERROR_SUCCESS) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <PasswordLabel> element."), NULL);
                    return false;
                }

            // <AuthenticationMethods>
            winstd::com_obj<IXMLDOMElement> pXmlElAuthenticationMethods;
            if ((dwResult = eapxml::create_element(pDoc, pConfigRoot, winstd::bstr(L"eap-metadata:AuthenticationMethods"), winstd::bstr(L"AuthenticationMethods"), bstrNamespace, &pXmlElAuthenticationMethods)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <AuthenticationMethods> element."), NULL);
                return false;
            }

            for (std::list<_Tmeth>::const_iterator method = m_methods.cbegin(), method_end = m_methods.cend(); method != method_end; ++method) {
                // <AuthenticationMethod>
                winstd::com_obj<IXMLDOMElement> pXmlElAuthenticationMethod;
                if ((dwResult = eapxml::create_element(pDoc, winstd::bstr(L"AuthenticationMethod"), bstrNamespace, &pXmlElAuthenticationMethod))) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <AuthenticationMethod> element."), NULL);
                    return false;
                }

                // <AuthenticationMethod>/...
                if (!method->save(pDoc, pXmlElAuthenticationMethod, ppEapError))
                    return false;

                if (FAILED(hr = pXmlElAuthenticationMethods->appendChild(pXmlElAuthenticationMethod, NULL))) {
                    *ppEapError = m_module.make_error(HRESULT_CODE(hr), 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error appending <AuthenticationMethod> element."), NULL);
                    return false;
                }
            }

            return true;
        }


        ///
        /// Load configuration from XML document
        ///
        /// \param[in]  pConfigRoot  Root element for loading configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool load(_In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError)
        {
            assert(pConfigRoot);
            assert(ppEapError);
            DWORD dwResult;
            std::wstring lang;
            LoadString(m_module.m_instance, 2, lang);

            // <ID>
            m_id.clear();
            eapxml::get_element_value(pConfigRoot, winstd::bstr(L"eap-metadata:ID"), m_id);

            // <ProviderInfo>
            m_lbl_alt_credential.clear();
            m_lbl_alt_identity.clear();
            m_lbl_alt_password.clear();
            winstd::com_obj<IXMLDOMElement> pXmlElProviderInfo;
            if (eapxml::select_element(pConfigRoot, winstd::bstr(L"eap-metadata:ProviderInfo"), &pXmlElProviderInfo) == ERROR_SUCCESS) {
                // <CredentialPrompt>
                eapxml::get_element_localized(pXmlElProviderInfo, winstd::bstr(L"eap-metadata:CredentialPrompt"), lang.c_str(), m_lbl_alt_credential);

                // <UserNameLabel>
                eapxml::get_element_localized(pXmlElProviderInfo, winstd::bstr(L"eap-metadata:UserNameLabel"), lang.c_str(), m_lbl_alt_identity);

                // <PasswordLabel>
                eapxml::get_element_localized(pXmlElProviderInfo, winstd::bstr(L"eap-metadata:PasswordLabel"), lang.c_str(), m_lbl_alt_password);
            }

            // Iterate authentication methods (<AuthenticationMethods>).
            m_methods.clear();
            winstd::com_obj<IXMLDOMNodeList> pXmlListMethods;
            if ((dwResult = eapxml::select_nodes(pConfigRoot, winstd::bstr(L"eap-metadata:AuthenticationMethods/eap-metadata:AuthenticationMethod"), &pXmlListMethods)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(ERROR_NOT_FOUND, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error selecting <AuthenticationMethods>/<AuthenticationMethod> elements."), NULL);
                return false;
            }
            long lCount = 0;
            pXmlListMethods->get_length(&lCount);
            for (long i = 0; i < lCount; i++) {
                winstd::com_obj<IXMLDOMNode> pXmlElMethod;
                pXmlListMethods->get_item(i, &pXmlElMethod);

                _Tmeth cfg(m_module);

                // Check EAP method type (<EAPMethod>).
                DWORD dwMethodID;
                if (eapxml::get_element_value(pXmlElMethod, winstd::bstr(L"eap-metadata:EAPMethod"), &dwMethodID) == ERROR_SUCCESS) {
                    if ((type_t)dwMethodID != cfg.get_method_id()) {
                        // Wrong type.
                        continue;
                    }
                }

                // Load configuration.
                if (!cfg.load(pXmlElMethod, ppEapError))
                    return false;

                // Add configuration to the list.
                m_methods.push_back(std::move(cfg));
            }

            return true;
        }

        /// @}

    public:
        std::wstring m_id;                      ///< Profile ID
        winstd::tstring m_lbl_alt_credential;   ///< Alternative label for credential prompt
        winstd::tstring m_lbl_alt_identity;     ///< Alternative label for identity prompt
        winstd::tstring m_lbl_alt_password;     ///< Alternative label for password prompt
        std::list<_Tmeth> m_methods;            ///< List of method configurations
    };


    template <class _Tprov>
    class config_providers : public config
    {
    public:
        ///
        /// Constructs configuration
        ///
        /// \param[in] mod  Reference of the EAP module to use for global services
        ///
        config_providers(_In_ module &mod) : config(mod)
        {
        }

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        config_providers(_In_ const config_providers &other) :
            m_providers(other.m_providers),
            config(other)
        {
        }

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        config_providers(_Inout_ config_providers &&other) :
            m_providers(std::move(other.m_providers)),
            config(std::move(other))
        {
        }

        ///
        /// Copies configuration
        ///
        /// \param[in] other  Configuration to copy from
        ///
        /// \returns Reference to this object
        ///
        config_providers& operator=(_In_ const config_providers &other)
        {
            if (this != &other) {
                (config&)*this = other;
                m_providers = other.m_providers;
            }

            return *this;
        }

        ///
        /// Moves configuration
        ///
        /// \param[in] other  Configuration to move from
        ///
        /// \returns Reference to this object
        ///
        config_providers& operator=(_Inout_ config_providers &&other)
        {
            if (this != &other) {
                (config&&)*this = std::move(other);
                m_providers     = std::move(other.m_providers);
            }

            return *this;
        }

        ///
        /// Clones configuration
        ///
        /// \returns Pointer to cloned configuration
        ///
        virtual config* clone() const { return new config_providers<_Tprov>(*this); }

        /// \name XML configuration management
        /// @{

        ///
        /// Save configuration to XML document
        ///
        /// \param[in]  pDoc         XML document
        /// \param[in]  pConfigRoot  Suggested root element for saving configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool save(_In_ IXMLDOMDocument *pDoc, _In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError) const
        {
            const winstd::bstr bstrNamespace(L"urn:ietf:params:xml:ns:yang:ietf-eap-metadata");
            DWORD dwResult;
            HRESULT hr;

            // Select <EAPIdentityProviderList> node.
            winstd::com_obj<IXMLDOMNode> pXmlElIdentityProviderList;
            if ((dwResult = eapxml::select_node(pConfigRoot, winstd::bstr(L"eap-metadata:EAPIdentityProviderList"), &pXmlElIdentityProviderList)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(ERROR_NOT_FOUND, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error selecting <EAPIdentityProviderList> element."), NULL);
                return false;
            }

            for (std::list<_Tprov>::const_iterator provider = m_providers.cbegin(), provider_end = m_providers.cend(); provider != provider_end; ++provider) {
                // <EAPIdentityProvider>
                winstd::com_obj<IXMLDOMElement> pXmlElIdentityProvider;
                if ((dwResult = eapxml::create_element(pDoc, winstd::bstr(L"EAPIdentityProvider"), bstrNamespace, &pXmlElIdentityProvider))) {
                    *ppEapError = m_module.make_error(dwResult, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error creating <EAPIdentityProvider> element."), NULL);
                    return false;
                }

                // <EAPIdentityProvider>/...
                if (!provider->save(pDoc, pXmlElIdentityProvider, ppEapError))
                    return false;

                if (FAILED(hr = pXmlElIdentityProviderList->appendChild(pXmlElIdentityProvider, NULL))) {
                    *ppEapError = m_module.make_error(HRESULT_CODE(hr), 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error appending <EAPIdentityProvider> element."), NULL);
                    return false;
                }
            }

            return true;
        }


        ///
        /// Load configuration from XML document
        ///
        /// \param[in]  pConfigRoot  Root element for loading configuration
        /// \param[out] ppEapError   Pointer to error descriptor in case of failure. Free using `module::free_error_memory()`.
        ///
        /// \returns
        /// - \c true if succeeded
        /// - \c false otherwise. See \p ppEapError for details.
        ///
        virtual bool load(_In_ IXMLDOMNode *pConfigRoot, _Out_ EAP_ERROR **ppEapError)
        {
            assert(pConfigRoot);
            assert(ppEapError);
            DWORD dwResult;

            // Iterate authentication providers (<EAPIdentityProvider>).
            winstd::com_obj<IXMLDOMNodeList> pXmlListProviders;
            if ((dwResult = eapxml::select_nodes(pConfigRoot, winstd::bstr(L"eap-metadata:EAPIdentityProviderList/eap-metadata:EAPIdentityProvider"), &pXmlListProviders)) != ERROR_SUCCESS) {
                *ppEapError = m_module.make_error(ERROR_NOT_FOUND, 0, NULL, NULL, NULL, _T(__FUNCTION__) _T(" Error selecting <EAPIdentityProviderList><EAPIdentityProvider> elements."), NULL);
                return false;
            }
            long lCount = 0;
            pXmlListProviders->get_length(&lCount);
            for (long i = 0; i < lCount; i++) {
                winstd::com_obj<IXMLDOMNode> pXmlElProvider;
                pXmlListProviders->get_item(i, &pXmlElProvider);

                _Tprov prov(m_module);

                // Load provider.
                if (!prov.load(pXmlElProvider, ppEapError))
                    return false;

                // Add provider to the list.
                m_providers.push_back(std::move(prov));
            }

            return true;
        }

        /// @}

    public:
        std::list<_Tprov> m_providers;  ///< List of provider configurations
    };
}


namespace eapserial
{
    template <class _Tcred>
    inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_method<_Tcred> &val)
    {
        pack(cursor, val.m_allow_save        );
        pack(cursor, val.m_anonymous_identity);
        pack(cursor, val.m_use_preshared     );
        pack(cursor, val.m_preshared         );
    }


    template <class _Tcred>
    inline size_t get_pk_size(const eap::config_method<_Tcred> &val)
    {
        return
            get_pk_size(val.m_allow_save        ) +
            get_pk_size(val.m_anonymous_identity) +
            get_pk_size(val.m_use_preshared     ) +
            get_pk_size(val.m_preshared         );
    }


    template <class _Tcred>
    inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_method<_Tcred> &val)
    {
        unpack(cursor, val.m_allow_save        );
        unpack(cursor, val.m_anonymous_identity);
        unpack(cursor, val.m_use_preshared     );
        unpack(cursor, val.m_preshared         );
    }


    template <class _Tmeth>
    inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_provider<_Tmeth> &val)
    {
        pack(cursor, val.m_id                );
        pack(cursor, val.m_lbl_alt_credential);
        pack(cursor, val.m_lbl_alt_identity  );
        pack(cursor, val.m_lbl_alt_password  );
        pack(cursor, val.m_methods           );
    }


    template <class _Tmeth>
    inline size_t get_pk_size(const eap::config_provider<_Tmeth> &val)
    {
        return
            get_pk_size(val.m_id                ) +
            get_pk_size(val.m_lbl_alt_credential) +
            get_pk_size(val.m_lbl_alt_identity  ) +
            get_pk_size(val.m_lbl_alt_password  ) +
            get_pk_size(val.m_methods           );
    }


    template <class _Tmeth>
    inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_provider<_Tmeth> &val)
    {
        unpack(cursor, val.m_id                );
        unpack(cursor, val.m_lbl_alt_credential);
        unpack(cursor, val.m_lbl_alt_identity  );
        unpack(cursor, val.m_lbl_alt_password  );

        std::list<_Tmeth>::size_type count = *(const std::list<_Tmeth>::size_type*&)cursor;
        cursor += sizeof(std::list<_Tmeth>::size_type);
        val.m_methods.clear();
        for (std::list<_Tmeth>::size_type i = 0; i < count; i++) {
            _Tmeth el(val.m_module);
            unpack(cursor, el);
            val.m_methods.push_back(std::move(el));
        }
    }


    template <class _Tprov>
    inline void pack(_Inout_ unsigned char *&cursor, _In_ const eap::config_providers<_Tprov> &val)
    {
        pack(cursor, val.m_providers);
    }


    template <class _Tprov>
    inline size_t get_pk_size(const eap::config_providers<_Tprov> &val)
    {
        return get_pk_size(val.m_providers);
    }


    template <class _Tprov>
    inline void unpack(_Inout_ const unsigned char *&cursor, _Out_ eap::config_providers<_Tprov> &val)
    {
        std::list<_Tprov>::size_type count = *(const std::list<_Tprov>::size_type*&)cursor;
        cursor += sizeof(std::list<_Tprov>::size_type);
        val.m_providers.clear();
        for (std::list<_Tprov>::size_type i = 0; i < count; i++) {
            _Tprov el(val.m_module);
            unpack(cursor, el);
            val.m_providers.push_back(std::move(el));
        }
    }
}