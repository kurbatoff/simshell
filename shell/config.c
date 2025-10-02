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

/*
	GSMA Live CI		81370F5125D0B1D408D4C3B232E6D25E795BEBFB
	GSMA test CI:		F54172BDF98A95D65CBEB88A38A1C11D800A85C3
	Idemia Mconnect: 	066D48A537D97191C7394ADC3DEC6519D1B0BF89
	DIGICERT Test CI:	665A1433D67C1A2C5DB8B52C967F10A057BA5CB2
	R2F test CI:		AE3F69C323330D5BD51B3CEFF49F683FAC019B47
*/

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "config.h"
#include "tools.h"

#ifdef _WIN32
	#include <windows.h>
#endif
#if defined (__APPLE__) || defined(__linux__) || defined(linux) || defined(__linux) || defined(__gnu_linux__)
	void strcpy_s(char* dest, size_t destsize, const char* src)
	{
		if (strlen(src) < destsize) {
			strcpy(dest, src);
		}
	}
#endif

char serialPort[32];

char QR_MID[128];
char QR_Address[128];
char QR_Port[8] = "80";

char eIM_Address[128];
char eIM_Port[8];

//uint8_t LocI_IMEI[32];
uint8_t LocI_NMR_UMTS[512];
uint8_t LocI_NMR_LTE[512];
uint8_t LocI_AT;

//static void read_ENC(char* src);
//static void read_MAC(char* src);

static void read_MatchingID(char* src);
static void read_InetAddress(char* src, char* dest, size_t destsize);
static void read_InetPort(char* src, char* dest, size_t destsize);
static void read_SerialPort(char* src, char* dest, size_t destsize);


static void dump_eUICC_Info1(uint8_t* s);
static void dump_Server_Signed1(uint8_t* s);
static void dump_Server_Signature1(uint8_t* s);
//
static void dump_AuthenticateServerResponse(uint8_t* src);
static void dump_profileMetadata(uint8_t* src);
static void dump_smdpSigned2(uint8_t* src);
static void dump_smdpSignature2(uint8_t* src);
//
static void dump_boundProfilePackage(uint8_t* src);


unsigned char config_name[256]; // exefile name but extension '.conf'

static void set_config_name(const char* exename)
{
	strcpy_s((char*)config_name, sizeof(config_name), exename);
	size_t len = strlen((const char*)config_name);

#ifdef _WIN32
	len -= 4; // to remove ".exe"
#endif

	config_name[len++] = '.';
	config_name[len++] = 'c';
	config_name[len++] = 'o';
	config_name[len++] = 'n';
	config_name[len++] = 'f';
	config_name[len++] = 0x00;
}

void print_hex(char* s_, unsigned char* bufhex_, int len_)
{
  printf("%s", s_);
  for (int i=0; i<len_; i++) printf("%.02X", bufhex_[i]); // note that signed char is printed as 8 hex signs
  printf("\n");
}

unsigned char hex_2_char(unsigned char* s)
{
  char c=0;
  unsigned char us = toupper(s[0]);
  if ( (us>=0x30) && (us<=0x39) ) // 0...9
    c = (us-0x30) * 16;
  if ( (us>=0x41) && (us<=0x46) ) // A...F
    c = (us-0x37) * 16;

  us = toupper(s[1]);
  if ( (us>=0x30) && (us<=0x39) ) // 0...9
    c += (us-0x30);
  if ( (us>=0x41) && (us<=0x46) ) // A...F
    c += (us-0x37);

  return c;
}

int str_remove_spaces(char* buf)
{
  int i = 0, len = 0;
  while (buf[i]) {
    if (buf[i] != 0x20)
      buf[len++] = buf[i];
    i++;
  }
  buf[len] = 0x00;
  return len;
}

int str_piece(char* src, char* dest, char del, int idx)
{
  if (idx==0) {
    *dest=0x00;
    return 0;
  }

  int del_count = 0, len = 0;
  while (*src) {
    if (*src == del) {
      del_count++;
      src++;
      continue;
    }

    if (del_count >= idx) break;

    if (del_count == (idx-1)) {
      dest[len++] = *src;
    }
    src++;
  }
  dest[len] = 0x00;
  return len;
}

void str_trunc(char* buf)
{
  int len, i, j;
  len = (int)strlen(buf);
  for (i=0; i<len; i++) {
    if (buf[i] == 0x09) buf[i]=0x20;  // change TABs to SPACEs
    if (buf[i] == 0x0D) buf[i]=0x20;  // change CRs to SPACEs
    if (buf[i] == 0x0A) buf[i]=0x20;  // change LFs to SPACEs
  }

  // remove all DOUBLE spaces
  i=0;
  while (buf[i] && buf[i+1]) {
    if ( (buf[i]==0x20) && (buf[i+1]==0x20) ) {
      j=i+1;
      while (buf[j-1]) {
        buf[j-1]=buf[j];
        j++;
      }
    } else i++;
  }

  // remove tail SPACE
  len = (int)strlen(buf);
  while (buf[len-1] == 0x20) {
    buf[--len] = 0x00;
  }

  // ... and remove leading SPACE
  while (buf[0] == 0x20) {
    for (i=0; i<len; i++) buf[i] = buf[i+1];
    len--;
  }
}


