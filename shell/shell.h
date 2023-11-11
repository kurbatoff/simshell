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

#ifdef __APPLE__
	typedef uint32_t DWORD;
#else
	#include <windows.h>
#endif

#define SHELL_NAME		"simsh"
#define shell_prompt	printf(SHELL_NAME "|-> ")

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Macro to return a pointer to the command
 */
//#define SHELL_COMMAND(command) &g_shellCommand##command

/**
 * @brief Execute the shell command
 * @param command: command line
 */
void SHELL_execute(char *command);

#if defined(__cplusplus)
}
#endif

#endif /* __SHELL_H_ */

