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

#ifndef HAL_AES_H_E393F0E735026057
#define HAL_AES_H_E393F0E735026057

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AES_ENCRYPT 1
#define AES_DECRYPT 0

#define AES_KEYSIZE_128 0 // If key size is 128 bits
#define AES_KEYSIZE_192 1 // If key size is 192 bits
#define AES_KEYSIZE_256 2 // If key size is 256 bits

void hal_AES128_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_AES192_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_AES256_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_AES128_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_AES192_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );
void hal_AES256_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode );

void hal_AES_CMAC( uint8_t* data_buffer, uint8_t length, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer );


#ifdef __cplusplus
}
#endif

#endif // Header guard
