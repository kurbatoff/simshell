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
#include <stdio.h>
#include <stdlib.h>

#include "mbedwrap.h"
#include "tools.h"

#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/cmac.h"


#define ecp_clear_precomputed( g )

int generate_random(void *rng_state, uint8_t *output, size_t len)
{
    if (rng_state != NULL) {
        rng_state  = NULL;
    }

	while (len--) {
		*output++ = (uint8_t) (rand() & 0xFF);
	}
    return 0;
}

void mbedtls_compute_public_keys(int curve_id, const uint8_t SK_buff[M2M_ECC_SECRET_KEY_LEN], uint8_t PK_buff[M2M_ECC_PUBLIC_KEY_LEN])
{
	mbedtls_mpi privatekey;
	mbedtls_ecp_point publickkey;
	int olen;
	char X[128]; // 65 is not enough !! ?? only 66 works
	char Y[128];

	mbedtls_ecp_group ecgroup;

	mbedtls_ecp_group_init(&ecgroup);
	mbedtls_ecp_group_load(&ecgroup, (mbedtls_ecp_group_id)curve_id);

	mbedtls_mpi_init(&privatekey);
	mbedtls_mpi_read_binary(&privatekey, SK_buff, M2M_ECC_SECRET_KEY_LEN);
	mbedtls_ecp_point_init(&publickkey);

	dump_hexascii_buffer("Private key:", SK_buff, M2M_ECC_SECRET_KEY_LEN);

	// calculate the public key
	mbedtls_ecp_mul(&ecgroup, &publickkey, &privatekey, &ecgroup.G, NULL, NULL);

	printf("Public key\n");

	mbedtls_mpi_write_string((const mbedtls_mpi*)&publickkey.X, 16, X, sizeof(X), (size_t*)&olen);
	printf(" X (%d): %s\n", olen, X);

	mbedtls_mpi_write_string((const mbedtls_mpi*)&publickkey.Y, 16, Y, sizeof(Y), (size_t*)&olen);
	printf(" Y (%d): %s\n", olen, Y);

	PK_buff[0] = 0x04;
	mbedtls_mpi_write_binary((const mbedtls_mpi*)&publickkey.X, &PK_buff[1], M2M_ECC_SECRET_KEY_LEN);
	mbedtls_mpi_write_binary((const mbedtls_mpi*)&publickkey.Y, &PK_buff[1 + M2M_ECC_SECRET_KEY_LEN], M2M_ECC_SECRET_KEY_LEN);

	dump_hexascii_buffer("Public key:", PK_buff, M2M_ECC_PUBLIC_KEY_LEN);

	printf(" ~~~\n");
}

void mbedtls_compute_ecdh_sharedsecret(int curve_id, uint8_t* SK_buff, uint8_t* PK_buff, uint8_t* shs)
{
    int ret;
    mbedtls_ecdh_context ctx_srv;

	mbedtls_ecdh_init( &ctx_srv );

#if PRINT_DEBUG_DATA
	dump_hexascii_buffer(" Secret key: ", SK_buff, M2M_ECC_SECRET_KEY_LEN);
	dump_hexascii_buffer(" Public key: ", PK_buff, M2M_ECC_PUBLIC_KEY_LEN);
#endif


    ret = mbedtls_ecp_group_load( &ctx_srv.grp, (mbedtls_ecp_group_id)curve_id);
    if ( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET " ! mbedtls_ecp_group_load returned %d\n", ret);
#endif
    }

    //
    // Server: read peer's key and generate shared secret
    //
    ret = mbedtls_mpi_lset( &ctx_srv.Qp.Z, 1);
    if ( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET "  ! mbedtls_mpi_lset returned %d\n", ret );
#endif
    }

    ret = mbedtls_mpi_read_binary( &ctx_srv.d, SK_buff, 0x20);
    if ( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET "  ! mbedtls_mpi_lset returned %d\n", ret );
#endif
    }

	ret = mbedtls_mpi_read_binary( &ctx_srv.Qp.X, &PK_buff[1], 0x20);
    if( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET " ! mbedtls_mpi_read_binary returned %d\n", ret );
#endif
    }

    ret = mbedtls_mpi_read_binary( &ctx_srv.Qp.Y, &PK_buff[0x21], 0x20);
    if( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET " ! mbedtls_mpi_read_binary returned %d\n", ret );
