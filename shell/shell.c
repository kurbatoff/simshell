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


/**
* @brief Execute the shell command
*
* @param command: command line
*/
void SHELL_execute(char *command)
{
	simshell_command_t* c;
	size_t len;

	for (int i = 0; i < SHELL_COMMAND_COUNT; i++) {
		c = (simshell_command_t*)&command_array[i];
		len = strlen(c->pcName) - 1; // exclude '\n'

		if (memcmp(command, c->pcName, len) == 0) {

			c->pCallBackFunction(command);

			return;
		}
	}
	/*
	size_t len = strlen(command);
	char* p = command;
	char* pe = p + len;
    uint8_t cc = 0;
    //shell_command_t *c = &helpcmd;

	simshell_command_t* cmd = &shellcommands[0];

	cmd->pFuncCallBack(argvc, argv);


    if (p && pe != p)
    {
        int argvc = 0;

        if (*pe != 0)
            *pe = 0;
        
		if (*(pe + 1) != 0)
            *(pe + 1) = 0;
		
		for (; p < pe; ++p)
		{
			if (*p == ' ' || *p == '\t')
			{
				*p = 0;
				++argvc;
			}
		}

        ++argvc;    
        char **argv = new char*[argvc];
        int i = 0;
        p = command;
        while (*p && i < argvc)
        {
			argv[i] = p;
            p += (strlen(p) + 1);
            ++i;
        }

        while (c)
        {
			if (strlen(argv[0]) == strlen(c->pcCommand))
			{
				if (_stricmp(argv[0], c->pcCommand) == 0)
				{
					if (argvc < (c->cExpectedNumberOfParameters + 1))
						printf(c->pcHelpString);
					else
					{
						int exec_time = 0;

						if (!c->pFuncCallBack)
							return false;

						c->pFuncCallBack(argvc, argv);
					}
					delete[] argv;
					return true;
				}
			}
			c = c->link;
        }

		delete[] argv;
    }

*/
	printf("\nCommand not found. Type \"help\" for a list of available commands\n");

	return;
}
