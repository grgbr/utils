/**
 * @file      dlist.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list implementation
 *
 * @defgroup dlist Doubly linked list
 *
 * This file is part of Utils
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <utils/dlist.h>

void dlist_splice(struct dlist_node * __restrict at,
                  struct dlist_node *            first,
                  struct dlist_node *            last)
{
	dlist_withdraw(first, last);
	dlist_embed(at, first, last);
}
