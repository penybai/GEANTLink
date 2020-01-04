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

#include "StdAfx.h"

#pragma comment(lib, "Eappprxy.lib")

using namespace std;
using namespace winstd;


//////////////////////////////////////////////////////////////////////
// eap::credentials_eaphost
//////////////////////////////////////////////////////////////////////

eap::credentials_eaphost::credentials_eaphost(_In_ module &mod) : credentials(mod)
{
}


eap::credentials_eaphost::credentials_eaphost(_In_ const credentials_eaphost &other) :
    m_cred_blob(other.m_cred_blob),
    credentials(other            )
{
}


eap::credentials_eaphost::credentials_eaphost(_Inout_ credentials_eaphost &&other) noexcept :
    m_cred_blob(std::move(other.m_cred_blob)),
    credentials(std::move(other            ))
{
}


eap::credentials_eaphost& eap::credentials_eaphost::operator=(_In_ const credentials_eaphost &other)
{
    if (this != &other) {
        (credentials&)*this = other;
        m_cred_blob         = other.m_cred_blob;
    }

    return *this;
}


eap::credentials_eaphost& eap::credentials_eaphost::operator=(_Inout_ credentials_eaphost &&other) noexcept
{
    if (this != &other) {
        (credentials&)*this = std::move(other);
        m_cred_blob         = std::move(other.m_cred_blob);
    }

    return *this;
}


eap::config* eap::credentials_eaphost::clone() const
{
    return new credentials_eaphost(*this);
}


void eap::credentials_eaphost::clear()
{
    credentials::clear();
    m_cred_blob.clear();
}


bool eap::credentials_eaphost::empty() const
{
    return m_cred_blob.empty();
}


void eap::credentials_eaphost::save(_In_ IXMLDOMDocument *pDoc, _In_ IXMLDOMNode *pConfigRoot) const
{
    assert(pDoc);
    assert(pConfigRoot);

    credentials::save(pDoc, pConfigRoot);

    HRESULT hr;

    // <Credentials>
    if (FAILED(hr = eapxml::put_element_base64(pDoc, pConfigRoot, bstr(L"Credentials"), namespace_eapmetadata, m_cred_blob.data(), m_cred_blob.size())))
        throw com_runtime_error(hr, __FUNCTION__ " Error creating <Credentials> element.");
}


void eap::credentials_eaphost::load(_In_ IXMLDOMNode *pConfigRoot)
{
    assert(pConfigRoot);
    HRESULT hr;

    credentials::load(pConfigRoot);

    std::wstring xpath(eapxml::get_xpath(pConfigRoot));

    m_cred_blob.clear();
    if (FAILED(hr = eapxml::get_element_base64(pConfigRoot, bstr(L"eap-metadata:Credentials"), m_cred_blob)))
        throw com_runtime_error(hr, __FUNCTION__ " Error reading <Credentials> element.");

    m_module.log_config_discrete((xpath + L"/Credentials").c_str(), m_cred_blob.data(), (ULONG)m_cred_blob.size());
}


void eap::credentials_eaphost::operator<<(_Inout_ cursor_out &cursor) const
{
    credentials::operator<<(cursor);
    cursor << m_cred_blob;
}


size_t eap::credentials_eaphost::get_pk_size() const
{
    return
        credentials::get_pk_size() +
        pksizeof(m_cred_blob);
}


void eap::credentials_eaphost::operator>>(_Inout_ cursor_in &cursor)
{
    credentials::operator>>(cursor);
    cursor >> m_cred_blob;
}


void eap::credentials_eaphost::store(_In_z_ LPCTSTR pszTargetName, _In_ unsigned int level) const
{
    assert(pszTargetName);

    data_blob cred_enc;
    if (!m_cred_blob.empty()) {
        // Encrypt credentials BLOB using user's key.
        DATA_BLOB cred_blob    = { (DWORD)m_cred_blob.size(), const_cast<LPBYTE>(m_cred_blob.data()) };
        DATA_BLOB entropy_blob = {        sizeof(s_entropy) , const_cast<LPBYTE>(s_entropy)          };
        if (!CryptProtectData(&cred_blob, NULL, &entropy_blob, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_AUDIT, &cred_enc))
            throw win_runtime_error(__FUNCTION__ " CryptProtectData failed.");
    }

    tstring target(target_name(pszTargetName, level));

    // Write credentials.
    assert(cred_enc.cbData     < CRED_MAX_CREDENTIAL_BLOB_SIZE);
    assert(m_identity.length() < CRED_MAX_USERNAME_LENGTH     );
    CREDENTIAL cred = {
        0,                                     // Flags
        CRED_TYPE_GENERIC,                     // Type
        const_cast<LPTSTR>(target.c_str()),    // TargetName
        _T(""),                                // Comment
        { 0, 0 },                              // LastWritten
        cred_enc.cbData,                       // CredentialBlobSize
        cred_enc.pbData,                       // CredentialBlob
        CRED_PERSIST_ENTERPRISE,               // Persist
        0,                                     // AttributeCount
        NULL,                                  // Attributes
        NULL,                                  // TargetAlias
        const_cast<LPTSTR>(m_identity.c_str()) // UserName
    };
    if (!CredWrite(&cred, 0))
        throw win_runtime_error(__FUNCTION__ " CredWrite failed.");
}


