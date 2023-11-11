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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "unzip.h"

static const char* COMP_HEADER		= "Header";
static const char* COMP_DIRECTORY	= "Directory";
static const char* COMP_APPLET		= "Applet";
static const char* COMP_IMPORT		= "Import";
static const char* COMP_CONSTANTPOOL = "ConstantPool";
static const char* COMP_CLASS		= "Class";
static const char* COMP_METHOD		= "Method";
static const char* COMP_STATICFIELD = "StaticField";
static const char* COMP_REFLOCATION = "RefLocation";
static const char* COMP_DESCRIPTOR	= "Descriptor";
static const char* COMP_DEBUG		= "Debug";

/**
 * @brief Look for a component (aka zip file)
 * Found file i.e. component becomes current
 * 
 * @par _pfile_info filled with file details
 */
static bool find_component(unzFile _cap, const char* _cname, unz_file_info* _pfile_info)
{
	unz_global_info pglobal_info;
	char szFileName[2048];
	char extraField[32];
	char szComment[256];

	unzGetGlobalInfo(_cap, &pglobal_info);

	for (size_t i = 0; i < pglobal_info.number_entry; i++)
	{
		char* component;
		unsigned int len;

		if (i == 0) {
			unzGoToFirstFile(_cap);
		}
		else unzGoToNextFile(_cap);

		unzGetCurrentFileInfo(_cap, _pfile_info,
			szFileName, sizeof(szFileName),
			extraField, sizeof(extraField),
			szComment, sizeof(szComment));

		component = &szFileName[strlen(szFileName) - 1];
		while (*component != '/')
			component--;

		component++; // next after '/'

		len = strlen(component);
		len -= 4; // exclude '.cap'

		if (memcmp(_cname, component, len) == 0)
			return true;
	}
	printf("Component %s not found..\n", _cname);
	return false;
}

void print_cap_info(const char* filename)
{
	unzFile cap;
	unz_global_info pglobal_info;
	unz_file_info pfile_info;
	char szFileName[2048];
	char extraField[32];
	char szComment[256];
	size_t len;
	uint8_t buffer[256];
	unsigned int len_read;

	cap = unzOpen(filename);

	if (cap == NULL) {
		printf("Failed to open CAP file\n");

		return;
	}
	// --- print CAP name
	printf(" CAP file name      : %s\n", filename);

	unzGetGlobalInfo(cap, &pglobal_info);

	printf(" CAP file components (%d)\n", pglobal_info.number_entry);
	for (size_t i = 0; i < pglobal_info.number_entry; i++)
	{
		char* component;

		if (i == 0) {
			unzGoToFirstFile(cap);
		}
		else unzGoToNextFile(cap);

		unzGetCurrentFileInfo(cap, &pfile_info,
			szFileName, 2048,
			extraField, 32,
			szComment, 256);

		len = pfile_info.uncompressed_size;

		component = &szFileName[strlen(szFileName) - 1];
		while (*component != '/')
			component--;

		component++; // next after '/'

		printf("   %-16s : %d bytes\n", component, len);
	}

	// --- print HEADER
	if (find_component(cap, COMP_HEADER, &pfile_info))
	{
		size_t offset;

		len = pfile_info.uncompressed_size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		unzOpenCurrentFile(cap);
		len_read = unzReadCurrentFile(cap, buffer, len);

		offset = 7;
		printf(" CAP file version   : %d.%d\n", buffer[offset], buffer[offset + 1]);
		offset += 2;

		printf(" Integer support    : %s\n", buffer[offset] & 0x01 ? "Yes" : "No");
		//printf(" Export component   : %s\n", buffer[offset] & 0x02 ? "Yes" : "No");
		//printf(" Applet component   : %s\n", buffer[offset] & 0x04 ? "Yes" : "No");
		offset++;

		printf(" Package version    : %d.%d\n", buffer[offset], buffer[offset + 1]);
		offset += 2;

		len = buffer[offset++];

		printf(" Package AID        : ");
		for (size_t j = 0; j < len; j++)
			printf("%02X", buffer[offset++]);
		printf("\n");

		len = buffer[offset++];

		printf(" Package name       : ");
		for (size_t j = 0; j < len; j++)
			printf("%c", buffer[offset++]);
		printf("\n");
	}


	// --- print IMPORT
	printf(" Import AIDs\n");
	if (find_component(cap, COMP_IMPORT, &pfile_info))
	{
		uint8_t count;
		size_t offset;

		len = pfile_info.uncompressed_size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		unzOpenCurrentFile(cap);
		len_read = unzReadCurrentFile(cap, buffer, len);

		offset = 3;
		count = buffer[offset++];

		while (count--)
		{
			uint8_t min = buffer[offset++];
			uint8_t maj = buffer[offset++];
			uint8_t sz = buffer[offset++];

			printf("   ");
			for (size_t j = 0; j < sz; j++)
				printf("%02X", buffer[offset++]);
			printf("  version %d.%d\n", maj, min);
		}
	}
	
	// --- print APPLETs
	printf(" Applets\n");
	if (find_component(cap, COMP_APPLET, &pfile_info))
	{
		uint8_t count;
		size_t offset;

		len = pfile_info.uncompressed_size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		unzOpenCurrentFile(cap);
		len_read = unzReadCurrentFile(cap, buffer, len);

		offset = 3;
		count = buffer[offset++];

		while (count--)
		{
			uint8_t sz = buffer[offset++];

			printf("   ");
			for (size_t j = 0; j < sz; j++)
				printf("%02X", buffer[offset++]);
			printf("\n");

			offset += 2; // u2 install_method_offset
		}
	}


	unzClose(cap);
}

void upload(const char* filename)
{
	unzFile cap;
	unz_global_info pglobal_info;
	unz_file_info pfile_info;
	char szFileName[2048];
	char extraField[32];
	char szComment[256];


	cap = unzOpen(filename);

	if (cap == NULL) {
		printf("Not OK\n");

		return;
	}

	unzGetGlobalInfo(cap, &pglobal_info);

	for (size_t i = 0; i < pglobal_info.number_entry; i++)
	{
		char* component;
		uint8_t buffer[256];
		unsigned int len;
		unsigned int len_read;

		if (i == 0) {
			unzGoToFirstFile(cap);
		}
		else unzGoToNextFile(cap);

		unzGetCurrentFileInfo(cap, &pfile_info,
			szFileName, 2048,
			extraField, 32,
			szComment, 256);

		len = pfile_info.uncompressed_size;

		component = &szFileName[strlen(szFileName) - 1];
		while (*component != '/')
			component--;

		component++; // next after '/'

		printf("File name: %s\n", component);
		printf("File size: %d\n", len);

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		unzOpenCurrentFile(cap);
		len_read = unzReadCurrentFile(cap, buffer, len);

		for (size_t j = 0; j < len; j++)
			printf("%02X ", buffer[j]);
		printf("\n\n");
	}

	unzClose(cap);
}