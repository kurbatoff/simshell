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

#ifndef __SCP02_H_
#define __SCP02_H_

#include <stdint.h>
#include <stdbool.h>

#define SECURE_CHANNEL_PROTOCOL_02		0x02

 /**
  * SCP02 derivation constants
  */
#define SCP02_DERIVATION_DATA_ENC_KEY		0x0182
#define SCP02_DERIVATION_DATA_CMAC_KEY		0x0101
#define SCP02_DERIVATION_DATA_DENC_KEY		0x0181
#define SCP02_DERIVATION_DATA_RMAC_KEY		0x0102


#if defined(__cplusplus)
extern "C" {
#endif

void scp02_generate_session_key(
	uint8_t* _cardchallenge,
	uint8_t* _enckey, uint8_t* _mackey, uint8_t* _dekkey);

void scp02_generate_cryptogramms(
	uint8_t* _hostchallenge, uint8_t* _cardchallenge,
	uint8_t* _hostcryptogramm, uint8_t* _cardcryptogramm);

void calculate_EMV_mac(uint8_t* in_buff, int len, uint8_t* iv, uint8_t* out_buff);
void scp02_calculate_c_mac(uint8_t* _cmd, int gCMDlen, uint8_t* _key, uint8_t* _mac);


void scp02_initialize_update();
void scp02_external_authenticate(void);
bool scp02_decrypt_cdata(void);

#if defined(__cplusplus)
}
#endif

#endif /* __SCP02_H_ */
