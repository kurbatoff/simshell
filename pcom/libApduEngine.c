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

/**
*   TODO
* [+]  1. .POWER_ON
* [+]  2. .POWER_OFF
*      3. .INSERT
* [+]  4. Check SW
* 
* [+]  5. Check DATA
* [+]  6. .SET_BUFFER
* [+]  7. Multi line
*      8. .CALL directive
*
*     20. malloc for buffers
* [+] 21. Display execution time
* 
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "libApduEngine.h"
#include "tools.h"
#include "pcscwrap.h"

#define BUFFER_ZERO 'g'
#define BUFFER_MAX_COUNT ('w' - BUFFER_ZERO + 1)

#define BUFF_G      ('g' - BUFFER_ZERO)
#define BUFF_H      ('h' - BUFFER_ZERO)
#define BUFF_I      ('i' - BUFFER_ZERO)
#define BUFF_J      ('j' - BUFFER_ZERO)
#define BUFF_K      ('k' - BUFFER_ZERO)
#define BUFF_L      ('l' - BUFFER_ZERO)
#define BUFF_M      ('m' - BUFFER_ZERO)
#define BUFF_N      ('n' - BUFFER_ZERO)
#define BUFF_O      ('o' - BUFFER_ZERO)
#define BUFF_P      ('p' - BUFFER_ZERO)
#define BUFF_Q      ('q' - BUFFER_ZERO)
#define BUFF_R      ('r' - BUFFER_ZERO)
#define BUFF_S      ('s' - BUFFER_ZERO)
#define BUFF_T      ('t' - BUFFER_ZERO)
#define BUFF_U      ('u' - BUFFER_ZERO)
#define BUFF_V      ('v' - BUFFER_ZERO)
#define BUFF_W      ('w' - BUFFER_ZERO)

static struct buffer_t BUFFs[BUFFER_MAX_COUNT];
static struct errorCount_t errors;

static char SWexp[32]; // Sequence of expected SW(s) in CAPITAL hex: '61XX91XX9000'
static char DATAexp[512]; // Expected DATA in CAPITAL hex

static char PartString[2048];

/**
* SEQUENSE of:
* {
*   a) name[] ZERO terminated
*   b) len[1..3] BER-TLV encoded
*   c) value[len]
* }
* 
*/
static uint8_t DEFs[8 * 1024];

static int replace_defines_and_buffers(char* _s);
static void print_syntax_error(const char* _str, int _pos);
static void print_sw_error(const char* sw);

static void leftshift(char* _s)
{
    int i = 0;
    while (_s[i]) {
        _s[i] = _s[i + 1];
        i++;
    }
}

static void leftshift_n(char* _s, int n)
{
    int i = 0;
    int len = (int)strlen(_s);

    if (len < n) {
        _s[0] = 0;
        return;
    }

    while (_s[i + n - 1]) {
        _s[i] = _s[i + n];
        i++;
    }
}

static void clear_all()
{
    for (int i = 0; i < BUFFER_MAX_COUNT; i++)
        BUFFs[i].len = 0;

    errors.comm = 0;
    errors.data = 0;
    errors.status = 0;
    errors.syntax = 0;

    DEFs[0] = 0;
    PartString[0] = 0;
}

static int errors_count()
{
    return errors.comm + errors.data + errors.status + errors.syntax;
}

static void print_Buffers_()
{
    char B[4] = { ' ', 'G', ':', 0 };

    printf(COLOR_YELLOW " Buffers:\n" COLOR_RESET);
    for (int i = 0; i < BUFFER_MAX_COUNT; i++) {
        if (BUFFs[i].len)
            dump_hexascii_buffer(B, BUFFs[i].data, BUFFs[i].len);
        B[1]++;
    }
    printf("\n");
}

static void set_Buffer(uint8_t idx, uint8_t* value, int len)
{
    BUFFs[idx].len = len;
    memcpy(BUFFs[idx].data, value, len);
}

static void print_Defines_()
{
    int offset = 0;
    int len;
    char c;

    printf(COLOR_YELLOW " Defines:\n" COLOR_RESET);

    //dump_hexascii_buffer(" DEFs:", DEFs, 128);

    while (DEFs[offset] != 0) {
        len = DEFs[offset++];

        c = DEFs[offset + len];
        DEFs[offset + len] = 0x00;
        printf("%s ", &DEFs[offset]);
        DEFs[offset + len] = c;
        offset += len;

        len = DEFs[offset++];
        switch (len) {
        case 0x81:
            len = DEFs[offset++];
            break;
        case 0x82:
            len = DEFs[offset++];
            len *= 0x100;
            len += DEFs[offset++];
            break;
        }
        c = DEFs[offset + len];
        DEFs[offset + len] = 0x00;
        printf("%s\n", &DEFs[offset]);
        DEFs[offset + len] = c;

        offset += len;
    }
}