void eap::credentials_eaphost::retrieve(_In_z_ LPCTSTR pszTargetName, _In_ unsigned int level)
{
    // Read credentials.
    unique_ptr<CREDENTIAL, CredFree_delete<CREDENTIAL> > cred;
    if (!CredRead(target_name(pszTargetName, level).c_str(), CRED_TYPE_GENERIC, 0, (PCREDENTIAL*)&cred))
        throw win_runtime_error(__FUNCTION__ " CredRead failed.");

    if (cred->CredentialBlobSize) {
        // Decrypt the credentials BLOB using user's key.
        DATA_BLOB cred_enc     = { cred->CredentialBlobSize, cred->CredentialBlob          };
        DATA_BLOB entropy_blob = { sizeof(s_entropy)       , const_cast<LPBYTE>(s_entropy) };
        data_blob cred_int;
        if (!CryptUnprotectData(&cred_enc, NULL, &entropy_blob, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_VERIFY_PROTECTION, &cred_int))
            throw win_runtime_error(__FUNCTION__ " CryptUnprotectData failed.");

        m_cred_blob.assign(cred_int.pbData, cred_int.pbData + cred_int.cbData);
        SecureZeroMemory(cred_int.pbData, cred_int.cbData);
    } else
        m_cred_blob.clear();

    if (cred->UserName)
        m_identity = cred->UserName;
    else
        m_identity.clear();

    wstring xpath(pszTargetName);
    m_module.log_config((xpath + L"/Identity").c_str(), m_identity.c_str());
    m_module.log_config_discrete((xpath + L"/Credentials").c_str(), m_cred_blob.data(), (ULONG)m_cred_blob.size());
}


LPCTSTR eap::credentials_eaphost::target_suffix() const
{
    return _T("BLOB");
}


