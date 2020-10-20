#ifndef _UTILS_BITMAP_H
#define _UTILS_BITMAP_H

#include <utils/bitops.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define fbmp_assert(_expr) \
	uassert("fbmp", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define fbmp_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline unsigned int
bmp_word_no(unsigned int bit_no)
{
	return bit_no >> UWORD_SHIFT;
}

static inline unsigned int
bmp_word_nr(unsigned int bit_nr)
{
	return bmp_word_no(bit_nr + __WORDSIZE - 1);
}

static inline unsigned int
bmp_word_bit_no(unsigned int bit_no)
{
	return bit_no & ((1UL << UWORD_SHIFT) - 1);
}

static inline unsigned long
bmp_word_bit_mask(unsigned int bit_no)
{
	return 1UL << bmp_word_bit_no(bit_no);
}

struct fbmp {
	unsigned int   nr;
	unsigned long *bits;
};

#define fbmp_assert_map(_bmp) \
	fbmp_assert(_bmp); \
	fbmp_assert((_bmp)->nr); \
	fbmp_assert((_bmp)->bits)

static inline unsigned int
fbmp_nr(const struct fbmp *bmp)
{
	fbmp_assert_map(bmp);

	return bmp->nr;
}

static inline bool
fbmp_test(const struct fbmp *bmp, unsigned int bit_no)
{
	fbmp_assert_map(bmp);

	return !!(bmp->bits[bmp_word_no(bit_no)] & bmp_word_bit_mask(bit_no));
}

extern bool
fbmp_test_range(const struct fbmp *bmp,
                unsigned int       start_bit,
                unsigned int       bit_count);

extern bool
fbmp_test_all(const struct fbmp *bmp);

static inline void
fbmp_set(struct fbmp *bmp, unsigned int bit_no)
{
	fbmp_assert_map(bmp);
	fbmp_assert(bit_no < bmp->nr);

	bmp->bits[bmp_word_no(bit_no)] |= bmp_word_bit_mask(bit_no);
}

static inline void
fbmp_set_all(struct fbmp *bmp)
{
	fbmp_assert_map(bmp);

	memset(bmp->bits, 0xff, bmp_word_nr(bmp->nr) * sizeof(*bmp->bits));
}


static inline void
fbmp_clear(struct fbmp *bmp, unsigned int bit_no)
{
	fbmp_assert_map(bmp);
	fbmp_assert(bit_no < bmp->nr);

	bmp->bits[bmp_word_no(bit_no)] &= ~(bmp_word_bit_mask(bit_no));
}

static inline void
fbmp_clear_all(struct fbmp *bmp)
{
	fbmp_assert_map(bmp);

	memset(bmp->bits, 0, bmp_word_nr(bmp->nr) * sizeof(*bmp->bits));
}

extern int
fbmp_init(struct fbmp *bmp, unsigned int bit_nr);

extern int
fbmp_init_set(struct fbmp *bmp, unsigned int bit_nr);

static inline void
fbmp_fini(struct fbmp *bmp)
{
	fbmp_assert_map(bmp);

	free(bmp->bits);
}

struct fbmp_iter {
	unsigned long      word;
	unsigned int       curr;
	unsigned int       nr;
	const struct fbmp *bmp;
};

extern int
fbmp_step_iter(struct fbmp_iter *iter);

extern int
fbmp_init_range_iter(struct fbmp_iter  *iter,
                     const struct fbmp *bmp,
                     unsigned int       start_bit,
                     unsigned int       bit_count);

#define fbmp_foreach_range_bit(_iter, _bmp, _start_bit, _bit_count, _bit_no) \
	for ((_bit_no) = fbmp_init_range_iter(_iter, \
	                                      _bmp, \
	                                      _start_bit, \
	                                      _bit_count); \
	     (_bit_no) >= 0; \
	     (_bit_no) = fbmp_step_iter(_iter))

static inline int
fbmp_init_iter(struct fbmp_iter *iter, const struct fbmp *bmp)
{
	return fbmp_init_range_iter(iter, bmp, 0, bmp->nr);
}

#define fbmp_foreach_bit(_iter, _bmp, _bit_no) \
	for ((_bit_no) = fbmp_init_iter(_iter, _bmp); \
	     (_bit_no) >= 0; \
	     (_bit_no) = fbmp_step_iter(_iter))

#endif /* _UTILS_BITMAP_H */
