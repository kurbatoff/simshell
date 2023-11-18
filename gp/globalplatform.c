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

#define _CRT_SECURE_NO_WARNINGS 

#include <stdlib.h>
#include <stdint.h>

#include "pcscwrap.h"
#include "hal_aes.h"
#include "hal_des.h"
#include "gp.h"
#include "keys.h"
#include "scp02.h"
#include "scp03.h"
#include "securechannel.h"
#include "globalplatform.h"
#include "getstatus.h"
#include "tools.h"
#include "iso7816.h"

#include "mbedwrap.h"

#define CHALLENGE_SZ		8
#define CRYPTOGRAMM_SZ		8
#define C_MAC_SZ			8

// SCP03
static uint8_t counter[3];

// SCP 02, 03
static uint8_t host_cryptogramm[CRYPTOGRAMM_SZ];
static uint8_t card_cryptogramm[CRYPTOGRAMM_SZ];
static uint8_t card_challenge[CHALLENGE_SZ];
static uint8_t host_challenge[CHALLENGE_SZ];
static uint8_t c_mac[C_MAC_SZ];

int select_ISD()
{
	apdu_t apdu;

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = INS_GP_SELECT;
	apdu.cmd[apdu.cmd_len++] = 0x04;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to select ISD\n" COLOR_RESET);
		return -1;
	}

	return 0;
}

int mutual_authentication()
{
	init_update();
	ext_authenticate();

	return 0;
}

int init_update()
{
	apdu_t apdu;
	sym_keyset_t* SCPkey;

	//random
	generate_random(NULL, host_challenge, CHALLENGE_SZ);

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_INIT_UPDATE;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = CHALLENGE_SZ;

	memcpy(&apdu.cmd[ apdu.cmd_len ], host_challenge, CHALLENGE_SZ);
	apdu.cmd_len += CHALLENGE_SZ;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to perform INIT UPDATE\r\n" COLOR_RESET);
		return -1;
	}

	SCPkey = find_keyset(apdu.resp[10]);
	if (SCPkey == NULL) {
		printf(" ERROR: Keyset %02X not found..\n", apdu.resp[10]);
		return -2;
	}


	CTX.scp_index = apdu.resp[11];

	switch (CTX.scp_index) {
	case SECURE_CHANNEL_PROTOCOL_02:
		memcpy(card_challenge, &apdu.resp[ 12 ], 8); // |counter[2] | challenge[6]|

		scp02_generate_session_key(card_challenge, SCPkey->enc, SCPkey->mac, SCPkey->dek);
		scp02_generate_cryptogramms(host_challenge, card_challenge, host_cryptogramm, card_cryptogramm);

		/*
		dump_hexascii_buffer("sENC:", buf_session_ENC, 16);
		dump_hexascii_buffer("sMAC:", buf_session_MAC, 16);
		dump_hexascii_buffer("sDEK:", buf_session_DEK, 16);
		dump_hexascii_buffer("hostCR:", host_cryptogramm, 8);
		dump_hexascii_buffer("cardCR:", card_cryptogramm, 8);
		// */

		// Verify card cryptogramm
		if (memcmp(card_cryptogramm, &apdu.resp[20], 8) != 0) {
			printf(COLOR_RED " Wrong card cryptogramm\n" COLOR_RESET);
			return -1;
		}
		break;

	case SECURE_CHANNEL_PROTOCOL_03:
		memcpy(counter, &apdu.resp[ 29 ], 3);
		memcpy(card_challenge, &apdu.resp[ 13 ], 8);

		scp03_calculate_cryptogram(SCPkey->enc, SCP03_DERIVE_S_ENC, host_challenge, 8, card_challenge, 8, buf_session_ENC, 128);
		scp03_calculate_cryptogram(SCPkey->mac, SCP03_DERIVE_S_MAC, host_challenge, 8, card_challenge, 8, buf_session_MAC, 128);
		// DEK key will be used static without session key generation
		scp03_calculate_cryptogram(SCPkey->mac, SCP03_DERIVE_S_RMAC, host_challenge, 8, card_challenge, 8, buf_session_RMAC, 128);

		scp03_calculate_cryptogram(buf_session_MAC, SCP03_DERIVE_CARD_CRYPTOGRAMM, host_challenge, 8, card_challenge, 8, card_cryptogramm, 64);
		scp03_calculate_cryptogram(buf_session_MAC, SCP03_DERIVE_HOST_CRYPTOGRAMM, host_challenge, 8, card_challenge, 8, host_cryptogramm, 64);

		/*
		#ifdef [-]
					dump_hexascii_buffer("sENC", buf_session_ENC, 16);
					dump_hexascii_buffer("sMAC", buf_session_MAC, 16);
					dump_hexascii_buffer("sRMAC", buf_session_RMAC, 16);
					dump_hexascii_buffer("cardCR", card_cryptogramm, CRYPTOGRAMM_SZ);
					dump_hexascii_buffer("hostCR", host_cryptogramm, CRYPTOGRAMM_SZ);
		#endif
		*/

		// Verify card cryptogramm
		if (memcmp(card_cryptogramm, &apdu.resp[21], 8) != 0) {
			printf(COLOR_RED " Wrong card cryptogramm\n" COLOR_RESET);
			return -1;
		}

		break;

	default:
		printf(COLOR_RED " Wrong (unexpected) SCP index: %02X\n" COLOR_RESET, CTX.scp_index);
		return -1;
	}

	return 0;
}