static int get_Defines_length()
{
    int offset = 0;
    int len;

    while (DEFs[offset] != 0) {
        len = DEFs[offset++];
        offset += len;

        len = DEFs[offset++];
        switch (len) {
        case 0x81:
            len = DEFs[offset++];
            break;
        case 0x82:
            len = DEFs[offset++];
            len *= 0x100;
            len = DEFs[offset++];
            break;
        }
        offset += len;
    }

    return offset;
}

static int find_define(char* _name, int _namelen, char** _define, int* _datalen)
{
    int offset = 0;
    int nlen;
    int vlen;
    uint8_t* name;

    while (DEFs[offset] != 0) {
        nlen = DEFs[offset++];
        name = &DEFs[offset];
        offset += nlen;

        vlen = DEFs[offset++];
        switch (vlen) {
        case 0x81:
            vlen = DEFs[offset++];
            break;
        case 0x82:
            vlen = DEFs[offset++];
            vlen *= 0x100;
            vlen = DEFs[offset++];
            break;
        }

        if ( (nlen == _namelen) && 0 == memcmp(_name, name, _namelen) ) {
            *_define = (char* )&DEFs[offset];
            *_datalen = vlen;
            
            return 0;
        }

        offset += vlen;
    }


    return -1;
}

static void add_Define_(const char* name, int name_len, char* value, int value_len)
{
    int len;
    
    len = get_Defines_length();

    DEFs[len++] = name_len;
    memcpy(&DEFs[len], name, name_len);
    len += name_len;

    if (value_len < 0x80) {
        DEFs[len++] = value_len;
    }
    else {
        if (value_len < 0x100) {
            DEFs[len++] = 0x81;
            DEFs[len++] = value_len;
        }
        else {
            DEFs[len++] = 0x82;
            DEFs[len++] = value_len >> 8;
            DEFs[len++] = value_len & 0xFF;
        }
    }
    memcpy(&DEFs[len], value, value_len);
    len += value_len;

    DEFs[len] = 0x00;

    //print_Defines();
}

static int process_SET_BUFFER(char* _cmd)
{
    char* name;
    char* value;
    int value_len;
    int buf_idx;

    // 1.1 Name: remove leading spaces
    name = _cmd;
    while (*name == 0x20)
        name++;

    // 1.1 Name: must be 1 char
    if (name[1] != 0x20) {
        return -1;
    }

    value = &name[2];

    // 2. Value: replace DEFINEs and BUFFERs
    replace_defines_and_buffers(value);

    // 3.1 Value: remove leading spaces
    while (*value == 0x20)
        value++;

    // 3.3 Value: count the length
    value_len = 0;
    while (value[value_len] != 0) {
        if ( (value[value_len] == '(') || (value[value_len] == '[') ) {
            break;
        }

        if (value[value_len] == 0x20) {
            leftshift(&value[value_len]);
        }
        else
            value_len++;
    }


    buf_idx = *name - BUFFER_ZERO;

    value_len /= 2;

    convert_hex2bin(value, (uint8_t* )value, value_len);
    set_Buffer(buf_idx, (uint8_t* )value, value_len);

    //print_Buffers();

    return 0;
}

static int process_DEFINE(char* _cmd)
{
    char* name;
    char* value;
    int name_len;
    int value_len;


    // 1.1 Name: remove leading spaces
    name = _cmd;
    while (*name == 0x20)
        name++;

    name++; //skip %

    // 1.1 Name: count the length
    name_len = 0;
    while (name[name_len] != 0x20)
        name_len++;

    value = &name[name_len];

    // 2. Value: replace DEFINEs and BUFFERs
    replace_defines_and_buffers(value);

    // 3.1 Value: remove leading spaces
    while (*value == 0x20)
        value++;

    // 3.3 Value: count the length
    value_len = 0;
    while (value[value_len] != 0) {
        if (value[value_len] == 0x20) {
            leftshift(&value[value_len]);
        } else
            value_len++;
    }

    add_Define_(name, name_len, value, value_len);
    
    //print_Defines();

    return 0;
}

