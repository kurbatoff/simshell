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
 * 1.       JVM CAP, IJC: /cap-info, upload
 * 1.a      cap2ijc converter												07 JAN 2024
 * 2.       JCShell script support
 * 2.a      Delete, install
 * 3.   [+] Lua support														19 Nov 2023
 * 4.   [+] Lua: execute JCShell command									12 Dec 2023
 * 5.   [+] Run PCOM scripts												10 Jan 2024
 * 6.   [+] get-data 00E0 etc												18 Nov 2023
 * 7.       SCP11 + ECC tests
 * 8.       SIM scan
 * 9.       Crypto support: shell & Lua
 * 10.      5G support / tests
 * 11.      Lua applications:
 * 11.a     SCP81 in offline mode			.lua
 * 11.b.    LPA in offline mode				.lua
 * 11.c     Milenage						.lua
 * 11.d     TUAK authentication				.lua
 * 11.e     Crypto pack						.lua
 * 12.      ATR parsing
 * 13.      JRCP client
 *
 * 21.      Shell functions: remember list of commands, Mac support, etc
 * 
 * 31.      Manual
 *
 * 41.      Articles
 *
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

#ifdef _WIN32
	// Workaround: CLear screen is required for proper Lua print() formatting
	system("cls");
	printf("\033[00;00m");
#endif

	printf("------------------------------------------------------------------------\n");
	printf(COLOR_WHITE " SIM, Global Platform and JVM" COLOR_RESET " shell [Version %s]\n", version); 
	printf(" (c) 2023, 2024 Intergalaxy. All rights reserved.\n");
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

	printf(" Start folder: %s\n", gStartFolder);

	init_keys();

	for (;;)
	{
		shell_prompt();

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
