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
#include "getstatus.h"
#include "globalplatform.h"
#include "scp11.h"
#include "iso7816.h"
#include "euicc.h"
#include "vars.h"
#include "keys.h"
#include "gp.h"
#include "cap.h"
#include "securechannel.h"

static void help(char* _cmd);
static void cmd_version(char* _cmd);

static void cmd_S_capinfo(char* _cmd);
static void cmd_S_cap2ijc(char* _cmd);
static void cmd_S_cap2apdu(char* _cmd);
static void cmd_S_listreaders(char* _cmd);
static void cmd_S_term(char* _cmd);
static void cmd_S_close(char* _cmd);
static void cmd_S_reset(char* _cmd);

static void cmd_S_atr(char* _cmd);
static void cmd_S_card(char* _cmd);
static void cmd_S_select(char* _cmd);
static void cmd_S_send(char* _cmd);

static void cmd_auth(char* _cmd);
static void cmd_initupdate(char* _cmd);
static void cmd_extauthenticate(char* _cmd);
static void cmd_ls(char* _cmd);
static void cmd_upload(char* _cmd);
static void cmd_delete(char* _cmd);

static void cmd_euicc(char* _cmd);
static void cmd_iotsafe(char* _cmd);

//static void cmd_S_echo(char* _cmd);
//static void cmd_S_sleep(char* _cmd);
//static void cmd_S_mode(char* _cmd);
//static void cmd_S_execute(char* _cmd);
//static void cmd_S_error(char* _cmd);
//static void cmd_getsdcert(char* _cmd);
//static void cmd_getdata(char* _cmd);
//static void cmd_install(char* _cmd);
//static void cmd_milenage(char* _cmd);
//static void cmd_tuak(char* _cmd);


// ---------------------------------------------------------------------------------------------------------------------
// Shell gCMDbuffs array
// ---------------------------------------------------------------------------------------------------------------------
simshell_command_t commands_array[SHELL_COMMANDS_COUNT] = {
	{
		"help",
		"\n\"help\": List available gCMDbuffs\n",
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
		"/list-readers",
		"\n\"list-readers\"\n",
		" /list-readers [-] List available card readers\n",
		cmd_S_listreaders
	},
	{
		"/terminal",
		"\n\"/terminal arg1\":\n Usage:\n    arg1: terminal type\n",
		" /terminal         List card readers and connect\n",
		cmd_S_term
	},
	{
		"/close",
		"\n\"/close\"\n\n",
		" /close            Close current terminal (card reader context)\n",
		cmd_S_close
	},
	{
		"/reset",
		"\n\"/reset\"\n\n",
		" /reset            Perform COLD RESET (power OFF and power ON)\n",
		cmd_S_reset
	},
	{
		"/card",
		"\n\"/card -a AID\":\n Usage:\n    AID: ISD (Card Manager) AID\n",
		" /card             Power on the card and select ISD \n",
		cmd_S_card
	},
	{
		"/atr",
		"\n\"led arg1 arg2\":\n Usage:\n    arg1: 1|2|3|4[-]         "
		"   /atr\n    arg2: on|off                Led status\n",
		" /atr              Power on the card and print ATR\n",
		cmd_S_atr
	},
	{
		"/select",
		"\n\"/select AID\":\n Usage:\n    AID: AID of the applet to select\n",
		" /select           Select an applet idetified by AID\n",
		cmd_S_select
	},
	{
		"init-update",
		"\n\"init-update\""
		"   init-update sec_level\n",
		" init-update       Perform initialize update\n",
		cmd_initupdate
	},
	{
		"ext-auth",
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
		"/cap-info",
		"\n\"/cap-info file.CAP\":\n Usage:\n    file: the .CAP file name\n",
		" /cap-info     [-] Print .CAP file information\n",
		cmd_S_capinfo
	},
	{
		"/cap2ijc",
		"\n\"/cap2ijc file.CAP\":\n Usage:\n    file: the .CAP file name\n",
		" /cap2ijc      [-] Convert .CAP file into .IJC format\n",
		cmd_S_cap2ijc
	},
	{
		"/cap2apdu",
		"\n\"/cap2apdu file.CAP\":\n Usage:\n    file: the .CAP file name\n",
		" /cap2apdu     [-] Convert .CAP file into APDU commands\n",
		cmd_S_cap2apdu
	},
	{
		"upload",
		"\n\"upload\"\n",
		" upload            Load .CAP or .IJC file\n",
		cmd_upload
	},
	{
		"delete",
		"\n\"delete\" [-r] AID\n",
		" delete            Delete Inastance or CAP file\n",
		cmd_delete
	},
	{
		"uicc",
		"\n\"uicc\" eid|info|pl|enable|disable|delete|download|reset\n",
		" uicc              Perform ISD-R function: eid|info|pl|enable|disable|delete|download|reset\n",
		cmd_euicc
	},
	{
		"iotsafe",
		"\n\"iotsafe\" dir|encrypt|decrypt|sign|verify|...\n",
		" iotsafe           Perform IoT SAFE function: dir|encrypt|decrypt|sign|verify|...t\n",
		cmd_iotsafe
	},
	{
		"ls",
		"\n\"ls\"\n",
		" ls                Retrieve and print GP registry (GET STATUS)\n",
		cmd_ls
	},
	{
		"get-data",
		"\n\"get-data\"\n",
		" get-data          GET DATA identified by tag\n",
		cmd_getdata
	},
	{
		"set-key",
		"\n\"set-key\"\n",
		" set-key       [-] Set context (shell enviroment only) Security Domain keys\n",
		cmd_setkey
	},
	{
		"put-keyset",
		"\n\"put-keyset\"\n",
		" put-keyset        Store context keyset into Security Domain\n",
		cmd_putkeyset
	},
	{
		"/send",
		"\n\"/send\":\n Usage:\n    APDU string\n",
		" /send             Send APDU-C to the active reader\n",
		cmd_S_send
	},
	{
		"/set-var",
		"\n\"/set-var\"\n\n",
		" /set-var      [-] Set variable name and value\n",
		cmd_S_setvar
	},
	{
		"/list-vars",
		"\n\"/list-vars\"\n\n",
		" /list-vars    [-] Print out all vars and values\n",
		cmd_S_listvars
	}
};

