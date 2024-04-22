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

#include "globalplatform.h"
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
#define COMP_APPLET_IDX				3
#define COMP_IMPORT_IDX				4
#define COMP_CONSTANTPOOL_IDX		5
#define COMP_CLASS_IDX				6
#define COMP_METHOD_IDX				7
#define COMP_STATICFIELD_IDX		8
#define COMP_REFERENCELOCATION_IDX	9
#define COMP_EXPORT_IDX				10
#define COMP_DESCRIPTOR_IDX			11
#define COMP_DEBUG_IDX				12

/*
#define HEADER_ORDER			1
#define DIRECTORY_ORDER			2
#define IMPORT_ORDER			3
#define APPLET_ORDER			4
#define CLASS_ORDER				5
#define METHOD_ORDER			6
#define STATICFIELD_ORDER		7
#define EXPORT_ORDER			8
#define CONSTANTPOOL_ORDER		9
#define REFERENCELOCATION_ORDER	10
#define DESCRIPTOR_ORDER		11
#define DEBUG_ORDER				12
*/

static const char* COMP_HEADER		= "Header.cap";
static const char* COMP_DIRECTORY	= "Directory.cap";
static const char* COMP_IMPORT		= "Import.cap";
static const char* COMP_APPLET		= "Applet.cap";
static const char* COMP_CLASS		= "Class.cap";
static const char* COMP_METHOD		= "Method.cap";
static const char* COMP_STATICFIELD	= "StaticField.cap";
static const char* COMP_EXPORT		= "Export.cap";
static const char* COMP_CONSTANTPOOL = "ConstantPool.cap";
static const char* COMP_REFLOCATION	= "RefLocation.cap";
static const char* COMP_DESCRIPTOR	= "Descriptor.cap"; // Only for cap2ijc
static const char* COMP_DEBUG		= "Debug.cap"; // Only for cap2ijc

uint8_t counterP2;

#define COMPONENT_COUNT		12
static const char* COMP_NAMES[COMPONENT_COUNT] = {
	"Header.cap",
	"Directory.cap",
	"Applet.cap",
	"Import.cap",
	"ConstantPool.cap",
	"Class.cap",
	"Method.cap",
	"StaticField.cap",
	"RefLocation.cap",
	"Export.cap",
	"Descriptor.cap",
	"Debug.cap"
};

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
//	dump_hexascii_buffer("Header", header, sizeof(header));

	if ( (header[0] == 1) // Header idx
		&& (header[1] == 0) // Header length: 1st byte
		// ..length second byte
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

	printf(" Not found:" COLOR_YELLOW " %s " COLOR_RESET "\n", _cname);

	return false;
}

static bool install_for_load_APDU(uint8_t* _buffer_header, int _total_sz)
{
	apdu_t apdu;
	int header_len;

	header_len = 3 + _buffer_header[2];

	// --- INSTALL for LOAD ---
	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_INSTALL;
	apdu.cmd[apdu.cmd_len++] = INSTALL_FOR_LOAD;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0; // To be updated

	//
	memcpy(&apdu.cmd[apdu.cmd_len], &_buffer_header[12], 1 + _buffer_header[12]);
	apdu.cmd_len += (1 + _buffer_header[12]);

	//apdu.cmd[apdu.cmd_len++] = 0; // Target SD (AID length)
	apdu.cmd[apdu.cmd_len++] = sizeof(GP_ISD);
	memcpy(&apdu.cmd[apdu.cmd_len], GP_ISD, sizeof(GP_ISD));
	apdu.cmd_len += sizeof(GP_ISD);

	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = 0;

	apdu.cmd[4] = apdu.cmd_len - 5;
	gp_send_APDU(&apdu);

	// --- Load header.CAP ---
	printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", COMP_HEADER, header_len);

	apdu.cmd_len = 0;
	apdu.cmd[apdu.cmd_len++] = 0x80;
	apdu.cmd[apdu.cmd_len++] = INS_GP_LOAD;
	apdu.cmd[apdu.cmd_len++] = 0;
	apdu.cmd[apdu.cmd_len++] = counterP2++;

	apdu.cmd[apdu.cmd_len++] = 0; // to be updated
	apdu.cmd[apdu.cmd_len++] = 0xC4;
	if (_total_sz > 255) {
		apdu.cmd[apdu.cmd_len++] = 0x82;
		apdu.cmd[apdu.cmd_len++] = _total_sz >> 8;
	}
	else
		if (_total_sz > 127) {
			apdu.cmd[apdu.cmd_len++] = 0x81;
		}
	apdu.cmd[apdu.cmd_len++] = _total_sz & 0xFF;

	memcpy(&apdu.cmd[apdu.cmd_len], _buffer_header, header_len);
	apdu.cmd_len += header_len;

	apdu.cmd[4] = apdu.cmd_len - 5;
	gp_send_APDU(&apdu);

	return true;
}

