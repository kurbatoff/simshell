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

#include <stdint.h>
#include <string.h>

#include "scp11.h"

#include "pcscwrap.h"
#include "mbedwrap.h"
#include "securechannel.h"
#include "globalplatform.h"
#include "tools.h"
#include "gp.h"
//#include "sw.h"

#include "mbedtls/sha256.h"

// TODO combine with other global temp buffers
static uint8_t PK_OCE_ECKA[0x41];

/**
 *
 *
 */
void cmd_scp11_perform_security_operation(void)
{
	uint8_t command[256 + 5];
	uint16_t commend_len;
	uint8_t response[256 + 2];
	uint16_t response_length;

	// --- 1 ---
	printf(COLOR_CYAN " Step 1: Select SCP11 SSD\n" COLOR_RESET);

	commend_len = 0;
	command[commend_len++] = 0x00;
	command[commend_len++] = INS_GP_SELECT;
	command[commend_len++] = 0x04;
	command[commend_len++] = 0x00;
	command[commend_len++] = 9;

	// scp11_ssd.
	command[commend_len++] = 's';
	command[commend_len++] = 'c';
	command[commend_len++] = 'p';
	command[commend_len++] = '1';
	command[commend_len++] = '1';
	command[commend_len++] = '_';
	command[commend_len++] = 's';
	command[commend_len++] = 's';
	command[commend_len++] = 'd';

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);


	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 != response[response_length - 2]) {
		printf(COLOR_RED " Failed to select SSD\n" COLOR_RESET);
		return;
	}


	// --- 2 ---
	printf(COLOR_CYAN " Step 2: a) Get SD certificate\n" COLOR_RESET);

	commend_len = 0;
	command[commend_len++] = 0x80;
	command[commend_len++] = INS_GP_GET_DATA_CA;
	command[commend_len++] = 0xBF;
	command[commend_len++] = 0x21;
	command[commend_len++] = 6;

	command[commend_len++] = 0xA6;
	command[commend_len++] = 0x04;
	command[commend_len++] = 0x83;
	command[commend_len++] = 0x02;
	command[commend_len++] = 0x11;
	command[commend_len++] = 0x20;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 != response[response_length - 2]) {
		printf(COLOR_RED " Failed fetch SD certificate\n" COLOR_RESET);
		return;
	}

	printf(COLOR_CYAN "         b) Parse SD certificate\n" COLOR_RESET);

	// --- 3 ---
	printf(COLOR_CYAN " Step 3: Perform security operation\n" COLOR_RESET);



	// --- 4 ---
	printf(COLOR_CYAN " Step 4: Mutual authentication\n" COLOR_RESET);

	// TODO parse certificate and verify signature
}

/**
 *
 *
 */
void cmd_scp11_internal_authenticate(void)
{
	//
}

/**
 *
 *
 */
