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

#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>

#include "tools.h"
#include "pcom_buffers.h"

struct buffer_t BUFFs[BUFFER_MAX_COUNT];

void clear_buffers(void)
{
    for (int i = 0; i < BUFFER_MAX_COUNT; i++)
        BUFFs[i].len = 0;
}

void print_Buffers(void)
{
    char B[4] = { ' ', 'G', ':', 0 };

    printf(COLOR_YELLOW " Buffers:\n" COLOR_RESET);
    for (int i = 0; i < BUFFER_MAX_COUNT; i++) {
        if (BUFFs[i].len)
            dump_hexascii_buffer(B, BUFFs[i].data, BUFFs[i].len);
        B[1]++;
    }
    printf("\n");
}

void print_Buffer(char b)
{
    char s[16];
    uint8_t idx;

    idx = b - BUFFER_ZERO;
    b = toupper(b);

    sprintf(s, " Buffer %c:", b);


    dump_hexascii_buffer(s, BUFFs[idx].data, BUFFs[idx].len);
    printf("\n");
}

void set_Buffer(uint8_t idx, uint8_t* value, int len)
{
    BUFFs[idx].len = len;
    memcpy(BUFFs[idx].data, value, len);
}


