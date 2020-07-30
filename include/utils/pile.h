#ifndef _UTILS_PILE_H
#define _UTILS_PILE_H

#include <utils/dlist.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define upile_assert(_expr) \
	uassert("upile", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define upile_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

/******************************************************************************
 * Pile generic handling.
 ******************************************************************************/

struct upile {
	struct dlist_node head;
	unsigned int      nr;
	size_t            size;
};

#define upile_assert_pile(_pile) \
	upile_assert(_pile); \
	upile_assert(!(dlist_empty(&(_pile)->head) ^ (_pile)->nr)); \
	upile_assert(!((_pile)->nr ^ (_pile)->size))

static inline unsigned int
upile_nr(const struct upile *pile)
{
	upile_assert_pile(pile);

	return pile->nr;
}

static inline size_t
upile_size(const struct upile *pile)
{
	upile_assert_pile(pile);

	return pile->size;
}

static inline size_t
upile_is_empty(const struct upile *pile)
{
	upile_assert_pile(pile);

	return !pile->nr;
}

extern void
upile_clear(struct upile *pile);

extern void
upile_init(struct upile *pile);

static inline void
upile_fini(struct upile *pile)
{
	upile_clear(pile);
}

/******************************************************************************
 * Pile of strings handling.
 ******************************************************************************/

extern char *
upile_iter_next_str(const struct upile *pile, const char *str);

extern char *
upile_begin_iter_str(const struct upile *pile);

#define upile_foreach_str(_pile, _str) \
	for ((_str) = upile_begin_iter_str(_pile); \
	     (_str); \
	     (_str) = upile_iter_next_str(_pile, _str))

extern size_t
upile_str_len(const char *str);

extern size_t
upile_str_size(const char *str);

extern char *
upile_clone_str(struct upile *pile, const char *str, size_t len);

extern char *
upile_create_str(struct upile *pile, const char *str, size_t max_size);

extern void
upile_destroy_str(struct upile *pile, char *str);

#endif /* _UTILS_PILE_H */