#endif
    }

    ret = mbedtls_ecdh_compute_shared( &ctx_srv.grp, &ctx_srv.z,
                                       &ctx_srv.Qp, &ctx_srv.d,
                                       generate_random, NULL);
    if( ret != 0 )
    {
#if PRINT_DEBUG_DATA
        printf(COLOR_RED " failed\n" COLOR_RESET " ! mbedtls_ecdh_compute_shared returned %d\n", ret );
#endif
    }


	mbedtls_mpi_write_binary((const mbedtls_mpi* )&ctx_srv.z, (unsigned char*)shs, M2M_ECC_SHARED_SECRET_LEN);

#if PRINT_DEBUG_DATA
	dump_hexascii_buffer(COLOR_GREEN " mbed ShS: " COLOR_RESET, shs, M2M_ECC_SHARED_SECRET_LEN);
#endif
}

void mbedtls_generate_ecc_keypair(int curve_id, uint8_t* eSK, uint8_t* ePK)
{
	mbedtls_mpi privatekey;
	mbedtls_ecp_point publickkey;
    
	mbedtls_ecp_group ecgroup;

	generate_random(NULL, eSK, M2M_ECC_SECRET_KEY_LEN);

	mbedtls_ecp_group_init(&ecgroup);
	mbedtls_ecp_group_load(&ecgroup, (mbedtls_ecp_group_id)curve_id);

    mbedtls_mpi_init(&privatekey);
    mbedtls_mpi_read_binary(&privatekey, eSK, M2M_ECC_SECRET_KEY_LEN);
    mbedtls_ecp_point_init(&publickkey);

#if PRINT_DEBUG_DATA
	dump_hexascii_buffer("Private key", eSK, M2M_ECC_SECRET_KEY_LEN);
#endif

    // calculate the public key
	mbedtls_ecp_mul(&ecgroup, &publickkey, &privatekey, &ecgroup.G, NULL, NULL);

	ePK[0] = 0x04;
	mbedtls_mpi_write_binary((const mbedtls_mpi* )&publickkey.X, &ePK[1], M2M_ECC_SECRET_KEY_LEN);
	mbedtls_mpi_write_binary((const mbedtls_mpi* )&publickkey.Y, &ePK[1 + M2M_ECC_SECRET_KEY_LEN], M2M_ECC_SECRET_KEY_LEN);

#if PRINT_DEBUG_DATA
    printf("Public key:\n");
	dump_hexascii_buffer("eSK.SD", eSK, 0x20);
	dump_hexascii_buffer("ePK.SD", ePK, 0x41);
	printf(" ~~~\n");
#endif
}

