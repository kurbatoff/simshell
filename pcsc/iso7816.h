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

#ifndef __ISO7816_H__
#define __ISO7816_H__

#include <stdint.h>

 // APDU
extern uint8_t gCMDbuff[256 + 5];
extern uint16_t gCMDlen;
extern uint8_t gRESPbuff[256 + 2];
extern uint16_t gRESPlen;


#if defined(__cplusplus)
extern "C" {
#endif

//
//
//

#if defined(__cplusplus)
}
#endif

#endif /* __ISO7816_H__ */

