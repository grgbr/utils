/**
 * @file      slist.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Jun 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list interface
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

#ifndef _UTILS_SLIST_H
#define _UTILS_SLIST_H

#include <utils/cdefs.h>
#include <stdbool.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define slist_assert(_expr) \
	uassert("slist", _expr)

#else  /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define slist_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

/**
 * Singly linked list node
 *
 * Describes a single entry linked into a slist.
 *
 * @ingroup slist
 */
struct slist_node {
	/** Node following this node into a slist. */
	struct slist_node * slist_next;
};

/**
 * Singly linked list
 *
 * @ingroup slist
 */
struct slist {
	/** slist head allowing to locate the first linked node. */
	struct slist_node   slist_head;
	/** Last entry in this slist. */
	struct slist_node * slist_tail;
};

/**
 * slist constant initializer.
 *
 * @param _list slist variable to initialize.
 *
 * @ingroup slist
 */
#define SLIST_INIT(_list)                                     \
	{                                                     \
		.slist_head.slist_next = NULL,                \
		.slist_tail            = &(_list)->slist_head \
	}

/**
 * Initialize a slist
 *
 * @param list slist to initialize.
 *
 * @ingroup slist
 */
static inline void slist_init(struct slist *list)
{
	slist_assert(list);

	list->slist_head.slist_next = NULL;
	list->slist_tail = &list->slist_head;
}

/**
 * Test wether a slist is empty or not.
 *
 * @param list slist to test.
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup slist
 */
static inline bool slist_empty(const struct slist * list)
{
	slist_assert(list);
	slist_assert(list->slist_tail);

	return list->slist_head.slist_next == NULL;
}

/**
 * Return slist head.
 *
 * @param list slist to get head from.
 *
 * @return Pointer to list's head node.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_head(struct slist * list)
{
	slist_assert(list);
	slist_assert(list->slist_tail);

	return &list->slist_head;
}

/**
 * Get node following specified node.
 *
 * @param node Node which to get the successor from.
 *
 * @return Pointer to following node.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_next(const struct slist_node * node)
{
	slist_assert(node);

	return node->slist_next;
}

/**
 * Get first node of specified slist.
 *
 * @param list slist to get the first node from.
 *
 * @retval Pointer to first list's node.
 * @retval NULL means list is empty.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_first(const struct slist * list)
{
	slist_assert(!slist_empty(list));

	return list->slist_head.slist_next;
}

/**
 * Get last node of specified slist.
 *
 * @param list slist to get the last node from.
 *
 * @retval Pointer to last list's node.
 * @retval Pointer to list's head node means list is empty.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @see slist_head()
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_last(const struct slist * list)
{
	slist_assert(!slist_empty(list));

	return list->slist_tail;
}

/**
 * Add a node into a slist.
 *
 * @param list     slist to add node to.
 * @param previous Node preceding the one to add.
 * @param node     Node to add.
 *
 * @ingroup slist
 */
static inline void slist_append(struct slist *                 list,
                                struct slist_node * __restrict previous,
                                struct slist_node * __restrict node)
{
	slist_assert(list);
	slist_assert(!list->slist_head.slist_next || list->slist_tail);
	slist_assert(previous);
	slist_assert(node);

	if (!previous->slist_next)
		/* Update tail pointer if previous points to last node. */
		list->slist_tail = node;

	node->slist_next = previous->slist_next;
	previous->slist_next = node;
}

/**
 * Remove a node from a slist.
 *
 * @param list     slist to remove node from.
 * @param previous Node preceding the one to remove.
 * @param node     Node to remove.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline void slist_remove(struct slist *                       list,
                                struct slist_node * __restrict       previous,
                                const struct slist_node * __restrict node)
{
	slist_assert(!slist_empty(list));
	slist_assert(previous);
	slist_assert(node);
	slist_assert(previous->slist_next == node);

	if (!node->slist_next)
		list->slist_tail = previous;

	previous->slist_next = node->slist_next;
}

/**
 * Move slist node from one location to another.
 *
 * @param list     slist to insert node into
 * @param at       list's node to insert node after
 * @param previous Node preceding the node to move
 * @param node     Node to move
 *
 * @ingroup slist
 */
extern void slist_move(struct slist *                 list,
                       struct slist_node * __restrict at,
                       struct slist_node * __restrict previous,
                       struct slist_node * __restrict node);

/**
 * Add a node to the end of a slist.
 *
 * @param list     slist to add node to.
 * @param node     Node to enqueue.
 *
 * @ingroup slist
 */
static inline void slist_nqueue(struct slist *                 list,
                                struct slist_node * __restrict node)
{
	slist_assert(list);
	slist_assert(!list->slist_head.slist_next || list->slist_tail);
	slist_assert(node);