int ext_authenticate()
{
	apdu_t apdu;
	uint8_t tmp_buffer[256 + LENGTH_OF_ICV]; // for SCP03
	int len;

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x84;
	apdu.cmd[apdu.cmd_len++] = INS_EXTERNAL_AUTHENTICATE;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = CHALLENGE_SZ + CRYPTOGRAMM_SZ;

	memcpy(&apdu.cmd[apdu.cmd_len], host_cryptogramm, 8);
	apdu.cmd_len += 8;

	switch (CTX.scp_index) {
	case SECURE_CHANNEL_PROTOCOL_02:
		memset(c_mac, 0, C_MAC_SZ);
		scp02_calculate_c_mac(apdu.cmd, apdu.cmd_len, buf_session_MAC, c_mac);

		memcpy(&apdu.cmd[ apdu.cmd_len ], c_mac, C_MAC_SZ);
		apdu.cmd_len += C_MAC_SZ;

		break;

	case SECURE_CHANNEL_PROTOCOL_03:
		memset(buf_ICV, 0, LENGTH_OF_ICV);
		memcpy(tmp_buffer, buf_ICV, LENGTH_OF_ICV);
		len = LENGTH_OF_ICV;

		memcpy(&tmp_buffer[len], apdu.cmd, 13);
		len += 13;

		hal_AES_CMAC(tmp_buffer, len, buf_ICV, buf_session_MAC, buf_ICV);

		memcpy(&apdu.cmd[apdu.cmd_len], buf_ICV, C_MAC_SZ);
		apdu.cmd_len += C_MAC_SZ;

		break;
	}

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	//			CTX.security_level = 
	//			CTX.security_status =

	return 0;
}

/**
 * @brief put-key callback function
 *
 * @param _cmd: command line string
 */
 /*
	 cm> put-keyset 32
		 => 80 D8 00 81 43 20 \
		 80 10 C8 E5 7F AF C5 7D D0 45 B2 0D BF 27 48 B7 01 85 03 8B AF 47 \
		 80 10 C8 E5 7F AF C5 7D D0 45 B2 0D BF 27 48 B7 01 85 03 8B	AF 47 \
		 80 10 C8 E5 7F AF C5 7D D0 45 B2 0D BF 27 48 B7 01 85 03 8B AF 47 00
		 (17769 usec [SYS])
		 <= 20 8B AF 47 8B AF 47 8B AF 47 90 00
	 Status: No Error

	 cm> put-keyset 48
	  => 80 D8 00 81 46 30 \
		 88 11 10 80 D2 A5 B0 8F A0 EE 51 14 3B 45 9E 63 81 06 DF 03 50 4A 77 \
		 88 11 10 80 D2 A5 B0 8F A0 EE 51 14 3B 45 9E 63 81 06 DF 03 50 4A 77 \
		 88 11 10 80 D2 A5 B0 8F A0 EE 51 14 3B 45 9E 63 81 06 DF 03 50 4A 77 00
	  (29880 usec [SYS])
	  <= 6A 80
	 Status: Wrong data
	 Add new key set didn't work, try replace ...
	  => 80 D8 30 81 46 30 88 11 10 80 D2 A5 B0 8F A0 EE
		 51 14 3B 45 9E 63 81 06 DF 03 50 4A 77 88 11 10
		 80 D2 A5 B0 8F A0 EE 51 14 3B 45 9E 63 81 06 DF
		 03 50 4A 77 88 11 10 80 D2 A5 B0 8F A0 EE 51 14
		 3B 45 9E 63 81 06 DF 03 50 4A 77 00
	  (16142 usec [SYS])
	  <= 30 50 4A 77 50 4A 77 50 4A 77 90 00
	 Status: No Error
 */
