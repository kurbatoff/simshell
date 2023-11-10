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

#ifndef __SCP03_H_
#define __SCP03_H_

#include <stdint.h>
#include <stdbool.h>

#define SECURE_CHANNEL_PROTOCOL_03				0x03

 /**
  * SCP03 derivation constants
  */
#define SCP03_DERIVE_CARD_CRYPTOGRAMM		0x00
#define SCP03_DERIVE_HOST_CRYPTOGRAMM		0x01

#define SCP03_DERIVE_S_ENC					0x04
#define SCP03_DERIVE_S_MAC					0x06
#define SCP03_DERIVE_S_RMAC					0x07

#if defined(__cplusplus)
extern "C" {
#endif

void scp03_initialize_update();
void scp03_external_authenticate(void);

void scp03_calculate_apdu_cmac(void);
bool scp03_decrypt_cdata(void);

//void scp03_begin_rmac_session(void);
//void scp03_end_rmac_session(void);

void scp03_calculate_cryptogram(uint8_t* key, uint8_t derivationConstant, uint8_t* context1, uint16_t context1_len,
uint8_t* context2, uint16_t context2_len, uint8_t* cryptogram, uint8_t cryptogram_size);

#if defined(__cplusplus)
}
#endif

#endif /* __SCP03_H_ */
