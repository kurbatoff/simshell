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

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "shell.h"
#include "gp.h"
#include "globalplatform.h"
#include "cap.h"
#include "tools.h"
#include "pcscwrap.h"

#define MAX_AID_HEX_LEN				32
#define MAX_PARAM_HEX_LEN			512

/**
 *
 * C functions exported to Lua:
 *
 */
static int Lua_connect_pcsc(lua_State* L);
static int Lua_select_ISD(lua_State* L);
static int Lua_send_apdu(lua_State* L);
//static int Lua_set_keyset(lua_State* L);
//static int Lua_authenticate(lua_State* L);
//static int Lua_upload(lua_State* L);
static int Lua_install(lua_State* L);
static int Lua_execute_shellcommand(lua_State* L);

static void lua_strcopy(lua_State* L, int idx, char* dest, int maxlen)
{
	char* str;
	size_t len;

	str = (char*)lua_tostring(L, idx);
	len = strlen(str);
	if (len > maxlen)
		len = maxlen;
	memcpy(dest, str, len);
	dest[len] = 0;
}

static int lua_popup_array(lua_State* L, uint8_t* apdubuff) //, int idx)
{
	uint32_t a_size;

	luaL_checktype(L, 1, LUA_TTABLE);

	// let alone excessive arguments (idiomatic), or do:
	lua_settop(L, 1);

	a_size = (uint32_t)lua_rawlen(L, 1); // absolute indexing for arguments

	for (uint32_t i = 1; i <= a_size; i++) {
		lua_pushinteger(L, i);
		lua_gettable(L, 1);

		if (lua_isnil(L, -1)) { // relative indexing for "locals"
			a_size = i - 1; // Fix actual size (e.g. 4th nil means a_size == 3)
			break;
		}

		if (!lua_isnumber(L, -1)) {
			return luaL_error(L, "item %d invalid (number required, got %s)", i, luaL_typename(L, -1));
		}

		lua_Integer b = lua_tointeger(L, -1);

		apdubuff[i - 1] = (uint8_t)(b & 0xFF); // Lua is 1-based, C is 0-based
		lua_pop(L, 1);
	}

	return a_size;
}

static int Lua_connect_pcsc(lua_State* L)
{
	printf(COLOR_GREEN " Connecting to PCSC..\n" COLOR_RESET);

	pcsc_listreaders();

	printf(COLOR_GREEN "  ..Ok.\n" COLOR_RESET);

	return 0;
}

static int Lua_select_ISD(lua_State* L)
{
	apdu_t apdu;

	if (connect_reader() != PCSC_SUCCESS) {
		//
		return -1;
	}
	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0xA4;
	apdu.cmd[apdu.cmd_len++] = 0x04;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0;

	gp_send_APDU(&apdu);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to select ISD\n" COLOR_RESET);
		return -1;
	}

	return 0;
}

static int Lua_send_apdu(lua_State* L)
{
	apdu_t apdu;

	apdu.cmd_len = lua_popup_array(L, apdu.cmd);

	gp_send_APDU(&apdu);

	lua_createtable(L, apdu.resp_len, 0);

	for (int i = 0; i < apdu.resp_len; i++) {
		lua_pushinteger(L, apdu.resp[i]);
		lua_rawseti(L, -2, i + 1); // As indexes in Lua start at 1
	}

	return 1;
}

static int Lua_upload(lua_State* L)
{
	char* filename;

	filename = (char*)lua_tostring(L, 1);

	printf(" Uploading CAP: %s\n", filename);

	upload((const char* )filename);

	return 0;
}

