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

#include "commands.h"

#include "tools.h"
#include "version.h"
#include "pcscwrap.h"
#include "globalplatform.h"
#include "scp11.h"

static void help(char* _cmd);
static void cmd_version(char* _cmd);

static void cmd_S_listreaders(char* _cmd);
static void cmd_S_term(char* _cmd);
static void cmd_S_close(char* _cmd);

static void cmd_S_atr(char* _cmd);
static void cmd_S_card(char* _cmd);

static void cmd_S_select(char* _cmd);
static void cmd_auth(char* _cmd);
static void cmd_initupdate(char* _cmd);
static void cmd_extauthenticate(char* _cmd);
static void cmd_ls(char* _cmd);

//static void cmd_S_send(char* _cmd);
//static void cmd_S_capinfo(char* _cmd);
//static void cmd_S_echo(char* _cmd);
//static void cmd_S_sleep(char* _cmd);
//static void cmd_S_mode(char* _cmd);
//static void cmd_S_execute(char* _cmd);
//static void cmd_S_error(char* _cmd);
//static void cmd_getsdcert(char* _cmd);
//static void cmd_setkey(char* _cmd);
//static void cmd_putkeyset(char* _cmd);
//static void cmd_getdata(char* _cmd);
//static void cmd_upload(char* _cmd);
//static void cmd_install(char* _cmd);
//static void cmd_S_setvar(char* _cmd);
//static void cmd_milenage(char* _cmd);
//static void cmd_tuak(char* _cmd);


// ---------------------------------------------------------------------------------------------------------------------
// Shell commands array
// ---------------------------------------------------------------------------------------------------------------------
simshell_command_t command_array[SHELL_COMMAND_COUNT] = {
	{
		"help",
		"\n\"help\": List available commands\n",
		" help          [-] Display help information\n",
		help
	},
	{
		"version",
		"\n\"version\": Print SIM shell version\n",
		" version       [-] Display current version of the JVM shell\n",
		cmd_version
	},
	{	
		"listreaders",
		"\n\"list-readers\"\n",
		" /list-readers [-] List available card readers\n",
		cmd_S_listreaders
	},
	{	
		"_term",
		"\n\"/term arg1\":\n Usage:\n    arg1: terminal type\n",
		" /term             List card readers and connect\n",
		cmd_S_term
	},
	{	
		"close",
		"\n\"/close\"\n\n",
		" /close            Close current terminal (card reader context)\n",
		cmd_S_close
	},
	{	
		"_card",
		"\n\"/card -a AID\":\n Usage:\n    AID: ISD (Card Manager) AID\n",
		" /card             Power on the card and select ISD \n",
		cmd_S_card
	},
	{	
		"_atr",
		"\n\"led arg1 arg2\":\n Usage:\n    arg1: 1|2|3|4[-]         "
		"   /atr\n    arg2: on|off                Led status\n",
		" /atr              Power on the card and print ATR\n",
		cmd_S_atr
	},
	{
		"_select",
		"\n\"/select AID\":\n Usage:\n    AID: AID of the applet to select\n",
		" /select           Select an applet idetified by AID\n",
		cmd_S_select
	},
	{
		"initupdate",
		"\n\"init-update\""
		"   init-update sec_level\n",
		" init-update       Perform initialize update\n",
		cmd_initupdate
	},
	{
		"extauthenticate",
		"\n\"ext-auth PLAIN|MAC|ENC"
		"   ext-auth sec_level\n",
		" ext-auth          Perform external authentication\n",
		cmd_extauthenticate
	},
	{
		"auth",
		"\n\"auth PLAIN|MAC|ENC"
		"   auth sec_level\n",
		" auth              Perform mutual authentication\n",
		cmd_auth
	},
	{
		"ls",
		"\n\"ls\"\n",
		" ls                Retrieve and print GP registry (GET STATUS)\n",
		cmd_ls
	}
};

/**
 * @brief Shell command HELP callback function
 *
 * @param argc: amount of help command aguments including help command itself as argv[0]
 * @param argv: help command aguments including help command itself as argv[0]
 */