	node->slist_next = NULL;

	list->slist_tail->slist_next = node;
	list->slist_tail = node;
}

/**
 * Remove a node of the begining of a slist and return it.
 *
 * @param list     slist to dequeue node from.
 *
 * @return Pointer to dequeued node.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_dqueue(struct slist * list)
{
	slist_assert(!slist_empty(list));
	slist_assert(list->slist_tail);

	struct slist_node * node = list->slist_head.slist_next;

	list->slist_head.slist_next = node->slist_next;

	if (!node->slist_next)
		list->slist_tail = &list->slist_head;

	return node;
}

/**
 * Extract / remove a portion of nodes from a slist.
 *
 * @param list  slist to remove nodes from.
 * @param first Node preceding the first node of the portion to remove.
 * @param last  Last node of portion to remove.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline void slist_withdraw(struct slist *                       list,
                                  struct slist_node * __restrict       first,
                                  const struct slist_node * __restrict last)
{
	slist_assert(list);
	slist_assert(!slist_empty(list));
	slist_assert(first);
	slist_assert(last);

	first->slist_next = last->slist_next;

	if (!last->slist_next)
		list->slist_tail = first;
}

/**
 * Insert a portion of nodes into a slist.
 *
 * @param list  slist to insert nodes into.
 * @param at    Node after which portion is inserted.
 * @param first First node of portion to insert.
 * @param last  Last node of portion to insert.
 *
 * @ingroup slist
 */
static inline void slist_embed(struct slist *                 list,
                               struct slist_node * __restrict at,
                               struct slist_node * __restrict first,
                               struct slist_node * __restrict last)
{
	slist_assert(list);
	slist_assert(!list->slist_head.slist_next || list->slist_tail);
	slist_assert(at);
	slist_assert(first);
	slist_assert(last);

	last->slist_next = at->slist_next;
	if (!last->slist_next)
		list->slist_tail = last;

	at->slist_next = first;

}

/**
 * Extract source list portion and move it to result list at specified location.
 *
 * @param result slist to insert nodes into.
 * @param at     result's node to insert nodes after.
 * @param source slist to extract nodes from.
 * @param first  Node preceding the nodes portion to move.
 * @param last   Last portions's node to move.
 *
 * @warning Behavior is undefined when called on an empty @p source slist.
 *
 * @ingroup slist
 */
extern void
slist_splice(struct slist * __restrict      result,
             struct slist_node * __restrict at,
             struct slist * __restrict      source,
             struct slist_node * __restrict first,
             struct slist_node * __restrict last);

/**
 * Iterate over slist nodes.
 *
 * @param _list  slist to iterate over.
 * @param _node  Pointer to current node.
 *
 * @ingroup slist
 */
#define slist_foreach_node(_list, _node)             \
	for (_node = (_list)->slist_head.slist_next; \
	     _node;                                  \
	     _node = _node->slist_next)

/**
 * Return type casted pointer to entry containing specified node.
 *
 * @param _node   slist node to retrieve container from.
 * @param _type   Type of container
 * @param _member Member field of container structure pointing to _node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

/**
 * Return type casted pointer to entry containing first node of specified slist.
 *
 * @param _list   slist to get the first node from.
 * @param _type   Type of container.
 * @param _member Member field of container structure pointing to first _list's
 *                node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_first_entry(_list, _type, _member) \
	slist_entry(slist_first(_list), _type, _member)

/**
 * Return type casted pointer to entry containing last node of specified slist.
 *
 * @param _list   slist to get the last node from.
 * @param _type   Type of container.
 * @param _member Member field of container structure pointing to last _list's
 *                node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_last_entry(_list, _type, _member) \
	slist_entry(slist_last(_list), _type, _member)


/**
 * Return type casted pointer to entry following specified entry.
 *
 * @param _entry  Entry containing slist node.
 * @param _member Member field of container structure pointing to slist node.
 *
 * @return Pointer to following entry.
 *
 * @ingroup slist
 */
#define slist_next_entry(_entry, _member) \
	slist_entry(slist_next(&(_entry)->_member), typeof(*(_entry)), _member)

/**
 * Iterate over slist node container entries.
 *
 * @param _list   slist to iterate over.
 * @param _entry  Pointer to entry containing @p _list's current node.
 * @param _member Member field of container structure pointing to slist node.
 *
 * @ingroup slist
 */
#define slist_foreach_entry(_list, _entry, _member)               \
	for (_entry = slist_entry((_list)->slist_head.slist_next, \
	                          typeof(*(_entry)), _member);    \
	     &(_entry)->_member;                                  \
	     _entry = slist_next_entry(_entry, _member))

#endif /* _UTILS_SLIST_H */
