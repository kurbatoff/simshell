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

int select_ISD_R()
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
  * @brief esim callback function
  *
  * @param _cmd: command line string
  */
void cmd_esim(char* _cmd)
{
	apdu_t apdu;

	select_ISD_R();

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_STORE_DATA;
	apdu.cmd[apdu.cmd_len++] = 0x91;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0x03;
	apdu.cmd[apdu.cmd_len++] = 0xBF;
	apdu.cmd[apdu.cmd_len++] = 0x2D;
	apdu.cmd[apdu.cmd_len++] = 0x00;

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

//	printf(COLOR_CYAN " esim " COLOR_RESET "implementation..\n");
}