void mbedtls_create_ecdsa_signature(int curve_id, const uint8_t secretkey[M2M_ECC_SECRET_KEY_LEN], const uint8_t* data_buff, int len, const uint8_t* precomputed_hash, uint8_t signature_buff[M2M_ECC_RS_SIGNATURE_LEN])
{
	int ret_sign;
	//int sign_len = 0;
	unsigned char hash[MBED_SHA256_DIGEST_LENGTH];

	mbedtls_ecdsa_context ecdsa;
	mbedtls_mpi mpir;
	mbedtls_mpi mpis;


#if PRINT_DEBUG_DATA
	printf(" --- sign -----------------------------------------------------\n");
	printf("  Input data len = %d\n", len);
	dump_hexascii_buffer("Input data", (const unsigned char*)data_buff, len);
	dump_hexascii_buffer("Key", (const unsigned char*)secretkey, M2M_ECC_SECRET_KEY_LEN);
#endif

	mbedtls_ecdsa_init(&ecdsa);
	mbedtls_ecp_group_init(&ecdsa.grp);
	mbedtls_ecp_group_load(&ecdsa.grp, (mbedtls_ecp_group_id)curve_id); // MBEDTLS_ECP_DP_SECP256R1

	mbedtls_mpi_init(&mpir);
	mbedtls_mpi_init(&mpis);

	// set SK, PK
	mbedtls_mpi_read_binary(&ecdsa.d, (const unsigned char*)secretkey, M2M_ECC_SECRET_KEY_LEN);


#if PRINT_DEBUG_DATA
	{
		// DEBUG: print input SK, calculated PK: X, Y
		char XX[128];
		int olen;

		mbedtls_ecp_mul(&ecdsa.grp, &ecdsa.Q, &ecdsa.d, &ecdsa.grp.G, NULL, NULL);

		mbedtls_mpi_write_string((const mbedtls_mpi*)&ecdsa.d, 16, XX, sizeof(XX), (size_t*)&olen);
		printf(" SK (%d): %s\n", olen, XX);

		mbedtls_mpi_write_string((const mbedtls_mpi*)&ecdsa.Q.X, 16, XX, sizeof(XX), (size_t*)&olen);
		printf(" PK X (%d): %s\n", olen, XX);

		mbedtls_mpi_write_string((const mbedtls_mpi*)&ecdsa.Q.Y, 16, XX, sizeof(XX), (size_t*)&olen);
		printf(" PK Y (%d): %s\n", olen, XX);
	}
#endif

	if (precomputed_hash != NULL) {
		memcpy(hash, precomputed_hash, MBED_SHA256_DIGEST_LENGTH);
	} else {
		mbedtls_sha256(data_buff, len, hash, 0);
	}

//	ret_sign = mbedtls_ecdsa_sign_det(&ecdsa.grp, &mpir, &mpis, (const mbedtls_mpi*)&ecdsa.d,
//		hash, sizeof(hash), MBEDTLS_MD_SHA256);
	ret_sign = mbedtls_ecdsa_sign_det_ext(&ecdsa.grp, &mpir, &mpis, (const mbedtls_mpi*)&ecdsa.d,
		hash, sizeof(hash), MBEDTLS_MD_SHA256,
		generate_random,
		NULL
		);

#if PRINT_DEBUG_DATA
	printf("Sign result: %d\n", ret_sign);
#endif

	{
		mbedtls_mpi_write_binary((const mbedtls_mpi*)&mpir, signature_buff, M2M_ECC_SECRET_KEY_LEN);
		mbedtls_mpi_write_binary((const mbedtls_mpi*)&mpis, &signature_buff[M2M_ECC_SECRET_KEY_LEN], M2M_ECC_SECRET_KEY_LEN);
	}

/*
		ret_write_sign = mbedtls_ecdsa_write_signature(&ecdsa, MBEDTLS_MD_SHA256,
			hash, sizeof(hash),
			sign_asn1, (size_t*)&sign_len, myrand, NULL);


#if PRINT_DEBUG_DATA
	printf(" ASN.1 len = %d\n", sign_len);
	print_hex("ASN.1 signature", sign_asn1, sign_len, 0);
#endif

	// Parse ASN.1 and extract signature S, R total 0x40 bytes
	parse_asn1_signature(sign_asn1, signature_buff);
*/

#if PRINT_DEBUG_DATA
	printf("Plain signature\n");
	dump_hexascii_buffer("  Hash", hash, sizeof(hash));
	dump_hexascii_buffer("  R", signature_buff, M2M_ECC_RS_SIGNATURE_LEN / 2);
	dump_hexascii_buffer("  S", &signature_buff[M2M_ECC_RS_SIGNATURE_LEN / 2], M2M_ECC_RS_SIGNATURE_LEN / 2);
#endif
}

