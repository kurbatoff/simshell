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

#include "vars.h" 
#include "tools.h" 

 /**
 * SEQUENSE of:
 * {
 *   a) name[] ZERO terminated
 *   b) len[1..3] BER-TLV encoded
 *   c) value[len]
 * }
 *
 */
static uint8_t VARs[16 * 1024];

static void print_VARs();
static int get_VARs_length();
static void add_VAR(const char* name, int name_len, char* value, int value_len);
// static int find_VAR(char* _name, int _namelen, char** _var, int* _datalen);

 /**
  * @brief /set-var command
  *
  * @param _cmd: command line string
  */
void cmd_S_setvar(char* _cmd)
{
    char* name;
    int name_len = 0;
    char* value;
    int value_len = 0;
    int offset;

    // /set-var X 0102030405

    offset = 0;

    // Find name 
    while (_cmd[offset++] != ' ');
    name = &_cmd[offset];

    while (_cmd[offset] != ' ')
    {
        offset++;
        name_len++;
    }

    // Find value
    while (_cmd[offset] == ' ') {
        offset++;
    }
    value = &_cmd[offset];

    while ( (_cmd[offset] != ' ') && (_cmd[offset] != 0x00) ) {
        value_len++;
        offset++;
    }

    add_VAR(name, name_len, value, value_len);
}

/**
 * @brief /list-vars command
 *
 * @param _cmd: command line string
 */
void cmd_S_listvars(char* _cmd)
{
    print_VARs();
}

static void print_VARs()
{
    int offset = 0;
    int len;
    char c;

    printf(COLOR_YELLOW " Variables:\n" COLOR_RESET);

    //dump_hexascii_buffer(" VARs:", VARs, 128);

    while (VARs[offset] != 0) {
        len = VARs[offset++];

        c = VARs[offset + len];
        VARs[offset + len] = 0x00;
        printf( "  %-16s  ", &VARs[offset]);
        VARs[offset + len] = c;
        offset += len;

        len = VARs[offset++];
        switch (len) {
        case 0x81:
            len = VARs[offset++];
            break;
        case 0x82:
            len = VARs[offset++];
            len *= 0x100;
            len += VARs[offset++];
            break;
        }
        c = VARs[offset + len];
        VARs[offset + len] = 0x00;
        printf("%s\n", &VARs[offset]);
        VARs[offset + len] = c;

        offset += len;
    }
}

static int get_VARs_length()
{
    int offset = 0;
    int len;

    while (VARs[offset] != 0) {
        len = VARs[offset++];
        offset += len;

        len = VARs[offset++];
        switch (len) {
        case 0x81:
            len = VARs[offset++];
            break;
        case 0x82:
            len = VARs[offset++];
            len *= 0x100;
            len = VARs[offset++];
            break;
        }
        offset += len;
    }

    return offset;
}

#if(0)
static int find_VAR(char* _name, int _namelen, char** _var, int* _datalen)
{
    int offset = 0;
    int nlen;
    int vlen;
    uint8_t* name;

    while (VARs[offset] != 0) {
        nlen = VARs[offset++];
        name = &VARs[offset];
        offset += nlen;

        vlen = VARs[offset++];
        switch (vlen) {
        case 0x81:
            vlen = VARs[offset++];
            break;
        case 0x82:
            vlen = VARs[offset++];
            vlen *= 0x100;
            vlen = VARs[offset++];
            break;
        }

        if ((nlen == _namelen) && 0 == memcmp(_name, name, _namelen)) {
            *_var = (char*)&VARs[offset];
            *_datalen = vlen;

            return 0;
        }

        offset += vlen;
    }


    return -1;
}
#endif
static void add_VAR(const char* name, int name_len, char* value, int value_len)
{
    int len;

    len = get_VARs_length();

    VARs[len++] = name_len;
    memcpy(&VARs[len], name, name_len);
    len += name_len;

    if (value_len < 0x80) {
        VARs[len++] = value_len;
    }
    else {
        if (value_len < 0x100) {
            VARs[len++] = 0x81;
            VARs[len++] = value_len;
        }
        else {
            VARs[len++] = 0x82;
            VARs[len++] = value_len >> 8;
            VARs[len++] = value_len & 0xFF;
        }
    }
    memcpy(&VARs[len], value, value_len);
    len += value_len;

    VARs[len] = 0x00;

    //print_VARs();
}
