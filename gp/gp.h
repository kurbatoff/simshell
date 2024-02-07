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

#ifndef __GP_H__
#define __GP_H__

static uint8_t GP_ISD[] = { 0xA0, 0, 0, 0x01, 0x51, 0, 0, 0 };


#define KEY_IDX_TOKEN_VERIFICATION			0x70
#define KEY_IDX_RECEIPT_GENERATION			0x71
#define KEY_IDX_DAP_VERIFICATION			0x73
#define KEY_IDX_CLFDB						0x75
// '20' to '2F' '01' to '03' Reserved for SCP '02'
// '30' to '3F' '01' to '03' Reserved for SCP '03'
// '40' to '4F' See [AmdB] Reserved for SCP '81'

// UICC config:
//Key Version number range ('01' to '0F') is reserved for SCP80
//Key Version Number '11' is reserved for DAP as specified in ETSI TS 102 226 [2]
//Key Version Number '74' is reserved for the CASD Keys (cf. section 9.2)


// SCP03
#define INS_INIT_UPDATE						0x50
#define INS_EXTERNAL_AUTHENTICATE			0x82
#define INS_BEGIN_R_MAC_SESSION				0x7A
#define INS_END_R_MAC_SESSION				0x78
#define INS_GP_MANAGE_CHANNEL				0x70

// SCP11
#define INS_PERFORM_SECURITY_OPERATION		0x2A
#define INS_MUTUAL_AUTHENTICATE				0x82
#define INS_INTERNAL_AUTHENTICATE			0x88

/*
  - If CLA = '00' - '0F', '40' - '4F', '60' - '6F', even or odd instruction code 'CA' or 'CB';
  - If CLA = '80' - '8F', 'C0' - 'CF' or 'E0' - 'EF', even instruction code 'CA'.
 */
#define INS_GET_DATA_CA					0xCA
#define INS_GET_DATA_CB					0xCB

#define INS_GP_SELECT						0xA4
#define INS_GET_RESPONSE					0xC0

#define INS_GP_PUT_KEY						0xD8
#define INS_GP_STORE_DATA					0xE2
#define INS_GP_DELETE						0xE4
#define INS_GP_INSTALL						0xE6
#define INS_GP_LOAD							0xE8

#define INS_GP_GET_STATUS					0xF2
#define INS_GP_SET_STATUS					0xF0

#define INSTALL_MORE_INSTALL					0x80
#define INSTALL_FOR_REGISTRY_UPDATE				0x40
#define INSTALL_FOR_PERSONALIZATION				0x20
#define INSTALL_FOR_EXTRADITION					0x10
#define INSTALL_FOR_MAKE_SELECTABLE				0x08
#define INSTALL_FOR_INSTALL						0x04
#define INSTALL_FOR_LOAD						0x02
#define INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE			0x0C


// Target instance type
//#define TARGET_SECURITY_DOMAIN			0x33
//#define TARGET_APPLICATION				0xAA

/*
 * Security domains and application privileges, as defined by GP 2.3
 */
#define PRIV_SECURITY_DOMAIN				0x80
#define PRIV_DAP_VERIFICATION				0x40
#define PRIV_DELEGATED_MANAGEMENT			0x20
#define PRIV_CARD_LOCK						0x10
#define PRIV_CARD_TERMINATE					0x08
#define PRIV_CARD_RESET						0x04
#define PRIV_CVM_MANAGEMENT					0x02
#define PRIV_MANDATED_DAP_VERIFICATION		0x01

#define PRIV_TRUSTED_PATH					0x80
#define PRIV_AUTHORIZED_MANAGEMENT			0x40
#define PRIV_TOKEN_VERIFICATION				0x20
#define PRIV_GLOBAL_DELETE					0x10
#define PRIV_GLOBAL_LOCK					0x08
#define PRIV_GLOBAL_REGISTRY				0x04
#define PRIV_FINAL_APPLICATION				0x02
#define PRIV_GLOBAL_SERVICE					0x01

#define PRIV_RECEIPT_GENERATION				0x80
#define PRIV_CLFDB							0x40
#define PRIV_CONTACTLESS_ACTIVATION			0x20
#define PRIV_CONTACTLESS_SELF_ACTIVATION	0x10
//#define PRIV_____RFU_8					0x08
//#define PRIV_____RFU_4					0x04
//#define PRIV_____RFU_2					0x02
//#define PRIV_____RFU_1					0x01

/*
 * Life cycle states for CARD, as defined by GP 2.2.1
 */
