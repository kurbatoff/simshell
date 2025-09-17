/**
 *  Copyright (c) 2025, Intergalaxy LLC
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

#include <stdio.h>
#include <string.h>

#include "serial.h"

#ifdef _WIN32
#include <windows.h>

static  HANDLE handleCOM;

// ---------------------------------------------------------------------------------------------------------------------
static HANDLE openSerialPort( const char* const comPortStr, int connectTryCount, DWORD readtimeout )
{
	HANDLE handleSerial = INVALID_HANDLE_VALUE;
	printf( "Opening port %s... ", comPortStr );

	for( int i = connectTryCount; i; --i )
	{
		handleSerial = CreateFileA(comPortStr, GENERIC_READ | GENERIC_WRITE, 0, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

		if( (handleSerial == INVALID_HANDLE_VALUE) && (connectTryCount > 1) )
		{
			printf( "\b\b%02d", i );
			Sleep(1000);
			continue;
		}

		break;
	}

	if( handleSerial == INVALID_HANDLE_VALUE )
	{
		if( GetLastError() == ERROR_FILE_NOT_FOUND )
		{
			printf( "port does not exist\n\n\n" );
		}
		else
		{
			printf( "\nOther error...\n" );
		}

		return handleSerial;
	}

	printf("Success\n\n");

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if( !GetCommState(handleSerial, &dcbSerialParams) )
	{
		printf("Error getting COM state\n");
	}
	dcbSerialParams.BaudRate = CBR_115200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!SetCommState(handleSerial, &dcbSerialParams))
	{
		printf( "Setting COM params error happened\n" );
	}


	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = readtimeout;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if( !SetCommTimeouts(handleSerial, &timeouts) )
	{
		printf( "Setting timeouts error occurred. Inform user\n" );
	}

	return handleSerial;
}
#endif // _WIN32

int serial_close(void)
{
#ifdef _WIN32
	CloseHandle(handleCOM);
#endif // _WIN32
	
	return 1;
}

int serial_open(const char* comPortStr)
{
#ifdef _WIN32
	
	handleCOM = openSerialPort(comPortStr, 1, 50);

	if (INVALID_HANDLE_VALUE == handleCOM)
	{
		return 0;
	}

	return 1;
#else
	return 0;
#endif // _WIN32
}

int comPort_send(const char* data, size_t dataLen)
{
#ifdef _WIN32
	size_t dwBytesRead;
	if (!WriteFile(handleCOM, data, (DWORD)dataLen, (LPDWORD)&dwBytesRead, NULL))
	{
		printf("Write COM error occurred. Report to user..\n");
		while (1);
	}
	
	return 1;
#else
	return 0;
#endif // _WIN32
}

int comPort_read(unsigned char* data, uint32_t dataLen)
{
#ifdef _WIN32
	size_t dwBytesRead = 0;
	while (dataLen)
	{
		if (!ReadFile(handleCOM, data, 1, (LPDWORD)&dwBytesRead, NULL))
		{
			printf("Read COM error occurred. Report to user..\n");
			while (1);
		}
		if (dwBytesRead == 1)
		{
			dataLen--;
			data++;
		}
	}

	return 1;
#else
	return 0;
#endif // _WIN32
}

size_t serial_receive(unsigned char* data)
{
#ifdef _WIN32
	size_t dwBytesRead = 0;
	int len = 0;

	while (1) {
		if (!ReadFile(handleCOM, &data[len], 1, (LPDWORD)&dwBytesRead, NULL))
		{
			printf("Read COM error occurred. Report to user..\n");
			while (1);
		}

		if (data[len] == 0x0A) {
			if (data[len - 1] == 0x0D) {
				len--;
			}
			data[len] = 0x00;

			break;
		}
		else len++;
	}

	printf("%s\n", data);

	return len;
#else
	return 0;
#endif // _WIN32
}

void serial_execute(const char* atcmd, char* atresp)
{
	char OK[1024];

	comPort_send(atcmd, strlen(atcmd));
	serial_receive(OK); // echo
	serial_receive(atresp);
	serial_receive(OK); // CRLF
	serial_receive(OK); // OK
}