static int load_size_cap(zip_t* _cap, struct zip_stat* _finfo)
{
	int size = 0;

	// --- 1 ---
	// Header size already taken

	// --- 2 ---
	if (find_component_zip(_cap, COMP_DIRECTORY, _finfo))
		size += (int)_finfo->size;

	// --- 3 ---
	if (find_component_zip(_cap, COMP_IMPORT, _finfo))
		size += (int)_finfo->size;

	// --- 4 ---
	if (find_component_zip(_cap, COMP_APPLET, _finfo))
		size += (int)_finfo->size;

	// --- 5 ---
	if (find_component_zip(_cap, COMP_CLASS, _finfo))
		size += (int)_finfo->size;

	// --- 6 ---
	if (find_component_zip(_cap, COMP_METHOD, _finfo))
		size += (int)_finfo->size;

	// --- 7 ---
	if (find_component_zip(_cap, COMP_STATICFIELD, _finfo))
		size += (int)_finfo->size;

	// --- 8 ---
	if (find_component_zip(_cap, COMP_EXPORT, _finfo))
		size += (int)_finfo->size;

	// --- 9 ---
	if (find_component_zip(_cap, COMP_CONSTANTPOOL, _finfo))
		size += (int)_finfo->size;

	// --- 10 ---
	if (find_component_zip(_cap, COMP_REFLOCATION, _finfo))
		size += (int)_finfo->size;

	return size;
}

static int load_size_ijc_(FILE* _fp)
{
	int size = 0;
	int comp_size;
	int comp_idx;
	uint8_t buffer[8];

	fseek(_fp, 0, SEEK_SET);

	while (1) {
		;
		if (3 != fread(buffer, 1, 3, _fp))
			break;

		comp_idx = buffer[0];
		comp_size = buffer[1] * 0x100 + buffer[2];

		switch (comp_idx) {
			//case COMP_HEADER_IDX: // Already taken
		case COMP_DIRECTORY_IDX:
		case COMP_APPLET_IDX:
		case COMP_IMPORT_IDX:
		case COMP_CONSTANTPOOL_IDX:
		case COMP_CLASS_IDX:
		case COMP_METHOD_IDX:
		case COMP_STATICFIELD_IDX:
		case COMP_REFERENCELOCATION_IDX:
		case COMP_EXPORT_IDX:
			//case COMP_DESCRIPTOR_IDX:
			//case COMP_DEBUG_IDX:
			size += (3 + comp_size);
			break;

		default:
			;
		}

		fseek(_fp, comp_size, SEEK_CUR);
	}

	return size;
}

static int copy_component(zip_t* _cap, const char* _cname, FILE* _dest, bool verbose)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	uint8_t component[256];

	int offset;
	int len;
	int len_read;
	int idx;

	zip_stat_init(&finfo);

	if (!find_component_zip(_cap, _cname, &finfo))
	{
		return 0;
	}

	idx = (int)finfo.index;
	len = (int)finfo.size;

	if (verbose)
		printf("Start copying " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", _cname, len);

	fd = zip_fopen_index(_cap, idx, 0);

	offset = 0;
	while (offset < len) {
		len_read = len - offset;
		if (len_read > 255)
			len_read = 255;

		len_read = (int)zip_fread(fd, component, len_read);

		fwrite(component, 1, len_read, _dest);

		offset += len_read;
	}

	return len;
}

