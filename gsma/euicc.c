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
				printf(" Nickname    : " COLOR_GREEN);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf("\n");
				break;
			case 0x91: // SPN
				printf(" SPN          : " COLOR_GREEN);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf("\n");
				break;

			case 0x92: // Profile name
				printf(" Profile name : " COLOR_GREEN);
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%c", tlvbuff[offset + i + 2 + j]);
				printf(COLOR_RESET "\n                ");
				for (int j = 0; j < tlvbuff[offset + i + 1]; j++)
					printf("%2X", tlvbuff[offset + i + 2 + j]);
				printf("\n");
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

static int select_ISD_R()
{
	apdu_t apdu;

	apdu.cmd_len = 0;
	apdu.cmd[ apdu.cmd_len++ ] = 0x00;
	apdu.cmd[ apdu.cmd_len++ ] = INS_GP_SELECT;
	apdu.cmd[ apdu.cmd_len++ ] = 0x04;
	apdu.cmd[ apdu.cmd_len++ ] = 0x00;
	apdu.cmd[ apdu.cmd_len++ ] = sizeof(ISDR);

	memcpy(&apdu.cmd[ apdu.cmd_len ], ISDR, sizeof(ISDR));
	apdu.cmd_len += sizeof(ISDR);

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[ apdu.resp_len - 2 ]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[ apdu.resp_len - 2 ]) {
		printf(COLOR_RED " Failed to select ISD-R\n" COLOR_RESET);
		return -1;
	}

	return 0;
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

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

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

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

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

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	printf(" eUICC Infor1: " COLOR_GREEN);
	for (int i = 0; i < 16; i++)
		printf("%02X", apdu.resp[5 + i]);
	printf(COLOR_RESET "\n");



	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 >> 8;
	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

}
