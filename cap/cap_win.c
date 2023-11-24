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
#include "tools.h"

/*
    // CAP file download order
    static final byte ORDER_HEADER = (byte)1;
    static final byte ORDER_DIRECTORY = (byte)2;
    static final byte ORDER_IMPORT = (byte)3;
    static final byte ORDER_APPLET = (byte)4;
    static final byte ORDER_CLASS = (byte)5;
    static final byte ORDER_METHOD = (byte)6;
    static final byte ORDER_STATICFIELD = (byte)7;
    static final byte ORDER_EXPORT = (byte)8;
    static final byte ORDER_CONSTANTPOOL = (byte)9;
    static final byte ORDER_REFERENCELOCATION = (byte)10;
    static final byte ORDER_DESCRIPTOR = (byte)11;
    //static final byte ORDER_DEBUG = (byte)12;
*/


static const char* COMP_HEADER		= "Header.cap";
static const char* COMP_DIRECTORY	= "Directory.cap";
static const char* COMP_APPLET		= "Applet.cap";
static const char* COMP_IMPORT		= "Import.cap";
static const char* COMP_CONSTANTPOOL	= "ConstantPool.cap";
static const char* COMP_CLASS		= "Class.cap";
static const char* COMP_METHOD		= "Method.cap";
static const char* COMP_STATICFIELD	= "StaticField.cap";
static const char* COMP_REFLOCATION	= "RefLocation.cap";
static const char* COMP_DESCRIPTOR	= "Descriptor.cap";
static const char* COMP_DEBUG		= "Debug.cap";
static const char* COMP_EXPORT		= "Export.cap";

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

		component++; // Component name start next char after '/'

		len = strlen(component);

		if (memcmp(_cname, component, len) == 0)
			return true;
	}
	printf("Component %s not found..\n", _cname);
	return false;
}

static void load_component(unzFile _cap, const char* _cname)
{
	unz_file_info pfile_info;
	uint8_t buffer[256];
	unsigned int len;
	unsigned int len_read;

	if (!find_component(_cap, _cname, &pfile_info))
	{
		return;
	}

	printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", _cname, (int)pfile_info.uncompressed_size);

	len = pfile_info.uncompressed_size;
	unzOpenCurrentFile(_cap);

	while (len > 0) {

		len_read = unzReadCurrentFile(_cap, buffer, sizeof(buffer));
		len -= len_read;

		for (size_t j = 0; j < len_read; j++)
			printf("%02X ", buffer[j]);
		printf("\n");
	}

	printf("\n");
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
	printf(" CAP file name      : " COLOR_GREEN "%s\n" COLOR_RESET, filename);

	unzGetGlobalInfo(cap, &pglobal_info);

	printf(" CAP file components (%d)\n", (int)pglobal_info.number_entry);
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

		printf("   %-16s : %d bytes\n", component, (int)len);
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

	printf("\n");
}

void upload_cap(const char* filename)
{
	unzFile cap;

	cap = unzOpen(filename);

	if (cap == NULL) {
		printf("Failed to open CAP file %s\n", filename);

		return;
	}

	load_component(cap, COMP_HEADER);
	load_component(cap, COMP_DIRECTORY);
	load_component(cap, COMP_IMPORT);
	load_component(cap, COMP_APPLET);
	load_component(cap, COMP_CLASS);
	load_component(cap, COMP_METHOD);
	load_component(cap, COMP_STATICFIELD);
	//load_component(cap, COMP_EXPORT);
	load_component(cap, COMP_CONSTANTPOOL);
	load_component(cap, COMP_REFLOCATION);
	//load_component(cap, COMP_DESCRIPTOR);
	//load_component(cap, COMP_DEBUG);

	unzClose(cap);
}
