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

#include "hal_des.h"
#include "mbedtls/des.h"

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES64_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des_context dc;
	uint16_t offset = 0;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des_setkey_enc( &dc, key_buffer );
	}
	else {
		mbedtls_des_setkey_dec( &dc, key_buffer );
	}

	while( offset < len ) {
		mbedtls_des_crypt_ecb( &dc, &data_buffer[ offset ], &Out_Buffer[ offset ] );
		offset += 8;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES64_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des_context dc;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des_setkey_enc( &dc, key_buffer );
	}
	else {
		mbedtls_des_setkey_dec( &dc, key_buffer );
	}
	mbedtls_des_crypt_cbc( &dc, Mode, len, iv, data_buffer, Out_Buffer );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES128_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des3_context d3c;
	uint16_t offset;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des3_set2key_enc( &d3c, key_buffer );
	}
	else {
		mbedtls_des3_set2key_dec( &d3c, key_buffer );
	}

	offset = 0;
	while( offset < len ) {
		mbedtls_des3_crypt_ecb( &d3c, data_buffer, Out_Buffer );

		data_buffer += 8;
		Out_Buffer += 8;
		offset += 8;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES128_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des3_context d3c;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des3_set2key_enc( &d3c, key_buffer );
	}
	else {
		mbedtls_des3_set2key_dec( &d3c, key_buffer );
	}
	mbedtls_des3_crypt_cbc( &d3c, Mode, len, iv, data_buffer, Out_Buffer );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES192_ECB_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des3_context d3c;
	uint16_t offset;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des3_set3key_enc( &d3c, key_buffer );
	}
	else {
		mbedtls_des3_set3key_dec( &d3c, key_buffer );
	}

	offset = 0;
	while( offset < len ) {
		mbedtls_des3_crypt_ecb( &d3c, data_buffer, Out_Buffer );

		data_buffer += 8;
		Out_Buffer += 8;
		offset += 8;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES192_CBC_crypt( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t* Out_Buffer, uint8_t Mode )
{
	mbedtls_des3_context d3c;

	if( MBEDTLS_DES_ENCRYPT == Mode ) {
		mbedtls_des3_set3key_enc( &d3c, key_buffer );
	}
	else {
		mbedtls_des3_set3key_dec( &d3c, key_buffer );
	}
	mbedtls_des3_crypt_cbc( &d3c, Mode, len, iv, data_buffer, Out_Buffer );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES64_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] )
{
	mbedtls_des_context dc;

	mbedtls_des_setkey_enc( &dc, key_buffer );
	mbedtls_des_crypt_cbc( &dc, MBEDTLS_DES_ENCRYPT, len, iv, data_buffer, data_buffer );

	memcpy( checksum_buffer, &data_buffer[ len - 8 ], 8 );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES128_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] )
{
	mbedtls_des3_context d3c;

	mbedtls_des3_set2key_enc( &d3c, key_buffer );
	mbedtls_des3_crypt_cbc( &d3c, MBEDTLS_DES_ENCRYPT, len, iv, data_buffer, data_buffer );

	memcpy( checksum_buffer, &data_buffer[ len - 8 ], 8 );
}

// ---------------------------------------------------------------------------------------------------------------------
void hal_DES192_signature( uint8_t* data_buffer, uint16_t len, uint8_t* iv, uint8_t* key_buffer, uint8_t checksum_buffer[ 8 ] )
{
	mbedtls_des3_context d3c;

	mbedtls_des3_set3key_enc( &d3c, key_buffer );
	mbedtls_des3_crypt_cbc( &d3c, MBEDTLS_DES_ENCRYPT, len, iv, data_buffer, data_buffer );

	memcpy( checksum_buffer, &data_buffer[ len - 8 ], 8 );
}
