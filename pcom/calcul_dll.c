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

#include <stdbool.h>
#include "calcul_dll.h"

#include "pcom_buffers.h"
#include "tools.h"

#include "mbedtls/des.h"

#define CALCUL_DLL_LOADED	0xA5A5
#define CALCUL_DLL_OFF		0x5050

uint8_t key[32];
uint8_t data[1024];
uint8_t cipher[1024];
uint8_t iv[16];
uint16_t data_len;

uint16_t load_status = CALCUL_DLL_OFF;

void static clear_dll(void)
{
	memset(key, 0, sizeof(key));
	memset(data, 0, sizeof(data));
	memset(cipher, 0, sizeof(cipher));
	memset(iv, 0, sizeof(iv));
}

void load_calcul_dll(void)
{
	clear_dll();
	load_status = CALCUL_DLL_LOADED;
}

void unload_calcul_dll(void)
{
	clear_dll();
	load_status = CALCUL_DLL_OFF;
}

bool is_calcul_loaded(void)
{
	return (CALCUL_DLL_LOADED == load_status);
}

void set_data(char* str)
{
	data_len = (uint16_t)strlen(str);
	data_len /= 2;

	if (data_len > sizeof(data))
		data_len = sizeof(data);

	convert_hex2bin(str, data, data_len);
}

void set_key(char* str)
{
	uint16_t len = (uint16_t)strlen(str);
	len /= 2;

	if (len > sizeof(key))
		len = sizeof(key);

	convert_hex2bin(str, key, len);
}

void set_init_vec(char* str)
{
	uint16_t len = (uint16_t)strlen(str);
	len /= 2;

	if (len > sizeof(iv))
		len = sizeof(iv);

	convert_hex2bin(str, iv, len);
}

void des3k(char* str)
{
	uint16_t offset;
	int dest_buf_idx = *str - BUFFER_ZERO;
	uint8_t padd = byte_from_hex_str(&str[2]);

	printf(" Processing " COLOR_GREEN "DES3K:" COLOR_RESET " DES ECB with 3 keys..\n");
	uint8_t* data_buffer = data;
	uint8_t* Out_Buffer = cipher;

	mbedtls_des3_context d3c;

	mbedtls_des3_set3key_enc(&d3c, key);

	if (data_len % 8) {
		// Padding with Par2 byte; then ZEROs
		data[data_len++] = padd;

		while (data_len % 8)
			data[data_len++] = 0x00;
	}


	//mbedtls_des3_crypt_ecb(&d3c, MBEDTLS_DES_ENCRYPT, data_len, iv, data, cipher);

	offset = 0;
	while (offset < data_len) {
		mbedtls_des3_crypt_ecb(&d3c, data_buffer, Out_Buffer);

		data_buffer += 8;
		Out_Buffer += 8;
		offset += 8;
	}

	set_Buffer(dest_buf_idx, cipher, data_len);
}

