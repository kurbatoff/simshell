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

#include "hal_aes.h"
#include "scp03.h"
#include "securechannel.h"

void scp03_initialize_update()
{

}

void scp03_external_authenticate(void)
{

}

 /**
 * This procedure calculates command MAC signature
 */
void scp03_calculate_apdu_cmac(void)
{
}

bool scp03_decrypt_cdata(void)
{

	return true;
}

void scp03_calculate_cryptogram(uint8_t* key, uint8_t derivationConstant, uint8_t* context1, uint16_t context1_len, 
		uint8_t* context2, uint16_t context2_len, uint8_t* cryptogram, uint8_t cryptogram_size) 
{
	uint8_t msg_buffer[48];
	uint8_t buffer_mac[16];
	uint8_t iv[16];

	memset(msg_buffer, 0, 48);
	memset(iv, 0, 16);

	msg_buffer[11] = derivationConstant; // "derivation constant" part of label
	msg_buffer[12] = 0x00; // "separation indicator"
	msg_buffer[13] = 0x00; // First byte of output length
	msg_buffer[14] = cryptogram_size; // Second byte of output length
	msg_buffer[15] = 0x01; // byte counter "i"

	memcpy(&msg_buffer[16], context1, context1_len);
	memcpy(&msg_buffer[16 + context1_len], context2, context2_len);

	hal_AES_CMAC(msg_buffer, (16 + context1_len + context2_len), iv, key, buffer_mac);

	memcpy(cryptogram, buffer_mac, cryptogram_size/8);
}

/*
void scp03_begin_rmac_session(void)
{
	resp_len = 0;

	if ( (0x10 != P1) && (0x30 != P1) ) {
		set_sw(SW_INCORRECT_P1P2);
		return;
	}

	if (0x01 != P2) {
		set_sw(SW_INCORRECT_P1P2);
		return;
	}

	set_sw(SW_OK);
}

void scp03_end_rmac_session(void)
{
	resp_len = 0;

	if (0x00 != P1) {
		set_sw(SW_INCORRECT_P1P2);
		return;
	}

	if (0x03 != P2) {
		set_sw(SW_INCORRECT_P1P2);
		return;
	}

//	set_sw(SW_OK);
}
*/
