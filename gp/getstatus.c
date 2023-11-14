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

#include <stdint.h>
#include <stdio.h>

#include "getstatus.h"
#include "tools.h"


#define KNOWN_ELF_COUNT		7

typedef struct known_elf_t
{
	const char* name;
	const int length;
	const uint8_t aid[16];
} known_elf_t;

static known_elf_t elf_array[KNOWN_ELF_COUNT] = {
	{
		"java.lang",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x00, 0x01}
	},
	{
		"javacard.framework",
		7,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x01, 0x01}
	},
	{
		"javacard.security",
		16,
		{0xA0, 0x00, 0x00, 0x00, 0x62, 0x01, 0x02}
	},
	{
		"ISD-R",
		16,
		{0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00}
	},
	{
		"ECASD",
		16,
		{0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x02, 0x00}
	},
	{
		"ARA",
		8,
		{0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C}
	},
	{
		"ARA-M",
		9,
		{0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C, 0x00}
	}
};

void find_elf_name(uint8_t* aid, int len)
{
	known_elf_t* elf;

	for (int i = 0; i < KNOWN_ELF_COUNT; i++)
	{
		elf = &elf_array[i];

		if ((elf->length == len) && (0 == memcmp(elf->aid, aid, len))) {
			printf(COLOR_MAGENTA " %s\n" COLOR_RESET, elf->name);
			break;
		}
	}
}