#define CARD_LIFE_CYCLE_OP_READY			0x01
#define CARD_LIFE_CYCLE_INITIALIZED			0x07
#define CARD_LIFE_CYCLE_SECURED				0x0F
#define CARD_LIFE_CYCLE_CARD_LOCKED			0x7F
#define CARD_LIFE_CYCLE_TERMINATED			0xFF

/*
 * Life cycle states SECURITY DOMAIN, as defined by GP 2.2.1
 */
#define SSD_LIFE_CYCLE_INSTALLED			0x03
#define SSD_LIFE_CYCLE_SELECTABLE			0x07
#define SSD_LIFE_CYCLE_PERSONALIZED			0x0F
#define SSD_LIFE_CYCLE_LOCKED				0x83

/*
 * Life cycle states for ELF, as defined by GP 2.2.1
 */
#define ELF_LIFE_CYCLE_LOADED				0x01

/*
 * Life cycle states for INSTANCE, as defined by GP 2.2.1
 */
#define INSTANCE_LIFE_CYCLE_INSTALLED						0x03
#define INSTANCE_LIFE_CYCLE_SELECTABLE						0x07
#define INSTANCE_LIFE_CYCLE_APPLICATION_SPECIFIC			0x07
#define INSTANCE_LIFE_CYCLE_APPLICATION_SPECIFIC_MASK		0x78
#define INSTANCE_LIFE_CYCLE_LOCKED							0x83

/*
 * Data object tags, as defined by GP 2.2.1
 */

/*
 * When DGI formatting is used, these data objects shall be embedded within template DGI '0070'.
 * Otherwise, these data objects shall be presented in BER-TLV format.
 */
#define DO_DGI							0x0070

#define DO_CERTIFICATE					0x7F21
#define DO_PROPRIETARY_WIPEOUT			0xDF1F
#define DO_PROPRIETARY_ID				0xDF21
#define DO_PROPRIETARY_CPLC				0xDF22
#define DO_SCP11_ECKA_CERTIFICATE		0xBF21

#define DO_DGI_KEYDATA					0x00B9 // Tag B9
#define DO_DGI_SYMMETRIC_KEY			0x8113
#define DO_DGI_KEY_MODULUS				0x0010
#define DO_DGI_PUBLIC_KEY_EXPONENT		0x0011
#define DO_DGI_PRIVATE_KEY_EXPONENT		0x8112
#define DO_DGI_RSACRT_Q_1_MOD_P			0x8121
#define DO_DGI_RSACRT_D_MOD_Q_1			0x8122
#define DO_DGI_RSACRT_D_MOD_P_1			0x8123
#define DO_DGI_RSACRT_PRIME_Q			0x8124
#define DO_DGI_RSACRT_PRIME_P			0x8125
#define DO_DGI_ECC_P					0x0030
#define DO_DGI_ECC_A					0x0031
#define DO_DGI_ECC_B					0x0032
#define DO_DGI_ECC_G					0x0033
#define DO_DGI_ECC_N					0x0034
#define DO_DGI_ECC_K					0x0035
#define DO_DGI_ECC_Q_PUBLIC_KEY			0x0036
#define DO_DGI_ECC_D_PRIVATE_KEY		0x8137


#define DO_DE_PROPRIETARY				0xDE // Tag DE: reserved by GP for proprietary use
#define DO_FE_PROPRIETARY				0xFE // Tag FE: reserved by GP for proprietary use

/*
 * Security Domains shall support at least the following data object tags:
 */
#define DO_42_IIN						0x42 // Tag '42': Issuer Identification Number (or Security Domain Provider Identification Number)
#define DO_45_CIN						0x45 // Tag '45': Card Image Number (or Security Domain Image Number)

#define DO_49____						0x49 // ???

#define DO_66_CARDDATA					0x66 // Tag '66': Card Data (or Security Domain Management Data)
#define DO_E0_KEYDATA					0xE0 // Tag 'E0': Key Information Template

/*
 * A Security Domain may support the following data object tags:
 */
#define DO_SEC_LEVEL					0xD3 // Tag 'D3': Current Security Level
#define DO_APP_LIST						0x2F00 // Tag ‘2F00’: List of Applications belonging to the Security Domain, or every application on the card if the Security Domain has Global Registry Privilege
#define DO_E_CARD_RESOURCES				0xFF21 // Tag ‘FF21’: Extended Card Resources Information available for Card Content Management, as defined in ETSI TS 102 226
/*
 * If present, the Security Domain with Receipt Generation privilege 
 *   shall support the following additional data object tag:
 */
#define DO_CONFIRMATION_COUNTER			0xC2 // Tag 'C2': Confirmation Counter

