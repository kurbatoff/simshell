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

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <zip.h>

#include "tools.h"
#include "pcscwrap.h"
#include "gp.h"

#define THE_LAST_COMPONENT		1

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

#define COMP_HEADER_IDX				1
#define COMP_DIRECTORY_IDX			2
#define COMP_IMPORT_IDX				3
#define COMP_APPLET_IDX				4
#define COMP_CLASS_IDX				5
#define COMP_METHOD_IDX				6
#define COMP_STATICFIELD_IDX		7
#define COMP_EXPORT_IDX				8
#define COMP_CONSTANTPOOL_IDX		9
#define COMP_REFERENCELOCATION_IDX	10
#define COMP_DESCRIPTOR_IDX			11
#define COMP_DEBUG_IDX				12

static const char* COMP_HEADER		= "Header.cap";
static const char* COMP_DIRECTORY	= "Directory.cap";
static const char* COMP_APPLET		= "Applet.cap";
static const char* COMP_IMPORT		= "Import.cap";
static const char* COMP_CONSTANTPOOL	= "ConstantPool.cap";
static const char* COMP_CLASS		= "Class.cap";
static const char* COMP_METHOD		= "Method.cap";
static const char* COMP_STATICFIELD	= "StaticField.cap";
static const char* COMP_REFLOCATION	= "RefLocation.cap";
//static const char* COMP_DESCRIPTOR	= "Descriptor.cap";
//static const char* COMP_DEBUG		= "Debug.cap";
static const char* COMP_EXPORT		= "Export.cap";

uint8_t counterP2;

static bool is_IJC(const char* _fname)
{
	FILE* fp;
	uint8_t header[8];
	bool yesIJC = false;

	fp = fopen(_fname, "rb");

	if (NULL == fp) {
		printf(" Failed to open file %s..\n", _fname);

		return false;
	}

	fread(header, 1, sizeof(header), fp);
	//dump_hexascii_buffer("Header", header, sizeof(header));

	if ( (header[0] == 1) // Header idx
		&& (header[1] == 0) // Header length: 1st byte
		&& (header[3] == 0xDE) // 4 bytes of magic world
		&& (header[4] == 0xCA)
		&& (header[5] == 0xFF)
		&& (header[6] == 0xED)
		)
	{
		yesIJC = true;
	}

	fclose(fp);

	return yesIJC;
}

/**
 * @brief Look for a component (aka zip file)
 * Found file i.e. component becomes current
 * 
 * @par _pfile_info filled with file details
 */
static bool find_component_zip(zip_t* _cap, const char* _cname, struct zip_stat* finfo)
{
	int len;
	int count = 0;

	while ((zip_stat_index(_cap, count, 0, finfo)) == 0)
	{
		char* component;

		component = (char* )&finfo->name[strlen(finfo->name) - 1];

		while (*component != '/')
			component--;

		component++; // next after '/'
		len = (int)strlen(component);

		if (memcmp(_cname, component, len) == 0) {
			return true;
		}

		count++; 
	}

	printf("Component %s not found..\n", _cname);
	return false;
}

static bool install_for_load(zip_t* _cap)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	int count = 0;
	int total_sz;
	uint8_t buffer_header[256];
	int header_len;
	apdu_t apdu;

	zip_stat_init(&finfo);

	// --- 1 ---
	if (!find_component_zip(_cap, COMP_HEADER, &finfo))
	{
		return false;
	}

	header_len = (int)finfo.size;
	fd = zip_fopen_index(_cap, finfo.index, 0);
    total_sz = (int)zip_fread(fd, buffer_header, header_len);

	// --- 2 ---
	if (find_component_zip(_cap, COMP_DIRECTORY, &finfo))
		total_sz += (int)finfo.size;

	// --- 3 ---
	if (find_component_zip(_cap, COMP_IMPORT, &finfo))
		total_sz += (int)finfo.size;

	// --- 4 ---
	if (find_component_zip(_cap, COMP_APPLET, &finfo))
		total_sz += (int)finfo.size;

	// --- 5 ---
	if (find_component_zip(_cap, COMP_CLASS, &finfo))
		total_sz += (int)finfo.size;

	// --- 6 ---
	if (find_component_zip(_cap, COMP_METHOD, &finfo))
		total_sz += (int)finfo.size;

	// --- 7 ---
	if (find_component_zip(_cap, COMP_STATICFIELD, &finfo))
		total_sz += (int)finfo.size;

	// --- 8 ---
	if (find_component_zip(_cap, COMP_EXPORT, &finfo))
		total_sz += (int)finfo.size;

	// --- 9 ---
	if (find_component_zip(_cap, COMP_CONSTANTPOOL, &finfo))
		total_sz += (int)finfo.size;

	// --- 10 ---
	if (find_component_zip(_cap, COMP_REFLOCATION, &finfo))
		total_sz += (int)finfo.size;

	// --- INSTALL for LOAD ---
	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_INSTALL;
	apdu.cmd[apdu.cmd_len++] = 0x02;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0; // To be updated

	//
	memcpy(&apdu.cmd[apdu.cmd_len], &buffer_header[12], 1 + buffer_header[12]);
	apdu.cmd_len += (1+ buffer_header[12]);

	//apdu.cmd[apdu.cmd_len++] = 0; // Target SD (AID length)
	apdu.cmd[apdu.cmd_len++] = 8;
	apdu.cmd[apdu.cmd_len++] = 0xA0;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0x01;
	apdu.cmd[apdu.cmd_len++] = 0x51;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;

	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;

	apdu.cmd[4] = apdu.cmd_len - 5;
	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	//dump_hexascii_buffer("header.CAP", apdu.cmd, apdu.cmd_len);

	// --- Load header.CAP ---
	printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", COMP_HEADER, header_len);

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_LOAD;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = counterP2++;
	
	apdu.cmd[apdu.cmd_len++] = 0; // to be updated
	apdu.cmd[apdu.cmd_len++] = 0xC4;
	if (total_sz > 255) {
		apdu.cmd[apdu.cmd_len++] = 0x82;
		apdu.cmd[apdu.cmd_len++] = total_sz >> 8;
	} else
	if (total_sz > 127) {
		apdu.cmd[apdu.cmd_len++] = 0x81;
	}
	apdu.cmd[apdu.cmd_len++] = total_sz & 0xFF;

	memcpy(&apdu.cmd[apdu.cmd_len], buffer_header, header_len);
	apdu.cmd_len += header_len;

	apdu.cmd[4] = apdu.cmd_len - 5;
	pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);

	return true;
}

