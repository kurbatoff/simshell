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

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>

extern int MSG_spi;
extern int MSG_kickid;
extern int MSG_tar;
extern unsigned char MSG_apdu[];
extern int MSG_apdu_len;

extern char serialPort[32];

extern char QR_MID[128];
extern char QR_Address[128];
extern char QR_Port[8];

extern char eIM_Address[128];
extern char eIM_Port[8];

//extern uint8_t LocI_IMEI[];
extern uint8_t LocI_NMR_UMTS[];
extern uint8_t LocI_NMR_LTE[];
extern uint8_t LocI_AT;

// ------------------------------------------------------------------------------------

unsigned char hex_2_char(unsigned char* s);
void str_trunc(char* buf);

int read_config(const char* exename);

#endif // __CONFIG_H_
