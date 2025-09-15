/**
 *  Copyright (c) 2025, Intergalaxy LLC
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

#ifndef __SERIAL_H_1BCAE72D2E8FB2DB
#define __SERIAL_H_1BCAE72D2E8FB2DB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

//int connectEmulator( const char* const comPortStr, int connectTryCount );
int serial_open(const char* comPortStr);
int comPort_send(const char* data, size_t dataLen);
int comPort_read(unsigned char* data, uint32_t dataLen);
size_t serial_receive(unsigned char* data);
int serial_close(void);
void serial_execute(const char* atcmd, char* atresp);

#ifdef __cplusplus
}
#endif

#endif // __SERIAL_H_1BCAE72D2E8FB2DB