int read_config(const char* exename)
{
	FILE* fc;
	char s[ 1024 ];

	set_config_name(exename);

	printf(" Reading config file: %s\n", config_name);

//	if (0 != fopen_s(&fc, (const char*)config_name, "r")) {
	if ((fc = fopen((const char*)config_name, "r")) == NULL) {
		printf("Cannot open config file %s\n", (const char* )config_name);
		return 1;
	}

	printf("%s ------------------------------------------------%s\n", COLOR_YELLOW, COLOR_RESET);

	while (fgets(s, sizeof(s), fc) != NULL) {
		str_trunc(s);
		if ( (s[0] == '#') || (s[0] == ';') || (s[0] == 0x00) ) continue;
		if ( (s[0] == '/') && (s[1] == '/') ) continue;

		if ('[' == s[0]) {
			printf(" %s%s%s\n", COLOR_YELLOW, s, COLOR_RESET);
			continue;
		}

		// UPPER CASE - only first world
		{
			for (int i = 0;;i++) {
				if ((0x00 == s[i]) || (0x20 == s[i])) {
					break;
				}

				s[i] = toupper(s[i]);
			}
		}

		// --- dump test data ---
		if (s == strstr(s, "MID")) {
			read_MatchingID(&s[3]);
			continue;
		}
		if (s == strstr(s, "SMDP_IP")) {
			read_InetAddress(&s[7], QR_Address, sizeof(QR_Address));
			continue;
		}
		if (s == strstr(s, "SMDP_PORT")) {
			read_InetPort(&s[9], QR_Port, sizeof(QR_Port));
			continue;
		}
		if (s == strstr(s, "SMDP_PORT")) {
			read_InetPort(&s[9], QR_Port, sizeof(QR_Port));
			continue;
		}

#ifdef _WIN32
		if (s == strstr(s, "SERIAL_WIN")) {
			read_SerialPort(&s[10], serialPort, sizeof(serialPort));
			continue;
		}
#endif

#ifdef __APPLE__
		if (s == strstr(s, "SERIAL_MAC")) {
			read_SerialPort(&s[10], serialPort, sizeof(serialPort));
			continue;
		}
#endif

/*
		if (s == strstr(s, "EIM_IP")) {
			read_InetAddress(&s[6], eIM_Address, sizeof(eIM_Address));
			continue;
		}
		if (s == strstr(s, "EIM_PORT")) {
			read_InetPort(&s[8], eIM_Port, sizeof(eIM_Port));
			continue;
		}
		if (s == strstr(s, "IMEI")) {
			read_Imei(&s[4]);
			continue;
		}
// */
	} // while (fgets())
  
	fclose(fc);
	printf("%s ------------------------------------------------%s\n", COLOR_YELLOW, COLOR_RESET);

	return 0;
}
#undef _CRT_SECURE_NO_WARNINGS

static char* skip_to_value(char* buff)
{
	while (' ' == *buff) buff++;
	while (':' == *buff) buff++;
	while ('=' == *buff) buff++;
	while (' ' == *buff) buff++;

	return buff;
}

static void read_MatchingID(char* src)
{
	src = skip_to_value(src);
	strcpy_s(QR_MID, sizeof(QR_MID), src);

	printf(" Matching ID: %s\n", QR_MID);
}
static void read_InetAddress(char* src, char* dest, size_t destsize)
{
	src = skip_to_value(src);
	strcpy_s(dest, destsize, src);

	printf(" Server Addr: %s\n", dest);
}
static void read_InetPort(char* src, char* dest, size_t destsize)
{
	src = skip_to_value(src);
	strcpy_s(dest, destsize, src);

	printf(" Server Port: %s\n", dest);
}
static void read_SerialPort(char* src, char* dest, size_t destsize)
{
	src = skip_to_value(src);
	strcpy_s(dest, destsize, src);

	printf(" Serial Port: %s\n", dest);
}

static void dump_eUICC_Info1(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
	uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" eUICC Info1: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");

	// BF20
	offset = print_taglen(buff, &len);
	printf("\n");

	// SVN
	offset += print_tlv(&buff[offset]);
	
	// Verif CI
	offset += print_taglen(&buff[offset], &len);
	printf("\n   ");
	offset += print_tlv(&buff[offset]);

	// Sign CI
	offset += print_taglen(&buff[offset], &len);
	printf("\n   ");
	offset += print_tlv(&buff[offset]);
}
static void dump_Server_Signed1(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
//	uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" ServerSigned1: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}

static void dump_Server_Signature1(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
	//uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" ServerSignature1: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}
//
static void dump_AuthenticateServerResponse(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
//	uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" Authenticate Server Response: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}

static void dump_profileMetadata(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
	uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" Profile metadata: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");

	// BF20
	offset = print_taglen(buff, &len);
	printf("\n");

	//
	offset += print_tlv(&buff[offset]);
	offset += print_tlv(&buff[offset]);
	offset += print_tlv(&buff[offset]);
	offset += print_tlv(&buff[offset]);

	offset += print_taglen(&buff[offset], &len);
	offset += print_taglen(&buff[offset], &len);
	printf("\n");

	offset += print_tlv(&buff[offset]);
	printf("   ");
	offset += print_tlv(&buff[offset]);
	printf("   ");

	offset += print_tlv(&buff[offset]);

}
static void dump_smdpSigned2(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
//	uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" SM-DP+ Signed2: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}
static void dump_smdpSignature2(uint8_t* src)
{
	uint8_t buff[16 * 1024];
	uint16_t offset = 0;
	size_t length;
	//uint16_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, strlen((const char* )src));

	printf(" ------------------------------------------------\n");
	printf(" SM-DP+ Signature2: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}
static void dump_boundProfilePackage(uint8_t* src)
{
	uint8_t buff[64 * 1024];
	uint16_t offset = 0;
	size_t length;
	size_t len;

	while (' ' == *src) src++;
	while (':' == *src) src++;
	while (' ' == *src) src++;

	len = strlen((const char* )src);
	//mbedtls_base64_decode(buff, sizeof(buff), &length, src, len);

	printf(" ------------------------------------------------\n");
	printf(" Bound Profile Package: %s\n", src);
	dump_hexascii_buffer(" hex:", buff, length);

	printf(" ---\n");
}