static int replace_one_buffer(char* _s)
{
    int buff_idx = _s[0] - BUFFER_ZERO;
    int start_idx; // Start index (1..) inside (): (2;1) or (2;2)
    int blen; // Len of the buffer string to be replaced, inc. ()
    int datalen; // Len of the data to be replaced by

    blen = 1;
    if ('(' == _s[1]) {
        int offset = 2;
        char delim;

        blen++; // Include (
        start_idx = 0;

        while ((_s[offset] != ':') && (_s[offset] != ';')) {
            start_idx *= 10;
            start_idx += _s[offset] - '0';
            blen++;
            offset++;
        }

        delim = _s[offset++];
        blen++;
        datalen = 0;

        while (_s[offset] != ')') {
            datalen *= 10;
            datalen += _s[offset] - '0';

            blen++;
            offset++;
        }

        blen++; // Include ')'

        // W(2;1)
        if (delim == ':') {
            datalen -= (start_idx - 1);
        }
    }
    else {
        start_idx = 1;
        datalen = BUFFs[buff_idx].len;
    }

    if (datalen > BUFFs[buff_idx].len) {
        // TODO
        datalen = BUFFs[buff_idx].len;
    }

    {
        char tail[2048];
        uint8_t* buff = BUFFs[buff_idx].data;

        strcpy(tail, &_s[blen]);
        convert_bin2hex(&buff[start_idx -1], (uint8_t* )_s, datalen);
        strcpy(&_s[datalen*2], tail);
    }

    return blen;
}

static int replace_one_define(char* _s)
{
    int idx; // position of %
    int nlen; // Define name length inc %
    int datalen; // Len of the data to be replaced by
    char* define;

//    print_Defines();

    idx = 0;
    while (_s[idx] != '%')
        idx++;

    nlen = 0;
    while ((_s[nlen] != 0x20) && (_s[nlen] != 0)
            && (_s[nlen] != '(') && (_s[nlen] != ')') && (_s[nlen] != '[') && (_s[nlen] != ']')
        )
        nlen++;

    if (find_define(&_s[idx + 1], nlen - 1, &define, &datalen) < 0)
        return -1;

    {
        char tail[1024];

        strcpy(tail, &_s[nlen]);

        memcpy(_s, define, datalen);

        strcpy(&_s[datalen], tail);
    }

    return datalen;
}

static int replace_defines_and_buffers(char* _s)
{
    int i = 0;
    int res;

    //printf(" >> %s\n", _s);
    while (_s[i]) {
        if ('%' == _s[i])
        {
            res = replace_one_define(&_s[i]);
            if (res < 0) {
                //printf_error(_s, "DEFINE not found", i);
                return res;
            }

            i += res; // Here we skip the inserted data
            continue;
        }

        if ((_s[i] >= BUFFER_ZERO) && (_s[i] <= 'w')) {
            res = replace_one_buffer(&_s[i]);
            if (res < 0) {
                //printf_error(_s, "BUFFER not found", i);
                return res;
            }

            i += res;
            continue;
        }

        i++;
    }
    //printf(" << %s\n", _s);

    return 0;
}

static int proceed_Directives(char* cmd)
{
    int res = 0;

    if (memcmp(cmd, ".insert", 7) == 0) {
        //
        return 0;
    }

    if (memcmp(cmd, ".power_on", 9) == 0) {
        if (connect_reader() != PCSC_SUCCESS) {
            errors.comm++;
            return -1;
        }
        return 0;
    }

    if (memcmp(cmd, ".power_off", 10) == 0) {
        disconnect_reader();
        return 0;
    }

    if (memcmp(cmd, ".define", 7) == 0) {
        res = process_DEFINE(&cmd[8]);

        return res;
    }

    if (memcmp(cmd, ".set_buffer", 11) == 0) {
        res = process_SET_BUFFER(&cmd[12]);

        return res;
    }

    if (memcmp(cmd, ".allundefine", 12) == 0) {
        DEFs[0] = 0;
        return 0;
    }

    if (memcmp(cmd, ".call", 5) == 0) {

        // TODO
        return res;
    }

    return 0;
}

static void print_syntax_error(const char* _str, int _pos)
{
    printf(COLOR_RED " ------------------------------------------------\n");
    for (int j = 0; j < _pos; j++)
        printf("%c", _str[j]);
    printf(COLOR_RED "%s\n" COLOR_RESET, &_str[_pos]);
    printf(" Syntax error (position: %d, char: %c)\n", _pos, _str[_pos]);

    errors.syntax++;
}

static void print_sw_error(const char* sw)
{
    int idx = 0;

    printf(COLOR_RED " ------------------------------------------------\n");
    printf(COLOR_RED "  STATUS WORD error \n" COLOR_RESET);
    printf("    Expected SW: ");

    while (1) {
        printf(COLOR_WHITE "%.4s" COLOR_RESET, &SWexp[idx]);
        idx += 4;

        if (SWexp[idx]) {
            printf(" or ");
        }
        else break;
    }

    printf("\n");
    printf("    Received SW: " COLOR_RED "%s\n" COLOR_RESET, sw);

    errors.status++;
}

