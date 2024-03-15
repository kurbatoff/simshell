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

#ifndef __EUICC_H__
#define __EUICC_H__

#include <stdint.h>

typedef enum _esim_command_t
{
	ESIM_RESET = 0,
	ESIM_EID,
	ESIM_INFO,
	ESIM_PL,
	ESIM_ENABLE,
	ESIM_DISABLE,
	ESIM_DELETE,
	ESIM_LOAD,
	ESIM_ATTACH
} esim_command_t;

#if defined(__cplusplus)
extern "C" {
#endif

void cmd_euicc_pl(void);
//void cmd_euicc_enable(char* _cmd);

#if defined(__cplusplus)
}
#endif

#endif /* __EUICC_H__ */

