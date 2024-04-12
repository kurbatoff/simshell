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

#include <stdio.h>

#include "euicc.h"
#include "gp.h"
#include "gsma.h"
#include "tools.h"
#include "pcscwrap.h"

static uint8_t ISDR[] = { 0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00 };

static int select_ISD_R()
{
	apdu_t apdu;

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = INS_GP_SELECT;
	apdu.cmd[apdu.cmd_len++] = 0x04;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = sizeof(ISDR);

	memcpy(&apdu.cmd[apdu.cmd_len], ISDR, sizeof(ISDR));
	apdu.cmd_len += sizeof(ISDR);

	pcsc_send_plain_APDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to select ISD-R\n" COLOR_RESET);
		return -1;
	}

	return 0;
}

static void print_UICC_Capability(uint8_t* tlvbuff)
{
	printf(COLOR_YELLOW_I "    TODO: decode...\n" COLOR_RESET);
}

static void print_RSP_Capability(uint8_t* tlvbuff)
{
	printf(COLOR_YELLOW_I "    TODO: decode...\n" COLOR_RESET);
}

static void print_card_resources(uint8_t* tlvbuff)
{
	int len;
	int i;
	int mem_sz;

	len = tlvbuff[1];

	i = 2; // skip tag, len
	while (i < len) {

		switch (tlvbuff[i]) {
		case 0x81:
			printf("    Instances    " COLOR_YELLOW "%d\n" COLOR_RESET, tlvbuff[i+2]);
			break;
		
		case 0x82:
			mem_sz = 0;
			for (int j = 0; j < tlvbuff[i + 1]; j++) {
				mem_sz *= 0x100;
				mem_sz += tlvbuff[i + 2 + j];
			}
			printf("    Free flash   " COLOR_YELLOW "%d\n" COLOR_RESET, mem_sz);
			break;

		case 0x83:
			mem_sz = 0;
			for (int j = 0; j < tlvbuff[i + 1]; j++) {
				mem_sz *= 0x100;
				mem_sz += tlvbuff[i + 2 + j];
			}
			printf("    Free RAM     " COLOR_YELLOW "%d\n" COLOR_RESET, mem_sz);
			break;
		default:
			print_tlv(&tlvbuff[i]);
		}

		i += tlvbuff[i + 1];
		i += 2;
	}
}

static void print_CIs(uint8_t* tlvbuff)
{
	int lenCI;

	lenCI = tlvbuff[1];
	for (int i = 0; i < lenCI; ) {
		i += 2; // skip tag, len

		printf("                 ");
		for (int j = 0; j < tlvbuff[i + 1]; j++)
			printf("%02X", tlvbuff[i + 2 + j]);

		i += tlvbuff[i + 1];
		printf("\n");
	}

	printf(COLOR_RESET);
}

