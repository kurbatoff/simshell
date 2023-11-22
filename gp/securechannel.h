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

#ifndef __SECURECHANNEL_H__
#define __SECURECHANNEL_H__

#include <stdint.h>

#define SECURITY_LEVEL_PLAIN		0x00
#define SECURITY_LEVEL_MAC			0x01
#define SECURITY_LEVEL_ENC			0x03
#define SECURITY_LEVEL_CRMAC		0x34 // SCP11

#define GPSYSTEM_NONE				0x00
#define GPSYSTEM_INITUPDATE			0x01 // Internal use
#define GPSYSTEM_AUTHENTICATED		0x80

#define LENGTH_OF_ICV				16
#define LENGTH_OF_MAC				8
#define DEK_BUFF_MAX_LENGTH			32
#define LENGTH_OF_SESSION_KEY		16


/**
 * Structure S_LOGICAL_CHANNEL_CONTEXT defines a [Logical channel] Context object
 *
 */
typedef struct S_SECURE_CHANNEL_CONTEXT {
//	uint8_t lc_idx; // Logical channel index

	uint8_t scp_index;

	uint8_t security_level;
	uint8_t security_status; // None / Authenticated

} Struct_Context;

extern Struct_Context CTX;

extern uint8_t buf_session_ENC[];
extern uint8_t buf_session_MAC[];
extern uint8_t buf_session_RMAC[];
extern uint8_t buf_session_DEK[];
extern uint8_t buf_ICV[];

void securechannel_reset_session(void);
//void gp_securechannel_initialize_update(void);
void gp_securechannel_external_authenticate(void);
uint8_t securechannel_decrypt(void);
uint8_t securechannel_verify_cmac(void);

#endif /* __SECURECHANNEL_H__ */
