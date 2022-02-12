/**
 * @file
 * Assertion implementation
 *
 * @ingroup      assert
 * @author       Grégor Boirie <gregor.boirie@free.fr>
 * @date         29 Aug 2017
 * @copyright    Copyright (C) 2017-2021 Grégor Boirie.
 * @licensestart GNU Lesser General Public License (LGPL) v3
 *
 * This file is part of libutils
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, If not, see <http://www.gnu.org/licenses/>.
 * @licenseend
 */
#include "utils/assert.h"
#include <stdio.h>
#include <stdlib.h>

void
uassert_fail(const char * __restrict prefix,
             const char * __restrict expr,
             const char * __restrict file,
             unsigned int            line,
             const char * __restrict func)
{
	fflush(NULL);

	fprintf(stderr,
	        "{assert} %s:%s:%u:%s:\'%s\' failed\n",
	        prefix,
	        file,
	        line,
	        func,
	        expr);

	abort();
}
