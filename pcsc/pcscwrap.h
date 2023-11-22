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

#ifndef __PCSCWRAP_H_
#define __PCSCWRAP_H_

#include <stdbool.h>
#include <stdint.h>

#define PCSC_APDU_C_BUFF_LEN            (256 + 5)
#define PCSC_APDU_R_BUFF_LEN            (256 + 2)

typedef struct apdu_t
{
	uint8_t cmd[ PCSC_APDU_C_BUFF_LEN ];
	uint16_t cmd_len;
	uint8_t resp[ PCSC_APDU_R_BUFF_LEN ];
	uint16_t resp_len;
} apdu_t;



#define PCSC_ERROR_UNKNOWN              0x6F00
#define PCSC_SUCCESS                    0x9000

typedef uint32_t pcsc_error_t;

#if defined(__cplusplus)
extern "C" {
#endif

extern uint8_t LChannel_ID;

uint16_t get_response(uint8_t response_len, uint8_t* response, uint16_t response_size);

void print_reader_name(void);

/**
 * \brief                     Send an APDU command
 *
 * \param _cmd                APDU command to send
 * \param _gCMDlen            Length of APDU command to send
 * \param response            Buffer used to save the response; must be allocated by the user
 * \param response_size       Size of the response buffer allocated by the user
 * \param gCMDlen     Length of the response
 * 
 * \return                    \c PCSC_SUCCESS on success.
 * \return                    An error code on failure.
 *
 */
pcsc_error_t pcsc_sendAPDU(uint8_t* _cmd, uint16_t _cmd_len,
	uint8_t* _response_buffer, uint16_t _response_buffer_sz, uint16_t* _response_length);

pcsc_error_t pcsc_listreaders(void);
pcsc_error_t connect_reader(void);
pcsc_error_t disconnect_reader(void);
pcsc_error_t pcsc_reset(void);

#if defined(__cplusplus)
}
#endif

#endif /* __PCSCWRAP_H_ */