static bool load_component_ijc(FILE* _fp, int _comp_idx, uint8_t _last)
{
	apdu_t apdu;

	int offset;
	int len;
	int len_read;
	int comp_len;
	uint8_t buffer[256];

	fseek(_fp, 0, SEEK_SET);

	while (1) {
		if (0 == fread(buffer, 1, 3, _fp))
			break;

		comp_len = (buffer[1] << 8) + buffer[2];

		if (_comp_idx == buffer[0]) {
			len = 3 + comp_len;

			if (_comp_idx < COMPONENT_COUNT) {
				printf("Start loading " COLOR_YELLOW "%s" COLOR_RESET " (%d byte)\n", COMP_NAMES[_comp_idx], len);
			}
			else {
				printf("Start loading " COLOR_RED "Component %d" COLOR_RESET " (%d byte)\n", _comp_idx, len);
			}

			apdu.cmd[0] = 0x80;
			apdu.cmd[1] = INS_GP_LOAD;
			apdu.cmd[2] = 0;
			//apdu.cmd[3] = counterP2;
			//apdu.cmd[4] = 0;

			fseek(_fp, -3, SEEK_CUR);
			offset = 0;
			while (offset < len) {
				len_read = len - offset;
				if (len_read > 255)
					len_read = 255;

				fread(&apdu.cmd[5], 1, len_read, _fp);

				apdu.cmd[3] = counterP2++;
				apdu.cmd[4] = len_read;
				apdu.cmd_len = 5 + len_read;

				offset += len_read;

				if (_last && offset == len)
					apdu.cmd[2] = 0x80;

				gp_send_APDU(&apdu);
				apdu.sw_ = apdu.resp[apdu.resp_len - 2] * 256 + apdu.resp[apdu.resp_len - 1];

				if (apdu.sw_ != 0x9000 && apdu.sw_ != 0x6101)
					return false;
			}

			break;
		}

		fseek(_fp, comp_len, SEEK_CUR);
	}

	return true;
}

static bool load_component_zip(zip_t* _cap, const char* _cname, uint8_t _last)
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
		return false;
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

		gp_send_APDU(&apdu);
		apdu.sw_ = apdu.resp[apdu.resp_len - 2] * 256 + apdu.resp[apdu.resp_len - 1];

		if (apdu.sw_ != 0x9000 && apdu.sw_ != 0x6101) {
			zip_fclose(fd);
			return false;
		}

	}
	zip_fclose(fd);

	return true;
}

static void save_apdu(FILE* fsimshell, FILE* fpcom, uint8_t* apdu, int len)
{
	uint8_t apdu_s[(255 + 5) * 2 + 5]; // +5 for SPACEs, CRLF

	convert_bin2hex(apdu, apdu_s, 4);
	apdu_s[8] = ' ';
	convert_bin2hex(&apdu[4], &apdu_s[9], 1);
	apdu_s[11] = ' ';
	convert_bin2hex(&apdu[5], &apdu_s[12], len - 5);

	fprintf(fpcom, "%s (6101)\n", apdu_s);
//	fprintf(fpcom, "00C00000 01 [00] (9000)\n");
	fprintf(fsimshell, "/send \"%s\" 009000\n", apdu_s);
}

static int component_zip_2_apdu(zip_t* _cap, const char* _cname, FILE* fsimshell, FILE* fpcom, uint8_t _last)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	uint8_t apdu[255 + 5];

	int offset;
	int comp_sz;
	int len_read;
	int apdu_len;
	int idx;

	zip_stat_init(&finfo);

	if (!find_component_zip(_cap, _cname, &finfo))
	{
		return 0;
	}

	idx = (int)finfo.index;
	comp_sz = (int)finfo.size;

	fd = zip_fopen_index(_cap, idx, 0);

	apdu[0] = 0x80;
	apdu[1] = INS_GP_LOAD;
	apdu[2] = 0;
	//apdu.cmd[3] = counterP2;
	//apdu.cmd[4] = 0;

	offset = 0;
	while (offset < comp_sz) {
		len_read = comp_sz - offset;
		if (len_read > 255)
			len_read = 255;

		len_read = (int)zip_fread(fd, &apdu[5], len_read);

		apdu[3] = counterP2++;
		apdu[4] = len_read;
		apdu_len = 5 + len_read;

		offset += len_read;

		if (_last && offset == comp_sz)
			apdu[2] = 0x80;

		save_apdu(fsimshell, fpcom, apdu, apdu_len);
	}

	return comp_sz;
}

