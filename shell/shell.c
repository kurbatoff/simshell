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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "shell.h"
#include "commands.h"

int find_shell_command(char* _cmd, int* _idx)
{
	simshell_command_t* c;
	size_t len;
	int count = 0;

	for (int i = 0; i < SHELL_COMMAND_COUNT; i++) {
		c = (simshell_command_t*)&command_array[i];

		len = 0;
		while (1) {
			char c;

			c = _cmd[len];
			if ((c == ' ') || (c == '\t') || (c == 0x0A) || (c == 0x0D) || (c == 0))
				break;

			len++;
		}

		if (memcmp(_cmd, c->pcName, len) == 0) {
			*_idx = i;
			count++;
		}
	}

	return count;
}

/**
* @brief Execute the shell command
*
* @param command: command line
*/
void SHELL_execute(char *command)
{
	simshell_command_t* c;
	int cmd_idx;
	int found;

	found = find_shell_command(command, &cmd_idx);

	switch (found) {
	case 0:
		printf("\nCommand not found. Type \"help\" for a list of available commands\n");
		return;

	case 1:
		// Execute single found

		c = (simshell_command_t*)&command_array[cmd_idx];

		c->pCallBackFunction(command);
		return;

	default:
		// Print the list of similar commands
		;
	}
}
