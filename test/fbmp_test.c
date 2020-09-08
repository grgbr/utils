#include <utils/bitmap.h>
#include <stdio.h>

int main(void)
{
	struct fbmp      bmp;
	struct fbmp_iter iter;
	int              b;
	int              ret;

	ret = fbmp_init(&bmp, 5 * __WORDSIZE / 2);
	if (ret) {
		printf("fbmp_init: %d\n", ret);
		return EXIT_FAILURE;
	}

	if (fbmp_test_all(&bmp)) {
		printf("fbmp_test_all: empty: NOK\n");
		return EXIT_FAILURE;
	}

	fbmp_set(&bmp, 0);
	if (!fbmp_test_all(&bmp)) {
		printf("fbmp_test_all: bit0: NOK\n");
		return EXIT_FAILURE;
	}
	if (!fbmp_test(&bmp, 0)) {
		printf("fbmp_test: bit0: NOK\n");
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, 0, 1)) {
		printf("fbmp_test_range: bit[0-0]: NOK\n");
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, 1, 1)) {
		printf("fbmp_test_range: bit[1-1]: NOK\n");
		return EXIT_FAILURE;
	}

	fbmp_clear_all(&bmp);
	if (fbmp_test_all(&bmp)) {
		printf("fbmp_clear_all: NOK\n");
		return EXIT_FAILURE;
	}
	fbmp_set(&bmp, __WORDSIZE - 1);
	if (!fbmp_test(&bmp, __WORDSIZE - 1)) {
		printf("fbmp_test: bit%u: NOK\n", __WORDSIZE - 1);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, 0, __WORDSIZE - 1)) {
		printf("fbmp_test_range: bit[0-%u]: NOK\n", __WORDSIZE - 2);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE - 1, 1)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n", __WORDSIZE - 1, __WORDSIZE - 1);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, 0, __WORDSIZE / 2)) {
		printf("fbmp_test_range: bit[0-%u]: NOK\n", (__WORDSIZE / 2) - 1);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE / 2, __WORDSIZE / 2)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n", __WORDSIZE / 2, __WORDSIZE - 1);
		return EXIT_FAILURE;
	}

	fbmp_clear_all(&bmp);
	if (fbmp_test_all(&bmp)) {
		printf("fbmp_clear_all: NOK\n");
		return EXIT_FAILURE;
	}
	fbmp_set(&bmp, __WORDSIZE / 2);
	if (!fbmp_test_all(&bmp)) {
		printf("fbmp_clear_all: NOK\n");
		return EXIT_FAILURE;
	}
	if (!fbmp_test(&bmp, __WORDSIZE / 2)) {
		printf("fbmp_test: bit%u: NOK\n", __WORDSIZE / 2);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, 0, __WORDSIZE / 2)) {
		printf("fbmp_test_range: bit[0-%u]: NOK\n", (__WORDSIZE / 2) - 1);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, 0, (__WORDSIZE / 2) + 1)) {
		printf("fbmp_test_range: bit[0-%u]: NOK\n", __WORDSIZE / 2);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE / 2, 1)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n", __WORDSIZE / 2, (__WORDSIZE / 2) + 1);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE / 4, __WORDSIZE / 2)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n", __WORDSIZE / 4, (3 * __WORDSIZE / 4) - 1);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, 3 * __WORDSIZE / 4, __WORDSIZE / 4)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n", 3 * __WORDSIZE / 4, __WORDSIZE - 1);
		return EXIT_FAILURE;
	}

	fbmp_set(&bmp, 3 * __WORDSIZE / 2);
	if (!fbmp_test_all(&bmp)) {
		printf("fbmp_test_all: NOK\n");
		return EXIT_FAILURE;
	}
	if (!fbmp_test(&bmp, 3 * __WORDSIZE / 2)) {
		printf("fbmp_test: bit%u: NOK\n", 3 * __WORDSIZE / 2);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE / 4, 2 * __WORDSIZE)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       __WORDSIZE / 4,
		       (9 * __WORDSIZE / 4) - 1);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, (__WORDSIZE / 2) - 1, 2 * __WORDSIZE)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       (__WORDSIZE / 2) - 1,
		       (5 * __WORDSIZE / 2) - 2);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, (__WORDSIZE / 2) - 1, __WORDSIZE)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       (__WORDSIZE / 2) - 1,
		       (3 * __WORDSIZE / 4) - 2);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, (__WORDSIZE / 2) + 1, __WORDSIZE / 2)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       (__WORDSIZE / 2) + 1,
		       __WORDSIZE);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, (__WORDSIZE / 2) + 1, __WORDSIZE - 1)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       (__WORDSIZE / 2) + 1,
		       (3 * __WORDSIZE / 2) - 2);
		return EXIT_FAILURE;
	}

	fbmp_clear(&bmp, 3 * __WORDSIZE / 2);
	if (!fbmp_test_range(&bmp, __WORDSIZE / 4, 2 * __WORDSIZE)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       __WORDSIZE / 4,
		       (9 * __WORDSIZE / 4) - 1);
		return EXIT_FAILURE;
	}
	if (fbmp_test_range(&bmp, (__WORDSIZE / 2) + 1, (2 * __WORDSIZE) - 1)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       (__WORDSIZE / 2) + 1,
		       (5 * __WORDSIZE / 2) - 1);
		return EXIT_FAILURE;
	}

	fbmp_clear(&bmp, __WORDSIZE / 2);
	fbmp_set(&bmp, 9 * __WORDSIZE / 4);
	if (fbmp_test_range(&bmp, __WORDSIZE / 4, 2 * __WORDSIZE)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       __WORDSIZE / 4,
		       (9 * __WORDSIZE / 4) - 1);
		return EXIT_FAILURE;
	}
	if (!fbmp_test_range(&bmp, __WORDSIZE / 4, (2 * __WORDSIZE) + 1)) {
		printf("fbmp_test_range: bit[%u-%u]: NOK\n",
		       __WORDSIZE / 4,
		       (9 * __WORDSIZE / 4));
		return EXIT_FAILURE;
	}

	fbmp_clear_all(&bmp);
	fbmp_set(&bmp, 0);
	fbmp_set(&bmp, 1);
	fbmp_set(&bmp, __WORDSIZE / 2);
	fbmp_set(&bmp, __WORDSIZE - 2);
	fbmp_set(&bmp, __WORDSIZE - 1);
	fbmp_set(&bmp, __WORDSIZE);
	fbmp_set(&bmp, __WORDSIZE + 1);
	fbmp_set(&bmp, (5 * __WORDSIZE / 2) - 1);

	fbmp_foreach_bit(&iter, &bmp, b)
		printf("%u ", b);
	putchar('\n');

	fbmp_foreach_range_bit(&iter, &bmp, 1, __WORDSIZE / 2, b)
		printf("%u ", b);
	putchar('\n');

	fbmp_foreach_range_bit(&iter, &bmp, __WORDSIZE / 2, __WORDSIZE / 2, b)
		printf("%u ", b);
	putchar('\n');

	fbmp_foreach_range_bit(&iter, &bmp, __WORDSIZE / 2, (__WORDSIZE / 2) - 1, b)
		printf("%u ", b);
	putchar('\n');

	fbmp_foreach_range_bit(&iter, &bmp, __WORDSIZE - 2, 7, b)
		printf("%u ", b);
	putchar('\n');


	fbmp_fini(&bmp);

	return 0;
}
