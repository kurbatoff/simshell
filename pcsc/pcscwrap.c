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

#include <stdbool.h>
#include <stdint.h>

#include "tools.h"
#include "pcscwrap.h"

#if defined(__APPLE__)
	typedef uint32_t DWORD;
#endif
#if defined(__APPLE__) || defined(__linux__) || defined(linux) || defined(__linux) || defined(__gnu_linux__)
	#include <PCSC/winscard.h>

	//typedef const int* LPTSTR;
	typedef uint8_t BYTE;
	typedef long LONG;

	#define RSTR	char*
	#define STRLEN	strlen

	#define SCARD_ATTR_VALUE(Class, Tag) ((((LONG)(Class)) << 16) | ((LONG)(Tag)))
	#define SCARD_CLASS_ICC_STATE       9   // ICC State specific definitions
	#define SCARD_ATTR_ATR_STRING SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303)
#endif

#ifdef _WIN32
	#include <winscard.h>

	#define RSTR	LPTSTR
	#define STRLEN	wcslen
#endif

RSTR mszReaders;

SCARDCONTEXT hContext;
SCARDHANDLE hCard;
DWORD dwReaders, dwActiveProtocol, dwRecvLength;
SCARD_IO_REQUEST pioSendPci;

uint8_t LChannel_ID;

#define ISO_INS_GET_RESPONSE 0xC0

uint16_t get_response(uint8_t response_len, uint8_t* _response_buff, uint16_t _response_buff_sz)
{
	uint8_t command[ 5 ];
	uint16_t resp_length;

	command[ 0 ] = 0x00;
	command[ 1 ] = ISO_INS_GET_RESPONSE;
	command[ 2 ] = 0x00;
	command[ 3 ] = 0x00;
	command[ 4 ] = response_len;

	pcsc_sendAPDU(command, 5, _response_buff, _response_buff_sz, &resp_length);

	return resp_length;
}

void print_reader_name()
{
	if (mszReaders == NULL)
		printf(" [No reader selected]");
	else {
#ifdef _WIN32
	printf(COLOR_BLUE " [%ls]" COLOR_RESET, mszReaders);
#endif
#ifdef __APPLE__
	printf(COLOR_BLUE " [%s]" COLOR_RESET, mszReaders);
#endif
	}
}

pcsc_error_t pcsc_sendAPDU(uint8_t* _cmd, uint16_t _cmd_len,
  uint8_t *_response_buffer, uint16_t _response_buffer_sz, uint16_t* _response_length)
{
	pcsc_error_t error_code = PCSC_ERROR_UNKNOWN;
	LONG rv;
	uint8_t recvBuffer[PCSC_APDU_C_BUFF_LEN];

	if (0x00 == hCard) {
	}

	dwRecvLength = sizeof(recvBuffer);

	{
		dump_hexascii_buffer(" Cmd :", _cmd, 5);
		dump_hexascii_buffer(" Data:", &_cmd[5], _cmd_len - 5);
	}

	rv = SCardTransmit(hCard, &pioSendPci, _cmd, _cmd_len, NULL, recvBuffer, &dwRecvLength);
	if (rv) {
		printf("Error on SCardTransmit: %ld\n", rv);
		return PCSC_ERROR_UNKNOWN;
	}

	{
		dump_hexascii_buffer(" Resp:", recvBuffer, dwRecvLength - 2);

		printf(" SW  :  " COLOR_CYAN " %02X %02X\n" COLOR_RESET, recvBuffer[dwRecvLength - 2], recvBuffer[dwRecvLength - 1]);

		printf("\n");
	}

	error_code = recvBuffer[dwRecvLength - 2] << 8 | recvBuffer[dwRecvLength-1];
	*_response_length = (uint16_t)dwRecvLength;

	*_response_length = (uint16_t)dwRecvLength;

	// Extract data if requested
	if (NULL != _response_buffer)
	{
		if (*_response_length <= _response_buffer_sz) {
			memcpy(_response_buffer, recvBuffer, *_response_length);
		}
		else
		{
			printf("Response too long for buffer (%d > %d)\n", *_response_length, _response_buffer_sz);
			return PCSC_ERROR_UNKNOWN;
		}
	}

/*
  rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
  if (rv)
  {
    printf("Error on SCardDisconnect: %ld\r\n", rv);
    return PCSC_ERROR_UNKNOWN;
  }

#ifdef SCARD_AUTOALLOCATE
  rv = SCardFreeMemory(hContext, mszReaders);
  if (rv)
  {
    printf("Error on SCardFreeMemory: %ld\r\n", rv);
    return PCSC_ERROR_UNKNOWN;
  }

#else
  free(mszReaders);
#endif

  rv = SCardReleaseContext(hContext);
  if (rv)
  {
    printf("SCardReleaseContext: %d\r\n", rv);
    return PCSC_ERROR_UNKNOWN;
  }
  */

	return error_code;
}

