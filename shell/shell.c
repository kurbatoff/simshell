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
#include "libApduEngine.h"


typedef enum _config_bits_t
{
	FILE_NOT_EXISTS = 0,
	SCRIPT_TYPE_SIMSHELL,
	SCRIPT_TYPE_JCSHELL,
	SCRIPT_TYPE_LUA,
	SCRIPT_TYPE_PCOM,
} scrypt_type_t;

char gStartFolder[1024];



static bool execute_file(const char* fname)
{
	FILE* file;
	char fullname[1024 + 8];
	scrypt_type_t stype = FILE_NOT_EXISTS;

	// 1, If the file exists 'As is'
	if ((file = fopen(fname, "r")))
	{
		size_t flen;
		size_t elen;

		fclose(file);

		strcpy(fullname, fname);

		flen = strlen(fname);

		elen = 5; // '.pcom'
		if (memcmp((const void* )".pcom", &fname[flen-elen], elen) == 0) {
			stype = SCRIPT_TYPE_PCOM;
		}
		
		if (FILE_NOT_EXISTS == stype) {
			elen = 4; // '.lua'
			if (memcmp((const void*)".lua", &fname[flen - elen], elen) == 0) {
				stype = SCRIPT_TYPE_PCOM;
			}
		}

		if (FILE_NOT_EXISTS == stype) {
			elen = SIMSHELL_EXT2_LEN;
			if (memcmp((const void*)SIMSHELL_EXT2, &fname[flen - elen], elen) == 0) {
				stype = SCRIPT_TYPE_PCOM;
			}
		}
	}

	// 2. Simshell script
	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s%s" SIMSHELL_EXT2, gStartFolder, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_SIMSHELL;
		}
	}

	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s" SIMSHELL_EXT2, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_SIMSHELL;
		}
	}

	// 3. Lua file
	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s%s.lua", gStartFolder, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_LUA;
		}
	}

	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s.lua", fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_LUA;
		}
	}

	// 4. PCOM file
	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s%s.pcom", gStartFolder, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_PCOM;
		}
	}

	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s/pcom/%s", gStartFolder, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_PCOM;
		}
	}

	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s\\pcom\\%s.pcom", gStartFolder, fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_PCOM;
		}
	}

	if (FILE_NOT_EXISTS == stype) {
		sprintf(fullname, "%s.pcom", fname);

		if ((file = fopen(fullname, "r")))
		{
			fclose(file);
			stype = SCRIPT_TYPE_PCOM;
		}
	}

	switch (stype) {
	case FILE_NOT_EXISTS:
		return false;

	case SCRIPT_TYPE_SIMSHELL:
	case SCRIPT_TYPE_JCSHELL:
	{
		char s[1024];

		file = fopen(fullname, "r");

		printf(" Executing script: " COLOR_CYAN "%s\n\n" COLOR_RESET, fullname);

		while (fgets(s, sizeof(s), file) != NULL) {
			SHELL_execute(s);
		}

		fclose(file);
	}
	break;
	
	case SCRIPT_TYPE_LUA:
		printf(" Executing Lua file: " COLOR_CYAN "%s\n\n" COLOR_RESET, fullname);
		Lua_execute(fullname);
		break;

	case SCRIPT_TYPE_PCOM:
		printf(" Executing PCOM file: " COLOR_CYAN "%s\n\n" COLOR_RESET, fullname);
		execute_PCOM(fullname, true);
		break;
	}

	return true;
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
