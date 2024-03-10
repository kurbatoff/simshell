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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "gp.h"
#include "securechannel.h"
#include "scp02.h"
#include "scp03.h"

Struct_Context CTX;

uint8_t buf_session_ENC[LENGTH_OF_SESSION_KEY];
uint8_t buf_session_MAC[LENGTH_OF_SESSION_KEY];
uint8_t buf_session_RMAC[LENGTH_OF_SESSION_KEY];
uint8_t buf_session_DEK[DEK_BUFF_MAX_LENGTH];
uint8_t buf_ICV[LENGTH_OF_ICV];


/**
 *
 *
 */
void securechannel_reset_session(void)
{
	CTX.security_level = SECURITY_LEVEL_PLAIN;
	CTX.security_status = GPSYSTEM_NONE;
	CTX.scp_index = 0x00;

	memset(buf_session_ENC, 0x00, LENGTH_OF_SESSION_KEY);
	memset(buf_session_MAC, 0x00, LENGTH_OF_SESSION_KEY);
	memset(buf_session_DEK, 0x00, DEK_BUFF_MAX_LENGTH);
	memset(buf_session_RMAC, 0x00, LENGTH_OF_SESSION_KEY);

	memset(buf_ICV, 0x00, LENGTH_OF_ICV);
}

/**
 *
 *
 */
void gp_securechannel_external_authenticate(void)
{
	switch (CTX.scp_index) {
	case SECURE_CHANNEL_PROTOCOL_02:
		scp02_external_authenticate();
		break;

	case SECURE_CHANNEL_PROTOCOL_03:
//		scp03_external_authenticate();
		break;

	default:
		securechannel_reset_session();

//		set_sw(SW_CONDITIONS_OF_USE_NOT_SATISFIED);
		return;
	}
}

/**
 *
 *
 */
uint8_t securechannel_decrypt(void)
{
	switch (CTX.scp_index) {
	case SECURE_CHANNEL_PROTOCOL_02:
		return scp02_decrypt_cdata();

	//case SECURE_CHANNEL_PROTOCOL_11:
	case SECURE_CHANNEL_PROTOCOL_03:
//		return scp03_decrypt_cdata();

	default:
		securechannel_reset_session();

//		set_sw(SW_CONDITIONS_OF_USE_NOT_SATISFIED);
		return false;
	}
	
//	return SECURE_FALSE_8;
}

/**
 *
 *
 */
uint8_t securechannel_verify_cmac(void)
{
	switch (CTX.scp_index) {
//	case SECURE_CHANNEL_PROTOCOL_02:
//		return scp02_verify_cmac();

	//case SECURE_CHANNEL_PROTOCOL_11:
	case SECURE_CHANNEL_PROTOCOL_03:
//		return scp03_verify_cmac();

	default:
		securechannel_reset_session();

		//set_sw(SW_CONDITIONS_OF_USE_NOT_SATISFIED);
		return false;
	}
	
}



