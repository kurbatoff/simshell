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

#include <stdint.h>
#include <string.h>

#include "hal_des.h"
#include "scp02.h"
#include "securechannel.h"

/**
 *
 *
 */
void scp02_generate_session_key(uint8_t* _cardchallenge, uint8_t* _enckey, uint8_t* _mackey, uint8_t* _dekkey)
{
	uint8_t buffer[32];
	uint8_t iv[8];

	// Prepare session key generation
	memset(&buffer[14], 0x00, 16);
	buffer[16] = _cardchallenge[0];
	buffer[17] = _cardchallenge[1];

	// Generate session keys: ENC-Key
	buffer[14] = SCP02_DERIVATION_DATA_ENC_KEY >> 8;
	buffer[15] = SCP02_DERIVATION_DATA_ENC_KEY & 0xFF;

	memset(iv, 0x00, 8);
	hal_DES128_CBC_crypt(&buffer[14], 16, iv, _enckey, buf_session_ENC, DES_ENCRYPT);

	// Generate session keys: CMAC-Key
	buffer[14] = SCP02_DERIVATION_DATA_CMAC_KEY >> 8;
	buffer[15] = SCP02_DERIVATION_DATA_CMAC_KEY & 0xFF;

	memset(iv, 0x00, 8);
	hal_DES128_CBC_crypt(&buffer[14], 16, iv, _mackey, buf_session_MAC, DES_ENCRYPT);

	// Generate session keys: DEK-Key
	buffer[14] = SCP02_DERIVATION_DATA_DENC_KEY >> 8;
	buffer[15] = SCP02_DERIVATION_DATA_DENC_KEY & 0xFF;

	memset(iv, 0x00, 8);
	hal_DES128_CBC_crypt(&buffer[14], 16, iv, _dekkey, buf_session_DEK, DES_ENCRYPT);

}

void scp02_generate_cryptogramms(uint8_t* _hostchallenge, uint8_t* _cardchallenge, uint8_t* _hostcryptogramm, uint8_t* _cardcryptogramm)
{
	uint8_t buffer[48];
	uint8_t iv[16];

	// Generate CARD cryptogramm
	// |Host Challenge[8]|Sequence Counter[2]|Card Challenge[6]|DES padding[8]|
	memcpy(buffer, _hostchallenge, 8);
	memcpy(&buffer[8], _cardchallenge, 8);
	buffer[16] = 0x80;
	memset(&buffer[17], 0x00, 7);

	memset(iv, 0x00, 8);
	hal_DES128_CBC_crypt(buffer, 24, iv, buf_session_ENC, &buffer[24], DES_ENCRYPT);
	memcpy(_cardcryptogramm, &buffer[40], 8);

	// Calculate HOST cryptogram
	// |Sequence Counter[2]|Card Challenge[6]|Host Challenge[8]|DES padding[8]|
	memcpy(buffer, _cardchallenge, 8);
	memcpy(&buffer[8], _hostchallenge, 8);
	buffer[16] = 0x80;
	memset(&buffer[17], 0x00, 7);

	memset(iv, 0x00, 8);
	hal_DES128_CBC_crypt(buffer, 24, iv, buf_session_ENC, &buffer[24], DES_ENCRYPT);
	memcpy(_hostcryptogramm, &buffer[40], 8);
}

/**
 *
 *
 */
void scp02_initialize_update()
{
}

/**
 *
 *
 */
void scp02_external_authenticate(void)
{
}

/**
 * ВНИМАНИЕ!
 *   буфер _cmd будет ИЗМЕНЕН, за длиной gCMDlen появится PADDING 0x8000..
 *
 */
void scp02_calculate_c_mac(uint8_t* _cmd, int gCMDlen, uint8_t* _key, uint8_t* _mac)
{
	for (int i=0; i<8; i++) {
		if (_mac[i] != 0x00) {
			hal_DES64_ECB_crypt(_mac, 8, _key, _mac, DES_ENCRYPT);
			break;
		}
	}

	calculate_EMV_mac(_cmd, gCMDlen, _mac, _mac);
}

void calculate_EMV_mac(uint8_t* in_buff, int len, uint8_t* iv, uint8_t* out_buff)
{
	uint8_t block_idx;
	uint8_t j;
	uint8_t x[8];

	in_buff[len++] = 0x80;
	while ((len % 8) > 0) {
		in_buff[len++] = 0x00;
	}

	block_idx = 0;
	while (block_idx*8 < (len-8)) {

		for (j=0; j<8; j++) {
			iv[j] ^= in_buff[block_idx * 8 + j];
		}

		//hal_encrypt_DES64_ECB_block(iv, iv, buf_session_MAC);
		hal_DES64_ECB_crypt(iv, 8, buf_session_MAC, x, DES_ENCRYPT);
		memcpy(iv, x, 8);

		block_idx++;
	}

	for (j=0; j<8; j++) {
		iv[j] ^= in_buff[len - 8 + j];
	}

	hal_DES128_ECB_crypt(iv, 8, buf_session_MAC, out_buff, DES_ENCRYPT);
}

bool scp02_decrypt_cdata(void)
{

	return false;
}
