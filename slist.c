/**
 * @file      slist.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      26 Jun 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list implementation
 *
 * @defgroup slist Singly linked list
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

#include <utils/slist.h>

void slist_move(struct slist *                 list,
                struct slist_node * __restrict at,
                struct slist_node * __restrict previous,
                struct slist_node * __restrict node)
{
	slist_remove(list, previous, node);
	slist_append(list, at, node);
}

void slist_splice(struct slist * __restrict      result,
                  struct slist_node * __restrict at,
                  struct slist * __restrict      source,
                  struct slist_node * __restrict first,
                  struct slist_node * __restrict last)
{
	struct slist_node *fst = slist_next(first);

	slist_withdraw(source, first, last);
	slist_embed(result, at, fst, last);
}
