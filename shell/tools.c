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

#include "tools.h"

uint16_t calculate_crc16(uint8_t* Data, uint16_t Len)
{
  uint16_t crc = 0;
  uint8_t i;

  while (Len-- > 0) {
    crc ^= (*Data++ << 8);
    for (i=0; i<8; i++){
      if (crc & 0x8000) {
        crc <<= 1;
		crc ^= 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}

void parse_asn1_signature(uint8_t* src, uint8_t* dest)
{
	int offset;
	int i;

	offset = 3;
	if (0x20 == src[offset]) {
		offset += 1;
	} else {
		offset += 2;
	}
	for (i=0; i < 0x20; i++) {
		dest[i] = src[offset++];
	}
	offset++;

	if (0x20 == src[offset]) {
		offset += 1;
	} else {
		offset += 2;
	}
	for (i=0; i < 0x20; i++) {
		dest[0x20 + i] = src[offset++];
	}
}

int construct_asn1_signature(uint8_t* src_bin, uint8_t* dest_asn1)
{
	int offset = 0;

	dest_asn1[offset++] = 0x30;
	dest_asn1[offset++] = 0x44; // can be later incremented (*)

	// R part
	dest_asn1[offset++] = 0x02;
	dest_asn1[offset++] = 0x20; // can be later incremented (**)

	if (0x80 == (0x80 & src_bin[0])) {
		dest_asn1[1]++; // (*) increment total length
		dest_asn1[offset-1]++; // (**) increment R length
		dest_asn1[offset++] = 0x00;
	}
	memcpy(&dest_asn1[offset], src_bin, 0x20);
	offset += 0x20;

	// S part
	dest_asn1[offset++] = 0x02;
	dest_asn1[offset++] = 0x20; // can be later incremented (**)

	if (0x80 == (0x80 & src_bin[0x20])) {
		dest_asn1[1]++; // (*) increment total length
		dest_asn1[offset - 1]++; // (**) increment S length
		dest_asn1[offset++] = 0x00;
	}
	memcpy(&dest_asn1[offset], &src_bin[0x20], 0x20);
	offset += 0x20;

	return offset;
}

uint8_t byte2nimble(uint8_t b)
{
	uint8_t res = 0;

	if (b >= '0' && b <= '9')
		res = b - 0x30;
	else if (b >= 'A' && b <= 'F')
		res = b - ('A' - 10);
	else if (b >= 'a' && b <= 'f')
		res = b - ('a' - 10);

	return res;
}

/**
 * Returns a byte of value equal to that represented by a 2 character hex string.
   Ex. input of "A4" returns 164. 
 */
uint8_t byte_from_hex_str(const char *hex_byte)
{
	uint8_t value;
	uint8_t value2;

	value = byte2nimble(hex_byte[0]);
	value2 = byte2nimble(hex_byte[1]);

	value = (value << 4) | value2;

	return value;
}

void convert_hex2bin(const char* src, uint8_t* dst, uint16_t dest_len)
{
	int i;

	for (i=0; i < dest_len; i++) {
		*dst++ = byte_from_hex_str((const char *)&src[i*2]);
	}
}

void convert_bin2hex(const uint8_t* src_bin, uint8_t* dest_hex, int src_len)
{
	uint8_t b1, b2;

	while (src_len--) {
		b1 = *src_bin >> 4;
		b2 = *src_bin & 0x0F;

		if (b1 < 0x0A) {
			*dest_hex = 0x30 + b1;
		} else {
			*dest_hex = 0x41 + b1 - 0x0A;
		}
		dest_hex++;

		if (b2 < 0x0A) {
			*dest_hex = 0x30 + b2;
		} else {
			*dest_hex = 0x41 + b2 - 0x0A;
		}
		dest_hex++;

		src_bin++;
	}

	*dest_hex = 0x00;
}

void change_endian(uint8_t* buffer, int length)
{
	int len = length / 2;
	int i;
	uint8_t b;

	for (i = 0; i < len; i++) {
		b = buffer[length - 1 - i];
		buffer[length - 1 - i] = buffer[i];
		buffer[i] = b;
	}
}

void shift_r(uint8_t* buffer, int length)
{
	int i;

	for (i = length; i > 0;  i--) {
		buffer[i] = buffer[i-1];
	}
}

static void print_ascii(const uint8_t* buff, int count)
{
	int j;
	unsigned char c;

	if (count < 16) {
		printf(" ");
		if (count < 8) {
			printf(" ");
		}
	}
	for (j = 0; j < (16 - count); j++) {
		printf("   ");
	}
	printf(" | ");

	for (j = 0; j < count; j++) {
		c = buff[j];
		if (c < 0x20) {
			printf(".");
		}
		else {
			printf("%c", c);
		}

	}

	printf("\n");
}


#define DUMP_INTEND	8
void dump_hexascii_buffer(const char* name, const uint8_t* recvbuf, size_t len)
{
	size_t i;

	printf("%s", name);
	if (0 == len) {
		printf("\n");
		return;
	}

	{
		int len;
		len = (int)strlen(name);
		if (len > DUMP_INTEND) {
			printf("\n");
			len = 0;
		}

		for (int j = 0; j < (DUMP_INTEND - len); j++)
			printf(" ");
	}

	for (i = 0; i < len; i++) {
		printf(" %02X", recvbuf[i]);
		if ((i + 1) % 8 == 0) {
			printf(" ");
		}
		if ((i + 1) % 16 == 0) {
			print_ascii(&recvbuf[i - 15], 16);

			if ((i+1) < len)
				for (int j = 0; j < DUMP_INTEND; j++)
					printf(" ");
		}
	}


	i = len % 16;
	if (i > 0) {
		print_ascii(&recvbuf[len - i], (int)i);
	}
}

void print_bin2hex(uint8_t* buffer_bin, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		printf("%02X", buffer_bin[i]);
	}
	printf("\n");
}

size_t delete_symbol__(uint8_t* buffer, size_t len, uint8_t symb)
{
	size_t i, j;

	for (i = 0; i < len; i++) {
		if (symb == buffer[i]) {
			for (j = 0; j < (len - i - 1); j++) {
				buffer[i + j] = buffer[i + j + 1];
			}
			len--;
		}
	}

	return len;
}

uint16_t print_taglen(uint8_t* buff, uint16_t* length)
{
	uint16_t offset = 0;
	uint16_t len;

	printf(" ");
	if (0x1F == (0x1F & buff[offset])) {
		printf("%02X%02X ", buff[offset], buff[offset + 1]);
		offset += 2;
	}
	else {
		printf("%02X ", buff[offset++]);
	}

	len = buff[offset++];
	switch (len) {
	case 0x81:
		len = buff[offset++];
		break;
	case 0x82:
		len = 0x100 * buff[offset++];
		len += buff[offset++];
		break;
	}

	if (len > 0xFF) {
		printf("%04X ", len);
	}
	else {
		printf("%02X ", len);
	}

	*length = len;
	return offset;
}
uint16_t print_tlv(uint8_t* buff)
{
	uint16_t len;
	uint16_t offset;

	offset = print_taglen(buff, &len);

	print_bin2hex(&buff[offset], len);

	return offset + len;
}

/**
* Set length byte at offset 1
* Shift the data if the length value >= 0x80
*/
uint8_t BERTLV_set_length(uint8_t* buffer, uint8_t len)
{
	uint16_t i;

	if (len < 0x82) {
		buffer[1] = len - 2;
	}
	else {
		if (buffer[1] != 0x81) {
			len++;
			for (i = len - 3; i>0; i--) {
				buffer[2 + i] = buffer[1 + i];
			}
			buffer[1] = 0x81;
		}
		buffer[2] = len - 3;
	}
	return len;
}