void cmd_putkeyset(char* _cmd)
{
	apdu_t apdu;
	int kvn;
	sym_keyset_t* key = NULL;
	uint8_t data[16];

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_PUT_KEY;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0x81;
	apdu.cmd[apdu.cmd_len++] = 0;

	while (*_cmd != ' ') // skip until SPACE
		_cmd++;
	while (*_cmd == ' ') // skip until KVN
		_cmd++;

	if (1 != sscanf(_cmd, "%d", &kvn))
	{
		printf(" Wrong or missing command arg..");
		return;
	}

	key = find_keyset(kvn);
	if (NULL == key)
	{
		printf(" Failed to find KVN %d..\n", kvn);
		return;
	}

	switch (key->type) {
	case KEY_TYPE_DES:
		memset(data, 0x00, sizeof(data));
		break;
	case KEY_TYPE_AES:
		memset(data, 0x01, sizeof(data));
		break;
	}

	apdu.cmd[apdu.cmd_len++] = kvn;

	// --- 1 ---
	// ENC
	apdu.cmd[apdu.cmd_len++] = key->type;
	if (key->type == KEY_TYPE_AES) {
		apdu.cmd[apdu.cmd_len++] = 1 + (key->keylen / 8);
	}
	apdu.cmd[apdu.cmd_len++] = (key->keylen / 8);

	if (CTX.scp_index == 03) {
		hal_AES128_crypt_ECB(key->enc, (key->keylen / 8), key->dek, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	}
	else
		hal_DES128_ECB_crypt(key->enc, (key->keylen / 8), buf_session_DEK, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);

	apdu.cmd_len += (key->keylen / 8);

	// KCV
	apdu.cmd[apdu.cmd_len++] = 3;
	if (key->type == KEY_TYPE_DES) {
		hal_DES128_ECB_crypt(data, 16, key->enc, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);
	} else
	hal_AES128_crypt_ECB(data, 16, key->enc, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	apdu.cmd_len += 3;

	// --- 2 ---
	// MAC
	apdu.cmd[apdu.cmd_len++] = key->type;
	if (key->type == KEY_TYPE_AES) {
		apdu.cmd[apdu.cmd_len++] = 1 + (key->keylen / 8);
	}
	apdu.cmd[apdu.cmd_len++] = (key->keylen / 8);

	if (CTX.scp_index == 03) {
		hal_AES128_crypt_ECB(key->mac, (key->keylen / 8), key->dek, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	}
	else
		hal_DES128_ECB_crypt(key->mac, (key->keylen / 8), buf_session_DEK, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);

	apdu.cmd_len += (key->keylen / 8);

	// KCV
	apdu.cmd[apdu.cmd_len++] = 3;
	if (key->type == KEY_TYPE_DES) {
		hal_DES128_ECB_crypt(data, 16, key->mac, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);
	}
	else
		hal_AES128_crypt_ECB(data, 16, key->mac, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	apdu.cmd_len += 3;

	// --- 3 ---
	// DEK
	apdu.cmd[apdu.cmd_len++] = key->type;
	if (key->type == KEY_TYPE_AES) {
		apdu.cmd[apdu.cmd_len++] = 1 + (key->keylen / 8);
	}
	apdu.cmd[apdu.cmd_len++] = (key->keylen / 8);

	if (CTX.scp_index == 03) {
		hal_AES128_crypt_ECB(key->dek, (key->keylen / 8), key->dek, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	} else
		hal_DES128_ECB_crypt(key->dek, (key->keylen / 8), buf_session_DEK, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);

	apdu.cmd_len += (key->keylen / 8);

	// KCV
	apdu.cmd[apdu.cmd_len++] = 3;
	if (key->type == KEY_TYPE_DES) {
		hal_DES128_ECB_crypt(data, 16, key->dek, &apdu.cmd[apdu.cmd_len], DES_ENCRYPT);
	}
	else
		hal_AES128_crypt_ECB(data, 16, key->dek, &apdu.cmd[apdu.cmd_len], AES_ENCRYPT);
	apdu.cmd_len += 3;

	// --- 4 ---
	// Length
	apdu.cmd[ISO1716_OFFSET_LC] = apdu.cmd_len - ISO1716_OFFSET_HEADER_LEN;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if ( (0x6A == apdu.resp[apdu.resp_len - 2]) && (0x80 == apdu.resp[apdu.resp_len - 1]) ) {
		apdu.cmd[ISO7816_OFFSET_P1] = kvn;

		printf("Add new key set didn't work, try replace ...\n");

		pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);
	}

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}
}
