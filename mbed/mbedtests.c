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

#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"

#include "mbedtests.h"
#include "mbedwrap.h"
#include "tools.h"

 /*
	SGP.22 - RSP Technical Specification
	---
	2.6.7 Elliptic Curves Algorithms
	2.6.7.1 Domain Parameters
	In order to facilitate interoperability, this specification is limited 
	to the three following curves (similar as SGP.02 [2]):
		* NIST P-256, defined in Digital Signature Standard [29] (recommended by NIST)
		* brainpoolP256r1, defined in RFC 5639 [18] (recommended by BSI)
		* FRP256V1, defined in ANSSI ECC [20] (recommended by ANSSI)
*/


	
static void mbedtls_simulate_scp11(void);
static void mbedtls_sign_verify(void);

static void test_dsa(void);

// ------------------------------------------------------------------------------
/*
 * Nice little macro to save a few lines.
 */
static void handleErrors(char *reason)
{
	printf(COLOR_RED "\n\n ---\n ERROR during ECC\n\n" COLOR_RESET);

	printf("%s/n", reason);
	
	while(1);
}

int ecc_main(void)
{
//	uint8_t sig_R[32];
//	uint8_t sig_S[32];

    unsigned char hash[] = "c7fbca202a95a570285e3d700eb04ca2";
	int status;

	const char* eSK_OCE_ECKA_str = "3018D631638E980547291C4F9723F49DF98B30F5CF8E01B3DDA167ED932C1FDF";
	const char* ePK_OCE_ECKA_str = "041B2F56D1156B4550A4BF044403E7DAC4C3F11A441F22844832DE752133F0C10343C386ECA245F78E54CD4419C6547122A7B52CC21C2910E19E07A536CA754E2F";
	const char* eSK_SD_ECKA_str = "94A9E5D3AF5A6E35A1D87726D5DA6C2CB72775C5FDD24C0604E4DE56FF6090A4";
	const char* ePK_SD_ECKA_str = "04D0FB48EFDB8EAF8C2729DE794CA570B2BCEBF4CC87AD993FF986FEC8A46B5031BC82B5B6BE3BE0277B2BCA88E64D27FAB67B1E26031CD7F45F4A288AA91C456E";
	const char* secretkey_ETH = "00d862c318d05de0a1c25242c21989e15e35e70c55996fbc4238cd2f2f6a8f62";
//	const char* publikkey_str = "048E72B73E15267F6793B9347BEAC23A6F1AE4C1D550A65752C8099177B6E47D3CA388D32C795FF325A1A2A0BFEDD7D7231DBA75390FCAC85ED25454BBDCD57F33";
//	const char publikkey_ETH[64];// = "048E72B73E15267F6793B9347BEAC23A6F1AE4C1D550A65752C8099177B6E47D3CA388D32C795FF325A1A2A0BFEDD7D7231DBA75390FCAC85ED25454BBDCD57F33";


	unsigned char dataplain[] = "EB80850BA43B7400825208947917bc33eea648809c285607579c9919fb864f8f8703BAF82D03A00080018080";
	const char* signature_str = "067940651530790861714b2e8fd8b080361d1ada048189000c07a66848afde4669b041db7c29dbcc6becf42017ca7ac086b12bd53ec8ee494596f790fb6a0a69";
	const char* hash_keccak = "a4060d01d4add248db470b4121616cbe5b2015daf328809000ec9a1d0954d649";

	uint8_t eSK_OCE_ECKA[M2M_ECC_SECRET_KEY_LEN];
	uint8_t ePK_OCE_ECKA[M2M_ECC_PUBLIC_KEY_LEN];
	uint8_t eSK_SD_ECKA[M2M_ECC_SECRET_KEY_LEN];
	uint8_t ePK_SD_ECKA[M2M_ECC_PUBLIC_KEY_LEN];
	uint8_t secretkey_buff[M2M_ECC_SECRET_KEY_LEN];
	uint8_t publickey_buff[M2M_ECC_PUBLIC_KEY_LEN];
	uint8_t signature_buff[M2M_ECC_PUBLIC_KEY_LEN];
	uint8_t signature_asn1[M2M_ECC_ASN1_SIGNATURE_MAXLEN];
	uint8_t data_buff[1024];
	int data_len = 0;

	convert_hex2bin(eSK_OCE_ECKA_str, eSK_OCE_ECKA, M2M_ECC_SECRET_KEY_LEN);
	convert_hex2bin(ePK_OCE_ECKA_str, ePK_OCE_ECKA, M2M_ECC_PUBLIC_KEY_LEN);
	convert_hex2bin(eSK_SD_ECKA_str, eSK_SD_ECKA, M2M_ECC_SECRET_KEY_LEN);
	convert_hex2bin(ePK_SD_ECKA_str, ePK_SD_ECKA, M2M_ECC_PUBLIC_KEY_LEN);
	//convert_hex2bin(publikkey_ETH, publickey_buff, M2M_ECC_PUBLIC_KEY_LEN);
	convert_hex2bin(secretkey_ETH, secretkey_buff, M2M_ECC_SECRET_KEY_LEN);

	// --- 1 ----------------------------------------
	//*
	{
		uint8_t secret[256];

		printf(" --- 1 --- Random data --------------------------------------\n");
		generate_random(NULL, secret, 32);
		dump_hexascii_buffer("Random data", secret, 32);
		printf(" --- done (rand) --\n\n");
	}
	// */

	// 2. ----------------------------------------
	/*
	printf(" --- 2 --- SCP11 --------------------------------------\n");
	mbedtls_simulate_scp11();
	printf(" -- done (SCP11) ---\n\n");
	// */

	// 3. ----------------------------------------
	/*
	{
		printf(" --- 3. ECDH ShS ----------------------------------------\n");
		uint8_t secret1[32];
		uint8_t secret2[32];

		mbedtls_compute_ecdh_sharedsecret(MBEDTLS_ECP_DP_SECP256R1, eSK_OCE_ECKA, ePK_SD_ECKA, secret1);
		mbedtls_compute_ecdh_sharedsecret(MBEDTLS_ECP_DP_SECP256R1, eSK_SD_ECKA, ePK_OCE_ECKA, secret2);

		if (memcmp(secret1, secret2, sizeof(secret1)) != 0) {
			printf(COLOR_RED " ECDH ShS generation failed!" COLOR_RESET);
		}
		else {
			printf(COLOR_GREEN " ECDH ShS generation PASSED\n" COLOR_RESET);
		}
		printf(" --- Done: ECDH ShS ---\n\n");
	}
	// */

	// 4.

	printf("\n\n 4. --- MBED ----------------------------------------\n");
	mbedtls_compute_public_keys(MBEDTLS_ECP_DP_SECP256R1, eSK_SD_ECKA, ePK_SD_ECKA);
	mbedtls_compute_public_keys(MBEDTLS_ECP_DP_BP256R1, eSK_SD_ECKA, ePK_SD_ECKA);
	mbedtls_compute_public_keys(MBEDTLS_ECP_DP_SECP256K1, secretkey_buff, publickey_buff);

	printf(" -- Done: public keys --\n\n");

	// 5.
	printf("\n\n 5. ----------------------------------------\n");
	//mbedtls_sign_verify();

	convert_hex2bin(secretkey_ETH, secretkey_buff, M2M_ECC_SECRET_KEY_LEN);
	mbedtls_compute_public_keys(MBEDTLS_ECP_DP_SECP256K1, secretkey_buff, publickey_buff);

	data_len = (int)(strlen((const char* )dataplain) / 2);
	convert_hex2bin((const char* )dataplain, data_buff, data_len);
	//memcpy(data_buff, dataplain, data_len);

	/*
		// sample ETH TX
		// https://lsongnotes.wordpress.com/2018/01/14/signing-an-ethereum-transaction-the-hard-way/

	const char* secretkey_ETH = "00d862c318d05de0a1c25242c21989e15e35e70c55996fbc4238cd2f2f6a8f62";
	unsigned char dataplain[] = "EB80850BA43B7400825208947917bc33eea648809c285607579c9919fb864f8f8703BAF82D03A00080018080";
	const char* hash_keccak = "a4060d01d4add248db470b4121616cbe5b2015daf328809000ec9a1d0954d649";
	const char* signature_str = "067940651530790861714b2e8fd8b080361d1ada048189000c07a66848afde4669b041db7c29dbcc6becf42017ca7ac086b12bd53ec8ee494596f790fb6a0a69";
	// PUBLIC key: f037ceb1a1993ac55296db1fc8765c8c72380d3a96e711960703a2379ed6f0426bc6473f754c77e5878cacdad124db72eff486dad933df8cc5ec97acb26144ae

	
	*/

	mbedtls_create_ecdsa_signature(MBEDTLS_ECP_DP_SECP256K1, secretkey_buff, data_buff, data_len, NULL, signature_buff);

	convert_hex2bin(signature_str, signature_buff, M2M_ECC_RS_SIGNATURE_LEN);
	construct_asn1_signature(signature_buff, signature_asn1);
	convert_hex2bin(hash_keccak, hash, MBED_SHA256_DIGEST_LENGTH);


	status = mbedtls_verify_ecdsa_signature(MBEDTLS_ECP_DP_SECP256K1, publickey_buff, data_buff, data_len, hash, signature_asn1);

	// 6.
	//printf("\n\n 6. --- test DSA -------------------------------------\n");
	//test_dsa();

	// 7.
	//printf("\n\n 7. --- test TUAK -------------------------------------\n");
	//test_tuak();


	/*
	printf("\n\nPress ENTER to exit\n");
	if ( 0x0A != (getchar()) ) {
		return 0x00;
	} else {
		return 0xAA;
	}
	// */

	return 0;
}


