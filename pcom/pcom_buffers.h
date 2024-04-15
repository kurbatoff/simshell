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

#ifndef __PCOM_BUFFERS_H__
#define __PCOM_BUFFERS_H__

#include <stdint.h>

#define BUFFER_ZERO 'g'
#define BUFFER_MAX_COUNT ('w' - BUFFER_ZERO + 1)

#define BUFF_G      ('g' - BUFFER_ZERO)
#define BUFF_H      ('h' - BUFFER_ZERO)
#define BUFF_I      ('i' - BUFFER_ZERO)
#define BUFF_J      ('j' - BUFFER_ZERO)
#define BUFF_K      ('k' - BUFFER_ZERO)
#define BUFF_L      ('l' - BUFFER_ZERO)
#define BUFF_M      ('m' - BUFFER_ZERO)
#define BUFF_N      ('n' - BUFFER_ZERO)
#define BUFF_O      ('o' - BUFFER_ZERO)
#define BUFF_P      ('p' - BUFFER_ZERO)
#define BUFF_Q      ('q' - BUFFER_ZERO)
#define BUFF_R      ('r' - BUFFER_ZERO)
#define BUFF_S      ('s' - BUFFER_ZERO)
#define BUFF_T      ('t' - BUFFER_ZERO)
#define BUFF_U      ('u' - BUFFER_ZERO)
#define BUFF_V      ('v' - BUFFER_ZERO)
#define BUFF_W      ('w' - BUFFER_ZERO)

#define BUFFER_MAX_LEN 2048

struct buffer_t {
    int len;
    uint8_t data[BUFFER_MAX_LEN];
};

extern struct buffer_t BUFFs[BUFFER_MAX_COUNT];

#if defined(__cplusplus)
extern "C" {
#endif

void clear_buffers(void);
void print_Buffers(void);
void print_Buffer(char b);
void set_Buffer(uint8_t idx, uint8_t* value, int len);

#if defined(__cplusplus)
}
#endif

#endif /* __PCOM_BUFFERS_H__ */