static void print_euicc_info1(uint8_t* tlvbuff)
{
	int buf_sz;
	int offset;

	offset = 2; // skip BF20
	switch (tlvbuff[offset]) {
	case 0x81:
		buf_sz = tlvbuff[offset + 1];
		buf_sz += 4;
		offset += 2;
		break;

	case 0x82:
		buf_sz = tlvbuff[offset + 1] * 0x100 + tlvbuff[offset + 2];
		buf_sz += 5;
		offset += 3;
		break;

	default:
		buf_sz = tlvbuff[offset];
		buf_sz += 3;
		offset++;
	}

	while (offset < buf_sz) {
		int tag;
		int len;

		tag = tlvbuff[offset];
		if (0x1F == (0x1F & tag)) {
			tag <<= 8;
			tag += tlvbuff[offset + 1];
		}

		switch (tag) {
		case 0x82: // SVN
			printf(" SGP.22 version  " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0xA9: // Verification CI
			printf(" Verification CI\n" COLOR_GREEN);
			print_CIs(&tlvbuff[offset]);
			break;

		case 0xAA: // Sign CI
			printf(" Sign CI\n" COLOR_GREEN);
			print_CIs(&tlvbuff[offset]);
			break;

		default:
			printf(COLOR_YELLOW_I " Unknown\n" COLOR_RESET);
			print_tlv(&tlvbuff[offset]);
		}

		// skip TAG byte(s)
		offset++;
		if (tag > 0xFF)
			offset++;

		// skip length
		switch (tlvbuff[offset]) {
		case 0x81:
			len = tlvbuff[offset + 1];
			offset += 2;
			break;
		case 0x82:
			len = 0x100 * tlvbuff[offset + 1] + tlvbuff[offset + 2];
			offset += 3;
			break;
		default:
			len = tlvbuff[offset++];
		}

		offset += len;
	}

	printf("\n");
}

static void print_euicc_info2(uint8_t* tlvbuff)
{
	int buf_sz;
	int offset;

	offset = 2; // skip BF22
	switch (tlvbuff[offset]) {
	case 0x81:
		buf_sz = tlvbuff[offset + 1];
		buf_sz += 4;
		offset += 2;
		break;

	case 0x82:
		buf_sz = tlvbuff[offset + 1] * 0x100 + tlvbuff[offset + 2];
		buf_sz += 5;
		offset += 3;
		break;

	default:
		buf_sz = tlvbuff[offset];
		buf_sz += 3;
		offset++;
	}

	while (offset < buf_sz) {
		int tag;
		int len;

		tag = tlvbuff[offset];
		if (0x1F == (0x1F & tag)) {
			tag <<= 8;
			tag += tlvbuff[offset + 1];
		}

		switch (tag) {
		case 0x81: // SIMA profile version
			printf(" SIMA version    " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x82: // SVN
			printf(" SGP.22 version  " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x83: // eUICC firmware
			printf(" Firmware ver.   " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x84: // extCardResource:
			printf(" Card resources  " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");

			print_card_resources(&tlvbuff[offset]);
			break;

		case 0x85: // UICC  Capability
			printf(" UICC Capability " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");

			print_UICC_Capability(&tlvbuff[offset]);
			break;

		case 0x86: // TS 102 241 Version
			printf(" TS 102.241 ver  " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x87: // Global Platform version
			printf(" GP version      " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x88: // RSP Capability
			printf(" RSP Capability  " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");

			print_RSP_Capability(&tlvbuff[offset]);
			break;

		case 0x99: // UICC Category
			printf(" UICC Category   " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0x04: // Forbidden Profile Policy Rules
			printf(" Forbidden Profile Policy Rules\n" COLOR_GREEN);
			printf("                 ");
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%02X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		case 0xA9: // Verification CI
			printf(" Verification CI\n" COLOR_GREEN);
			print_CIs(&tlvbuff[offset]);
			break;

		case 0xAA: // Sign CI
			printf(" Sign CI\n" COLOR_GREEN);
			print_CIs(&tlvbuff[offset]);
			break;

		case 0x0C: // SAS Acreditation Number
			printf(" SAS Acreditation Number\n");
			printf("                 "  COLOR_YELLOW);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%c", tlvbuff[offset + 2 + j]);
			printf("\n");
			printf("                 " COLOR_GREEN);
			for (int j = 0; j < tlvbuff[offset + 1]; j++)
				printf("%2X", tlvbuff[offset + 2 + j]);
			printf(COLOR_RESET "\n");
			break;

		default:
			printf(COLOR_YELLOW_I " Unknown\n" COLOR_RESET);
			print_tlv(&tlvbuff[offset]);
		}

		// skip TAG byte(s)
		offset++;
		if (tag > 0xFF)
			offset++;

		// skip length
		switch (tlvbuff[offset]) {
		case 0x81:
			len = tlvbuff[offset + 1];
			offset += 2;
			break;
		case 0x82:
			len = 0x100 * tlvbuff[offset + 1] + tlvbuff[offset + 2];
			offset += 3;
			break;
		default:
			len = tlvbuff[offset++];
		}

		offset += len;
	}
}

static void print_profile_list(uint8_t* tlvbuff)
{
	uint8_t iccid[10];
	int buf_sz;
	int offset;
	int profile_idx = 1;

//	print_tlv(tlvbuff);

	offset = 2; // skip BF2D
	switch (tlvbuff[offset]) {
	case 0x81:
		buf_sz = tlvbuff[offset + 1];
		buf_sz += 4;
		offset += 2;
		break;

	case 0x82:
		buf_sz = tlvbuff[offset + 1] * 0x100 + tlvbuff[offset + 2];
		buf_sz += 5;
		offset += 3;
		break;

	default:
		buf_sz = tlvbuff[offset];
		buf_sz += 3;
		offset++;
	}

	offset++; // skip A0
	switch (tlvbuff[offset]) {
	case 0x81:
		offset += 2;
		break;

	case 0x82:
		offset += 3;
		break;

	default:
		offset++;
	}

	while (offset < buf_sz) {
		int lenE3;
		int len;
		int tag;

		printf(COLOR_YELLOW " -- profile %d ---\n" COLOR_RESET, profile_idx++);

		offset++; // skip E3
		switch (tlvbuff[offset]) {
		case 0x81:
			lenE3 = tlvbuff[offset + 1];
			offset += 2;
			break;

		case 0x82:
			lenE3 = tlvbuff[offset + 1] * 0x100 + tlvbuff[offset + 2];
			offset += 3;
			break;

		default:
			lenE3 = tlvbuff[offset];
			offset++;
		}

		for (int i = 0; i < lenE3; ) {
			tag = tlvbuff[offset + i];
			if (0x1F == (0x1F & tag)) {
				tag <<= 8;
				tag += tlvbuff[offset + i + 1];
			}

			switch (tag) {
			case 0x5A: // ICCID
				memcpy(iccid, &tlvbuff[offset + i + 2], 10);
				swapbibbles_bin(iccid, 10);

				printf(" ICCID        : " COLOR_GREEN);
				for (int j = 0; j < 10; j++)
					printf("%02X", iccid[j]);
				printf(COLOR_RESET "\n");
				break;

			case 0x4F: // ISD-P AID
				printf(" ISD-P AID    : " COLOR_GREEN);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%02X", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n");
				break;

			case 0x90: // Nick
				printf(" Nickname    : " COLOR_YELLOW);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_GREEN "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n");
				break;
			case 0x91: // SPN
				printf(" SPN          : " COLOR_YELLOW);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_GREEN "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n");
				break;

			case 0x92: // Profile name
				printf(" Profile name : " COLOR_YELLOW);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_GREEN "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n");
				break;

			case 0x93:
				printf(" Icon type\n");
				print_tlv(&tlvbuff[offset + i]);
				break;

			case 0x94:
				printf(" Icon\n");
				print_tlv(&tlvbuff[offset + i]);
				break;

			case 0x95:
				printf(" Profile class\n");
				print_tlv(&tlvbuff[offset + i]);
				break;

			case 0x9F70:
				printf(" Profile state: " COLOR_WHITE "%s" COLOR_RESET "\n", (tlvbuff[offset + i + 3] == 1 ? "Active" : ""));
				print_tlv(&tlvbuff[offset + i]);
				break;

			default:
				print_tlv(&tlvbuff[offset + i]);
			}

			// skip TAG byte(s)
			i++;
			if (tag > 0xFF)
				i++;

			// skip length
			switch (tlvbuff[offset + i]) {
			case 0x81:
				len = tlvbuff[offset + i];
				i += 2;
				break;
			case 0x82:
				len = 0x100 * tlvbuff[offset + i + 1] + tlvbuff[offset + i + 2];
				i += 3;
				break;
			default:
				len = tlvbuff[offset + i++];
			}

			// skip value
			i += len;
		}

		offset += lenE3;

		printf("\n");
	}
}

/**
  * @brief esim ISD-R function: Profile list
  *
  */
void cmd_euicc_pl(void)
{
	apdu_t apdu;
	uint8_t buffer_pl[16 * 0x100];
	int pl_sz;

	select_ISD_R();

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_GET_PROFILE_INFO >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_GET_PROFILE_INFO & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_send_plain_APDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	pl_sz = 0;
	while (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));

		memcpy(&buffer_pl[pl_sz], apdu.resp, apdu.resp_len - 2);
		pl_sz += apdu.resp_len - 2;
	}

	print_profile_list(buffer_pl);
}

/**
  * @brief esim ISD-R function
  *
  * @param _cmd: command line string
  */
void cmd_euicc_eid(void)
{
	apdu_t apdu;

	select_ISD_R();

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 6;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_GET_EID >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_GET_EID & 0xFF;
	
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = 0x5C;
	apdu.cmd[apdu.cmd_len++] = 0x01;
	apdu.cmd[apdu.cmd_len++] = 0x5A;

	pcsc_send_plain_APDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	printf(" EID: " COLOR_GREEN);
	for (int i = 0; i < 16; i++)
		printf("%02X", apdu.resp[5 + i]);
	printf(COLOR_RESET "\n");
}

void cmd_euicc_info(void)
{
	apdu_t apdu;

	select_ISD_R();

	// 1. Info1
	printf(COLOR_WHITE "\n eUICC Info1" COLOR_RESET "\n");

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_send_plain_APDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	print_euicc_info1(apdu.resp);

	// 2. Info2
	printf(COLOR_WHITE "\n eUICC Info2" COLOR_RESET "\n");

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_send_plain_APDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	print_euicc_info2(apdu.resp);
}