static void print_components_zip(zip_t* _cap, struct zip_stat* _finfo)
{
	int count;
	int len;

	count = 0;
	while ((zip_stat_index(_cap, count, 0, _finfo)) == 0)
	{
		char* component;

		len = (int)strlen(_finfo->name);
		component = (char*)&_finfo->name[len - 3];

		// To exclude \META-INF\MANIFEST.MF etc
		if (component[0] == 'c' && component[1] == 'a' && component[2] == 'p') {
			len = (int)_finfo->size;

			while (*component != '/')
				component--;

			component++; // next after '/'

			printf("   %-16s : %d bytes\n", component, (int)len);
		}

		count++;
	}
}
static void print_header_component(uint8_t* _buffer)
{
	int len;
	int offset;
	int comp_sz;

	comp_sz = _buffer[1] * 0x100 + _buffer[2] + 3;

	offset = 7;
	printf(" CAP file version   : %d.%d\n", _buffer[offset + 1], _buffer[offset]);
	offset += 2;

	printf(" Integer support    : %s\n", _buffer[offset] & 0x01 ? "Yes" : "No");
	printf(" Export component   : %s\n", _buffer[offset] & 0x02 ? "Yes" : "No");
	//printf(" Applet component   : %s\n", _buffer[offset] & 0x04 ? "Yes" : "No");
	offset++;

	printf(" Package version    : %d.%d\n", _buffer[offset + 1], _buffer[offset]);
	offset += 2;

	len = _buffer[offset++];

	printf(" Package AID        : ");
	for (int j = 0; j < len; j++)
		printf("%02X", _buffer[offset++]);
	printf("\n");

	printf(" Internal pkg name  : ");
	if (comp_sz > offset) {
		len = _buffer[offset++];

		for (int j = 0; j < len; j++)
			printf("%c", _buffer[offset++]);
	}
	printf("\n");
}

static void print_import_component(uint8_t* _buffer)
{
	int offset;
	uint8_t impcnt;

	offset = 3;
	impcnt = _buffer[offset++];

	while (impcnt--)
	{
		uint8_t min = _buffer[offset++];
		uint8_t maj = _buffer[offset++];
		uint8_t sz = _buffer[offset++];

		printf("   ");
		for (size_t j = 0; j < sz; j++)
			printf("%02X", _buffer[offset++]);
		printf("  version %d.%d\n", maj, min);
	}
}

static void print_applet_component(uint8_t* _buffer)
{
	int offset;
	uint8_t appcnt;

	offset = 3;
	appcnt = _buffer[offset++];

	while (appcnt--)
	{
		uint8_t sz = _buffer[offset++];

		printf("   ");
		for (size_t j = 0; j < sz; j++)
			printf("%02X", _buffer[offset++]);
		printf("\n");

		offset += 2; // u2 install_method_offset
	}
}

static void print_header_zip(zip_t* _cap, struct zip_stat* _finfo)
{
	int len;
	uint8_t buffer[256];
	zip_file_t* fd = NULL;

	if (find_component_zip(_cap, COMP_HEADER, _finfo))
	{
		int len_read;

		len = (int)_finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(_cap, _finfo->index, 0);
		len_read = (int)zip_fread(fd, buffer, len);
		zip_fclose(fd);

		print_header_component(buffer);
	}
}

static void print_import_zip(zip_t* _cap, struct zip_stat* _finfo)
{
	int len;
	uint8_t buffer[256];
	zip_file_t* fd = NULL;

	if (find_component_zip(_cap, COMP_IMPORT, _finfo))
	{
		int len_read;

		len = (int)_finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(_cap, _finfo->index, 0);
		len_read = (int)zip_fread(fd, buffer, len);
		zip_fclose(fd);

		print_import_component(buffer);
	}
}


static void print_applet_zip(zip_t* _cap, struct zip_stat* _finfo)
{
	int len;
	uint8_t buffer[256];
	zip_file_t* fd = NULL;

	if (find_component_zip(_cap, COMP_APPLET, _finfo))
	{
		int len_read;

		len = (int)_finfo->size;

		if (len > sizeof(buffer))
			len = sizeof(buffer);

		fd = zip_fopen_index(_cap, _finfo->index, 0);
		len_read = (int)zip_fread(fd, buffer, len);
		zip_fclose(fd);

		print_applet_component(buffer);
	}
}

