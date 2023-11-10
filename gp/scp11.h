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

#ifndef __SCP11_H_
#define __SCP11_H_

#define SECURE_CHANNEL_PROTOCOL_11		0x11

#if defined(__cplusplus)
extern "C" {
#endif
	
void cmd_scp11_perform_security_operation(void); // SCP11.A
void cmd_scp11_mutual_authenticate(void); // SCP11.A
void cmd_scp11_internal_authenticate(void); // SCP11.B

#if defined(__cplusplus)
}
#endif

#endif /* __SCP11_H_ */
