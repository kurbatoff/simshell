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

#ifndef __MBEDTESTS_H__
#define __MBEDTESTS_H__

#include "mbedwrap.h"

// Elliptic curves supported, as defined by GP 2.3 Table B-2: Key Parameter Reference Values
#define CURVE_P256				00 // as specified in [FIPS 186-4]		NID_X9_62_prime256v1
#define CURVE_P384				01 // -"-
#define CURVE_P521				02 // -"-
#define CURVE_brainpoolP256r1	03 // as specified in [RFC 5639]		NID_brainpoolP256r1
#define CURVE_brainpoolP256t1	04 // -"-								NID_brainpoolP256t1
#define CURVE_brainpoolP384r1	05 // -"-
#define CURVE_brainpoolP384t1	06 // -"-
#define CURVE_brainpoolP512r1	07 // -"-
#define CURVE_brainpoolP512t1	08 // -"-

//#define NID_X9_62_prime256v1            415

int ecc_main(void);

void mbedtls_compute_public_keys(int curve_id, const uint8_t SK_buff[M2M_ECC_SECRET_KEY_LEN], uint8_t PK_buff[M2M_ECC_PUBLIC_KEY_LEN]);


#endif /* __MBEDTESTS_H__ */

