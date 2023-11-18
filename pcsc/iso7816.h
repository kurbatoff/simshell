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

#define ISO7816_OFFSET_CLA		0x00
#define ISO7816_OFFSET_INS		0x01
#define ISO7816_OFFSET_P1		0x02
#define ISO7816_OFFSET_P2		0x03
#define ISO1716_OFFSET_LC		0x04
#define ISO7816_OFFSET_CDATA	0x05

#define ISO1716_OFFSET_HEADER_LEN	5

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