static void print_data_error(const char* _data)
{
    int idx = 0;

    printf(COLOR_RED " ------------------------------------------------\n");
    printf(COLOR_RED "  DATA error \n" COLOR_RESET);
    printf("    Expected DATA: " COLOR_WHITE "%s\n" COLOR_RESET, DATAexp);
    printf("    Received DATA: " COLOR_RED "%s\n" COLOR_RESET, _data);

    errors.data++;
}

static int check_SW(uint8_t* _sw)
{
    char SWs[5];
    char* sw;
    int digits;

    if (*SWexp == 0x00)
        return 0;

    convert_bin2hex(_sw, (uint8_t* )SWs, 2);

    SWs[4] = 0;

    sw = SWexp;
    while (*sw) {
        digits = 0;

        for (int j = 0; j < 4; j++) {
            if ((sw[j] == 'X') || (sw[j] == SWs[j])) {
                digits++;
            }
        }
        if (digits == 4)
            return 0;

        sw += 4;
    }

    print_sw_error(SWs);
    return -1;
}

static int check_DATA(uint8_t* _data, int _len)
{
    char DATAcard[513];
    char* dig_exp;
    char* dig_card;

    if (*DATAexp == 0x00)
        return 0;

    convert_bin2hex(_data, (uint8_t*)DATAcard, _len);

    DATAcard[_len * 2] = 0;

    dig_exp = DATAexp;
    dig_card = DATAcard;

    while (*dig_exp) {
        if ((*dig_exp != 'X') && (*dig_exp != *dig_card)) {

            print_data_error(DATAcard);
            return -1;
        }

        dig_exp++;
        dig_card++;
    }

    return 0;
}

static void finishExecution(clock_t duration)
{
    int tm_hour, tm_min, tm_sec, tm_ms;

#ifdef __APPLE__
    duration /= 10;
#endif
    tm_ms = duration % 1000;
    duration /= 1000;
    tm_sec = duration % 60;
    duration /= 60;
    tm_min = duration % 60;
    duration /= 60;
    tm_hour = duration;

    if (errors_count() == 0) {
        printf(COLOR_GREEN " ------------------------------------------------\n" COLOR_RESET);
        printf(COLOR_GREEN "  Script executed successfully\n");
        printf(COLOR_GREEN " ------------------------------------------------\n" COLOR_RESET);
    }
    else
    {
        printf(COLOR_RED " ------------------------------------------------\n");
        printf("  Errors occured during execution: \n" COLOR_RESET);

        if (errors.comm > 0)
            printf(COLOR_WHITE " % d COMMunication error(s)\n", errors.comm);
        if (errors.status > 0)
            printf(COLOR_WHITE " % d STATUS error(s)\n", errors.status);
        if (errors.syntax > 0)
            printf(COLOR_WHITE " % d SYNTAX error(s)\n", errors.syntax);
        if (errors.data > 0)
            printf(COLOR_WHITE " % d DATA error(s)\n", errors.data);
        printf(COLOR_RED " ------------------------------------------------\n" COLOR_RESET);
    }

    printf("  Script execution time:");
    printf(COLOR_CYAN " %02d:%02d:%02d.%03d\n" COLOR_RESET, tm_hour, tm_min, tm_sec, tm_ms);
}

void execute_PCOM(const char* _filename)
{
    FILE* fc;
    int i = 1;
    char fileline[1024];
    clock_t start, stop;

    if (pcsc_listreaders() != PCSC_SUCCESS) {
        errors.comm++;
        return;
    }

    if ((fc = fopen(_filename, "r")) == NULL) {
        printf("Cannot open PCOM file %s\n", _filename);
        return;
    }

    printf("%s ------------------------------------------------%s\n", COLOR_YELLOW, COLOR_RESET);
    clear_all();

    start = clock();
    while (fgets(fileline, sizeof(fileline), fc) != NULL) {
        printf(COLOR_CYAN "%.4d" COLOR_RESET " : %s\n", i++, fileline);

        if (execute_OneLine(fileline) < 0)
            break;
    }
    stop = clock();

    fclose(fc);

    finishExecution(stop-start);
}

