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
#include "scp02.h"
#include "scp03.h"
#include "securechannel.h"
#include "globalplatform.h"
#include "tools.h"

#define GET_STATUS_MODE				0x02 // Expanded
#define GET_STATUS_ISD				0x80
#define GET_STATUS_APPLICATIONS		0x40
#define GET_STATUS_PACKAGES			0x10

#define CHALLENGE_SZ		8
#define CRYPTOGRAMM_SZ		8
#define C_MAC_SZ			8

static uint8_t host_cryptogramm[CRYPTOGRAMM_SZ];
static uint8_t card_cryptogramm[CRYPTOGRAMM_SZ];
static uint8_t card_challenge[CHALLENGE_SZ];
static uint8_t host_challenge[CHALLENGE_SZ];
static uint8_t c_mac[C_MAC_SZ];

uint8_t KEY[] = {
	0x40, 0x41, 0x42, 0x43,  0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B,  0x4C, 0x4D, 0x4E, 0x4F
};

void generate_random(uint8_t* dst, uint8_t len)
{
	while (len--) {
		*dst++ = (uint8_t)(rand() & 0xFF);
	}
}

/**
 * @_data poniter to the 1st tag within E3 block (excluding E3 and len)
 * @_length Length of TLV set 
 */
static void print_application_status(uint8_t _type, uint8_t* _data, uint16_t _length)
{
	uint16_t tag;
	uint16_t offset = 0;
	uint8_t len;

	while (offset < _length) {
		tag = _data[offset++];
		if (0x1F == (tag & 0x1F))
			tag = (tag << 8) | _data[offset++];
		len = _data[offset++];

		switch (tag) {
		case 0x4F:
			printf(" AID  :  ");
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset++]);
			printf("\n");
			break;

		case 0x9F70:
			printf(" State:  ");

			if (GET_STATUS_ISD == _type) {
				// Table 11-6: Card Life Cycle Coding
				switch (_data[offset]) {
				case 0x01:
					printf("OP_READY\n");
					break;
				case 0x07:
					printf("INITIALIZED\n");
					break;
				case 0x0F:
					printf("SECURED\n");
					break;
				case 0x7F:
					printf("CARD_LOCKED\n");
					break;
				case 0xFF:
					printf("TERMINATED\n");
					break;

				default:
					printf(COLOR_RED "Unkown ISD state: " COLOR_WHITE "%02X\n" COLOR_RESET, _data[offset]);
				}
			}

			if (GET_STATUS_APPLICATIONS == _type) {
				// Table 11-4: Application Life Cycle Coding
				// Table 11-5: Security Domain Life Cycle Coding
				switch (_data[offset]) {
				case 0x02:
					printf("INSTALLED\n");
					break;
				case 0x07:
					printf("SELECTABLE\n");
					break;
				case 0x0F:
					printf("PERSONALIZED\n");
					break;
				case 0x13:
					printf("LOCKED\n");
					break;

				default:
					printf(COLOR_RED "Unkown Application state: " COLOR_WHITE "%02X\n" COLOR_RESET, _data[offset]);
				}
			}

			if (GET_STATUS_PACKAGES == _type) {
				// Table 11-3: Executable Load File Life Cycle Coding
				switch (_data[offset]) {
				case 0x01:
					printf("LOADED\n");
					break;

				default:
					printf(COLOR_RED "Unkown Package state: " COLOR_WHITE "%02X\n" COLOR_RESET, _data[offset]);
				}
			}

			offset += len;
			break;

		default:
			printf(" %02X   :  ", tag);
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset++]);
			printf("\n");
		}
	} // while()

	printf(COLOR_YELLOW " ---\n" COLOR_RESET);
}