/**
 * @brief Shell command HELP callback function
 *
 * @param argc: amount of help command aguments including help command itself as argv[0]
 * @param argv: help command aguments including help command itself as argv[0]
 */
static void help(char* _cmd)
{
	simshell_command_t* c;
	simshell_command_t* cj;

	int sortedlist[SHELL_COMMANDS_COUNT];

	// Copy indexes for sorting
	for (int i = 0; i < SHELL_COMMANDS_COUNT; i++)
	{
		sortedlist[i] = i;
	}

	// Sort gCMDbuff names alphabetically
	for (int i = 0; i < SHELL_COMMANDS_COUNT; i++)
	{
		for (int j = i; j < SHELL_COMMANDS_COUNT; j++)
		{
			c = (simshell_command_t*)&commands_array[sortedlist[i]];
			cj = (simshell_command_t*)&commands_array[sortedlist[j]];

			if (strcmp(c->pcName, cj->pcName) > 0)
			{
				int x;

				x = sortedlist[i];
				sortedlist[i] = sortedlist[j];
				sortedlist[j] = x;
			}
		}
	}

	// Print our the sorted list
	for (int i = 0; i < SHELL_COMMANDS_COUNT; i++)
	{
		c = (simshell_command_t*)&commands_array[sortedlist[i]];

		printf(" %s", c->pcShortHelp);
	}
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
	" /execute      [-] Execute host OS gCMDbuff\n",
	cmd_S_execute

	_error,
	"\n\"/error\"\n",
	" /error        [-] Stop script execution and generate error\n",
	cmd_S_error

	getsdcert,
	"\n\"get-sd-certificate arg1 arg2 arg3\":\n Usage:\n    arg1: 1|2|3|4...         \n",
	" get-sd-certif..   Get SD certificate i.e. start SCP11 authentication\n",
	cmd_getsdcert

	quit,
	"\n\"quit\"\n\n",
	" quit          [-] Exit SIM shell\n",
	NULL

	install,
	COLOR_YELLOW "install" COLOR_RESET " [-i AID] -q par1 ELF_AID EM_AID\n",
	" install           Install applet instance out of .CAP file\n",
	cmd_install

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
	init_update();
}

/**
 * @brief ext-authenticte callback function
 *
 * @param _cmd: command line string
 */
static void cmd_extauthenticate(char* _cmd)
{
	uint8_t sec_level = SECURITY_LEVEL_PLAIN;

	// TODO

	ext_authenticate(sec_level);
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
	int offset;
	int len = (int)strlen(_cmd);
	uint8_t sec_level = SECURITY_LEVEL_PLAIN;

	offset = 4; // just after auth
	while (offset < len) {
		switch (_cmd[offset]) {
		case ' ':
		case '\t':
			offset++;
			continue;
		}

		if (_cmd[offset] == 'm' && _cmd[offset + 1] == 'a' && _cmd[offset + 2] == 'c') {
			sec_level = SECURITY_LEVEL_MAC;
		}

		if (_cmd[offset] == 'e' && _cmd[offset + 1] == 'n' && _cmd[offset + 2] == 'c') {
			sec_level = SECURITY_LEVEL_ENC;
		}

		break;
	}


	mutual_authentication(sec_level);
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
	apdu_t apdu;
	int len;
	int offset = 6; // after "/send "
	
	len = (int)strlen(_cmd);

	apdu.cmd_len = 0;
	while (offset < len) {
		apdu.cmd[apdu.cmd_len++] = byte_from_hex_str(&_cmd[offset]);
		offset += 2;
	}

	gp_send_APDU(&apdu);
}

/**
 * @brief /cap-info callback function
 * 
 * @param _cmd: command line string
 */
