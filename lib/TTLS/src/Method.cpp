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

#include "StdAfx.h"

using namespace std;
using namespace winstd;


//////////////////////////////////////////////////////////////////////
// eap::method_ttls
//////////////////////////////////////////////////////////////////////

eap::method_ttls::method_ttls(_In_ module &module, _In_ config_method_ttls &cfg, _In_ credentials_ttls &cred) :
    m_outer(module, cfg.m_outer, cred.m_outer),
    method(module, cfg, cred)
{
}


eap::method_ttls::method_ttls(_In_ const method_ttls &other) :
    m_outer(other.m_outer),
    method(other)
{
}


eap::method_ttls::method_ttls(_Inout_ method_ttls &&other) :
    m_outer(std::move(other.m_outer)),
    method(std::move(other))
{
}


eap::method_ttls& eap::method_ttls::operator=(_In_ const method_ttls &other)
{
    if (this != std::addressof(other)) {
        (method&)*this = other;
        m_outer        = other.m_outer;
    }

    return *this;
}


eap::method_ttls& eap::method_ttls::operator=(_Inout_ method_ttls &&other)
{
    if (this != std::addressof(other)) {
        (method&)*this = std::move(other);
        m_outer        = std::move(other.m_outer);
    }

    return *this;
}


bool eap::method_ttls::begin_session(
    _In_        DWORD         dwFlags,
    _In_  const EapAttributes *pAttributeArray,
    _In_        HANDLE        hTokenImpersonateUser,
    _In_        DWORD         dwMaxSendPacketSize,
    _Out_       EAP_ERROR     **ppEapError)
{
    if (!m_outer.begin_session(dwFlags, pAttributeArray, hTokenImpersonateUser, dwMaxSendPacketSize, ppEapError))
        return false;

    return true;
}


bool eap::method_ttls::process_request_packet(
    _In_bytecount_(dwReceivedPacketSize) const EapPacket           *pReceivedPacket,
    _In_                                       DWORD               dwReceivedPacketSize,
    _Out_                                      EapPeerMethodOutput *pEapOutput,
    _Out_                                      EAP_ERROR           **ppEapError)
{
    // Is this a valid EAP-TTLS packet?
    if (dwReceivedPacketSize < 6) {
        *ppEapError = m_module.make_error(EAP_E_EAPHOST_METHOD_INVALID_PACKET, _T(__FUNCTION__) _T(" Packet is too small. EAP-%s packets should be at least 6B."));
        return false;
    } else if (pReceivedPacket->Data[0] != eap_type_ttls) {
        *ppEapError = m_module.make_error(EAP_E_EAPHOST_METHOD_INVALID_PACKET, wstring_printf(_T(__FUNCTION__) _T(" Packet is not EAP-TTLS (expected: %u, received: %u)."), eap_type_ttls, pReceivedPacket->Data[0]).c_str());
        return false;
    }

    if (pReceivedPacket->Code == EapCodeRequest && (pReceivedPacket->Data[1] & ttls_flags_start)) {
        // This is a start EAP-TTLS packet.

        // Determine minimum EAP-TTLS version supported by server and us.
        version_t ver_remote = (version_t)(pReceivedPacket->Data[1] & ttls_flags_ver_mask);
        m_version = std::min<version_t>(ver_remote, version_0);
        m_module.log_event(&EAPMETHOD_HANDSHAKE_START1, event_data((unsigned int)eap_type_ttls), event_data((unsigned char)m_version), event_data((unsigned char)ver_remote), event_data::blank);
    }

    return m_outer.process_request_packet(pReceivedPacket, dwReceivedPacketSize, pEapOutput, ppEapError);
}


bool eap::method_ttls::get_response_packet(
    _Inout_bytecap_(*dwSendPacketSize) EapPacket *pSendPacket,
    _Inout_                            DWORD     *pdwSendPacketSize,
    _Out_                              EAP_ERROR **ppEapError)
{
    if (!m_outer.get_response_packet(pSendPacket, pdwSendPacketSize, ppEapError))
        return false;

    // Change packet type to EAP-TTLS, and add EAP-TTLS version.
    pSendPacket->Data[0]  = (BYTE)eap_type_ttls;
    pSendPacket->Data[1] &= ~ttls_flags_ver_mask;
    pSendPacket->Data[1] |= m_version;

    return true;
}


bool eap::method_ttls::get_result(
    _In_  EapPeerMethodResultReason reason,
    _Out_ EapPeerMethodResult       *ppResult,
    _Out_ EAP_ERROR                 **ppEapError)
{
    return m_outer.get_result(reason, ppResult, ppEapError);
}