int select_ISD()
{
	uint8_t command[256 + 5];
	uint16_t commend_len;
	uint8_t response[256 + 2];
	uint16_t response_length;

	commend_len = 0;
	command[commend_len++] = 0x00;
	command[commend_len++] = INS_GP_SELECT;
	command[commend_len++] = 0x04;
	command[commend_len++] = 0x00;
	command[commend_len++] = 0x00;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 != response[response_length - 2]) {
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
	//	if (!LChannel_ID) {
	//		open_LC();
	//	}

	uint8_t command[256 + 5];
	uint16_t commend_len;
	uint8_t response[256 + 2];
	uint16_t response_length;

	//random
	generate_random(host_challenge, CHALLENGE_SZ);

	commend_len = 0;
	command[commend_len++] = 0x80;
	command[commend_len++] = INS_INIT_UPDATE;
	command[commend_len++] = 0x00;
	command[commend_len++] = 0x00;
	command[commend_len++] = CHALLENGE_SZ;
	
	memcpy(&command[commend_len], host_challenge, CHALLENGE_SZ);
	commend_len += CHALLENGE_SZ;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 != response[response_length - 2]) {
		printf(COLOR_RED " Failed to perform INIT UPDATE\r\n" COLOR_RESET);
		return -1;
	}

	{
		CTX.scp_index = response[11];
		uint8_t counter[3];
		uint8_t tmp_buffer[256 + LENGTH_OF_ICV]; // for SCP03
		int len;

		switch (CTX.scp_index) {
		case SECURE_CHANNEL_PROTOCOL_02:
			memcpy(card_challenge, &response[12], 8); // |counter[2]|challenge[6]|

			scp02_generate_session_key(card_challenge, KEY, KEY, KEY);
			scp02_generate_cryptogramms(host_challenge, card_challenge, host_cryptogramm, card_cryptogramm);

/*
#ifdef ...
			dump_hexascii_buffer("sENC:", buf_session_ENC, 16);
			dump_hexascii_buffer("sMAC:", buf_session_MAC, 16);
			dump_hexascii_buffer("sDEK:", buf_session_DEK, 16);
			dump_hexascii_buffer("hostCR:", host_cryptogramm, 8);
			dump_hexascii_buffer("cardCR:", card_cryptogramm, 8);
#endif
*/

			// Verify card cryptogramm
			if (memcmp(card_cryptogramm, &response[20], 8) != 0) {
				printf(COLOR_RED " Wrong card cryptogramm\n" COLOR_RESET);
				return -1;
			}

			command[0] = 0x84;
			command[1] = INS_EXTERNAL_AUTHENTICATE;
			command[2] = 0x00;
			command[3] = 0x00;
			command[4] = CHALLENGE_SZ + CRYPTOGRAMM_SZ;

			memcpy(&command[5], host_cryptogramm, 8);

			memset(c_mac, 0, C_MAC_SZ);
			scp02_calculate_c_mac(command, 13, buf_session_MAC, c_mac);

			memcpy(&command[13], c_mac, C_MAC_SZ);

			pcsc_sendAPDU(command, 5 + 16, response, sizeof(response), &response_length);

//			CTX.security_level = 
//			CTX.security_status =

			break;

		case SECURE_CHANNEL_PROTOCOL_03:
			memcpy(counter, &response[29], 3);
			memcpy(card_challenge, &response[13], 8); // |counter[2]|challenge[6]|

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
			if (memcmp(card_cryptogramm, &response[21], 8) != 0) {
				printf(COLOR_RED " Wrong card cryptogramm\n" COLOR_RESET);
				return -1;
			}

			command[0] = 0x84;
			command[1] = INS_EXTERNAL_AUTHENTICATE;
			command[2] = 0x00;
			command[3] = 0x00;
			command[4] = CHALLENGE_SZ + CRYPTOGRAMM_SZ;

			memcpy(&command[5], host_cryptogramm, 8);

			memset(buf_ICV, 0, LENGTH_OF_ICV);
			memcpy(tmp_buffer, buf_ICV, LENGTH_OF_ICV);
			len = LENGTH_OF_ICV;

			memcpy(&tmp_buffer[len], command, 13);
			len += 13;

			hal_AES_CMAC(tmp_buffer, len, buf_ICV, buf_session_MAC, buf_ICV);

			memcpy(&command[13], buf_ICV, C_MAC_SZ);

			pcsc_sendAPDU(command, 5 + CHALLENGE_SZ + CRYPTOGRAMM_SZ,
				response, sizeof(response), &response_length);

			//			CTX.security_level = 
			//			CTX.security_status =

			break;

		default:
			printf(COLOR_RED " Wrong (unexpected) SCP index: %02X\n" COLOR_RESET, CTX.scp_index);
			return -1;
		}

	}

	return 0;
}

int ext_authenticate()
{
	return 0;
}

int get_status()
{
	uint8_t command[256 + 5];
	uint16_t commend_len;
	uint8_t response[256 + 2];
	uint16_t response_length;

	// --- step 1: Issuer Security Domain ---
	commend_len = 0;
	command[commend_len++] = 0x80;
	command[commend_len++] = INS_GP_GET_STATUS;
	command[commend_len++] = GET_STATUS_ISD;
	command[commend_len++] = GET_STATUS_MODE;
	command[commend_len++] = 2;
	command[commend_len++] = 0x4F;
	command[commend_len++] = 0x00;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 == response[response_length - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Issuer Security Domain\n" COLOR_RESET);

		response_length -= 2; // Exclude SW

		if (response[offset++] != 0xE3) {
			printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
		}
		else {
			len = response[offset++];
			//if 
			print_application_status(GET_STATUS_ISD, &response[offset], len);
		}
	}

	// --- step 2: Applications, including Security Domains ---
	commend_len = 0;
	command[commend_len++] = 0x80;
	command[commend_len++] = INS_GP_GET_STATUS;
	command[commend_len++] = GET_STATUS_APPLICATIONS;
	command[commend_len++] = GET_STATUS_MODE;
	command[commend_len++] = 2;
	command[commend_len++] = 0x4F;
	command[commend_len++] = 0x00;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 == response[response_length - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Applications, including Security Domains\n" COLOR_RESET);

		response_length -= 2; // Exclude SW

		if (0 == response_length > 0) {
			printf(COLOR_WHITE " [none]\n" COLOR_RESET);
		} else {
			while (offset < response_length) {
				if (response[offset++] != 0xE3) {
					printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
				}
				else {
					len = response[offset++];

					print_application_status(GET_STATUS_APPLICATIONS, &response[offset], len);
					offset += len;
				}
			}
		}
	}

	// --- step 3: Executable Load Files and Executable Modules ---
	commend_len = 0;
	command[commend_len++] = 0x80;
	command[commend_len++] = INS_GP_GET_STATUS;
	command[commend_len++] = GET_STATUS_PACKAGES;
	command[commend_len++] = GET_STATUS_MODE;
	command[commend_len++] = 2;
	command[commend_len++] = 0x4F;
	command[commend_len++] = 0x00;

	pcsc_sendAPDU(command, commend_len, response, sizeof(response), &response_length);

	if (0x61 == response[response_length - 2]) {
		response_length = get_response(response[response_length - 1], response, sizeof(response));
	}

	if (0x90 == response[response_length - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Executable Load Files and Executable Modules\n" COLOR_RESET);

		response_length -= 2; // Exclude SW

		if (0 == response_length > 0) {
			printf(COLOR_WHITE " [none]\n" COLOR_RESET);
		}
		else {
			while (offset < response_length) {
				if (response[offset++] != 0xE3) {
					printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
				}
				else {
					len = response[offset++];

					print_application_status(GET_STATUS_PACKAGES , &response[offset], len);
					offset += len;
				}
			}
		}
	}


	return 0;
}

int put_key()
{

	return 0;
}