void cmd_scp11_mutual_authenticate(void)
{
}
/*
	Struct_keyset keyset;
	Struct_key key;
	uint16_t sector_id;
	uint8_t key_id;
	uint8_t key_kvn;

	uint8_t sha[M2M_SHA256_DIGEST_LENGTH];
	uint8_t sha_input[192]; // big enough to include 151 bytes data for receipt

	uint8_t* rec_data = sha_input;
	uint8_t receipt_key[16];
	uint16_t receipt_data_len;

	uint8_t SK_SD_ECKA[0x20];
	uint8_t* ePK_OCE_ECKA;
	uint8_t ePK_SD_ECKA[0x41];
	uint8_t eSK_SD_ECKA[0x20];

	uint16_t curveId = MBEDTLS_ECP_DP_SECP256R1;
	
	key_kvn = P1 & 0x7F;
	key_id = P2 & 0x7F;

	resp_len = 0;

	// only allowed after PERFORM SECURITY OPERATION
	if (GP_MODE_PERFORM_SEC_OPERATION != pCONTEXT->gp_mode) {
		set_sw(SW_CONDITIONS_OF_USE_NOT_SATISFIED);
		return;
	}

	// In any case context gets cleared
	pCONTEXT->gp_mode = GP_MODE_NONE;

	// Check APDU len
	if (P3 < 0x53) {
		set_sw(SW_WRONG_LENGTH);
		return;
	}

	// Check APDU format
	if ( (pbApduBuffer[ISO_OFFSET_CDATA] != 0xA6) || (pbApduBuffer[ISO_OFFSET_CDATA+1] < 0x0D) ) {
		set_sw(SW_WRONG_DATA);
		return;
	}

	// 1. Generate Ephemeral keypair ePK_SD_ECKA, eSK_SD_ECKA
	memset(eSK_SD_ECKA, 0x00, 0x20);
	memset(ePK_SD_ECKA, 0x00, 0x41);

	// 2. Locate secret key SK_SD_ECKA
	sector_id = pCONTEXT->ssd.KVN_list.first;

	while (sector_id) {
		load_object(sector_id, (uint8_t* )&keyset, sizeof(Struct_keyset));

		if (keyset.index == key_kvn) {
			// reuse [sector_id] for inner loop
			sector_id = keyset.key_list.first;

			while (sector_id) {
				load_object(sector_id, (uint8_t* )&key, sizeof(Struct_key));
			
				if (key.id == key_id) {
					break;
				}

				sector_id = key.next_offset;
			}

			if (sector_id != 0x00) {
				break;
			}
		}

		sector_id = keyset.next_offset;
	}

	if (0x00 == sector_id) {
		set_sw(SW_REFERENCE_DATA_NOT_FOUND);
		return;
	}

	load_plain_memory(key.data_offset, SK_SD_ECKA, key.length);

	/*
	 * извлекаем ID эллиптической кривой и конвертируем его из GP в тип MBEDTLS_*
	 * Card Specification – Public Release v2.3
	 * Table B-2: Key Parameter Reference Values
	 * /
	switch (keyset.param_reference) {
	case 0x00: // P-256 as specified in [FIPS 186-4]
		curveId = MBEDTLS_ECP_DP_SECP256R1;
		break;
	case 0x03: // brainpoolP256r1 as specified in [RFC 5639]
		curveId = MBEDTLS_ECP_DP_BP256R1;
		break;
	case 0x04: // brainpoolP256t1 as specified in [RFC 5639]


		// TODO fix?
		// НЕ СОВПАДАЕТ с GP ???
		curveId = MBEDTLS_ECP_DP_FRP256; /*!< 256-bits ...... curve * /
		break;
	default:
		// ERROR
		break;
	}

	mbedtls_generate_ecc_keypair(curveId, eSK_SD_ECKA, ePK_SD_ECKA);

	// 3. PK_OCE_ECKA was extracted from command PERFORM SECURITY OPERATION

	
	// 4. Get reference to incoming public key ePK_OCE_ECKA
	ePK_OCE_ECKA = &pbApduBuffer[ISO_OFFSET_CDATA + 0x12];


	// Generate shared secrets
	mbedtls_compute_ecdh_sharedsecret(curveId, SK_SD_ECKA, PK_OCE_ECKA, &sha_input[32]); // Static ShSs
	mbedtls_compute_ecdh_sharedsecret(curveId, eSK_SD_ECKA, ePK_OCE_ECKA, sha_input); // Ephemeral ShSe

	sha_input[64] = 0;
	sha_input[65] = 0;
	sha_input[66] = 0;
	sha_input[67] = 1;
	sha_input[68] = 0x34;
	sha_input[69] = 0x88;
	sha_input[70] = 0x10;

	// generate session keys

	// round 1
	mbedtls_sha256_ret(sha_input, 71, sha, 0);

	memcpy(receipt_key, sha, 16);
	memcpy(buf_session_ENC, &sha[16], 16);

	sha_input[67]++;

	// round 2
	mbedtls_sha256_ret(sha_input, 71, sha, 0);

	memcpy(buf_session_MAC, sha, 16);
	memcpy(buf_session_RMAC, &sha[16], 16);

	sha_input[67]++;

	// round 3
	mbedtls_sha256_ret(sha_input, 71, sha, 0);

	memcpy(buf_session_DEK, sha, 16);

	sha_input[67]++;

	print_hex("RECEIPT key", receipt_key, 16, 0);
	print_hex("ENC key", buf_session_ENC, 16, 0);
	print_hex("MAC key", buf_session_MAC, 16, 0);
	print_hex("RMAC key", buf_session_RMAC, 16, 0);
	print_hex("DEK key", buf_session_DEK, 16, 0);

	// prepare data for RECEIPT
	receipt_data_len = 0;
	
	// --- 1 ---
	rec_data[receipt_data_len++] = 0xA6;
	rec_data[receipt_data_len++] = pbApduBuffer[ISO_OFFSET_CDATA + 1];

	memcpy(&rec_data[receipt_data_len], &pbApduBuffer[ISO_OFFSET_CDATA + 2], pbApduBuffer[ISO_OFFSET_CDATA + 1]);
	receipt_data_len += pbApduBuffer[ISO_OFFSET_CDATA + 1];

	// --- 2 ---
	rec_data[receipt_data_len++] = TAG_ECC_PUBLIC_KEY >> 8;
	rec_data[receipt_data_len++] = TAG_ECC_PUBLIC_KEY & 0xFF;
	rec_data[receipt_data_len++] = 0x41;
	memcpy(&rec_data[receipt_data_len], ePK_OCE_ECKA, 0x41);
	receipt_data_len += 0x41;
	
	// --- 3 ---
	rec_data[receipt_data_len++] = TAG_ECC_PUBLIC_KEY >> 8;
	rec_data[receipt_data_len++] = TAG_ECC_PUBLIC_KEY & 0xFF;
	rec_data[receipt_data_len++] = 0x41;
	memcpy(&rec_data[receipt_data_len], ePK_SD_ECKA, 0x41);
	receipt_data_len += 0x41;
	
	// calculate RECEIPT
	calculate_CMAC_aes(receipt_key, rec_data, receipt_data_len, sha);
	print_hex("RECEIPT: ", sha, 16, 0);

	// The receipt key shall be deleted after calculating the receipt
	memset(receipt_key, 0xCC, sizeof(receipt_key));

	// Return ePK.SD.ECKA
	pbRespBuffer[resp_len++] = TAG_ECC_PUBLIC_KEY >> 8;
	pbRespBuffer[resp_len++] = TAG_ECC_PUBLIC_KEY & 0xFF;
	pbRespBuffer[resp_len++] = 0x41;

	memcpy(&pbRespBuffer[resp_len], ePK_SD_ECKA, 0x41);
	resp_len += 0x41;

	// Append Receipt
	pbRespBuffer[resp_len++] = 0x86;
	pbRespBuffer[resp_len++] = 0x10;

	memcpy(&pbRespBuffer[resp_len], sha, 0x10);
	resp_len += 0x10;

	// update ICV and security status
	pCONTEXT->security_status = GPSYSTEM_AUTHENTICATED;
	pCONTEXT->security_level = SECURITY_LEVEL_CRMAC;
	pCONTEXT->scp_index = SECURE_CHANNEL_PROTOCOL_11;

	memcpy(buf_ICV, sha, LENGTH_OF_ICV);

	set_sw(SW_OK);
}
*/

