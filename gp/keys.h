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

#ifndef __KEYS_H__
#define __KEYS_H__

#include <stdint.h>

typedef struct sym_keyset_t
{
	uint8_t status; // 1/0 Initialized / no
	uint8_t kvn;
	uint8_t type; // KEY_TYPE_DES or KEY_TYPE_AES
	uint8_t keylen; // 128 or 192 or 256
	uint8_t enc[32];
	uint8_t mac[32];
	uint8_t dek[32];
} sym_keyset_t;

#if defined(__cplusplus)
extern "C" {
#endif

void init_keys(void);
void cmd_setkey(char* _cmd);
void cmd_putkeyset(char* _cmd);
sym_keyset_t* find_keyset(int kvn);

#if defined(__cplusplus)
}
#endif

#endif /* __KEYS_H__ */

