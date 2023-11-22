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

#ifndef __SHELL_H_
#define __SHELL_H_

#include <stdint.h>

extern char gStartFolder[1024];

#ifdef __APPLE__
	typedef uint32_t DWORD;
#else
	#include <windows.h>
#endif

#define SIMSHELL_PROMTH			"simsh"
#define SIMSHELL_EXT			"simsh"

#if defined(__cplusplus)
extern "C" {
#endif

void shell_prompt(void);
void SHELL_execute(char *gCMDbuff);

#if defined(__cplusplus)
}
#endif

#endif /* __SHELL_H_ */