static void mbedtls_simulate_scp11(void)
{
	unsigned char shss[32];
	unsigned char sha[32];
	unsigned char sha_input[71];
	unsigned char receipt_key[16];
	unsigned char enc_key[16];
	unsigned char mac_key[16];
	unsigned char rmac_key[16];
	unsigned char dek_key[16];
	unsigned char SK_SD_ECKA[M2M_ECC_SECRET_KEY_LEN];
	unsigned char eSK_OCE_ECKA[M2M_ECC_SECRET_KEY_LEN];
	unsigned char ePK_SD_ECKA[M2M_ECC_PUBLIC_KEY_LEN];
	unsigned char PK_OCE_ECKA[M2M_ECC_PUBLIC_KEY_LEN];

	// data for  ShSs
    const char* SK_SD_ECKA_str = "A1CE38B2FDDE7F0BFA8FD09CF8784BE813C18701EACDFC45FB190AB898A7E8AE";
	const char* PK_OCE_ECKA_str = "04397835E8012825A13862BB20071A4E118529D14413DB4A90289CBBFA0EE641B04149C41D8425109591C8A9457E7D8624989CF9CBEB24A9751DE646E23257E823";
	
	// data for ShSe
	const char* eSK_OCE_ECKA_str = "3BE44C7D24B0A39C6D88696BB59E705D940BF7122E5944A4F9CD7BA005FFB6CC";
	//const char* ePK_OCE_ECKA = "04736EED0974F46EC373E234F488DF7F110A31B8D93892D5796654BF74595692A869D838B6BE7F2378C4C270364F9C702FC357BA02FBFCEA8AFE86B362E1C73F99";
	
	//const char* eSK_SD_ECKA = "???";
	const char* ePK_SD_ECKA_str = "04618DC59E98833490541C18900C8F0918462B3767FC0E93586A3B279F938730AB7178F4311DC8BFB087DC5199123945DCEF525D6BD7633FEE414837622424C0E3";

	// len 302
	const char* receipt_data_str = "A60D900211019501348001888101105F494104736EED0974F46EC373E234F488DF7F110A31B8D93892D5796654BF74595692A869D838B6BE7F2378C4C270364F9C702FC357BA02FBFCEA8AFE86B362E1C73F995F494104618DC59E98833490541C18900C8F0918462B3767FC0E93586A3B279F938730AB7178F4311DC8BFB087DC5199123945DCEF525D6BD7633FEE414837622424C0E3";
	unsigned char rec_data[151];

	mbedtls_generate_ecc_keypair(MBEDTLS_ECP_DP_SECP256R1, eSK_OCE_ECKA, ePK_SD_ECKA);

	convert_hex2bin(SK_SD_ECKA_str, SK_SD_ECKA, M2M_ECC_SECRET_KEY_LEN);
	convert_hex2bin(eSK_OCE_ECKA_str, eSK_OCE_ECKA, M2M_ECC_SECRET_KEY_LEN);
	convert_hex2bin(PK_OCE_ECKA_str, PK_OCE_ECKA, M2M_ECC_PUBLIC_KEY_LEN);
	convert_hex2bin(ePK_SD_ECKA_str, ePK_SD_ECKA, M2M_ECC_PUBLIC_KEY_LEN);


	mbedtls_compute_ecdh_sharedsecret(MBEDTLS_ECP_DP_SECP256R1, SK_SD_ECKA, PK_OCE_ECKA, shss);
	mbedtls_compute_ecdh_sharedsecret(MBEDTLS_ECP_DP_SECP256R1, eSK_OCE_ECKA, ePK_SD_ECKA, sha_input);

	memcpy(&sha_input[32], shss, 32);
	sha_input[64] = 0;
	sha_input[65] = 0;
	sha_input[66] = 0;
	sha_input[67] = 1;
	sha_input[68] = 0x34;
	sha_input[69] = 0x88;
	sha_input[70] = 0x10;

	dump_hexascii_buffer("SHA input", sha_input, 71);

	// round 1
	mbedtls_sha256(sha_input, 71, sha, 0);

	memcpy(receipt_key, sha, 16);
	memcpy(enc_key, &sha[16], 16);

	sha_input[67]++;

	// round 2
	mbedtls_sha256(sha_input, 71, sha, 0);

	memcpy(mac_key, sha, 16);
	memcpy(rmac_key, &sha[16], 16);

	sha_input[67]++;

	// round 3
	mbedtls_sha256(sha_input, 71, sha, 0);

	memcpy(dek_key, sha, 16);

	sha_input[67]++;

	dump_hexascii_buffer("RECEIPT key", receipt_key, 16);
	dump_hexascii_buffer("ENC key", enc_key, 16);
	dump_hexascii_buffer("MAC key", mac_key, 16);
	dump_hexascii_buffer("RMAC key", rmac_key, 16);
	dump_hexascii_buffer("DEK key", dek_key, 16);

	// calculate RECEIPT
	convert_hex2bin(receipt_data_str, rec_data, 151);
	
	calculate_CMAC_aes(receipt_key, rec_data, 151, sha);
	dump_hexascii_buffer("RECEIPT: ", sha, 16);

	// The receipt key shall be deleted after calculating the receipt
}

