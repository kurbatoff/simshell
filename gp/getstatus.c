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
#include "iso7816.h"
#include "tools.h"


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
			printf(COLOR_MAGENTA " %s\n" COLOR_RESET, elf->name);
			break;
		}
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
				printf("%02X", _data[offset + i]);

			find_elf_name(&_data[offset], len);
			printf("\n");

			offset += len;
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

int get_status()
{
	// --- step 1: Issuer Security Domain ---
	gCMDlen = 0;
	gCMDbuff[gCMDlen++] = 0x80;
	gCMDbuff[gCMDlen++] = INS_GP_GET_STATUS;
	gCMDbuff[gCMDlen++] = GET_STATUS_ISD;
	gCMDbuff[gCMDlen++] = GET_STATUS_MODE_EXPANDED;
	gCMDbuff[gCMDlen++] = 2;
	gCMDbuff[gCMDlen++] = 0x4F;
	gCMDbuff[gCMDlen++] = 0x00;

	pcsc_sendAPDU(gCMDbuff, gCMDlen, gRESPbuff, sizeof(gRESPbuff), &gRESPlen);

	if (0x61 == gRESPbuff[gRESPlen - 2]) {
		gRESPlen = get_response(gRESPbuff[gRESPlen - 1], gRESPbuff, sizeof(gRESPbuff));
	}

	if (0x90 == gRESPbuff[gRESPlen - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Issuer Security Domain\n" COLOR_RESET);

		gRESPlen -= 2; // Exclude SW

		if (gRESPbuff[offset++] != 0xE3) {
			printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
		}
		else {
			len = gRESPbuff[offset++];
			//if 
			print_application_status(GET_STATUS_ISD, &gRESPbuff[offset], len);
		}
	}

	// --- step 2: Applications, including Security Domains ---
	gCMDlen = 0;
	gCMDbuff[gCMDlen++] = 0x80;
	gCMDbuff[gCMDlen++] = INS_GP_GET_STATUS;
	gCMDbuff[gCMDlen++] = GET_STATUS_APPLICATIONS;
	gCMDbuff[gCMDlen++] = GET_STATUS_MODE_EXPANDED;
	gCMDbuff[gCMDlen++] = 2;
	gCMDbuff[gCMDlen++] = 0x4F;
	gCMDbuff[gCMDlen++] = 0x00;

	pcsc_sendAPDU(gCMDbuff, gCMDlen, gRESPbuff, sizeof(gRESPbuff), &gRESPlen);

	if (0x61 == gRESPbuff[gRESPlen - 2]) {
		gRESPlen = get_response(gRESPbuff[gRESPlen - 1], gRESPbuff, sizeof(gRESPbuff));
	}

	if (0x90 == gRESPbuff[gRESPlen - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Applications, including Security Domains\n" COLOR_RESET);

		gRESPlen -= 2; // Exclude SW

		if (0 == gRESPlen) {
			printf(COLOR_WHITE " [none]\n" COLOR_RESET);
		}
		else {
			while (offset < gRESPlen) {
				if (gRESPbuff[offset++] != 0xE3) {
					printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
				}
				else {
					len = gRESPbuff[offset++];

					print_application_status(GET_STATUS_APPLICATIONS, &gRESPbuff[offset], len);
					offset += len;
				}
			}
		}
	}

	// --- step 3: Executable Load Files and Executable Modules ---
	gCMDlen = 0;
	gCMDbuff[gCMDlen++] = 0x80;
	gCMDbuff[gCMDlen++] = INS_GP_GET_STATUS;
	gCMDbuff[gCMDlen++] = GET_STATUS_PACKAGES;
	gCMDbuff[gCMDlen++] = GET_STATUS_MODE_EXPANDED;
	gCMDbuff[gCMDlen++] = 2;
	gCMDbuff[gCMDlen++] = 0x4F;
	gCMDbuff[gCMDlen++] = 0x00;

	pcsc_sendAPDU(gCMDbuff, gCMDlen, gRESPbuff, sizeof(gRESPbuff), &gRESPlen);

	if (0x61 == gRESPbuff[gRESPlen - 2]) {
		gRESPlen = get_response(gRESPbuff[gRESPlen - 1], gRESPbuff, sizeof(gRESPbuff));
	}

	if (0x90 == gRESPbuff[gRESPlen - 2]) {
		int offset = 0;
		int len;

		printf(COLOR_YELLOW " Executable Load Files and Executable Modules\n" COLOR_RESET);

		gRESPlen -= 2; // Exclude SW

		if (0 == gCMDlen > 0) {
			printf(COLOR_WHITE " [none]\n" COLOR_RESET);
		}
		else {
			while (offset < gRESPlen) {
				if (gRESPbuff[offset++] != 0xE3) {
					printf(COLOR_RED " Wrong GET STATUS data..\n" COLOR_RESET);
				}
				else {
					len = gRESPbuff[offset++];

					print_application_status(GET_STATUS_PACKAGES, &gRESPbuff[offset], len);
					offset += len;
				}
			}
		}
	}

	return 0;
}