static void help(char* _cmd)//int32_t argc, char** argv)
{
//	shell_command_t* p = &shellcommands[0];

	//char* sortedlist[SHELL_COMMAND_COUNT];


/*
	if (argc - 1)
	{
		while (p)
		{
			if (strlen(argv[1]) == strlen(p->pcCommand))
			{
				if (strcmp(argv[1], p->pcCommand) == 0)
				{
					printf(p->pcHelpString);
					return shellStatus_Success;
				}
			}
			p = p->link;
		}
	}
*/

	// Collect command names
//	p = &helpcmd;
//	int cnt = 0;

/*
	// Fill command names
	shell_command_t** cmds = new shell_command_t * [cnt];

	p = &helpcmd;
	int i = 0;
	while (p)
	{
		cmds[i++] = p;
		p = p->link;
	}

	// Sort command names alphabetically
	for (i = 0; i < (cnt - 1); ++i)
	{
		for (int j = i; j < cnt; ++j)
		{
			if (strcmp(cmds[i]->pcCommand, cmds[j]->pcCommand) > 0)
			{
				shell_command_t* t = cmds[i];
				cmds[i] = cmds[j];
				cmds[j] = t;
			}
		}
	}


	printf("\n");
	delete[] cmds;
*/

// Print sorted command names
	printf("\n");
	for (int i = 0; i < SHELL_COMMAND_COUNT; i++)
		printf(command_array[i].pcShortHelp);
	printf("\n");
}

/**
 * @brief /term callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_term(char* _cmd)
{
	pcsc_listreaders();
}

/*
	_send,
	"\n\"/send\":\n Usage:\n    APDU string\n",
	" /send             Send APDU-C to the active reader\n",
	cmd_S_send
                     
	_capinfo,
    "\n\"/cap-info file.CAP\":\n Usage:\n    file: the .CAP file name\n",
	" /cap-info     [-] Print .CAP file information\n",
	cmd_S_capinfo

	_sleep,
    "\n\"/sleep arg1\":\n Usage:\n    arg1: seconds/milliseconds\n",
	" /sleep        [-] Suspend execution for x seconds (milliseconds)\n",
	cmd_S_sleep

	_echo,
	"\n\"/echo arg1\":\n Usage:\n    arg1: on|off|?\n",
	" /echo         [-] Echo message\n",
	cmd_S_echo

	_mode,
	"\n\"/mode arg1\":\n Usage:\n    arg1: on|off|?\n",
	" /mode         [-] Control SIM shell behaviour\n",
	cmd_S_mode

	_execute,
	"\n\"/execute\"\n",
	" /execute      [-] Execute host OS command\n",
	cmd_S_execute

	_error,
	"\n\"/error\"\n",
	" /error        [-] Stop script execution and generate error\n",
	cmd_S_error

	getsdcert,
	"\n\"get-sd-certificate arg1 arg2 arg3\":\n Usage:\n    arg1: 1|2|3|4...         \n",
	" get-sd-certif..   Get SD certificate i.e. start SCP11 authentication\n",
	cmd_getsdcert

	setkey,
	"\n\"set-key\"\n",
	" set-key       [-] Set context (Security Domain) keys\n",
	cmd_setkey

	putkeyset,
	"\n\"put-keyset\"\n",
	" put-keyset        Store context keyset into Security Domain\n",
	cmd_putkeyset

	getdata,
	"\n\"get-data\"\n",
	" get-data          GET DATA identified by tag\n",
	cmd_getdata

	upload,
	"\n\"upload\"\n",
	" upload            Load .CAP file\n",
	cmd_upload

	quit,
	"\n\"quit\"\n\n",
	" quit          [-] Exit SIM shell\n",
	NULL

	install,
	COLOR_YELLOW "install" COLOR_RESET " [-i AID] -q par1 ELF_AID EM_AID\n",
	" install           Install applet instance out of .CAP file\n",
	cmd_install

	_setvar,
	"\n\"/set-var\"\n\n",
	" /set-var      [-] Set variable name and value\n",
	cmd_S_setvar
								
	milenage,
	"\n\"milenage\"\n\n",
	" milenage          Execute MILENAGE authentication procedure\n",
	cmd_milenage

	tuak,
	"\n\"tuak\"\n\n",
	" tuak              Execute TUAK authentication procedure\n",
	cmd_tuak
*/

/**
 * @brief init-update callback function
 *
 * @param _cmd: command line string
 */
static void cmd_initupdate(char* _cmd)
{
	printf(COLOR_CYAN " init-update " COLOR_RESET "under implementation..\n");
}

