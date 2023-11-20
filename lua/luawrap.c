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

//#include "euicc.h"
//#include "gp.h"
//#include "gsma.h"
#include "tools.h"
#include "pcscwrap.h"

/**
 *
 * C functions exported to Lua:
 *
 */
static int Lua_connect_pcsc(lua_State* L);
static int Lua_select_ISD(lua_State* L);
static int Lua_send_apdu(lua_State* L);
//static int Lua_sent_keyset(lua_State* L);
//static int Lua_authenticate(lua_State* L);
//static int Lua_upload(lua_State* L);
//static int Lua_install(lua_State* L);
//static int Lua_execute_shellcommand(lua_State* L);


static int lua_popup_apdu(lua_State* L, uint8_t* apdubuff)
{
	int a_size;

	luaL_checktype(L, 1, LUA_TTABLE);

	// let alone excessive arguments (idiomatic), or do:
	lua_settop(L, 1);

	a_size = lua_rawlen(L, 1); // absolute indexing for arguments

	for (int i = 1; i <= a_size; i++) {
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

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

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

	apdu.cmd_len = lua_popup_apdu(L, apdu.cmd);

	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	lua_createtable(L, apdu.resp_len, 0);

	for (int i = 0; i < apdu.resp_len; i++) {
		lua_pushinteger(L, apdu.resp[i]);
		lua_rawseti(L, -2, i + 1); // As indexes in Lua start at 1
	}

	return 1;
}



void Lua_execute(char* filename)
{
	lua_State* L;

	L = luaL_newstate();

	luaL_openlibs(L);

	lua_register(L, "C_connect_PCSC", Lua_connect_pcsc);
	lua_register(L, "C_select_ISD", Lua_select_ISD);
	lua_register(L, "C_send_apdu", Lua_send_apdu);

	luaL_dofile(L, filename);

	lua_close(L);
}