static bool read_component_ijc(FILE* fp, uint8_t comp_idx, uint8_t* _dest)
{
	//
	// Only for small components: Header, Import, Applet
	//

	int comp_len;
	bool found = false;

	fseek(fp, 0, SEEK_SET);

	while (1) {
		if (0 == fread(_dest, 1, 3, fp))
			break;

		//comp_len = (_dest[1] << 8) + _dest[2];
		comp_len = _dest[2]; // To prevent destination buffer overflow

		if (_dest[0] == comp_idx)
		{
			fread(&_dest[3], 1, comp_len, fp);

			//dump_hexascii_buffer("Component %d", comp_idx, _dest, comp_len+3);

			found = true;
			break;
		}

		fseek(fp, comp_len, SEEK_CUR);
	}

	return found;
}

static void print_components_icj(FILE* _fp)
{
	uint8_t buffer[8];
	int comp_len;

	fseek(_fp, 0, SEEK_SET);

	while (1) {
		if (0 == fread(buffer, 1, 3, _fp))
			break;
		
		//printf("Component : %02X %02X %02X\n", buffer[0], buffer[1], buffer[2]);

		comp_len = (buffer[1] << 8) + buffer[2];

		switch (buffer[0]) {
		case COMP_HEADER_IDX:
			printf("   %-16s :", COMP_HEADER);
			break;
		case COMP_DIRECTORY_IDX:
			printf("   %-16s :", COMP_DIRECTORY);
			break;
		case COMP_IMPORT_IDX:
			printf("   %-16s :", COMP_IMPORT);
			break;
		case COMP_APPLET_IDX:
			printf("   %-16s :", COMP_APPLET);
			break;
		case COMP_CLASS_IDX:
			printf("   %-16s :", COMP_CLASS);
			break;
		case COMP_METHOD_IDX:
			printf("   %-16s :", COMP_METHOD);
			break;
		case COMP_STATICFIELD_IDX:
			printf("   %-16s :", COMP_STATICFIELD);
			break;
		case COMP_EXPORT_IDX:
			printf("   %-16s :", COMP_EXPORT);
			break;
		case COMP_CONSTANTPOOL_IDX:
			printf("   %-16s :", COMP_CONSTANTPOOL);
			break;
		case COMP_REFERENCELOCATION_IDX:
			printf("   %-16s :", COMP_REFLOCATION);
			break;
		case COMP_DESCRIPTOR_IDX:
			printf("   %-16s :", COMP_DESCRIPTOR);
			break;
		case COMP_DEBUG_IDX:
			printf("   %-16s :", COMP_DEBUG);
			break;
		default:
			printf(" Unknown component: %d bytes\n", comp_len);
		}
		printf(" %d bytes\n", comp_len + 3);

		fseek(_fp, comp_len, SEEK_CUR);
	}
}

static void print_header_ijc(FILE* _fp)
{
	uint8_t buffer[256];

	if (read_component_ijc(_fp, COMP_HEADER_IDX, buffer))
	{
		print_header_component(buffer);
	}
}

static void print_import_ijc(FILE* _fp)
{
	uint8_t buffer[256];

	if (read_component_ijc(_fp, COMP_IMPORT_IDX, buffer))
	{
		print_import_component(buffer);
	}
}

static void print_applet_ijc(FILE* _fp)
{
	uint8_t buffer[256];

	if (read_component_ijc(_fp, COMP_APPLET_IDX, buffer))
	{
		print_applet_component(buffer);
	}
}

void print_cap_info(const char* _filename)
{
	FILE* fp = NULL;
	zip_t* cap = NULL;
	struct zip_stat finfo;
	int errorp = 0;
	bool is_zip;

	printf(" CAP file name: " COLOR_GREEN "%s\n" COLOR_RESET, _filename);

	is_zip = !is_IJC(_filename);

	if (is_zip) {
		cap = zip_open(_filename, 0, &errorp);

		if (cap == NULL) {
			printf("Failed to open CAP file\n");
			return;
		}

		zip_stat_init(&finfo);

		print_components_zip(cap, &finfo);
		print_header_zip(cap, &finfo);
	}
	else {
		fp = fopen(_filename, "rb");

		if (NULL == fp) {
			printf(" Failed to open file %s..\n", _filename);

			return;
		}

		print_components_icj(fp);
		print_header_ijc(fp);
	}

	// --- print IMPORT
	printf(" Import AIDs\n");
	if (is_zip) {
		print_import_zip(cap, &finfo);
	}
	else {
		print_import_ijc(fp);
	}
	
	// --- print APPLETs
	printf(" Applets\n");
	if (is_zip) {
		print_applet_zip(cap, &finfo);
	}
	else {
		print_applet_ijc(fp);
	}

	if (is_zip) {
		if (0 != zip_close(cap)) {
			printf(" ERROR closing ZIP..\n");
		}
	}
	else {
		fclose(fp);
	}
}

