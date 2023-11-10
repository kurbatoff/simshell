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

#ifndef HAL_DES_H_DDB726AABF81C4F1
#define HAL_DES_H_DDB726AABF81C4F1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DES_DECRYPT 0
#define DES_ENCRYPT 1

void hal_DES64_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_DES64_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_DES128_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_DES128_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_DES192_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_DES192_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );

void hal_DES64_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] );
void hal_DES128_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] );
void hal_DES192_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] );


#ifdef __cplusplus
}
#endif

#endif // Header guard