int execute_OneLine(const char* _fileline)
{
    char cmd[2048];
    int count = 0;
    int i;
    int len;

    SWexp[0] = 0x00; // By default no check expected
    DATAexp[0] = 0x00;

#ifdef _WIN32
    strcpy_s(cmd, sizeof(cmd), _fileline);
#endif
#ifdef __APPLE__
    strcpy(cmd, _fileline);
#endif

    if (*PartString) {
        // Insert previous line..
        char line[2048];
        int len_1;
        int len_2;

        len_2 = (int)strlen(cmd);
        memcpy(line, cmd, len_2);

        len_1 = (int)strlen(PartString);
        memcpy(cmd, PartString, len_1);

        memcpy(&cmd[len_1], line, len_2);
        cmd[len_1 + len_2] = 0;

        PartString[0] = 0;
    }

    // 1. a) Lower case
    //    b) Replace TABs
    //    c) And cut the string
    len = (int)strlen(cmd);
    for (i = 0; i < len; i++) {
        cmd[i] = tolower(cmd[i]);

        switch (cmd[i]) {
        case 0x09:
            cmd[i] = 0x20;
            break;

        case '(':
            count++;
            break;
        case ')':
            count--;
            break;

        // Remove the tails: comment or CR or LF
        case 0x0A:
        case 0x0D:
        case '#':
        case ';':
        case '*':
            if (0 == count) {
                cmd[i] = 0x00;
                i = len;
            }
            break;
        case '\\':
            cmd[i] = 0;
            memcpy(PartString, cmd, i + 1);
            return 0;
        } // switch

    }

    // 2. remove leading SPACEs
    while (0x20 == *cmd) {
        leftshift(cmd);
    }

    // 3.a Directives
    if ('.' == *cmd) {
        return proceed_Directives(cmd);
    }

    if (0 == *cmd)
        return 0;

    replace_defines_and_buffers(cmd);

    i = 0;
    while (cmd[i] != 0) {
        switch (cmd[i]) {
        case 0x09:
        case 0x20:
        case 0x0A:
        case 0x0D:
            leftshift(&cmd[i]);
            continue;

        case '(':
        {
            int j = 1;
            int len = 0;
            char* sw = &cmd[i];

            while ( (sw[j] != 0x00) && (sw[j] != ')') ) {
                if ( (sw[j] != ',') && (sw[j] != ' ') )
                    SWexp[len++] = toupper(sw[j]); // SW verification requires UPPER case
                j++;
            }
            SWexp[len] = 0x00;

            if (len % 4) {
                print_syntax_error(cmd, i);
                return -1;
            }
        }
            cmd[i] = 0;
            continue;

        case '[':
            {
                int j = 1;
                int len = 0;
                char* data = &cmd[i];

                while ((data[j] != 0x00) && (data[j] != ']')) {
                    if (data[j] != ' ')
                        DATAexp[len++] = toupper(data[j]); // DATA verification requires UPPER case

                    switch (data[j]) {
                    case '(':
                    case ')':
                    case '[':
                    case 0x00:
                    case 0x0D:
                    case 0x0A:
                        print_syntax_error(cmd, i + j);
                        return -1;
                    }
                    j++;
                }
                DATAexp[len] = 0x00;

                leftshift_n(&cmd[i], j + 2);

                if (len % 2) {
                    print_syntax_error(cmd, i);
                    return -1;
                }
            }
            continue;
        } // switch

        if ((cmd[i] >= 'a') && (cmd[i] <= 'f')) {
            i++;
            continue;
        }
        if ((cmd[i] >= 'A') && (cmd[i] <= 'F')) {
            i++;
            continue;
        }
        if ((cmd[i] >= '0') && (cmd[i] <= '9')) {
            i++;
            continue;
        }


        print_syntax_error(cmd, i);
        return -1;

    } // while

    if (0 == *cmd)
        return 0;

    if (strlen(cmd) % 2) {
        // The APDU string must contain even number of hex chars

        print_syntax_error(cmd, i);
        return -1;
    }


    {
        apdu_t apdu;
//        uint8_t apdu[255 + 5] = { 0, 0, 0, 0, 0 };
//        uint8_t response[258];
//        uint16_t response_length;
//        uint8_t lc;
//        uint8_t le;
//        uint8_t with_le;
        int err_data, err_sw;

        i = 0;
        while (cmd[i * 2] != 0) {
            apdu.cmd[i] = byte_from_hex_str(&cmd[i * 2]);
            i++;
        }
        apdu.cmd_len = i;

        pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

        set_Buffer(BUFF_W, &apdu.resp[apdu.resp_len - 2], 2);
        set_Buffer(BUFF_R, apdu.resp, apdu.resp_len - 2);

        
        err_data = check_DATA(apdu.resp, apdu.resp_len - 2);
        err_sw = check_SW(&apdu.resp[apdu.resp_len - 2]);

        if ( (err_data < 0) || (err_sw < 0) ) {
            return -1;
        }
    }

    return 0;
}