/**
 * @brief ext-authenticte callback function
 *
 * @param _cmd: command line string
 */
static void cmd_extauthenticate(char* _cmd)
{
	printf(COLOR_CYAN " ext-authenticte " COLOR_RESET "under implementation..\n");
}
/**
 * @brief get-sd-certificate callback function
 *
 * @param _cmd: command line string
 */
static void cmd_getsdcert(char* _cmd)
{
	cmd_scp11_perform_security_operation();

	printf(COLOR_CYAN " get-sd-certificate " COLOR_RESET "still under implementation..\n");
}

/**
 * @brief auth callback function
 *
 * @param _cmd: command line string
 */
static void cmd_auth(char* _cmd)
{
	mutual_authentication();
}

/**
 * @brief /atr callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_atr(char* _cmd)
{
	if (connect_reader() != PCSC_SUCCESS) {
		//
	}
}

/**
 * @brief /select callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_select(char* _cmd)
{
	select_ISD();
}

/**
 * @brief /send callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_send(char* _cmd)
{
	printf(COLOR_CYAN " /send " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /cap-info callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_capinfo(char* _cmd)
{
	printf(COLOR_CYAN " /cap-info " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /sleep callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_sleep(char* _cmd)
{
	printf(COLOR_CYAN " /sleep " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /echo callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_echo(char* _cmd)
{
	printf(COLOR_CYAN " /echo " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /mode callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_mode(char* _cmd)
{
	printf(COLOR_CYAN " /mode " COLOR_RESET "under implementation..\n");
}


/**
 * @brief /card callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_card(char* _cmd)
{
	if (connect_reader() != PCSC_SUCCESS) {
		//
		return;
	}

	select_ISD();
}


/**
 * @brief /execute callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_execute(char* _cmd)
{
	printf(COLOR_CYAN " /execute " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /error callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_error(char* _cmd)
{
	printf(COLOR_CYAN " /error " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /list-readers callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_listreaders(char* _cmd)
{
	printf(COLOR_CYAN " /list-readers " COLOR_RESET "under implementation..\n");
}

/**
 * @brief set-key callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_setkey(char* _cmd)
{
	printf(COLOR_CYAN " set-key " COLOR_RESET "under implementation..\n");
}

/**
 * @brief put-key callback function
 *
 * @param _cmd: command line string
 */
static void cmd_putkeyset(char* _cmd)
{
	printf(COLOR_CYAN " put-keyset " COLOR_RESET "under implementation..\n");
}

/**
 * @brief get-data callback function
 *
 * @param _cmd: command line string
 */
static void cmd_getdata(char* _cmd)
{
	printf(COLOR_CYAN " get-data " COLOR_RESET "under implementation..\n");
}

/**
 * @brief ls callback function
 *
 * @param _cmd: command line string
 */
static void cmd_ls(char* _cmd)
{
	get_status();
}

/**
 * @brief upload callback function
 *
 * @param _cmd: command line string
 */
static void cmd_upload(char* _cmd)
{
	printf(COLOR_CYAN " upload " COLOR_RESET "under implementation..\n");
}

/**
 * @brief version: Shows current versions
 * 
 * @param _cmd: command line string
 */
static void cmd_version(char* _cmd)
{
	printf(COLOR_WHITE "\nSIM and Global Platform shell" COLOR_RESET " version: %s\n", version);

//	printf("OS version: %s\n", os_version);
}

/**
 * @brief install callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_install(char* _cmd)
{
	printf(COLOR_CYAN " install " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /close command
 * 
 * @param _cmd: command line string
 */
static void cmd_S_close(char* _cmd)
{
	printf(COLOR_CYAN " /close " COLOR_RESET "under implementation..\n");
}

/**
 * @brief /set-var command
 *
 * @param _cmd: command line string
 */
static void cmd_S_setvar(char* _cmd)
{
	printf(COLOR_CYAN " /set-var " COLOR_RESET "under implementation..\n");
}

/**
 * @brief milenage command
 *
 * @param _cmd: command line string
 */
static void cmd_milenage(char* _cmd)
{
	printf(COLOR_CYAN " milenage " COLOR_RESET "under implementation..\n");
}

/**
 * @brief tuak command
 *
 * @param _cmd: command line string
 */
static void cmd_tuak(char* _cmd)
{
	printf(COLOR_CYAN " TUAK " COLOR_RESET "under implementation..\n");
}
