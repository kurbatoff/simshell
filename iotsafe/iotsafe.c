/**
 *  Copyright (c) 2024, Intergalaxy LLC
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

#include "iotsafe.h"
#include "gp.h"
#include "tools.h"
#include "pcscwrap.h"

static uint8_t IOTSAFE_AID[] = {
//	Instance AID details :
	0xA0, 0x00, 0x00, 0x05, 0x59, // GSMA RID
	0x00, 0x10, // IoT SAFE type 1
	0x49, 0x47, // IG in hex
	0x01, 0x05, // IoT SAFE specification v. 1.05
	0, 0, 0, 0, 0 // Padding
};

static int select_IOT_SATE()
{
	apdu_t apdu;

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = INS_GP_SELECT;
	apdu.cmd[apdu.cmd_len++] = 0x04;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = sizeof(IOTSAFE_AID);

	memcpy(&apdu.cmd[apdu.cmd_len], IOTSAFE_AID, sizeof(IOTSAFE_AID));
	apdu.cmd_len += sizeof(IOTSAFE_AID);

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to select IoT SAFE\n" COLOR_RESET);
		return -1;
	}

	return 0;
}

static void print_IOTSafe_Capability(uint8_t* tlvbuff)
{
	printf(COLOR_YELLOW_I "    TODO: decode...\n" COLOR_RESET);
}

static void print_IOTSafe_resources(uint8_t* tlvbuff)
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


void cmd_IOTSafe_info(void)
{
	apdu_t apdu;

	select_IOT_SATE();

	// 1. Info1
	printf(COLOR_WHITE "\n eUICC Info1" COLOR_RESET "\n");

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
//	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 >> 8;
//	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_1 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	//print_euicc_info1(apdu.resp);

	// 2. Info2
	printf(COLOR_WHITE "\n eUICC Info2" COLOR_RESET "\n");

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 3;
//	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 >> 8;
//	apdu.cmd[apdu.cmd_len++] = DO_GSMA_EUICC_INFO_2 & 0xFF;
	apdu.cmd[apdu.cmd_len++] = 0;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	//print_euicc_info2(apdu.resp);
}
