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

#include <stdlib.h>
#include <stdint.h>

#include "pcscwrap.h"
#include "hal_aes.h"
#include "gp.h"
#include "keys.h"
#include "scp02.h"
#include "scp03.h"
#include "securechannel.h"
#include "globalplatform.h"
#include "getstatus.h"
#include "tools.h"

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

	CTX.scp_index = apdu.resp[11];

	switch (CTX.scp_index) {
	case SECURE_CHANNEL_PROTOCOL_02:
		memcpy(card_challenge, &apdu.resp[ 12 ], 8); // |counter[2] | challenge[6]|

		scp02_generate_session_key(card_challenge, KEY, KEY, KEY);
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

		scp03_calculate_cryptogram(KEY, SCP03_DERIVE_S_ENC, host_challenge, 8, card_challenge, 8, buf_session_ENC, 128);
		scp03_calculate_cryptogram(KEY, SCP03_DERIVE_S_MAC, host_challenge, 8, card_challenge, 8, buf_session_MAC, 128);
		// DEK key will be used static without session key generation
		scp03_calculate_cryptogram(KEY, SCP03_DERIVE_S_RMAC, host_challenge, 8, card_challenge, 8, buf_session_RMAC, 128);

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