eap::credentials::source_t eap::credentials_eaphost::combine(
    _In_             DWORD         dwFlags,
    _In_opt_         HANDLE        hTokenImpersonateUser,
    _In_opt_   const credentials   *cred_cached,
    _In_       const config_method &cfg,
    _In_opt_z_       LPCTSTR       pszTargetName)
{
    // When cached credentials are available, EapHost calls EapPeerGetIdentity() anyway.
    // This allows each peer to decide to reuse or drop cached credentials itself.
    // To mimic that behaviour, we do the same:
    // 1. Retrieve credentials from cache, store, or configuration
    // 2. Call EapHostPeerGetIdentity()
    source_t src = source_t::unknown;

    if (cred_cached) {
        // Using EAP service cached credentials.
        *this = *dynamic_cast<const credentials_eaphost*>(cred_cached);
        m_module.log_event(&EAPMETHOD_TRACE_EVT_CRED_CACHED2, event_data((unsigned int)cfg.get_method_id()), event_data(get_name()), event_data(pszTargetName), event_data::blank);
        src = source_t::cache;
    }

    // Note: Currently we do not provide credential storage for EapHost methods within configuration.
    // EapHost credentials will never get loaded from configuration, since config_method_eaphost is config_method based, not config_method_with_cred.
    // The code is kept (and maintained) for consistency with another methods, if we choose to provide that feature at a later time.
    if (src == source_t::unknown) {
        auto cfg_with_cred = dynamic_cast<const config_method_with_cred*>(&cfg);
        if (cfg_with_cred && cfg_with_cred->m_use_cred) {
            // Using configured credentials.
            *this = *dynamic_cast<const credentials_eaphost*>(cfg_with_cred->m_cred.get());
            m_module.log_event(&EAPMETHOD_TRACE_EVT_CRED_CONFIG2, event_data((unsigned int)cfg.get_method_id()), event_data(credentials_eaphost::get_name()), event_data(pszTargetName), event_data::blank);
            src = source_t::config;
        }
    }

    if (src == source_t::unknown && pszTargetName) {
        // Switch user context.
        user_impersonator impersonating(hTokenImpersonateUser);

        try {
            credentials_eaphost cred_loaded(m_module);
            cred_loaded.retrieve(pszTargetName, cfg.m_level);

            // Using stored credentials.
            *this = std::move(cred_loaded);
            m_module.log_event(&EAPMETHOD_TRACE_EVT_CRED_STORED2, event_data((unsigned int)cfg.get_method_id()), event_data(get_name()), event_data(pszTargetName), event_data::blank);
            src = source_t::storage;
        } catch (...) {
            // Not actually an error.
        }
    }

    auto cfg_eaphost = dynamic_cast<const config_method_eaphost*>(&cfg);
    BOOL fInvokeUI = FALSE;
    DWORD cred_data_size = 0;
    eap_blob_runtime cred_data;
    unique_ptr<WCHAR[], EapHostPeerFreeRuntimeMemory_delete> identity;
    eap_error error; // MSDN says to use EapHostPeerFreeErrorMemory()/eap_error, but given the context of execution, eap_error_runtime might be the right choice.
    DWORD dwResult = EapHostPeerGetIdentity(
        0,
        dwFlags,
        cfg_eaphost->get_type(),
        (DWORD)cfg_eaphost->m_cfg_blob.size(), cfg_eaphost->m_cfg_blob.data(),
        src != source_t::unknown ? (DWORD)m_cred_blob.size() : 0, src != source_t::unknown ? m_cred_blob.data() : NULL,
        hTokenImpersonateUser,
        &fInvokeUI,
        &cred_data_size, get_ptr(cred_data),
        get_ptr(identity),
        get_ptr(error),
        NULL);
    if (dwResult == ERROR_SUCCESS) {
        if (identity && !fInvokeUI) {
            // Inner EAP method provided identity and does not require additional UI prompt.
            m_identity = identity.get();
            BYTE *_cred_data = cred_data.get();
            m_cred_blob.assign(_cred_data, _cred_data + cred_data_size);
            SecureZeroMemory(_cred_data, cred_data_size);
            m_module.log_event(&EAPMETHOD_TRACE_EVT_CRED_EAPHOST, event_data((unsigned int)cfg.get_method_id()), event_data(get_name()), event_data(pszTargetName), event_data::blank);
            return source_t::lower;
        } else
            SecureZeroMemory(cred_data.get(), cred_data_size);
    } else if (error) {
        // An EAP error in inner EAP method occurred.
        m_module.log_error(error.get());
    } else {
        // A runtime error in inner EAP method occurred.
        m_module.log_event(&EAPMETHOD_TRACE_EVT_WIN_ERROR, event_data((unsigned int)dwResult), event_data(__FUNCTION__ " EapHostPeerGetIdentity failed."), event_data::blank);
    }

    return source_t::unknown;
}


