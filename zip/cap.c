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

#include "zip.h"
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
static bool find_component(zip_t* _cap, const char* _cname, int* count, struct zip_stat* finfo)
{
	int len;

	*count = 0;
	while ((zip_stat_index(_cap, *count, 0, finfo)) == 0)
	{
		char* component;

		component = (char* )&finfo->name[strlen(finfo->name) - 1];

		while (*component != '/')
			component--;

		component++; // next after '/'
		len = strlen(component);

		if (memcmp(_cname, component, len) == 0) {
			return true;
		}

		(*count)++; 
	}

	printf("Component %s not found..\n", _cname);
	return false;
}

static void load_component(zip_t* _cap, const char* _cname)
{
	struct zip_stat* finfo = NULL;
	zip_file_t* fd = NULL; 

	uint8_t* buffer;
	int len;
	int len_read;
	int count = 0;

	finfo = malloc(sizeof(zip_stat));
	zip_stat_init(finfo);

	if (!find_component(_cap, _cname, &count, finfo))
	{
		return;
	}

	len = (int)finfo->size;
	buffer = malloc(len); 

	printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", _cname, len);

	fd = zip_fopen_index(_cap, count, 0);

    len_read = (int)zip_fread(fd, buffer, len); 
	for (size_t j = 0; j < len_read; j++)
		printf("%02X ", buffer[j]);
	printf("\n");
	printf("\n");

	free(buffer);
}

void print_cap_info(const char* filename)
{
	zip_t* cap = NULL; 
	struct zip_stat* finfo = NULL;
	zip_file_t* fd = NULL; 
	int errorp = 0;
	int count;
	int len;

	uint8_t buffer[256];
	unsigned int len_read;

	printf(" CAP file name      : " COLOR_GREEN "%s\n" COLOR_RESET, filename);

	cap = zip_open(filename, 0, &errorp); 
	if (cap == NULL) {
		printf("Failed to open CAP file\n");
		return;
	}

	finfo = malloc(sizeof(zip_stat));
	zip_stat_init(finfo);

	count = 0;
	while ((zip_stat_index(cap, count, 0, finfo)) == 0)
	{
		char* component;

		len = (int)finfo->size;
		component = (char* )&finfo->name[strlen(finfo->name) - 1];

		while (*component != '/')
			component--;

		component++; // next after '/'

		printf("   %-16s : %d bytes\n", component, (int)len);

		count++; 
	}

	// --- print HEADER
	if (find_component(cap, COMP_HEADER, &count, finfo))
	{
		size_t offset;
		int len_read;

		len = (int)finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, count, 0);
		len_read = (int)zip_fread(fd, buffer, len); 

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
	if (find_component(cap, COMP_IMPORT, &count, finfo))
	{
		uint8_t impcnt;
		size_t offset;

		len = (int)finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, count, 0);
		len_read = (int)zip_fread(fd, buffer, len); 

		offset = 3;
		impcnt = buffer[offset++];

		while (impcnt--)
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
	if (find_component(cap, COMP_APPLET, &count, finfo))
	{
		uint8_t appcnt;
		size_t offset;

		len = (int)finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, count, 0);
		len_read = (int)zip_fread(fd, buffer, len); 

		offset = 3;
		appcnt = buffer[offset++];

		while (appcnt--)
		{
			uint8_t sz = buffer[offset++];

			printf("   ");
			for (size_t j = 0; j < sz; j++)
				printf("%02X", buffer[offset++]);
			printf("\n");

			offset += 2; // u2 install_method_offset
		}
	}

	free(finfo);
	zip_close(cap);
}

void upload_cap(const char* filename)
{
	zip_t* cap = NULL; 
	int errorp = 0;

	cap = zip_open(filename, 0, &errorp); 

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
	//load_component(cap, COMP_EXPORT); // -- skip loading
	load_component(cap, COMP_CONSTANTPOOL);
	load_component(cap, COMP_REFLOCATION);
	//load_component(cap, COMP_DESCRIPTOR); // -- skip loading
	//load_component(cap, COMP_DEBUG); // -- skip loading

	zip_close(cap);
}