/*
 * A Security Domain supporting Secure Channel Protocol '02' and/or '03' shall support the following data object tag:
 */
#define DO_SEQUENCE_COUNTER				0xC1 // Tag 'C1': Sequence Counter of the default Key Version Number

// CPLC
#define DO_CPLC							0x9F7F // Tag '9F7F': CPLC
#define DO_CF_KEY_DERIVATION_DATA		0xCF


/*
 * Data object tags, as defined by GP 2.3
 */
//#define '00'-'7F' //Reserved for private use
#define KEY_TYPE_DES				0x80 // DES – mode (ECB/CBC) implicitly known
#define KEY_TYPE_3DES				0x81 // Reserved (Triple DES)
#define KEY_TYPE_3DES_CBC			0x82 // Triple DES in CBC mode
#define KEY_TYPE_DES_ECB			0x83 // DES in ECB mode
#define KEY_TYPE_DES_CBC			0x84 // DES in CBC mode
#define KEY_TYPE_TLS				0x85 // Pre-Shared Key for Transport Layer Security
#define KEY_TYPE_RFU_86				0x86 // 86'-'87' // RFU (symmetric algorithms)
#define KEY_TYPE_RFU_87				0x87 // 86'-'87' // RFU (symmetric algorithms)
#define KEY_TYPE_AES				0x88 // AES (16, 24, or 32 long keys)
//#define KEY_TYPE_'89'-'8F' // RFU (symmetric algorithms)
#define KEY_TYPE_HMAC_SHA1			0x90 // HMAC-SHA1 – length of HMAC is implicitly known
#define KEY_TYPE_HMAC_SHA1_160		0x91 // HMAC-SHA1-160 – length of HMAC is 160 bits
//#define KEY_TYPE_'92'-'9F' // RFU (symmetric algorithms)
#define KEY_TYPE_RSA_PUBLIC_E		0xA0 // RSA Public Key - public exponent e component (clear text)
#define KEY_TYPE_RSA_PUBLIC_N		0xA1 // RSA Public Key - modulus N component (clear text)
#define KEY_TYPE_RSA_PRIVATE_N		0xA2 // RSA Private Key - modulus N component
#define KEY_TYPE_RSA_PRIVATE_D		0xA3 // RSA Private Key - private exponent d component
#define KEY_TYPE_RSA_PRIVATE_P		0xA4 // RSA Private Key - Chinese Remainder P component
#define KEY_TYPE_RSA_PRIVATE_Q		0xA5 // RSA Private Key - Chinese Remainder Q component
#define KEY_TYPE_RSA_PRIVATE_QP		0xA6 // RSA Private Key - Chinese Remainder QP component ( q-1 mod p )
#define KEY_TYPE_RSA_PRIVATE_DP1	0xA7 // RSA Private Key - Chinese Remainder DP1 component ( d mod (p-1) )
#define KEY_TYPE_RSA_PRIVATE_DQ1	0xA8 // RSA Private Key - Chinese Remainder DQ1 component ( d mod (q-1) )
//#define KEY_TYPE_ 'A9'-'AF' // RFU (asymmetric algorithms)
#define KEY_TYPE_ECC_PUBLIC_KEY		0xB0 // ECC public key
#define KEY_TYPE_ECC_PRIVATE_KEY	0xB1 // ECC private key
#define KEY_TYPE_ECC_P				0xB2 // ECC field parameter P (field specification)
#define KEY_TYPE_ECC_A				0xB3 // ECC field parameter A (first coefficient)
#define KEY_TYPE_ECC_B				0xB4 // ECC field parameter B (second coefficient)
#define KEY_TYPE_ECC_G				0xB5 // ECC field parameter G (generator)
#define KEY_TYPE_ECC_N				0xB6 // ECC field parameter N (order of generator)
#define KEY_TYPE_ECC_K				0xB7 // ECC field parameter k (cofactor of order of generator)
//#define KEY_TYPE_ECC_	'B8' - 'EF' // RFU (asymmetric algorithms)
#define KEY_TYPE_ECC_PARAM_REFERENCE 0xF0 // ECC key parameters reference
//#define KEY_TYPE_ 'F1' - 'FE' // RFU (asymmetric algorithms)
#define KEY_TYPE_FF					0xFF // Extended format (usage defined for specific APDU gCMDbuffs; e.g. PUT KEY)

#define TAG_AID						0x4F
#define TAG_ECC_PUBLIC_KEY			0x5F49
#define TAG_LIFE_CYCLE_STATE		0x9F70


#endif /* __GP_H__ */
