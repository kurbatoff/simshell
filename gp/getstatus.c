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
#include <stdio.h>

#include "gp.h"
#include "getstatus.h"
#include "pcscwrap.h"
#include "tools.h"
#include "iso7816.h"


#define KNOWN_ELF_COUNT		7

typedef struct known_elf_t
{
	const char* name;
	const int length;
	const uint8_t aid[16];
} known_elf_t;

static known_elf_t elf_array[KNOWN_ELF_COUNT] = {
	{
		"java.lang",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x00, 0x01}
	},
	{
		"javacard.framework",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x01, 0x01}
	},
	{
		"javacard.security",
		16,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x01, 0x02}
	},
	{
		"ISD-R",
		16,
		{0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00}
	},
	{
		"ECASD",
		16,
		{0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x02, 0x00}
	},
	{
		"ARA",
		8,
		{0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C}
	},
	{
		"ARA-M",
		9,
		{0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C, 0x00}
	}
};

void find_elf_name(uint8_t* aid, int len)
{
	known_elf_t* elf;

	for (int i = 0; i < KNOWN_ELF_COUNT; i++)
	{
		elf = &elf_array[i];

		if ((elf->length == len) && (0 == memcmp(elf->aid, aid, len))) {
			printf(COLOR_MAGENTA " %-22s" COLOR_RESET, elf->name);
			return;;
		}
	}

	printf("                       ");
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
		case TAG_AID: // 0x4F
			printf(" AID  :  ");
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset + i]);

			for (int i = 0; i < (17-len); i++)
				printf("  ");

			find_elf_name(&_data[offset], len);

			//printf("\n");

			offset += len;
			break;

		case 0x84:
			printf("  EM  :  ");
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset + i]);

			printf("\n");

			offset += len;
			break;

		case 0x9F70:
			//printf(" State:  ");

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
			/*
			printf(" %02X   :  ", tag);
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset]);

			printf("\n");
			*/

			offset += len;

		}
	} // while()
}

int get_status()
{
	apdu_t apdu;
	int len_do_isd = 0;
	int len_do_apps = 0;
	int len_do_elfs = 0;
	uint8_t do_isd[256];// = malloc(256);
	uint8_t do_apps[2048];// = malloc(1024);
	uint8_t do_elfs[2048];// = malloc(1024);

	// --- step 1: Issuer Security Domain ---
	printf(COLOR_YELLOW " Issuer Security Domain\n" COLOR_RESET);

	apdu.cmd_len = 0;
	apdu.cmd[ apdu.cmd_len++ ] = 0x80;
	apdu.cmd[ apdu.cmd_len++ ] = INS_GP_GET_STATUS;
	apdu.cmd[ apdu.cmd_len++ ] = GET_STATUS_ISD;
	apdu.cmd[ apdu.cmd_len++ ] = GET_STATUS_MODE_EXPANDED;
	apdu.cmd[ apdu.cmd_len++ ] = 2;
	apdu.cmd[ apdu.cmd_len++ ] = TAG_AID;
	apdu.cmd[ apdu.cmd_len++ ] = 0x00;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	memcpy(&do_isd[len_do_isd], apdu.resp, apdu.resp_len - 2);
	len_do_isd += (apdu.resp_len - 2);

	// --- step 2: Applications, including Security Domains ---
	printf(COLOR_YELLOW " Applications, including Security Domains\n" COLOR_RESET);

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_GET_STATUS;
	apdu.cmd[apdu.cmd_len++] = GET_STATUS_APPLICATIONS;
	apdu.cmd[apdu.cmd_len++] = GET_STATUS_MODE_EXPANDED;
	apdu.cmd[apdu.cmd_len++] = 2;
	apdu.cmd[apdu.cmd_len++] = TAG_AID;
	apdu.cmd[apdu.cmd_len++] = 0x00;

	while (1) {
		pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

		if (0x61 == apdu.resp[apdu.resp_len - 2]) {
			apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
		}

		if ((0x63 == apdu.resp[apdu.resp_len - 2]) || (0x90 == apdu.resp[apdu.resp_len - 2])) {
			memcpy(&do_apps[len_do_apps], apdu.resp, apdu.resp_len - 2);
			len_do_apps += (apdu.resp_len - 2);
		}

		if ((0x63 == apdu.resp[apdu.resp_len - 2]) && (0x10 == apdu.resp[apdu.resp_len - 1])) {
			apdu.cmd[ ISO_OFFSET_P2 ] = GET_STATUS_MODE_EXPANDED | 0x01; // Get next
			continue;
		}

		break;
	}

	// --- step 3: Executable Load Files and Executable Modules ---
	printf(COLOR_YELLOW " Executable Load Files and Executable Modules\n" COLOR_RESET);

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_GET_STATUS;
	apdu.cmd[apdu.cmd_len++] = GET_STATUS_PACKAGES;
	apdu.cmd[apdu.cmd_len++] = GET_STATUS_MODE_EXPANDED;
	apdu.cmd[apdu.cmd_len++] = 2;
	apdu.cmd[apdu.cmd_len++] = TAG_AID;
	apdu.cmd[apdu.cmd_len++] = 0x00;

	while (1) {
		pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

		if (0x61 == apdu.resp[apdu.resp_len - 2]) {
			apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
		}

		if ((0x63 == apdu.resp[apdu.resp_len - 2]) || (0x90 == apdu.resp[apdu.resp_len - 2])) {
			memcpy(&do_elfs[len_do_elfs], apdu.resp, apdu.resp_len - 2);
			len_do_elfs += (apdu.resp_len - 2);
		}


		if ( (0x63 == apdu.resp[apdu.resp_len - 2]) && (0x10 == apdu.resp[apdu.resp_len - 1]) ) {
			apdu.cmd[ ISO_OFFSET_P2 ] = GET_STATUS_MODE_EXPANDED | 0x01; // Get next
			continue;
		}

		break;
	}

	// ---
	printf(COLOR_YELLOW " Issuer Security Domain\n" COLOR_RESET);

	if (do_isd[0] != 0xE3) {
		printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
	}
	else {
		int offset = 1;
		int len;

		len = do_isd[offset++];
		//if 
		print_application_status(GET_STATUS_ISD, &do_isd[offset], len);
	}

	printf(COLOR_YELLOW " Applications, including Security Domains\n" COLOR_RESET);

	if (0 == len_do_apps) {
		printf(COLOR_WHITE " [none]\n" COLOR_RESET);
	}
	else {
		int offset = 0;
		int len;

		while (offset < len_do_apps) {
			if (do_apps[offset++] != 0xE3) {
				printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
			}
			else {
				len = do_apps[offset++];

				print_application_status(GET_STATUS_APPLICATIONS, &do_apps[offset], len);
				offset += len;
			}
		}
	}

	printf(COLOR_YELLOW " Executable Load Files and Executable Modules\n" COLOR_RESET);

	if (0 == len_do_elfs) {
		printf(COLOR_WHITE " [none]\n" COLOR_RESET);
	}
	else {
		int offset = 0;
		int len;

		while (offset < len_do_elfs) {
			if (do_elfs[offset++] != 0xE3) {
				printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
			}
			else {
				len = do_elfs[offset++];

				print_application_status(GET_STATUS_PACKAGES, &do_elfs[offset], len);
				offset += len;
			}
		}
	}


//	free(do_elfs);
//	free(do_apps);
//	free(do_isd);

	return 0;
}
