#include <utils/bitmap.h>
#include <errno.h>

#define fbmp_assert_range(_bmp, _start_bit, _bit_count) \
	fbmp_assert_map(_bmp); \
	fbmp_assert(_bit_count); \
	fbmp_assert(_start_bit < (_bmp)->nr); \
	fbmp_assert((_start_bit + _bit_count) <= (_bmp)->nr)

/*
 * Return a bit mask suitable for masking out all bits lower that the bit index
 * given in argument.
 */
static unsigned long
fbmp_word_high_mask(unsigned int bit_no)
{
	return ~(0UL) << bmp_word_bit_no(bit_no);
}

bool
fbmp_test_range(const struct fbmp *bmp,
                unsigned int       start_bit,
                unsigned int       bit_count)
{
	fbmp_assert_range(bmp, start_bit, bit_count);

	unsigned int  curr;
	unsigned int  last;
	unsigned long msb = fbmp_word_high_mask(start_bit);
	unsigned long lsb = ~(0UL) >> (__WORDSIZE -
	                               bmp_word_bit_no(start_bit + bit_count));

	curr = bmp_word_no(start_bit);
	last = bmp_word_no(start_bit + bit_count - 1);

	if (curr != last) {
		/*
		 * Mask out unwanted least significant bits and check if 0 or
		 * not.
		 */
		if (bmp->bits[curr++] & msb)
			return true;
	}
	else
		/*
		 * Range applies to a single word only: mask out unwanted most
		 * and least significant bits and check if 0 or not.
		 */
		return !!(bmp->bits[curr] & msb & lsb);

	/* Iterate over all remaining words but the last and test if 0. */
	while (curr < last)
		if (bmp->bits[curr++])
			return true;

	/*
	 * Mask out last word's unwanted most significant bits and check if 0 or
	 * not.
	 */
	return !!(bmp->bits[curr] & lsb);
}

bool
fbmp_test_all(const struct fbmp *bmp)
{
	fbmp_assert_map(bmp);

	unsigned int w;

	for (w = 0; w < bmp_word_nr(bmp->nr); w++)
		if (bmp->bits[w])
			return true;

	return false;
}

int
fbmp_init_clear(struct fbmp *bmp, unsigned int bit_nr)
{
	fbmp_assert(bmp);
	fbmp_assert(bit_nr);

	bmp->bits = calloc(bmp_word_nr(bit_nr), sizeof(*bmp->bits));
	if (!bmp->bits)
		return -ENOMEM;

	bmp->nr = bit_nr;

	return 0;
}

int
fbmp_init_set(struct fbmp *bmp, unsigned int bit_nr)
{
	fbmp_assert(bmp);
	fbmp_assert(bit_nr);

	unsigned int wnr = bmp_word_nr(bit_nr);

	bmp->bits = malloc(wnr * sizeof(*bmp->bits));
	if (!bmp->bits)
		return -ENOMEM;

	memset(bmp->bits, 0xff, wnr * sizeof(*bmp->bits));

	bmp->nr = bit_nr;

	return 0;
}

#define fbmp_assert_iter(_iter) \
	fbmp_assert(_iter); \
	fbmp_assert_map((_iter)->bmp); \
	fbmp_assert((_iter)->nr <= (_iter)->bmp->nr); \
	fbmp_assert((_iter)->curr < bmp_word_nr(iter->nr))

int
fbmp_step_iter(struct fbmp_iter *iter)
{
	fbmp_assert_iter(iter);

	unsigned int bit_no;

	while (!iter->word) {
		if (++iter->curr >= bmp_word_nr(iter->nr))
			return -ENOENT;

		iter->word = iter->bmp->bits[iter->curr];
	}

	bit_no = bops_ffs(iter->word) - 1;
	iter->word &= ~(1UL << bit_no);

	bit_no += (iter->curr * __WORDSIZE);
	if (bit_no >= iter->nr)
		return -ENOENT;

	return bit_no;
}

int
fbmp_init_range_iter(struct fbmp_iter  *iter,
                     const struct fbmp *bmp,
                     unsigned int       start_bit,
                     unsigned int       bit_count)
{
	fbmp_assert(iter);
	fbmp_assert_range(bmp, start_bit, bit_count);

	iter->curr = bmp_word_no(start_bit);
	iter->word = bmp->bits[iter->curr] & fbmp_word_high_mask(start_bit);
	iter->nr = start_bit + bit_count;
	iter->bmp = bmp;

	return fbmp_step_iter(iter);
}
