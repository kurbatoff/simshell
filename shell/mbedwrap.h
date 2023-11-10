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

#ifndef __MBEDWRAP_H__
#define __MBEDWRAP_H__

#include <stdint.h>

#include "mbedtls\ecp.h"

#define M2M_ECC_SECRET_KEY_LEN			0x20
#define M2M_ECC_PUBLIC_KEY_LEN			(1 + 2 * M2M_ECC_SECRET_KEY_LEN)
#define M2M_ECC_RS_SIGNATURE_LEN		0x40
#define M2M_ECC_ASN1_SIGNATURE_MAXLEN		0x48
#define M2M_ECC_SHARED_SECRET_LEN		0x20
#define MBED_SHA256_DIGEST_LENGTH		0x20 /* SHA256_DIGEST_LENGTH */


#define PRINT_DEBUG_DATA			1

int myrand(void *rng_state, uint8_t *output, size_t len);

void mbedtls_compute_public_keys(int curve_id, const uint8_t SK_buff[M2M_ECC_SECRET_KEY_LEN], uint8_t PK_buff[M2M_ECC_PUBLIC_KEY_LEN]);
void mbedtls_generate_ecc_keypair(int curve_id, uint8_t* eSK, uint8_t* ePK);

void mbedtls_create_ecdsa_signature(int curve_id, const uint8_t secretkey[M2M_ECC_SECRET_KEY_LEN], const uint8_t* data_buff, int len, const uint8_t* precomputed_hash, uint8_t signature_buff[M2M_ECC_RS_SIGNATURE_LEN]);
int mbedtls_verify_ecdsa_signature(int curve_id, const uint8_t publickey[M2M_ECC_PUBLIC_KEY_LEN], const uint8_t* data_buff, int data_len, const uint8_t* precomputed_hash, uint8_t signature_buff[M2M_ECC_RS_SIGNATURE_LEN]);

void mbedtls_compute_ecdh_sharedsecret(int curve_id, uint8_t* SK_buff, uint8_t* PK_buff, uint8_t* shs);

//void mbedtls_calculate_sha256(const uint8_t* message, int messageLength, uint8_t hash[MBED_SHA256_DIGEST_LENGTH]);

#endif /* __MBEDWRAP_H__ */