static void mbedtls_sign_verify(void)
{
	uint8_t signature_bin[M2M_ECC_RS_SIGNATURE_LEN];
	uint8_t asn1_signature_bin[M2M_ECC_ASN1_SIGNATURE_MAXLEN];

	const char* data = "12345678";
	int data_len;
	const char* SK_ECDSA = "94A9E5D3AF5A6E35A1D87726D5DA6C2CB72775C5FDD24C0604E4DE56FF6090A4";
	const char* PK_ECDSA = "04D0FB48EFDB8EAF8C2729DE794CA570B2BCEBF4CC87AD993FF986FEC8A46B5031BC82B5B6BE3BE0277B2BCA88E64D27FAB67B1E26031CD7F45F4A288AA91C456E";

	uint8_t data_bin[10240]; // TODO fix length
	uint8_t secretkey_bin[M2M_ECC_SECRET_KEY_LEN];
	uint8_t publickey_bin[M2M_ECC_PUBLIC_KEY_LEN];

	// Convert to BIN
	data_len = (int)(strlen(data) / 2);

	convert_hex2bin(data, data_bin, data_len);
	convert_hex2bin(SK_ECDSA, secretkey_bin, M2M_ECC_SECRET_KEY_LEN);

		
	mbedtls_create_ecdsa_signature(MBEDTLS_ECP_DP_SECP256R1, (const uint8_t*)secretkey_bin, data_bin, data_len, NULL, signature_bin);

	asn1_signature_bin[0] = 0x30;
	asn1_signature_bin[1] = 0x44;
	asn1_signature_bin[2] = 0x02;
	asn1_signature_bin[3] = 0x20;
	memcpy(&asn1_signature_bin[4], signature_bin, 0x20);

	asn1_signature_bin[0x20 + 4] = 0x02;
	asn1_signature_bin[0x20 + 5] = 0x20;
	memcpy(&asn1_signature_bin[0x20 + 6], &signature_bin[0x20], 0x20);

	convert_hex2bin(PK_ECDSA, publickey_bin, M2M_ECC_PUBLIC_KEY_LEN);

	mbedtls_verify_ecdsa_signature(MBEDTLS_ECP_DP_SECP256R1, publickey_bin, data_bin, data_len, NULL, asn1_signature_bin);

	printf(" -- Done: mbedtls CREATE, openssl VERIFY --\n\n");
}