static void cmd_S_capinfo(char* _cmd)
{
	print_cap_info(&_cmd[10]);
}

/**
 * @brief /cap2ijc callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_cap2ijc(char* _cmd)
{
	char ijcname[1024];
	int flen;

	char* capname = &_cmd[9];

	strcpy(ijcname, capname);

	flen = (int)strlen(ijcname);
	ijcname[flen - 3] = 'i';
	ijcname[flen - 2] = 'j';
	ijcname[flen - 1] = 'c';

	cap2ijc(capname, ijcname, true, true);
}

/**
 * @brief /cap2apdu callback function
 *
 * @param _cmd: command line string
 */
static void cmd_S_cap2apdu(char* _cmd)
{
	//cap2apdu(&_cmd[10]);
	cap2apdu(&_cmd[10], 160);
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
	int len;
	int offset;

	len = (int)strlen(_cmd);

	offset = 6; // just after upload
	while (offset < len) {
		switch (_cmd[offset]) {
		case ' ':
		case '\t':
		offset++;
			continue;
		}

		break;
	}

	upload(&_cmd[offset]);
}

/**
 * @brief delete callback function
 *
 * @param _cmd: command line string
 */
static void cmd_delete(char* _cmd)
{
	apdu_t apdu;
	int len;
	int offset;
	uint8_t del_all = 0;

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_DELETE;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0; // Might be updated
	apdu.cmd[apdu.cmd_len++] = 0; // WIll be updated at idx 4
	apdu.cmd[apdu.cmd_len++] = 0x4f;
	apdu.cmd[apdu.cmd_len++] = 0; // WIll be updated at idx 6

	len = (int)strlen(_cmd);

	offset = 6; // just after delete
	while (offset < len) {
		switch (_cmd[offset]) {
		case ' ':
		case '\t':
		offset++;
			continue;
		}

		break;
	}

	if (_cmd[offset] == '-' && _cmd[offset+1] == 'r') {
		offset += 3;
		apdu.cmd[3] = 0x80;
	}

	while (offset < len) {
		apdu.cmd[apdu.cmd_len++] = byte_from_hex_str(&_cmd[offset]);
		offset += 2;
	}

	apdu.cmd[4] = apdu.cmd_len - 5;
	apdu.cmd[6] = apdu.cmd_len - 7;

	gp_send_APDU(&apdu);
}

/**
 * @brief iotsafe: Implements set of IoT SAFE commands
 *
 * @param _cmd: command line string
 */
static void cmd_iotsafe(char* _cmd)
{
	printf(COLOR_CYAN "\n IoT SAFE-R" COLOR_RESET " commands\n");
}

/**
 * @brief euicc: Implements set of eUICC commands
 *
 * @param _cmd: command line string
 */
static void cmd_euicc(char* _cmd)
{
	int len;
	int offset;

	offset = 4;

	len = (int)strlen(_cmd);

	while (offset < len) {
		switch (_cmd[offset]) {
		case ' ':
		case '\t':
			offset++;
			continue;
		}

		break;
	}

	if (0 == memcmp(&_cmd[offset], "pl", 2)) {
		printf(COLOR_WHITE "\n ISD-R Profile list" COLOR_RESET "\n");

		cmd_euicc_pl();

		return;
	}

	if (0 == memcmp(&_cmd[offset], "eid", 3)) {
		printf(COLOR_WHITE "\n eUICC EID" COLOR_RESET "\n");

		cmd_euicc_eid();

		return;
	}

	if (0 == memcmp(&_cmd[offset], "info", 4)) {
		cmd_euicc_info();
		return;
	}

	if (0 == memcmp(&_cmd[offset], "enable", 6)) {
		printf(COLOR_WHITE "\n Enable ISD-P profile" COLOR_RESET "\n");

		//cmd_euicc_enable();

		return;
	}

	if (0 == memcmp(&_cmd[offset], "disable", 7)) {
		printf(COLOR_WHITE "\n Disable ISD-P profile" COLOR_RESET "\n");
		return;
	}

	if (0 == memcmp(&_cmd[offset], "delete", 6)) {
		printf(COLOR_WHITE "\n Delete ISD-P profile" COLOR_RESET "\n");
		return;
	}

	if (0 == memcmp(&_cmd[offset], "download", 8)) {
		printf(COLOR_WHITE "\n Download ISD-P profile" COLOR_RESET "\n");
		return;
	}

	if (0 == memcmp(&_cmd[offset], "notifications", 13)) {
		printf(COLOR_WHITE "\n Process ISD-R notifications" COLOR_RESET "\n");
		return;
	}

	printf(COLOR_WHITE "\n Unknown ISD-R command: " COLOR_RED "%s" COLOR_RESET "\n", &_cmd[offset]);
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
 * @brief /reset command
 *
 * @param _cmd: command line string
 */
static void cmd_S_reset(char* _cmd)
{
	printf(COLOR_CYAN " /reset " COLOR_RESET "under implementation..\n");
}

/**
 * @brief milenage command
 *
 * @param _cmd: gCMDbuff line string
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
