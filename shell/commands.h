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

#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#define SHELL_COMMANDS_COUNT		19

typedef void (*shellcommand_f)(char* _cmd);

typedef struct simshell_command_t
{
	const char* pcName;
	const char* pcHelpString;
	const char* pcShortHelp;
	const shellcommand_f pCallBackFunction;
} simshell_command_t;

extern simshell_command_t commands_array[SHELL_COMMANDS_COUNT];

#endif /* __COMMANDS_H_ */