static void load_component(zip_t* _cap, const char* _cname, uint8_t _last)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	apdu_t apdu;

	int offset;
	int len;
	int len_read;
	int idx;

	zip_stat_init(&finfo);

	if (!find_component_zip(_cap, _cname, &finfo))
	{
		return;
	}

	idx = (int)finfo.index;
	len = (int)finfo.size;

	printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", _cname, len);

	fd = zip_fopen_index(_cap, idx, 0);

	apdu.cmd[0] = 0x80;
	apdu.cmd[1] = INS_GP_LOAD;
	apdu.cmd[2] = 0;
	//apdu.cmd[3] = counterP2;
	//apdu.cmd[4] = 0;
	
	offset = 0;
	while (offset < len) {
		len_read = len - offset;
		if (len_read > 255)
			len_read = 255;

		len_read = (int)zip_fread(fd, &apdu.cmd[5], len_read);

		apdu.cmd[3] = counterP2++;
		apdu.cmd[4] = len_read;
		apdu.cmd_len = 5 + len_read;

		offset += len_read;

		if (_last && offset == len)
			apdu.cmd[2] = 0x80;

		pcsc_sendAPDU(apdu.cmd, apdu.cmd_len, apdu.resp, sizeof(apdu.resp), &apdu.resp_len);
	}
}

void print_cap_info(const char* _filename)
{
	zip_t* cap = NULL; 
	struct zip_stat finfo;
	zip_file_t* fd = NULL; 
	int errorp = 0;
	int count;
	int len;

	uint8_t buffer[256];
	unsigned int len_read;

	printf(" CAP file name: " COLOR_GREEN "%s\n" COLOR_RESET, _filename);

	if (is_IJC(_filename)) {
		printf(".IJC files are not yet supported..\n");
		return;
	}

	cap = zip_open(_filename, 0, &errorp); 
	if (cap == NULL) {
		printf("Failed to open CAP file\n");
		return;
	}

	zip_stat_init(&finfo);

	count = 0;
	while ((zip_stat_index(cap, count, 0, &finfo)) == 0)
	{
		char* component;

		len = (int)finfo.size;
		component = (char* )&finfo.name[strlen(finfo.name) - 1];

		while (*component != '/')
			component--;

		component++; // next after '/'

		printf("   %-16s : %d bytes\n", component, (int)len);

		count++; 
	}

	// --- print HEADER
	if (find_component_zip(cap, COMP_HEADER, &finfo))
	{
		size_t offset;
		int len_read;

		len = (int)finfo.size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, finfo.index, 0);
		len_read = (int)zip_fread(fd, buffer, len); 

		offset = 7;
		printf(" CAP file version   : %d.%d\n", buffer[offset], buffer[offset + 1]);
		offset += 2;

		printf(" Integer support    : %s\n", buffer[offset] & 0x01 ? "Yes" : "No");
		printf(" Export component   : %s\n", buffer[offset] & 0x02 ? "Yes" : "No");
		//printf(" Applet component   : %s\n", buffer[offset] & 0x04 ? "Yes" : "No");
		offset++;

		printf(" Package version    : %d.%d\n", buffer[offset], buffer[offset + 1]);
		offset += 2;

		len = buffer[offset++];

		printf(" Package AID        : ");
		for (int j = 0; j < len; j++)
			printf("%02X", buffer[offset++]);
		printf("\n");

		len = buffer[offset++];

		printf(" Package name       : ");
		for (int j = 0; j < len; j++)
			printf("%c", buffer[offset++]);
		printf("\n");
	}

	// --- print IMPORT
	printf(" Import AIDs\n");
	if (find_component_zip(cap, COMP_IMPORT, &finfo))
	{
		uint8_t impcnt;
		size_t offset;

		len = (int)finfo.size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, finfo.index, 0);
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
	if (find_component_zip(cap, COMP_APPLET, &finfo))
	{
		uint8_t appcnt;
		size_t offset;

		len = (int)finfo.size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(cap, finfo.index, 0);
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

	counterP2 = 0;
	if (!install_for_load(cap))
		return;

	//load_component(cap, COMP_HEADER, 0); // Gets loaded inside INSTALL_for_LOAD()
	load_component(cap, COMP_DIRECTORY, 0);
	load_component(cap, COMP_IMPORT, 0);
	load_component(cap, COMP_APPLET, 0);
	load_component(cap, COMP_CLASS, 0);
	load_component(cap, COMP_METHOD, 0);
	load_component(cap, COMP_STATICFIELD, 0);
	load_component(cap, COMP_EXPORT, 0);
	load_component(cap, COMP_CONSTANTPOOL, 0);
	load_component(cap, COMP_REFLOCATION, THE_LAST_COMPONENT);
	//load_component(cap, COMP_DESCRIPTOR, 0); // -- skip loading
	//load_component(cap, COMP_DEBUG, THE_LAST_COMPONENT); // -- skip loading

	zip_close(cap);
}
