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

#include "keys.h"
#include "gp.h"
#include "tools.h"

#define SYMMETRIC_KEYSET_COUNT		8

uint8_t DEFAULT_VISA_OP_KEY[] = {
	0x40, 0x41, 0x42, 0x43,  0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B,  0x4C, 0x4D, 0x4E, 0x4F
};

sym_keyset_t KEYs[SYMMETRIC_KEYSET_COUNT];

/**
 * 
 * @brief Initialize KEYs context
 *
 */
void init_keys(void)
{
	for (int i = 0; i < SYMMETRIC_KEYSET_COUNT; i++)
		memset(&KEYs[i], 0, sizeof(sym_keyset_t));

	KEYs[0].status = 1;
	KEYs[0].kvn = 0xFF;
	KEYs[0].type = KEY_TYPE_DES; // KEY_TYPE_DES or KEY_TYPE_AES
	KEYs[0].keylen = 128; // 128 or 192 or 256

	memcpy(KEYs[0].enc, DEFAULT_VISA_OP_KEY, 16);
	memcpy(KEYs[0].mac, DEFAULT_VISA_OP_KEY, 16);
	memcpy(KEYs[0].dek, DEFAULT_VISA_OP_KEY, 16);
}

sym_keyset_t* find_keyset(int kvn)
{
	sym_keyset_t* key = NULL;

	for (int i = 0; i < SYMMETRIC_KEYSET_COUNT; i++)
		if (KEYs[i].kvn == kvn) {
			key = &KEYs[i];
			break;
		}

	return key;
}

static void set_one_key(int kvn, uint8_t idx, uint8_t type, uint8_t keylen, uint8_t* keyvalue)
{
	sym_keyset_t* key = NULL;

	for (int i = 0; i < SYMMETRIC_KEYSET_COUNT; i++)
		if (KEYs[i].kvn == kvn) {
			key = &KEYs[i];
			break;
		}

	if (key == NULL)
	{
		for (int i = 0; i < SYMMETRIC_KEYSET_COUNT; i++)
			if (KEYs[i].status == 0) {
				key = &KEYs[i];
				break;
			}
	}

	if (key != NULL)
	{
		key->status = 1;
		key->kvn = kvn;
		key->type = type;
		key->keylen = keylen * 8;

		if (idx == 1)
			memcpy(key->enc, keyvalue, keylen);
		if (idx == 2)
			memcpy(key->mac, keyvalue, keylen);
		if (idx == 3)
			memcpy(key->dek, keyvalue, keylen);
	}
}

/**
 * @brief set-key callback function
 *
 * @param _cmd: command line string
 */
void cmd_setkey(char* _cmd)
{
	sym_keyset_t* key;

	if (strlen(_cmd) == 7) {
		// No arguments: print all defined keys

		for (int i = 0; i < SYMMETRIC_KEYSET_COUNT; i++) {
			key = &KEYs[i];
			if (key->status) {
				printf(" Keyset version = 0x%02X (%d)\n", key->kvn, key->kvn);
				printf("   Key length   = %d\n", key->keylen);
				printf("   Keyset type:   %s\n", key->type == KEY_TYPE_DES ? "DES" : "AES");

				printf("   ENC key: ");
				for (int j = 0; j < key->keylen / 8; j++)
					printf("%02X", key->enc[j]);
				printf("\n");

				printf("   MAC key: ");
				for (int j = 0; j < key->keylen / 8; j++)
					printf("%02X", key->mac[j]);
				printf("\n");

				printf("   DEK key: ");
				for (int j = 0; j < key->keylen / 8; j++)
					printf("%02X", key->dek[j]);
				printf("\n");
			}
		}
		printf("\n");

		return;
	}

	//set-key 255/1/DES-ECB/29e06ac5a8882b09300e6dc4d11e7f1d 255/2/DES-ECB/7b858d366900ddf9c41289240d788149 255/3/DES-ECB/3889fbe36d14110a42ba0019964d192d
	//set-key 32/1/DES-ECB/404142434445464748494A4B4C4D4E4F 32/2/DES-ECB/404142434445464748494A4B4C4D4E4F 32/3/DES-ECB/404142434445464748494A4B4C4D4E4F
	//set-key 48/1/AES/404142434445464748494A4B4C4D4E4F 48/2/AES/404142434445464748494A4B4C4D4E4F 48/3/AES/404142434445464748494A4B4C4D4E4F

	_cmd += 8; // skip until KVN

	while ((*_cmd != ' ') && (*_cmd != '\t') && (*_cmd != '\n') && (*_cmd != 0x00))
	{
		int kvn = 0;
		uint8_t idx;
		uint8_t type = 0;
		uint8_t keylen = 0;
		uint8_t keyvalue[32];

		while (*_cmd != '/') {
			kvn *= 10;
			kvn += (*_cmd++ - 0x30);
		}
		_cmd++; // skip '/'

		idx = *_cmd++ - 0x30;
		_cmd++; // skip '/'

		if (memcmp(_cmd, "DES-ECB", 7) == 0) {
			type = KEY_TYPE_DES;
			_cmd += 8;
		} else
		if (memcmp(_cmd, "AES", 3) == 0) {
			type = KEY_TYPE_AES;
			_cmd += 4;
		}

		if (type == 0) {
			printf(" Wrong key data..\n");
			return;
		}

		while ((*_cmd != ' ') && (*_cmd != '\t') && (*_cmd != '\n') && (*_cmd != 0x00))
		{
			keyvalue[keylen++] = byte_from_hex_str(_cmd);
			_cmd += 2;
		}

		while ((*_cmd == ' ') || (*_cmd == '\t') )
		{
			_cmd++;
		}

		set_one_key(kvn, idx, type, keylen, keyvalue);
	}
}
