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

/**
 * TODO features
 * 
 * 1.      JVM CAP: /cap-info, upload, delete, install
 * 2.      JCShell script support
 * 3.  [+] Lua support: 19 Nov 2023
 * 4.      SCP81 in offline mode		.lua
 * 5.      LPA in offline mode			.lua
 * 6.      Run PCOM scripts
 * 7.  [+] get-data 00E0 etc
 * 8.      SCP11 + ECC tests
 * 9.      Milenage
 * 10.     TUAK authentication
 * 
 * 33.     Manual
 */

#define _CRT_SECURE_NO_WARNINGS 

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "shell.h"
#include "commands.h"
#include "keys.h"
#include "version.h"
#include "tools.h"

#if defined(_WIN32) || defined(WIN32)
	#define pathSeparator '\\'
#else
	#define pathSeparator '/'
#endif


/**
* @brief Main function
*
* @param argc: the number of command line arguments including the application file name
* @param argv: command line arguments including the application file name
* @retval the resulting code returned by the application
*/
int //__cdecl 
main(int argc, char* argv[])
{
//	FILE* fp_eeprom;

	char commandline[4096];
	char* str;

	size_t len;
	clock_t start;
	clock_t finish;
	//double interval;

	// Workaround: CLear screen is required for proper Lua print() formatting
	system("cls");
	printf("\033[00;00m");

	printf("------------------------------------------------------------------------\n");
	printf(COLOR_WHITE " SIM, Global Platform and JVM" COLOR_RESET " shell [Version %s]\n", version); 
	printf(" (c) 2023 Intergalaxy. All rights reserved.\n");
	printf("------------------------------------------------------------------------\n\n");


	strcpy(gStartFolder, argv[0]);

	len = strlen(gStartFolder);
	while (gStartFolder[len] != pathSeparator) {
		gStartFolder[len] = 0;

		len--;
		if (len == 0) {
			gStartFolder[len] = 0;
			break;
		}
	}

	printf(" Start folder: %s\n\n", gStartFolder);

	init_keys();

	for (;;)
	{
		shell_prompt;

		fgets(commandline, sizeof(commandline) - 1, stdin);
		str = commandline;

		if (strcmp(str, "cls\n") == 0) {
			system("cls");
			printf("\033[00;00m");
			continue;
		}

		if (strcmp(str, "?\n") == 0) {
			simshell_command_t* c;

			c = (simshell_command_t* ) & commands_array[0];
			c->pCallBackFunction(str);

			continue;
		}

		start = clock();

		SHELL_execute(str);

		finish = clock() - start;
//		interval = finish / (double)CLOCKS_PER_SEC;
//		printf(" [Processing time: %.03f s]\n", interval);
	}

	return 0;
}