static bool install_for_load_zip(zip_t* _cap)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	int count = 0;
	int total_sz;
	uint8_t buffer_header[256];
	int header_len;

	zip_stat_init(&finfo);

	// --- 1 ---
	if (!find_component_zip(_cap, COMP_HEADER, &finfo))
	{
		return false;
	}

	header_len = (int)finfo.size;
	fd = zip_fopen_index(_cap, finfo.index, 0);
	total_sz = (int)zip_fread(fd, buffer_header, header_len);
	zip_fclose(fd);

	total_sz += load_size_cap(_cap, &finfo);

	return install_for_load_APDU(buffer_header, total_sz);
}

static bool install_for_load_2_apdu(zip_t * _cap, FILE* fsimshell, FILE* fpcom, bool load_header)
{
	struct zip_stat finfo;
	zip_file_t* fd = NULL;
	int count = 0;
	int total_sz;
	uint8_t buffer_header[256];
	int header_len;

	int len = 0;
	uint8_t apdu[255 + 5];

	zip_stat_init(&finfo);

	// --- 1 ---
	if (!find_component_zip(_cap, COMP_HEADER, &finfo))
	{
		return false;
	}

	header_len = (int)finfo.size;
	fd = zip_fopen_index(_cap, finfo.index, 0);
	total_sz = (int)zip_fread(fd, buffer_header, header_len);

	total_sz += load_size_cap(_cap, &finfo);

	// --- INSTALL for LOAD ---
	len = 0;
	apdu[len++] = 0x80;
	apdu[len++] = INS_GP_INSTALL;
	apdu[len++] = INSTALL_FOR_LOAD;
	apdu[len++] = 0;
	apdu[len++] = 0; // To be updated

	//
	memcpy(&apdu[len], &buffer_header[12], 1 + buffer_header[12]);
	len += (1 + buffer_header[12]);

	//apdu.cmd[apdu.cmd_len++] = 0; // Target SD (AID length)
	apdu[len++] = sizeof(GP_ISD);
	memcpy(&apdu[len], GP_ISD, sizeof(GP_ISD));
	len += sizeof(GP_ISD);

	apdu[len++] = 0;
	apdu[len++] = 0;
	apdu[len++] = 0;

	apdu[4] = len - 5;

	save_apdu(fsimshell, fpcom, apdu, len);

	// --- Load header.CAP ---
	if (load_header) {
		len = 0;
		apdu[len++] = 0x80;
		apdu[len++] = INS_GP_LOAD;
		apdu[len++] = 0;
		apdu[len++] = counterP2++;

		apdu[len++] = 0; // to be updated
		apdu[len++] = 0xC4;
		if (total_sz > 255) {
			apdu[len++] = 0x82;
			apdu[len++] = total_sz >> 8;
		}
		else
			if (total_sz > 127) {
				apdu[len++] = 0x81;
			}
		apdu[len++] = total_sz & 0xFF;

		memcpy(&apdu[len], buffer_header, header_len);
		len += header_len;

		apdu[4] = len - 5;

		save_apdu(fsimshell, fpcom, apdu, len);
	}

	return true;
}


void static upload_cap(const char* filename)
{
	zip_t* cap = NULL; 
	int errorp = 0;

	cap = zip_open(filename, 0, &errorp); 

	if (cap == NULL) {
		printf("Failed to open CAP file %s\n", filename);

		return;
	}

	counterP2 = 0;
	if (!install_for_load_zip(cap))
		return;

	//load_component_zip(cap, COMP_HEADER, 0); // Gets loaded inside INSTALL_for_LOAD()
	if (!load_component_zip(cap, COMP_DIRECTORY, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_IMPORT, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_APPLET, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_CLASS, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_METHOD, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_STATICFIELD, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_EXPORT, 0)) {
		//goto end_load_cap; // Do not stop if EXPORT not found
	}
	if (!load_component_zip(cap, COMP_CONSTANTPOOL, 0))
		goto end_load_cap;
	if (!load_component_zip(cap, COMP_REFLOCATION, THE_LAST_COMPONENT))
		goto end_load_cap;
	//if (!load_component_zip(cap, COMP_DESCRIPTOR, 0)) // -- skip loading
	//	goto end_load_cap;
	//if (!load_component_zip(cap, COMP_DEBUG, 0))
	//	goto end_load_cap;

end_load_cap:

	zip_close(cap);
}