static int Lua_install(lua_State* L)
{
	apdu_t apdu;
	char aid_instance[MAX_AID_HEX_LEN + 1]; // to be NULL terminated
	char aid_package[MAX_AID_HEX_LEN + 1];
	char aid_applet[MAX_AID_HEX_LEN + 1];
	char params[MAX_PARAM_HEX_LEN + 1];
	uint16_t len;

	// TODO chack type:
	// TABLE or STRING
	
	// Package, Applet, Instance AIDs
	lua_strcopy(L, 1, aid_package, MAX_AID_HEX_LEN);
	lua_strcopy(L, 2, aid_applet, MAX_AID_HEX_LEN);
	lua_strcopy(L, 3, aid_instance, MAX_AID_HEX_LEN);

	// Params
	lua_strcopy(L, 4, params, MAX_PARAM_HEX_LEN);

	printf("Install: 80E6 0000 .. %s %s %s %s\n\n", aid_package, aid_applet, aid_instance, params);


	if (connect_reader() != PCSC_SUCCESS) {
		//
		return -1;
	}

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_INSTALL;
	apdu.cmd[apdu.cmd_len++] = INSTALL_FOR_INSTALL | INSTALL_FOR_MAKE_SELECTABLE;
	apdu.cmd[apdu.cmd_len++] = 0x00;
	apdu.cmd[apdu.cmd_len++] = 0;

	// ELF
	len = (uint16_t)( strlen(aid_package) / 2 );
	apdu.cmd[apdu.cmd_len++] = len & 0xFF;
	convert_hex2bin(aid_package, &apdu.cmd[apdu.cmd_len], len);
	apdu.cmd_len += len;

	// EM
	len = (uint16_t)(strlen(aid_applet) / 2);
	apdu.cmd[apdu.cmd_len++] = len & 0xFF;
	convert_hex2bin(aid_applet, &apdu.cmd[apdu.cmd_len], len);
	apdu.cmd_len += len;

	// Instance
	len = (uint16_t)(strlen(aid_instance) / 2);
	if (0 == len) {
		memcpy(aid_instance, aid_applet, sizeof(aid_applet));
		len = (uint16_t)(strlen(aid_instance) / 2);
	}
	apdu.cmd[apdu.cmd_len++] = len & 0xFF;
	convert_hex2bin(aid_instance, &apdu.cmd[apdu.cmd_len], len);
	apdu.cmd_len += len;

	//
	apdu.cmd[apdu.cmd_len++] = 0x01;
	apdu.cmd[apdu.cmd_len++] = 0x00;

	// Parameters
	len = (uint16_t)(strlen(params) / 2);
	apdu.cmd[apdu.cmd_len++] = len & 0xFF;
	convert_hex2bin(params, &apdu.cmd[apdu.cmd_len], len);
	apdu.cmd_len += len;

	apdu.cmd[apdu.cmd_len++] = 0x00;

	/*
	
	cm> install -i a0000005591010ffffffff8900000100  -q C9#() a0000005591010ffffffff8900000000 a0000005591010ffffffff8900000100

	80 E6 0C 00 39 
	10 A0 00 00 05 59 10 10 FF FF FF FF 89 00 00 00 00
	10 A0 00 00 05 59 10 10 FF FF FF FF 89 00 00 01 00
	10 A0 00 00 05 59 10 10 FF FF FF FF 89 00 00 01 00
	01 00 02 C9 00 00
	
	00

	*/

	apdu.cmd[4] = apdu.cmd_len - 5;

	gp_send_APDU(&apdu);

	if (0x61 == apdu.resp[apdu.resp_len - 2]) {
		apdu.resp_len = get_response(apdu.resp[apdu.resp_len - 1], apdu.resp, sizeof(apdu.resp));
	}

	if (0x90 != apdu.resp[apdu.resp_len - 2]) {
		printf(COLOR_RED " Failed to INSTALL and MAKE SELECTABLE..\n" COLOR_RESET);
		return -1;
	}

	return 0;
}

static int Lua_execute_shellcommand(lua_State* L)
{
	char command[1024];

	lua_strcopy(L, 1, command, 1023);

	SHELL_execute(command);

	return 0;
}

void Lua_execute(char* filename)
{
	lua_State* L;

	L = luaL_newstate();

	luaL_openlibs(L);

	lua_register(L, "C_connect_PCSC", Lua_connect_pcsc);
	lua_register(L, "C_select_ISD", Lua_select_ISD);
	lua_register(L, "C_send_apdu", Lua_send_apdu);
	lua_register(L, "C_upload", Lua_upload);
	lua_register(L, "C_install", Lua_install);
	lua_register(L, "C_execute", Lua_execute_shellcommand);


	luaL_dofile(L, filename);

	lua_close(L);
}
