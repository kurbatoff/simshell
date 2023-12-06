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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "shell.h"
#include "pcscwrap.h"
#include "commands.h"
#include "securechannel.h"
#include "tools.h"
#include "luawrap.h"

char gStartFolder[1024];

static bool execute_file(const char* fname)
{
	FILE* file;
	char fullname[1024 + 8];

	// 1. Simshell script
	sprintf(fullname, "%s%s." SIMSHELL_EXT, gStartFolder, fname);

	if ((file = fopen(fullname, "r")))
	{
		char s[512];

		printf(" Executing script: " COLOR_CYAN "%s." SIMSHELL_EXT "\n\n" COLOR_RESET, fname);

		while (fgets(s, sizeof(s), file) != NULL) {
			SHELL_execute(s);
		}

		fclose(file);
		return true;
	}

	// 2. Lua file
	sprintf(fullname, "%s%s.lua", gStartFolder, fname);

	if ((file = fopen(fullname, "r")))
	{
		fclose(file);

		printf(" Executing Lua file: " COLOR_CYAN "%s.lua\n\n" COLOR_RESET, fname);

		Lua_execute(fullname);

		return true;
	}

	return false;
}

static int find_shell_command(char* _cmd, int* _idx)
{
	simshell_command_t* c;
	size_t len;
	int count = 0;

	for (int i = 0; i < SHELL_COMMANDS_COUNT; i++) {
		c = (simshell_command_t*)&commands_array[i];

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

void shell_prompt(void)
{
	print_reader_name();

	if (CTX.security_status == GPSYSTEM_NONE) {
		printf(" [No secure channel]\n");
	}
	else if (CTX.security_status == GPSYSTEM_INITUPDATE) {
		printf(COLOR_MAGENTA " [Init-update]" "\n" COLOR_RESET);
	}
	else {
		switch (CTX.security_level) {
		case SECURITY_LEVEL_PLAIN:
			printf(COLOR_GREEN " [SCP %02d, Plain]" "\n" COLOR_RESET, CTX.scp_index);
			break;

		case SECURITY_LEVEL_MAC:
			printf(COLOR_GREEN " [SCP %02d, MAC]" "\n" COLOR_RESET, CTX.scp_index);
			break;

		case SECURITY_LEVEL_ENC:
			printf(COLOR_GREEN " [SCP %02d, ENC]" "\n" COLOR_RESET, CTX.scp_index);
			break;

		case SECURITY_LEVEL_CRMAC:
			printf(COLOR_GREEN " [SCP %02d, CR-MAC]" "\n" COLOR_RESET, CTX.scp_index);
			break;
		}
	}

	printf(SIMSHELL_PROMTH "|-> ");
}

/**
* @brief Execute the shell command
*
* @param _command: shell command line
*/
void SHELL_execute(char *_command)
{
	simshell_command_t* c;
	int cmd_idx;
	int found;
	int len;

	// remove trailing SPACE and LF
	len = (int)strlen(_command);
	while (len && (_command[len - 1] == ' ' || _command[len - 1] == '\n' || _command[len - 1] == '\t')) {
		_command[--len] = 0x00;
	}

	// remove leading SPACEs and TABs
	while ((*_command == ' ' || *_command == '\t') && (*_command != 0x00)) {
		_command++;
	}

	if (0 == strlen(_command)) {
		return;
	}

	// Skip comment line
	if ('#' == _command[0]) {
		return;
	}

	found = find_shell_command(_command, &cmd_idx);

	switch (found) {
	case 0:
		if (!execute_file(_command)) {
			printf("\nCommand not found. Type \"help\" for a list of available commands\n");
		}
		return;

	case 1:
		// Execute single finding

		c = (simshell_command_t*)&commands_array[cmd_idx];

		c->pCallBackFunction(_command);
		return;

	default:
		// Print the list of similar commands
		;
	}
}