static bool install_for_load_ijc(FILE* _fp)
{
	int count = 0;
	int total_sz;
	uint8_t buffer_header[256];

	// --- 1 ---
	if (!read_component_ijc(_fp, COMP_HEADER_IDX, buffer_header))
	{
		return false;
	}

	fseek(_fp, 0, SEEK_SET);

	total_sz = 3 + buffer_header[2]; // Header component length

	total_sz += load_size_ijc_(_fp);

	install_for_load_APDU(buffer_header, total_sz);

	return true;
}

void static upload_ijc(const char* _filename)
{
	FILE* fp;
	bool found = false;

	fp = fopen(_filename, "rb");

	if (NULL == fp) {
		printf(" Failed to open file %s..\n", _filename);

		return;
	}

	counterP2 = 0;
	if (!install_for_load_ijc(fp)) {
		fclose(fp);

		return;
	}

	//load_component_ijc(fp, COMP_HEADER_IDX, 0); // Gets loaded inside INSTALL_for_LOAD()
	if (!load_component_ijc(fp, COMP_DIRECTORY_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_IMPORT_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_APPLET_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_CLASS_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_METHOD_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_STATICFIELD_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_EXPORT_IDX, 0)) {
		//goto end_load_zip; // Do not stop if EXPORT not found
	}
	if (!load_component_ijc(fp, COMP_CONSTANTPOOL_IDX, 0))
		goto end_load_zip;
	if (!load_component_ijc(fp, COMP_REFERENCELOCATION_IDX, THE_LAST_COMPONENT))
		goto end_load_zip;
	//if (!load_component_ijc(fp, COMP_DESCRIPTOR_IDX, 0)) // -- skip loading
	//	goto end_load_zip;
	//if (!load_component_ijc(fp, THE_LAST_COMPONENT, 0)) // -- skip loading
	//	goto end_load_zip;

end_load_zip:
	fclose(fp);
}

void upload(const char* _filename)
{
	if (is_IJC(_filename)) {
		upload_ijc(_filename);
	}
	else {
		upload_cap(_filename);
	}
}

void cap2ijc(char* _capname, char* _ijcname, bool verbose, bool all_components)
{
	zip_t* cap = NULL;
	FILE* fijc;
	int errorp = 0;
	int flen;

	cap = zip_open(_capname, 0, &errorp);

	if (cap == NULL) {
		printf("Failed to open CAP file %s\n", _capname);

		return;
	}

	fijc = fopen(_ijcname, "wb+");

	flen = 0;
	flen += copy_component(cap, COMP_HEADER, fijc, verbose);
	flen += copy_component(cap, COMP_DIRECTORY, fijc, verbose);
	flen += copy_component(cap, COMP_IMPORT, fijc, verbose);
	flen += copy_component(cap, COMP_APPLET, fijc, verbose);
	flen += copy_component(cap, COMP_CLASS, fijc, verbose);
	flen += copy_component(cap, COMP_METHOD, fijc, verbose);
	flen += copy_component(cap, COMP_STATICFIELD, fijc, verbose);
	flen += copy_component(cap, COMP_EXPORT, fijc, verbose);
	flen += copy_component(cap, COMP_CONSTANTPOOL, fijc, verbose);
	flen += copy_component(cap, COMP_REFLOCATION, fijc, verbose);
	if (all_components) {
		flen += copy_component(cap, COMP_DESCRIPTOR, fijc, verbose);
		flen += copy_component(cap, COMP_DEBUG, fijc, verbose);
	}

	zip_close(cap);
	fclose(fijc);

	if (verbose)
		printf("\n Total IJC size: %d bytes\n", flen);
}

