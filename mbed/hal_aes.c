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

#include <string.h>

#include "hal_aes.h"
#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES128_crypt_block( uint8_t* data_buffer, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;

	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		//AES128_ECB_encrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_enc( &actx, key_buffer, 128 );
		mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_ENCRYPT, data_buffer, Out_Buffer );
	}
	else {
		//AES128_ECB_decrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_dec( &actx, key_buffer, 128 );
		mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_DECRYPT, data_buffer, Out_Buffer );
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES128_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	uint16_t offset = 0;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		//AES128_ECB_encrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_enc( &actx, key_buffer, 128 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_ENCRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	else {
		//AES128_ECB_decrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_dec( &actx, key_buffer, 128 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_DECRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES192_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	uint16_t offset = 0;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		//AES128_ECB_encrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_enc( &actx, key_buffer, 192 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_ENCRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	else {
		//AES128_ECB_decrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_dec( &actx, key_buffer, 192 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_DECRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES256_crypt_ECB( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	uint16_t offset = 0;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		//AES128_ECB_encrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_enc( &actx, key_buffer, 256 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_ENCRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	else {
		//AES128_ECB_decrypt(data_buffer, key_buffer, Out_Buffer);
		mbedtls_aes_setkey_dec( &actx, key_buffer, 256 );
		while( offset < len ) {
			mbedtls_aes_crypt_ecb( &actx, MBEDTLS_AES_DECRYPT, &data_buffer[ offset ], &Out_Buffer[ offset ] );
			offset += 16;
		}
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES128_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		mbedtls_aes_setkey_enc( &actx, key_buffer, 128 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_ENCRYPT, len, iv, data_buffer, Out_Buffer );
	}
	else {
		mbedtls_aes_setkey_dec( &actx, key_buffer, 128 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_DECRYPT, len, iv, data_buffer, Out_Buffer );
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES192_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		mbedtls_aes_setkey_enc( &actx, key_buffer, 192 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_ENCRYPT, len, iv, data_buffer, Out_Buffer );
	}
	else {
		mbedtls_aes_setkey_dec( &actx, key_buffer, 192 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_DECRYPT, len, iv, data_buffer, Out_Buffer );
	}
	mbedtls_aes_free( &actx );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_AES256_crypt_CBC( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_aes_context actx;
	mbedtls_aes_init( &actx );

	if( AES_ENCRYPT == Mode ) {
		mbedtls_aes_setkey_enc( &actx, key_buffer, 256 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_ENCRYPT, len, iv, data_buffer, Out_Buffer );
	}
	else {
		mbedtls_aes_setkey_dec( &actx, key_buffer, 256 );
		mbedtls_aes_crypt_cbc( &actx, MBEDTLS_AES_DECRYPT, len, iv, data_buffer, Out_Buffer );
	}
	mbedtls_aes_free( &actx );
}

void hal_AES_CMAC( uint8_t* data_buffer, uint16_t length, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer )
{
	const mbedtls_cipher_info_t* cipher_info;
	cipher_info = mbedtls_cipher_info_from_type( MBEDTLS_CIPHER_AES_128_ECB );
	//AES_CMAC
	mbedtls_cipher_cmac( cipher_info, key_buffer, 128, data_buffer, length, Out_Buffer );
}