/// \cond internal
const unsigned char eap::credentials_eaphost::s_entropy[1024] = {
    0xe6, 0x01, 0x7b, 0x5f, 0xe5, 0x32, 0xee, 0x8c, 0x57, 0x41, 0x52, 0x95, 0xab, 0xe5, 0x65, 0xdd,
    0xb3, 0x12, 0x7c, 0xcb, 0xdb, 0x37, 0x03, 0x76, 0xfc, 0x53, 0x4a, 0xf9, 0x3f, 0xf1, 0xd8, 0x7e,
    0x60, 0x9a, 0x49, 0x93, 0xeb, 0x2e, 0x18, 0xd0, 0xfb, 0x40, 0xa9, 0x95, 0x66, 0x8a, 0xdd, 0x99,
    0x51, 0x1c, 0xfb, 0x73, 0xa9, 0x7c, 0x31, 0x2b, 0xe6, 0x54, 0x61, 0x64, 0x25, 0x86, 0x36, 0xd4,
    0xde, 0xc8, 0x6d, 0x3e, 0x6f, 0x47, 0x40, 0x6f, 0xd6, 0x5b, 0xe2, 0x0c, 0x92, 0x16, 0xe2, 0xdc,
    0x45, 0x2f, 0x44, 0xf4, 0x87, 0x64, 0x6d, 0x4e, 0x5c, 0x24, 0x8a, 0x10, 0xb1, 0x2f, 0xa7, 0xe3,
    0x7d, 0x60, 0x98, 0x1c, 0xd4, 0x8d, 0xbb, 0x27, 0xb9, 0x02, 0xb1, 0x4a, 0x49, 0xd9, 0x80, 0xe1,
    0x7d, 0x20, 0xac, 0xba, 0x12, 0x23, 0x67, 0x28, 0x9e, 0xe8, 0xb6, 0x27, 0x4a, 0x90, 0x47, 0xcf,
    0x92, 0x00, 0xde, 0x95, 0xeb, 0x23, 0x3b, 0x0d, 0x5e, 0x08, 0xe9, 0x45, 0x42, 0x7a, 0x6a, 0x73,
    0xbb, 0x44, 0xfb, 0x92, 0xc0, 0x24, 0xe2, 0x41, 0xdf, 0x97, 0xb9, 0x02, 0xd4, 0xfd, 0x75, 0x72,
    0x99, 0x6d, 0xbc, 0xf8, 0x20, 0xa4, 0x25, 0x99, 0x5b, 0xb6, 0xfe, 0xf9, 0x1e, 0x4c, 0x02, 0x81,
    0xe8, 0xb7, 0x5f, 0x5f, 0x01, 0xbc, 0xa3, 0xf6, 0xef, 0x8e, 0x28, 0x9d, 0x20, 0x80, 0xc1, 0xb2,
    0xd5, 0x80, 0x44, 0x8d, 0xf3, 0x01, 0x71, 0x3c, 0x0c, 0xb8, 0xc1, 0x0f, 0xc4, 0x79, 0x5a, 0x4d,
    0xd3, 0xd6, 0xe8, 0x6d, 0xe2, 0x6c, 0x50, 0x49, 0x54, 0x27, 0x9b, 0x2f, 0xf2, 0x79, 0xbd, 0xa3,
    0x25, 0xa5, 0x2c, 0x5c, 0x62, 0x89, 0x13, 0xc3, 0x81, 0x31, 0xdd, 0x31, 0x61, 0x43, 0xce, 0xa6,
    0x67, 0x63, 0x25, 0xa4, 0xd0, 0xa7, 0x4c, 0x6c, 0x51, 0x7b, 0xaf, 0x8e, 0xdb, 0xaf, 0x77, 0xeb,
    0x31, 0xfc, 0xb2, 0xdb, 0xc1, 0x89, 0x6a, 0xa8, 0x5e, 0xae, 0xf4, 0xff, 0x55, 0x84, 0xb0, 0x62,
    0x5f, 0x41, 0xde, 0x43, 0x97, 0x2c, 0xe8, 0x86, 0xbe, 0x94, 0xb6, 0xb3, 0x08, 0x38, 0x32, 0xc6,
    0x71, 0x10, 0x5c, 0x82, 0x6c, 0x5d, 0x31, 0x5b, 0x09, 0xd7, 0x92, 0x14, 0xab, 0xcf, 0x8d, 0xc7,
    0xc3, 0xdd, 0x2f, 0xef, 0x20, 0x6e, 0x84, 0x4d, 0xb5, 0x9a, 0x44, 0xac, 0x3f, 0xe4, 0x30, 0xa7,
    0x7a, 0xff, 0xe3, 0xf6, 0x90, 0xa9, 0xeb, 0xca, 0x49, 0x52, 0x89, 0xd6, 0xca, 0x7b, 0xf2, 0x68,
    0xda, 0xe0, 0x88, 0xb0, 0xa2, 0x47, 0xbc, 0x81, 0x2e, 0x58, 0xe0, 0x48, 0xac, 0x6f, 0xf3, 0x66,
    0xd1, 0xa7, 0xd6, 0xda, 0x16, 0x88, 0x69, 0x46, 0x95, 0x5f, 0x35, 0x0c, 0x8d, 0x50, 0xbc, 0x27,
    0xdb, 0xc5, 0x49, 0x9a, 0xf4, 0x4a, 0x7a, 0x03, 0xad, 0xfc, 0x0f, 0x72, 0x5d, 0x6c, 0x62, 0x06,
    0x48, 0x68, 0x75, 0x02, 0xbd, 0xdd, 0xf3, 0xb1, 0xa1, 0x20, 0x64, 0xaf, 0x6f, 0xf2, 0xc0, 0x8c,
    0xe8, 0x3c, 0x58, 0x3b, 0xa7, 0x05, 0x2f, 0x4b, 0xef, 0x29, 0x8b, 0x6f, 0x64, 0x39, 0x03, 0x97,
    0x8b, 0x91, 0x41, 0xbc, 0xa2, 0x02, 0xa9, 0x0c, 0x5c, 0x52, 0x32, 0xf3, 0xe5, 0x4d, 0x5b, 0x7d,
    0xfe, 0x67, 0xe1, 0x82, 0x21, 0x9e, 0x83, 0xf3, 0xd1, 0x5e, 0x37, 0xd9, 0xc6, 0x38, 0x2d, 0x02,
    0x1e, 0x18, 0xa3, 0x47, 0xcf, 0xad, 0x99, 0xe7, 0xe1, 0xc9, 0x86, 0x52, 0xdc, 0x18, 0xe7, 0x3b,
    0x5f, 0x1d, 0xd8, 0x9c, 0xbe, 0xfb, 0x24, 0x09, 0xe9, 0x51, 0x02, 0x51, 0x01, 0xd4, 0xc5, 0x49,
    0xb5, 0x87, 0xd4, 0x5f, 0x7c, 0xdc, 0xf9, 0xc7, 0x7a, 0xf5, 0xb7, 0x1e, 0x6d, 0xc9, 0xc1, 0x1f,
    0x27, 0xd1, 0x77, 0x0e, 0xbb, 0xf8, 0x79, 0x48, 0x55, 0x73, 0x8e, 0xc9, 0x14, 0x8d, 0x6f, 0xf6,
    0xe5, 0xbe, 0x6c, 0xff, 0xa4, 0x4a, 0xa9, 0x03, 0x08, 0xa2, 0xe4, 0xda, 0xcd, 0x8a, 0x83, 0x86,
    0xbd, 0x6e, 0x99, 0xce, 0x98, 0x11, 0xfb, 0x2a, 0x17, 0xd6, 0x79, 0x80, 0x92, 0x11, 0x8d, 0xc4,
    0x4a, 0xef, 0x97, 0xb8, 0x0c, 0x9d, 0xce, 0x2a, 0xda, 0xc9, 0x8f, 0xd9, 0x63, 0x89, 0xf0, 0x44,
    0x94, 0x75, 0xbf, 0x69, 0x8a, 0xe2, 0x6d, 0x40, 0x75, 0x47, 0xd8, 0x4c, 0x91, 0x85, 0x8d, 0x51,
    0xe6, 0xa2, 0x31, 0x13, 0xe5, 0x87, 0x59, 0xb0, 0xf3, 0x89, 0x51, 0xc1, 0xe0, 0xa7, 0xb4, 0x8f,
    0x5e, 0xdd, 0x10, 0x80, 0xd8, 0x4a, 0x69, 0x93, 0x14, 0xf5, 0x2c, 0xef, 0xfb, 0xf0, 0xcb, 0x70,
    0x72, 0x95, 0xb7, 0xec, 0xa4, 0x79, 0xa3, 0xa2, 0x44, 0xfa, 0x02, 0x9f, 0x2a, 0xbf, 0x8b, 0xe7,
    0x87, 0xd4, 0xc2, 0x72, 0x46, 0xd4, 0xf7, 0x57, 0xbe, 0x9a, 0x4e, 0xd4, 0xb5, 0x0f, 0x8d, 0x46,
    0x81, 0x6e, 0x1b, 0xe2, 0x85, 0x3b, 0x78, 0x78, 0x9f, 0xa1, 0xd7, 0x27, 0x2d, 0x92, 0x28, 0x62,
    0x6d, 0xcd, 0xfe, 0x48, 0x18, 0x19, 0x6c, 0x61, 0x6c, 0x8b, 0xbe, 0xe5, 0xb0, 0xff, 0x80, 0x4d,
    0x60, 0x71, 0x89, 0x79, 0x41, 0x08, 0x8e, 0x81, 0x84, 0x71, 0xb8, 0xcd, 0x00, 0x27, 0x37, 0xa2,
    0xad, 0x47, 0xc5, 0x6a, 0xf9, 0xb3, 0x00, 0x67, 0x11, 0x60, 0x93, 0xd1, 0xe1, 0x89, 0x9f, 0xec,
    0x92, 0xb4, 0x48, 0xa6, 0x11, 0x5f, 0xc1, 0x62, 0xb9, 0xd2, 0x3f, 0xb0, 0x4e, 0xd2, 0xdb, 0x1d,
    0x80, 0xd6, 0x02, 0x82, 0x39, 0xb4, 0xbb, 0x3b, 0x51, 0x26, 0xb5, 0x0c, 0xb9, 0xff, 0x9a, 0x38,
    0xa8, 0x49, 0x58, 0x70, 0xdd, 0xec, 0x71, 0x27, 0xec, 0x3c, 0x13, 0x8b, 0x2f, 0xf3, 0x38, 0xa8,
    0x6e, 0xf2, 0xe2, 0xa0, 0xcd, 0xa4, 0x2a, 0x8b, 0xd4, 0xaa, 0x31, 0x83, 0x4c, 0xe6, 0x98, 0xf5,
    0x11, 0xea, 0x40, 0xf9, 0x22, 0xf8, 0x30, 0x56, 0x58, 0xe6, 0xe2, 0x69, 0x49, 0xec, 0x50, 0xbf,
    0x39, 0x10, 0x77, 0xa1, 0x54, 0xf8, 0x82, 0x19, 0x7b, 0xa2, 0xc0, 0x45, 0x25, 0x9a, 0xb0, 0xff,
    0x3e, 0x7a, 0x61, 0xd6, 0xf4, 0xfb, 0xb9, 0x3a, 0x1e, 0x07, 0xee, 0xd4, 0xf2, 0x73, 0x98, 0x1b,
    0xfa, 0xae, 0xe2, 0x3e, 0x5f, 0x5a, 0xd1, 0xda, 0x86, 0x48, 0xfd, 0x43, 0xf4, 0x97, 0x69, 0x58,
    0x0e, 0xb8, 0xca, 0xd2, 0x65, 0xb9, 0x64, 0xdb, 0x2b, 0xe8, 0x26, 0x4c, 0x35, 0xc8, 0x86, 0x9a,
    0xe7, 0xc1, 0x99, 0x39, 0x85, 0x87, 0xd9, 0x1c, 0x5d, 0xea, 0xa2, 0x6f, 0x5b, 0x81, 0x1a, 0x73,
    0x08, 0xf8, 0xf3, 0x07, 0xcb, 0x9c, 0x32, 0x3a, 0x2c, 0x8b, 0x44, 0xe6, 0x48, 0x1e, 0x66, 0x1b,
    0x5e, 0xe3, 0x54, 0xef, 0x68, 0xf7, 0x28, 0xf8, 0xd6, 0x16, 0xe5, 0xde, 0xb0, 0xbc, 0x2d, 0x15,
    0xa0, 0x9f, 0xa6, 0x91, 0x4c, 0x1c, 0x91, 0xc9, 0xf2, 0x63, 0x32, 0xf2, 0xfb, 0xd9, 0x5e, 0x53,
    0xd6, 0x72, 0x8c, 0x1b, 0xe5, 0xf1, 0x80, 0xcc, 0x21, 0xa9, 0x87, 0xaf, 0x64, 0x8d, 0x0f, 0xae,
    0xe9, 0x0c, 0x31, 0x26, 0xa4, 0x72, 0x30, 0x8d, 0x0c, 0xfe, 0x5a, 0x25, 0x2d, 0x18, 0xe5, 0x39,
    0xcc, 0x04, 0x56, 0xa3, 0x65, 0x84, 0x95, 0x43, 0x40, 0x94, 0x6d, 0x16, 0x15, 0x79, 0x35, 0xa1,
    0x7f, 0x4b, 0x3e, 0x08, 0xf1, 0x53, 0xf2, 0xc6, 0x31, 0x7e, 0xb8, 0x29, 0x9b, 0xa4, 0xe9, 0x9d,
    0x6b, 0x95, 0xae, 0x37, 0xdf, 0x1d, 0x8c, 0xea, 0xc2, 0x50, 0x14, 0x48, 0x05, 0xd0, 0xcd, 0xd1,
    0x25, 0x25, 0x10, 0xa2, 0x85, 0x6f, 0x88, 0xe1, 0x22, 0x9d, 0xd2, 0xbe, 0x59, 0x88, 0x86, 0x20,
    0x93, 0x6a, 0x44, 0xed, 0xc8, 0xee, 0x73, 0xe7, 0x1a, 0xc3, 0x16, 0x23, 0xff, 0x69, 0x8c, 0xd0,
};
/// \endcond