/**
* Verifies ECDSA signature
* @param curve_id [in] Elliptic Curve ID
* @param const uint8_t* publickey [in]
* @param const uint8_t* data_buff [in]
* @param int data_len [in]
* @param uint8_t signature_buff[] [out]
*
* @return 0 on success, other value on error
*/
int mbedtls_verify_ecdsa_signature(int curve_id, const uint8_t publickey[M2M_ECC_PUBLIC_KEY_LEN], const uint8_t* data_buff, int data_len, const uint8_t* precomputed_hash, uint8_t signature_buff[M2M_ECC_RS_SIGNATURE_LEN])
{
	int ret_verify;
    unsigned char hash[MBED_SHA256_DIGEST_LENGTH];

	mbedtls_ecdsa_context ecdsa;

#if PRINT_DEBUG_DATA
	printf(" --- verify -----------------------------------------------------\n");
	printf("  Input data len = %d\n", data_len);
	dump_hexascii_buffer("Input data", (const unsigned char* )data_buff, data_len);
	dump_hexascii_buffer("ASN1. Signature", (const unsigned char* )signature_buff, 2+signature_buff[1]);
	dump_hexascii_buffer("Public key", (const unsigned char* )publickey, M2M_ECC_PUBLIC_KEY_LEN);
#endif

	mbedtls_ecdsa_init(&ecdsa);

	ret_verify = mbedtls_ecp_group_load(&ecdsa.grp, (mbedtls_ecp_group_id)curve_id); // MBEDTLS_ECP_DP_SECP256R1
	if (ret_verify != 0) {
		printf("mbedtls_ecp_group_load = %04X\n", 0x10000 - (0xFFFF & ret_verify));
	}

    mbedtls_ecp_point_init(&ecdsa.Q);

	// set PK
	ret_verify = mbedtls_mpi_read_binary((mbedtls_mpi* )&ecdsa.Q.X, &publickey[1], M2M_ECC_SECRET_KEY_LEN);
	if (ret_verify != 0) {
		printf("mbedtls_mpi_read_binary1 = %04X\n", 0x10000 - (0xFFFF & ret_verify));
	}

	ret_verify = mbedtls_mpi_read_binary((mbedtls_mpi* )&ecdsa.Q.Y, &publickey[1+M2M_ECC_SECRET_KEY_LEN], M2M_ECC_SECRET_KEY_LEN);
	if (ret_verify != 0) {
		printf("mbedtls_mpi_read_binary2 = %04X\n", 0x10000 - (0xFFFF & ret_verify));
	}

	hash[0] = 0x01;
	ret_verify = mbedtls_mpi_read_binary((mbedtls_mpi* )&ecdsa.Q.Z, &hash[0], 1);

#if PRINT_DEBUG_DATA
	{
		// DEBUG: print input SK, calculated PK: X, Y
		char XX[128];
		int olen;

		mbedtls_mpi_write_string((const mbedtls_mpi* )&ecdsa.Q.X, 16, XX, sizeof(XX), (size_t* )&olen);
		printf(" PK X (%d): %s\n", olen, XX);

		mbedtls_mpi_write_string((const mbedtls_mpi* )&ecdsa.Q.Y, 16, XX, sizeof(XX), (size_t* )&olen);
		printf(" PK Y (%d): %s\n", olen, XX);

		mbedtls_mpi_write_string((const mbedtls_mpi* )&ecdsa.Q.Z, 16, XX, sizeof(XX), (size_t* )&olen);
		printf(" PK Z (%d): %s\n", olen, XX);
	}
#endif

	if (precomputed_hash != NULL) {
		memcpy(hash, precomputed_hash, MBED_SHA256_DIGEST_LENGTH);
	} else {
		mbedtls_sha256(data_buff, data_len, hash, 0);
	}

	ret_verify = mbedtls_ecdsa_read_signature(&ecdsa, hash, sizeof( hash ), signature_buff, 2 + signature_buff[1]);

#if PRINT_DEBUG_DATA
	if (ret_verify != 0x00) {
		printf("ret_verify = %08X\n", ret_verify);
		printf("error code = %04X\n", 0x10000 - (0xFFFF & ret_verify));
	} else {
		printf("Verification OK\n");
	}
#endif

	return ret_verify;
}

/**
* Calculates a message digest SHA256
* @param *message [in] The message to calculate the SHA256 digest for
* @param messageLength [in] The message length
* @param mac [out] The calculated digest
*
* @return none
* /
void mbedtls_calculate_sha256(const uint8_t* message, int messageLength, uint8_t hash[MBED_SHA256_DIGEST_LENGTH])
{
mbedtls_sha256_context sha256_ctx;

mbedtls_sha256_init( &sha256_ctx);
mbedtls_sha256_starts( &sha256_ctx, 0);
mbedtls_sha256_update( &sha256_ctx, message, messageLength);
mbedtls_sha256_finish( &sha256_ctx, hash);

#if PRINT_DEBUG_DATA
print_hex("Hash: ", (const unsigned char *)hash, 32, 1);
#endif
}
// */

/**
 * Calculates a message authentication code, using AES-128 in CBC mode. This is the algorithm specified in NIST 800-38B
 * @param key [in] The AES-128 key to use
 * @param *message [in] The message to calculate the MAC for
 * @param messageLength [in] The message length
 * @param mac [out] The calculated MAC
 *
 * @return none
 */
void calculate_CMAC_aes(uint8_t key[16], uint8_t* message, int messageLength, uint8_t cmac[16])
{
	mbedtls_cipher_context_t pCtx;

	mbedtls_cipher_cmac_starts(&pCtx, key, 128);
	mbedtls_cipher_cmac_update(&pCtx, message, messageLength);
	mbedtls_cipher_cmac_finish(&pCtx, cmac);

	return;
}


