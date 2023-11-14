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

#ifndef __SW_H__
#define __SW_H__

#include <stdint.h>

// Status word SW constants
#define SW_OK						0x9000
#define SW_GP_AUTHENTICATION_FAILED			0x6300
#define SW_NO_SPECIFIC_DIAGNOSTIC			0x6400
#define SW_WRONG_LENGTH					0x6700
#define SW_LOGICAL_CHANNEL_NOT_SUPPORTED		0x6881
#define SW_SECURITY_STATUS_NOT_SATISFIED		0x6982
#define SW_CONDITIONS_OF_USE_NOT_SATISFIED		0x6985
#define SW_WRONG_DATA					0x6A80
#define SW_FILE_NOT_FOUND				0x6A82
#define SW_INCORRECT_P1P2				0x6A86
#define SW_REFERENCE_DATA_NOT_FOUND			0x6A88
#define SW_INS_NOT_SUPPORTED				0x6D00
#define SW_CLA_NOT_SUPPORTED				0x6E00
//#define SW_AVAILABLE_61				0x6100

#if defined(__cplusplus)
extern "C" {
#endif

void interpret_sw(uint16_t sw);

#if defined(__cplusplus)
}
#endif

#endif /* __SW_H__ */

