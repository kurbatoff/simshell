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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define COLOR_RESET      "\033[0m"
#define COLOR_BLACK      "\033[30m"
#define COLOR_RED        "\033[31m"
#define COLOR_GREEN      "\033[32m"
#define COLOR_YELLOW     "\033[33m"
#define COLOR_BLUE       "\033[34m"
#define COLOR_MAGENTA    "\033[35m"
#define COLOR_CYAN       "\033[36m"
#define COLOR_GRAY       "\033[37m"
#define COLOR_DARK_GRAY  "\033[90m"
#define COLOR_RED_I      "\033[91m"
#define COLOR_GREEN_I    "\033[92m"
#define COLOR_YELLOW_I   "\033[93m"
#define COLOR_BLUE_I     "\033[94m"
#define COLOR_MAGENTA_I  "\033[95m"
#define COLOR_CYAN_I     "\033[96m"
#define COLOR_WHITE      "\033[97m"

uint16_t calculate_crc16(uint8_t* Data, uint16_t Len);
void parse_asn1_signature(uint8_t* src, uint8_t* dest);
int construct_asn1_signature(uint8_t* src_bin, uint8_t* dest_asn1);


uint8_t byte2nimble(uint8_t b);
uint8_t byte_from_hex_str(const char *hex_string);
void convert_hex2bin(const char* src, uint8_t* dst, uint16_t dest_len);
void convert_bin2hex(const uint8_t* src_bin, uint8_t* dest_hex, int src_len);

void change_endian(uint8_t* buffer, int length);
void shift_r(uint8_t* buffer, int length);
void dump_hexascii_buffer(const char* name, const uint8_t* recvbuf, size_t len);
void print_bin2hex(uint8_t* buffer_bin, size_t len);

uint16_t print_taglen(uint8_t* buff, uint16_t* length);
uint16_t print_tlv(uint8_t* buff);
uint8_t BERTLV_set_length(uint8_t* buffer, uint8_t len);

size_t delete_symbol(uint8_t* buffer, size_t len, uint8_t symb);

#if defined(__cplusplus)
}
#endif

#endif /* __TOOLS_H__ */
