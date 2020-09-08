#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#define upath_assert(...) assert(__VA_ARGS__)
#define ustr_assert(...) assert(__VA_ARGS__)

/////////////////////
static inline size_t
ustr_skip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && (*str == ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	while ((str >= string) && (*str == ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

static inline size_t
ustr_skip_notchar(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && *str && (*str != ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_notchar(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	if (!*str)
		return 0;

	while ((str >= string) && (*str != ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

/////////////////////

struct upath_comp {
	const char *start;
	size_t      len;
};

static int
upath_next_comp(struct upath_comp *comp, const char *path, size_t size)
{
	upath_assert(comp);
	upath_assert(path);
	upath_assert(size);

	size_t len;

	len = ustr_skip_char(path, '/', size);
	comp->start = path + len;

	if (size - len)
		comp->len = ustr_skip_notchar(comp->start, '/', size - len);
	else
		comp->len = 0;

	upath_assert((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

static int
upath_prev_comp(struct upath_comp *comp, const char *path, size_t size)
{
	upath_assert(comp);
	upath_assert(path);
	upath_assert(size);

	size_t len;

	len = ustr_rskip_char(path, '/', size);

	if (size - len)
		comp->len = ustr_rskip_notchar(path, '/', size - len);
	else
		comp->len = 0;

	comp->start = path + size - (len + comp->len);

	upath_assert((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

/////////////////////

struct upath_comp_iter {
	struct upath_comp  curr;
	const char        *stop;
};

const struct upath_comp *
upath_comp_iter_next(struct upath_comp_iter *iter)
{
	upath_assert(iter);
	upath_assert(iter->stop);
	upath_assert((iter->curr.start + iter->curr.len) <= iter->stop);

	int         err;
	const char *next = iter->curr.start + iter->curr.len;
	size_t      sz = iter->stop - next;

	if (!sz) {
		errno = ENOENT;
		return NULL;
	}

	err = upath_next_comp(&iter->curr, next, iter->stop - next);
	if (err) {
		errno = -err;
		return NULL;
	}

	return &iter->curr;
}

const struct upath_comp *
upath_comp_iter_first(struct upath_comp_iter *iter,
                      const char             *path,
                      size_t                  size)
{
	upath_assert(iter);
	upath_assert(path);

	iter->curr.start = path;
	iter->curr.len = 0;
	iter->stop = path + size;

	return upath_comp_iter_next(iter);
}

#define upath_foreach_comp_forward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_first(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_next(_iter))

const struct upath_comp *
upath_comp_iter_prev(struct upath_comp_iter *iter)
{
	upath_assert(iter);
	upath_assert(iter->stop);
	upath_assert(iter->curr.start >= iter->stop);

	int    err;
	size_t sz = iter->curr.start - iter->stop;

	if (!sz) {
		errno = ENOENT;
		return NULL;
	}

	err = upath_prev_comp(&iter->curr, iter->stop, sz);
	if (err) {
		errno = -err;
		return NULL;
	}

	return &iter->curr;
}

const struct upath_comp *
upath_comp_iter_last(struct upath_comp_iter *iter,
                     const char             *path,
                     size_t                  size)
{
	upath_assert(iter);
	upath_assert(path);

	iter->curr.start = path + size;
	iter->curr.len = 0;
	iter->stop = path;

	return upath_comp_iter_prev(iter);
}

#define upath_foreach_comp_backward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_last(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_prev(_iter))

int main(void)
{
	struct upath_comp_iter   iter;
	const struct upath_comp *comp;
	const char               path[] = "///leading//slash/";


	upath_foreach_comp_forward(&iter, comp, path, 0)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_forward(&iter, comp, path + 3, 0)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_forward(&iter, comp, path, sizeof(path))
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_forward(&iter, comp, path + 3, sizeof(path) - 3)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);

	upath_foreach_comp_forward(&iter, comp, path, sizeof(path) - 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_forward(&iter, comp, path + 3, sizeof(path) - 4)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);

	upath_foreach_comp_forward(&iter, comp, path, sizeof(path) - 4)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_forward(&iter, comp, path + 3, sizeof(path) - 7)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);

	upath_foreach_comp_backward(&iter, comp, path, 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + 3, 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + sizeof(path) - 1, 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + sizeof(path) - 2, 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + sizeof(path) - 3, 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + sizeof(path) - 3, 2)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path, sizeof(path) - 1)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + 3, sizeof(path) - 4)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);

	upath_foreach_comp_backward(&iter, comp, path, sizeof(path) - 2)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + 3, sizeof(path) - 5)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);

	upath_foreach_comp_backward(&iter, comp, path, sizeof(path) - 4)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
	upath_foreach_comp_backward(&iter, comp, path + 3, sizeof(path) - 7)
		printf("%*.*s(%zu)|", (int)comp->len, (int)comp->len, comp->start, comp->len);
	printf("errno=%d\n", errno);
}
