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

#include <stdint.h>
#include <stdio.h>

#include "gp.h"
#include "getstatus.h"
#include "pcscwrap.h"
#include "tools.h"
#include "iso7816.h"


#define KNOWN_ELF_COUNT		13

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
		"GSMA RSP",
		16,
		{0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x00, 0x00}
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
		"GP SSD",
		7,
		{0xA0, 0x00, 0x00, 0x01, 0x51, 0x53, 0x50}
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
	},
	{
		"Visa",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10}
	},
	{
		"Mastercard",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10}
	},
	{
		//  PSE
		"1PAY.SYS.DDF01",
		14,
		{0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31}
	},
	{
		//  PPSE
		"2PAY.SYS.DDF01",
		14,
		{0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31}
	}
};

void print_elf_name22(uint8_t* _aid, int _len)
{
	known_elf_t* elf;

	for (int i = 0; i < KNOWN_ELF_COUNT; i++)
	{
		elf = &elf_array[i];

		if ((elf->length == _len) && (0 == memcmp(elf->aid, _aid, _len))) {
			printf(COLOR_BLUE " %-22s" COLOR_RESET, elf->name);
			return;;
		}
	}

	printf("                       "); // Alignment for not found AID
}

static void print_status(uint8_t _type, uint8_t _value)
{
	if (GET_STATUS_ISD == _type) {
		// Table 11-6: Card Life Cycle Coding
		switch (_value) {
		case 0x01:
			printf("OP_READY");
			break;
		case 0x07:
			printf("INITIALIZED");
			break;
		case 0x0F:
			printf("SECURED");
			break;
		case 0x7F:
			printf("CARD_LOCKED");
			break;
		case 0xFF:
			printf("TERMINATED");
			break;

		default:
			printf(COLOR_RED "Unkown Card Manager state: " COLOR_WHITE "%02X" COLOR_RESET, _value);
		}
	}

	if (GET_STATUS_APPLICATIONS == _type) {
		// Table 11-4: Application Life Cycle Coding
		// Table 11-5: Security Domain Life Cycle Coding
		switch (_value) {
		case 0x02:
			printf("INSTALLED");
			break;
		case 0x07:
			printf("SELECTABLE");
			break;
		case 0x0F:
			printf("PERSONALIZED");
			break;
		case 0x13:
			printf("LOCKED");
			break;

		default:
			printf(COLOR_RED "Unkown Application state: " COLOR_WHITE "%02X" COLOR_RESET, _value);
		}
	}

	if (GET_STATUS_PACKAGES == _type) {
		// Table 11-3: Executable Load File Life Cycle Coding
		switch (_value) {
		case 0x01:
			printf("LOADED");
			break;

		default:
			printf(COLOR_RED "Unkown Package state: " COLOR_WHITE "%02X" COLOR_RESET, _value);
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
		case TAG_AID: // 0x4F
			printf(" AID  :  ");
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset + i]);

			for (int i = 0; i < (17-len); i++)
				printf("  ");

			print_elf_name22(&_data[offset], len);

			//printf("\n");

			offset += len;
			break;

		case 0x84:
			printf("  EM     ");
			for (int i = 0; i < len; i++)
				printf("%02X", _data[offset + i]);

			printf("\n");

			offset += len;
			break;

		case TAG_LIFE_CYCLE_STATE: // 0x9F70:
			//printf(" State:  ");

			print_status(_type, _data[offset]);
			printf("\n");
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

	// Print privileges
	//printf("         (-- -- -- -- -- -- -- --) (-- -- -- -- -- -- -- --) (-- -- -- -- -- -- -- --)\n");
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
			apdu.cmd[ISO7816_OFFSET_P2] = GET_STATUS_MODE_EXPANDED | 0x01; // Get next
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
			apdu.cmd[ISO7816_OFFSET_P2] = GET_STATUS_MODE_EXPANDED | 0x01; // Get next
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

	printf("\n");

//	free(do_elfs);
//	free(do_apps);
//	free(do_isd);

	return 0;
}

void cmd_getdata(char* _cmd)
{
	apdu_t apdu;
	int tag;
	int keytype;

	while (*_cmd != ' ') // skip until SPACE
		_cmd++;
	while (*_cmd == ' ') // skip until TAG
		_cmd++;

	if (1 != sscanf(_cmd, "%04X", &tag))
	{
		printf(" Wrong or missing command arg..");
		return;
	}

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GET_DATA_CA;
	apdu.cmd[apdu.cmd_len++] = tag >> 8;
	apdu.cmd[apdu.cmd_len++] = tag & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 == apdu.resp[apdu.resp_len - 2]) {

		// Table 11-28: Key Information Data Structure – Basic
		// Table 11-29: Key Information Data Structure – Extended
		if (DO_E0_KEYDATA == tag) {
			int offset = 0;

			apdu.resp_len -= 2; // exclude SW

			offset++; // E0
			offset++; // Length

			while (offset < apdu.resp_len) {
				if (apdu.resp[offset++] == 0xC0) {
					offset++; // len
					printf(" Key index   :       %d\n", apdu.resp[offset++]);
					printf(" Key version :     %d\n", apdu.resp[offset++]);
					keytype = apdu.resp[offset++];
					printf("   Type & Length:    %s %d\n", (keytype == KEY_TYPE_AES ? "AES" : "DES"), apdu.resp[offset++]);
				}
				else {
					printf(COLOR_RED " Wrong keydata object structure..\n" COLOR_RESET);
					break;
				}
			}
		}
	}
}