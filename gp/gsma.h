/**
 *  Copyright (c) 2023, Intergalaxy LLC
 *  This file is part of SIMSHELL.
 *
 *  SIMSHELL is a free software: you can redistribute it and/or modify
 *  it under the terms of the GNU GENERAL PUBLIC LICENSE as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with SIMSHELL. 
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  SIMSHELL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU GENERAL PUBLIC LICENSE for more details.
 */

#ifndef __GSMA_H__
#define __GSMA_H__

/*
 * SGP.02 M2M definitions
 */
#if 0
#define DO_GSMA_CERTIFICATE_OF_OCE			0x3A01 // Certificate of off-card entity
#define DO_GSMA_KEY_ESTABLISHMENT			0x3A02 // Key Establishment
#define DO_GSMA_ENABLE_PROFILE				0x3A03 // Enable Profile
#define DO_GSMA_DISABLE_PROFILE				0x3A04 // Disable Profile
#define DO_GSMA_SET_FALLBACK_ATTRIBUTE		0x3A05 // Set Fall-back Attribute
#define DO_GSMA_SMSR_ADDRESSING_PARAMS		0x3A07 // SM-SR Addressing Parameters
#define DO_GSMA_NOTIFICATION_CONFIRMATION	0x3A08 // Notification Confirmation
#define DO_GSMA_POL1_SECURITY_RULES			0x3A06 // POL1 Policy Rules

#define DO_GSMA_FORWARDED_CASD_DATA			0xBF30 //Forwarded CASD Data tag
#endif

/*
 * SGP.22 Consumer device definitions
 */
#define DO_GSMA_EUICC_INFO_1				0xBF20 // GetEuiccInfo1Request or EUICCInfo1
#define DO_GSMA_PREPARE_DOWNLOAD			0xBF21 // PrepareDownloadRequest or PrepareDownloadResponse
#define DO_GSMA_EUICC_INFO_2				0xBF22 // GetEuiccInfo2Request or EUICCInfo2
#define DO_GSMA_INIT_SECURE_CHANNEL			0xBF23 // InitialiseSecureChannelRequest
#define DO_GSMA_CONFIGURE_ISDP_REQ			0xBF24 // ConfigureISDPRequest
#define DO_GSMA_STORE_METADATA_REQ			0xBF25 // StoreMetadataRequest
#define DO_GSMA_REPLACE_SESSION_KEYS		0xBF26 // ReplaceSessionKeysRequest
//#define DO_GSMA_RFU						0xBF27 // Reserved
#define DO_GSMA_LIST_NOTIFICATION			0xBF28
#define DO_GSMA_SET_NICKNAME				0xBF29 // SetNicknameRequest or SetNicknameResponse
#define DO_GSMA_UPDATE_METADATA				0xBF2A // UpdateMetadataRequest
#define DO_GSMA_PENDING_NOTIF_LIST			0xBF2B // PendingNotificationsListRequest or PendingNotificationsListResponse
//#define DO_GSMA_RFU						0xBF2C // Reserved
#define DO_GSMA_GET_PROFILE_INFO			0xBF2D
#define DO_GSMA_GET_EUICC_CHALLENGE			0xBF2E
#define DO_GSMA_NOTIFICATION_METADATA		0xBF2F // NotificationMetadata
#define DO_GSMA_NOTIFICATION_SENT			0xBF30 // NotificationSentRequest or NotificationSentResponse
#define DO_GSMA_ENABLE_PROFILE				0xBF31
#define DO_GSMA_DISABLE_PROFILE				0xBF32
#define DO_GSMA_DELETE_PROFILE				0xBF33
#define DO_GSMA_EUICC_MEMORY_RESET			0xBF34 // EuiccMemoryResetRequest or EuiccMemoryResetResponse
#define DO_GSMA_LOAD_CRL					0xBF35 // LoadCRLRequest and LoadCRLResponse
#define DO_GSMA_BOUND_PROFILE_PACKAGE		0xBF36 // BoundProfilePackage
#define DO_GSMA_PROFILE_INSTALL_RESULT		0xBF37 // ProfileInstallationResult
#define DO_GSMA_AUTHENTICATE_SERVER			0xBF38 // AuthenticateServerRequest or AuthenticateServerResponse
#define DO_GSMA_INIT_AUTHENTICATION			0xBF39 // InitiateAuthenticationRequest or InitiateAuthenticationResponse
#define DO_GSMA_GET_BOUND_PROFILE_PACKAGE	0xBF3A // GetBoundProfilePackageRequest or GetBoundProfilePackageResponse
#define DO_GSMA_AUTHENTICATE_CLIENT			0xBF3B // AuthenticateClientRequest or AuthenticateClientResponse
#define DO_GSMA_EUICC_CONFIG_ADDRESSES		0xBF3C // EuiccConfiguredAddressesRequest or EuiccConfiguredAddressesResponse
#define DO_GSMA_HANDLE_NOTIFICATION			0xBF3D // handleNotification
#define DO_GSMA_GET_EID						0xBF3E
#define DO_GSMA_SET_DEFAULT_DP_ADDRESS		0xBF3F // SetDefaultDpAddressRequest or SetDefaultDpAddressResponse
#define DO_GSMA_AUTH_CLIENT_RESP_ES11		0xBF40 // AuthenticateClientResponseEs11
#define DO_GSMA_CANCEL_SESSION				0xBF41 // CancelSessionRequest or CancelSessionResponse or cancelSessionRequestEs9 or cancelSessionResponseEs9
#define DO_GSMA_LPAE_ACTIVATION				0xBF42 // LpaeActivationRequest or LpaeActivationResponse
#define DO_GSMA_GET_RAT						0xBF43 // GetRatRequest or GetRatResponse
#define DO_GSMA_PROFILE_INFO				0xE3 // ProfileInfo

#endif /* __GSMA_H__ */

