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

 /**
  * @file   globalplatform.h
  * @brief  GLOBAL PLATFORM functions
  */

#ifndef __GLOBALPLATFORM_H_
#define __GLOBALPLATFORM_H_

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

int select_ISD();
int mutual_authentication(uint8_t _sec_level);
int init_update();
int ext_authenticate(uint8_t sec_level);

void cmd_putkeyset(char* _cmd);

void securechannel_wrap(uint8_t* cmd, uint16_t* cmd_len);

#if defined(__cplusplus)
}
#endif

#endif /* __GLOBALPLATFORM_H_ */