void cap2apdu(char* _capname, uint8_t block)
{
	zip_t* cap = NULL;
	FILE* fpcom;
	FILE* fsimshell;
	int errorp = 0;
	int flen;
	char pcom_name[1024];
	char simshell_name[1024];

	cap = zip_open(_capname, 0, &errorp);

	if (cap == NULL) {
		printf("Failed to open CAP file %s\n", _capname);

		return;
	}

	flen = (int)strlen(_capname);
	strcpy(simshell_name, _capname);
	simshell_name[flen++] = '.';
	simshell_name[flen++] = 's';
	simshell_name[flen++] = 'i';
	simshell_name[flen++] = 'm';
	simshell_name[flen++] = 's';
	simshell_name[flen++] = 'h';
	simshell_name[flen++] = 0x00;

	flen = (int)strlen(_capname);
	strcpy(pcom_name, _capname);
	pcom_name[flen++] = '.';
	pcom_name[flen++] = 'p';
	pcom_name[flen++] = 'c';
	pcom_name[flen++] = 'o';
	pcom_name[flen++] = 'm';
	pcom_name[flen++] = 0x00;

	fsimshell = fopen(simshell_name, "wt+");
	fpcom = fopen(pcom_name, "wt+");

//	fprintf(simshell_name, "/card\n");
//	fprintf(simshell_name, "auth\n\n");

	flen = 0;
	counterP2 = 0;

	if (!install_for_load_2_apdu(cap, fsimshell, fpcom, (block == 0)))
		return;

	if (0 == block) {
		// COMP_HEADER loaded during Install for LOAD
		// flen += component_zip_2_apdu(cap, COMP_HEADER, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_DIRECTORY, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_IMPORT, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_APPLET, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_CLASS, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_METHOD, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_STATICFIELD, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_EXPORT, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_CONSTANTPOOL, fsimshell, fpcom, false);
		flen += component_zip_2_apdu(cap, COMP_REFLOCATION, fsimshell, fpcom, true);
		//flen += component_zip_2_apdu(cap, COMP_DESCRIPTOR, fsimshell, fpcom, false);
		//flen += component_zip_2_apdu(cap, COMP_DEBUG, fsimshell, fpcom, true);
	}
	zip_close(cap);

	if (block > 0) {
		char ijcname[1024];
		FILE* fijc;
		int ijc_sz;
		uint8_t apdu[255 + 5];

		flen = (int)strlen(_capname);
		strcpy(ijcname, _capname);
		ijcname[flen++] = '~';
		ijcname[flen++] = 'i';
		ijcname[flen++] = 'j';
		ijcname[flen++] = 'c';
		ijcname[flen++] = 0x00;

		cap2ijc(_capname, ijcname, 
			false, // == Verbose
			false); // == All components

		fijc = fopen(ijcname, "rb");

		fseek(fijc, 0, SEEK_END);
		ijc_sz = ftell(fijc);

		fseek(fijc, 0, SEEK_SET);

		counterP2 = 0;

		apdu[0] = 0x80;
		apdu[1] = INS_GP_LOAD;
		apdu[2] = 0;
		apdu[3] = counterP2++;
		apdu[4] = 0; // To be updated

		{
			int len_read;

			int len = 5;
			apdu[len++] = 0xC4;
			if (ijc_sz > 255) {
				apdu[len++] = 0x82;
				apdu[len++] = ijc_sz >> 8;
			}
			else
				if (ijc_sz > 127) {
					apdu[len++] = 0x81;
				}
			apdu[len++] = ijc_sz & 0xFF;

			len_read = (int)fread(&apdu[len], 1, block + 5 - len, fijc);
			apdu[4] = block;
			ijc_sz -= len_read;

			save_apdu(fsimshell, fpcom, apdu, block + 5);

			while (ijc_sz) {
				len_read = (int)fread(&apdu[5], 1, block, fijc);

				apdu[3] = counterP2++;
				apdu[4] = len_read;

				ijc_sz -= len_read;

				if (ijc_sz == 0)
					apdu[2] = 0x80;

				save_apdu(fsimshell, fpcom, apdu, len_read + 5);
			}
		}


		fclose(fijc);
		remove(ijcname);
	}

	fclose(fsimshell);
	fclose(fpcom);

	printf("\n Total APDU payload: %d bytes\n", flen);
}
