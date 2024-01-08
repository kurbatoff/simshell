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

#ifndef __LIBAPDUENGINE_H_
#define __LIBAPDUENGINE_H_

#include <stdint.h>

#define BUFFER_MAX_LEN 2048

struct errorCount_t {
	int status;
	int syntax;
	int data;
	int comm;
};

struct buffer_t {
    int len;
    uint8_t data[BUFFER_MAX_LEN];
};

void execute_PCOM(const char* _filename);
int execute_OneLine(const char* _fileline);

#endif // __LIBAPDUENGINE_H_
