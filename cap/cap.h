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

#ifndef __CAP_H_
#define __CAP_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void print_cap_info(const char* _filename);
void upload(const char* filename);
void cap2ijc(char* _capname, char* _ijcname, bool verbose, bool all_components);
void cap2apdu(char* _capname, uint8_t block);

#ifdef __cplusplus
}
#endif

#endif /* __CAP_H_ */