static void test_dsa(void)
{
	static uint8_t message[1024];
	static uint16_t msg_len;
	static uint8_t ecurve = MBEDTLS_ECP_DP_SECP256R1;
	static uint8_t hash[32];
	static uint8_t sig[MBEDTLS_ECDSA_MAX_LEN];
	static size_t sig_len;
	static mbedtls_ecdsa_context ctx_sign, ctx_verify;

    int ok = -1;

	message[0] = 0x12;
	message[1] = 0x34;
	message[2] = 0x56;
	message[3] = 0x78;
	msg_len = 4;

    mbedtls_ecdsa_init(&ctx_sign);
    mbedtls_ecdsa_init(&ctx_verify);

/*
    if (mbedtls_ecdsa_genkey(&ctx_sign, (mbedtls_ecp_group_id)ecurve, myrand, NULL) == 0) {
        if (mbedtls_sha256_ret(message, msg_len, hash, 0) == 0)
			if (mbedtls_ecdsa_write_signature(&ctx_sign, MBEDTLS_MD_SHA256, hash, sizeof(hash), sig, &sig_len, myrand, NULL) == 0)
				if (mbedtls_ecp_group_copy(&ctx_verify.grp, &ctx_sign.grp) == 0)
					if (mbedtls_ecp_copy(&ctx_verify.Q, &ctx_sign.Q) == 0)
						if (mbedtls_ecdsa_read_signature(&ctx_verify, hash, sizeof(hash), sig, sig_len) == 0)
							ok = 0;
	}

	printf(" Verification result: %d\n", ok);
*/

    mbedtls_ecdsa_free(&ctx_verify);
    mbedtls_ecdsa_free(&ctx_sign);
}