pcsc_error_t pcsc_listreaders(void)
{
	LONG rv;

	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	if (rv)
	{
		printf("Error on SCardEstablishContext: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}

#ifdef SCARD_AUTOALLOCATE
	dwReaders = SCARD_AUTOALLOCATE;

	rv = SCardListReaders(hContext, NULL, (RSTR)&mszReaders, &dwReaders);
	if (rv)
	{
		printf("Error on SCardListReaders: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}
#else
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	if (rv)
	{
		printf("Error on SCardListReaders: %ld\r\n", rv);
		return PCSC_ERROR_UNKNOWN;
	}

	mszReaders = calloc(dwReaders, sizeof(char));
	rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
	if (rv)
	{
		printf("Error on SCardListReaders: %ld\r\n", rv);
		return PCSC_ERROR_UNKNOWN;
	}
#endif

	{
		RSTR readers;
		int i;
		int idx;
		int idx_default = 0;
		RSTR name_default = mszReaders;
		char cardIN[4];

		uint8_t cards[] = {0, 0, 0, 0,  0, 0, 0, 0};

		// --- 1 --- Check all readers for inserted acrd
		i = 0;
		readers = mszReaders;
		while (readers[0]) {
			rv = SCardConnect(hContext, readers, SCARD_SHARE_SHARED,
				SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
			if (0 == rv)
			{
				if (i<sizeof(cards))
					cards[i] = 1;

				if (idx_default == 0)
					idx_default = i + 1;
			}
			rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);

			i++;

			readers += STRLEN(readers);
			readers++;
		}

		// --- 2 --- Print the list of readers
		i = 0;
		readers = mszReaders;
		while (readers[0]) {

			if (cards[i]) {
				cardIN[0] = '[';
				cardIN[1] = '+';
				cardIN[2] = ']';
				cardIN[3] = 0;
			}
			else {
				cardIN[0] = ' ';
				cardIN[1] = ' ';
				cardIN[2] = ' ';
				cardIN[3] = 0;
			}
			
			i++;
#ifdef _WIN32
			printf(" %d. %s %ls\n", i, cardIN, readers);
#endif
#ifdef __APPLE__
			printf(" %d. %s %s\n", i, cardIN, readers);
#endif
			if (i == idx_default) {
				name_default = readers;
			}
			readers += STRLEN(readers);
			readers++;
		}

		if (0 == idx_default) {
			// If no default

#ifdef _WIN32
			printf(" Choose reader (Default: %ls) > ", name_default);
#endif
#ifdef __APPLE__
			printf(" Choose reader (Default: %s) > ", name_default);
#endif
			idx = getchar();
			if (0x0A == idx) {
				idx = idx_default;
			}
			else {
				idx -= 0x30;
			}
		}
		else {
			// Choose default
			idx = idx_default;
		}

		i = 0;
		readers = mszReaders;
		while (readers[0]) {
			if (idx == ++i) {
				mszReaders = readers;
#ifdef _WIN32
				printf("  >> Selected: " COLOR_YELLOW "%ls\n" COLOR_RESET, mszReaders);
#endif
#ifdef __APPLE__
				printf("  >> Selected: " COLOR_YELLOW "%s\n" COLOR_RESET,mszReaders);
#endif
			}

			readers += STRLEN(readers);
			readers++;
		}
	}

	return PCSC_SUCCESS;
}

pcsc_error_t connect_reader(void)
{
	LONG rv;
	uint8_t ATR[48];
	DWORD ATRlen = sizeof(ATR);

	rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
	if (rv)
	{
		printf("Error on SCardConnect: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}

	switch (dwActiveProtocol)
	{
	case SCARD_PROTOCOL_T0:
		pioSendPci = *SCARD_PCI_T0;
		break;

	case SCARD_PROTOCOL_T1:
		pioSendPci = *SCARD_PCI_T1;
		break;
	}


	rv = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING, ATR, &ATRlen);
	if (rv)
	{
		printf("Error on SCardGetAttrib: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}

	printf(COLOR_YELLOW_I "ATR: ");
	for (uint8_t i = 0; i < ATRlen; i++)
		printf("%02X", ATR[i]);
	printf("\n" COLOR_RESET);

	return PCSC_SUCCESS;
}

pcsc_error_t disconnect_reader(void)
{
	LONG rv;

	rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
	if (rv)
	{
		printf("Error on SCardDisconnect: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}

	return PCSC_SUCCESS;
}

pcsc_error_t pcsc_reset(void)
{
	// Added on May, 25 by AK!
	// NOT working yet
	// TODO finish

	LONG rv;

	rv = SCardReconnect(hCard, SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, 0, &dwActiveProtocol);
	if (rv)
	{
		printf("Error on SCardReconnect: " COLOR_RED " % ld\n" COLOR_RESET, rv);
		return PCSC_ERROR_UNKNOWN;
	}

	return PCSC_SUCCESS;
}
