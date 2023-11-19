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

#ifndef __GETSTATUS_H__
#define __GETSTATUS_H__

#include <stdint.h>

#define GET_STATUS_MODE_LEGACY		0x00
#define GET_STATUS_MODE_EXPANDED	0x02
#define GET_STATUS_ISD				0x80
#define GET_STATUS_APPLICATIONS		0x40
#define GET_STATUS_PACKAGES			0x10


#if defined(__cplusplus)
extern "C" {
#endif

void print_elf_name22(uint8_t* aid, int len);
int get_status();
void cmd_getdata(char* _cmd);

#if defined(__cplusplus)
}
#endif

#endif /* __GETSTATUS_H__ */

