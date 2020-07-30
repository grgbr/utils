#include <utils/pile.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/******************************************************************************
 * Pile generic handling.
 ******************************************************************************/

static void
upile_register(struct upile *pile, struct dlist_node *node, size_t size)
{
	upile_assert_pile(pile);
	upile_assert(!dlist_empty(node));
	upile_assert(size);

	dlist_append(&pile->head, node);
	pile->nr++;
	pile->size += size;
}

static void
upile_unregister(struct upile *pile, struct dlist_node *node, size_t size)
{
	upile_assert_pile(pile);
	upile_assert(!dlist_empty(node));
	upile_assert(size);

	dlist_remove(node);
	pile->nr--;
	pile->size -= size;
}

void
upile_clear(struct upile *pile)
{
	upile_assert_pile(pile);

	while (!dlist_empty(&pile->head)) {
		struct dlist_node *node;

		node = dlist_next(&pile->head);
		dlist_remove(node);

		free(node);
	}

	pile->nr = 0;
	pile->size = 0;
}

void
upile_init(struct upile *pile)
{
	upile_assert(pile);

	dlist_init(&pile->head);
	pile->nr = 0;
	pile->size = 0;
}

/******************************************************************************
 * Pile of strings handling.
 ******************************************************************************/

#define upile_assert_str(_ustr) \
	upile_assert(_ustr); \
	upile_assert(!(_ustr)->size)

struct upile_str {
	struct dlist_node node;
	size_t            size;
	char              data[0];
};

static struct upile_str *
upile_str_from_data(const char *data)
{
	return containerof(data, struct upile_str, data);
}

static struct upile_str *
upile_str_from_node(const struct dlist_node *node)
{
	upile_assert_str((struct upile_str *)node);

	return (struct upile_str *)node;
}

size_t
upile_str_len(const char *str)
{
	return upile_str_from_data(str)->size - 1;
}

size_t
upile_str_size(const char *str)
{
	return upile_str_from_data(str)->size;
}

char *
upile_iter_next_str(const struct upile *pile, const char *str)
{
	const struct dlist_node *next;

	next = dlist_next(&upile_str_from_data(str)->node);
	if (next == &pile->head)
		return NULL;

	return &upile_str_from_node(next)->data[0];
}

char *
upile_begin_iter_str(const struct upile *pile)
{
	if (!pile->nr)
		return NULL;

	return &upile_str_from_node(dlist_next(&pile->head))->data[0];
}

static struct upile_str *
upile_alloc_str(struct upile *pile, size_t size)
{
	upile_assert_pile(pile);
	upile_assert(size);

	struct upile_str *ustr;

	ustr = malloc(sizeof(*ustr) + size);
	if (!ustr)
		return NULL;

	ustr->size = size;

	upile_register(pile, &ustr->node, size);

	return ustr;
}

char *
upile_clone_str(struct upile *pile, const char *str, size_t len)
{
	upile_assert(str);

	struct upile_str *ustr;

	ustr = upile_alloc_str(pile, len + 1);
	if (!ustr)
		return NULL;

	memcpy(&ustr->data[0], str, len);
	ustr->data[len] = '\0';

	return &ustr->data[0];
}

char *
upile_create_str(struct upile *pile, const char *str, size_t max_size)
{
	upile_assert(str);

	size_t len;

	len = strnlen(str, max_size);
	if (len >= max_size) {
		errno = E2BIG;
		return NULL;
	}

	return upile_clone_str(pile, str, len);
}

void
upile_destroy_str(struct upile *pile, char *str)
{
	upile_assert(str);

	struct upile_str *ustr;

	ustr = upile_str_from_data(str);
	upile_assert_str(ustr);

	upile_unregister(pile, &ustr->node, ustr->size);

	free(ustr);
}
