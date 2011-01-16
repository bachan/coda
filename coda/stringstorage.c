#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "stringstorage.h"

#define STORAGE_BLOCK_SIZE 4096

void string_storage_free_struct(struct string_struct *s)
{
	unsigned i;
	if (!s) return;

	for (i = 0; i < s->child_count; i++)
	{
		string_storage_free_struct(&(s->children[i]));
	}

	if (s->children) free(s->children);
}

void string_storage_free(string_storage *ss)
{
	unsigned i;
	if (!ss) return;

	for (i = 0; i < ss->sb.block_count; i++)
	{
		free(ss->sb.blocks[i]);
	}

	if (ss->sb.blocks) free(ss->sb.blocks);
	string_storage_free_struct(&(ss->root));
}

void string_storage_init(string_storage *ss, size_t udbs)
{
	ss->sb.user_data_size = udbs;
	ss->sb.used_bytes = 0;
	ss->sb.current = 0;
	ss->sb.block_count = 0;
	ss->sb.blocks = 0;
	ss->root.value = 0;
	ss->root.child_count = 0;
	ss->root.children = 0;
}

void *allocate_reallocate_array(void *a, size_t cnt, size_t bsz)
{
	if (cnt == 0)
	{
		a = (void*)calloc(0x02, bsz);
	}
	else if (cnt == 0x02)
	{
		a = (void*)realloc(a, 0x10 * bsz);
	}
	else if ((cnt & 0x0F) == 0)
	{
		a = (void*)realloc(a, (cnt + 0x10) * bsz);
	}
	return a;
}

inline size_t get_common_prefix_size(const char *s, const char *t)
{
	size_t ret = 0;
	while (s[ret] && (s[ret] == t[ret])) ret++;
	return ret;
}

void *internal_find_add(storage_blocks *sb, struct string_struct *r, uint8_t prev_bytes, const char *s, int doadd)
{
	size_t begin = 0;
	size_t end = r->child_count;
	uint8_t common_bytes = 0;
	size_t len;
	char *newvalue;

	while (begin < end)
	{
		size_t mid = (begin + end) >> 1;
		int res = strcmp(s, r->children[mid].value + sb->user_data_size + 1);
		unsigned char sz_p1 = *(r->children[mid].value);
		if ((res == 0) && (sz_p1 == prev_bytes))
		{
			return (void*)(r->children[mid].value + 1);
		}
		else if (sz_p1 == prev_bytes)
		{
			if (res < 0) end = mid; else begin = mid + 1;
		}
		else
		{
			if (sz_p1 < prev_bytes) end = mid; else begin = mid + 1;
		}
	}

	/* 2. check how many bytes common with previous string */
	if (begin != 0) common_bytes = get_common_prefix_size(s, r->children[begin - 1].value + sb->user_data_size + 1);

	if (common_bytes > 8)
	{
		return internal_find_add(sb, &r->children[begin - 1], prev_bytes + common_bytes, s + common_bytes, doadd);
	}

	if (doadd == 0) return 0;

	/* 3. insert string after begin */
	r->children = allocate_reallocate_array(r->children, r->child_count, sizeof(struct string_struct));
	if (r->children == 0) return 0;
	if (begin != r->child_count)
	{
		memmove(&r->children[begin + 1], &r->children[begin],
		  (r->child_count - begin) * sizeof(struct string_struct));
	}

	len = strlen(s) + 1;
	if ((sb->current == 0) || (sb->used_bytes + 1 + len + sb->user_data_size > STORAGE_BLOCK_SIZE))
	{
		char *newcurrent = (char*)calloc(1, STORAGE_BLOCK_SIZE);
		sb->blocks = allocate_reallocate_array(sb->blocks, sb->block_count, sizeof(char*));
		if (sb->blocks == 0) return 0;

		sb->blocks[sb->block_count++] = newcurrent;
		sb->current = newcurrent;
		sb->used_bytes = 0;
	}

	newvalue = sb->current + sb->used_bytes;
	r->children[begin].value = newvalue;
	r->children[begin].child_count = 0;
	r->children[begin].children = 0;
	*(uint8_t*)(newvalue) = prev_bytes;
	memcpy(newvalue + sb->user_data_size + 1, s, len);
	sb->used_bytes += sb->user_data_size + 1 + len;
	r->child_count++;
	return (void*)(r->children[begin].value + 1);
}

void *string_storage_add(string_storage *ss, const char *s)
{
	size_t len = strlen(s) + 1;

	if (len + ss->sb.user_data_size > STORAGE_BLOCK_SIZE) return 0;

	return internal_find_add(&ss->sb, &ss->root, 0, s, 1);
}

void *string_storage_find(string_storage *ss, const char *s)
{
	return internal_find_add(&ss->sb, &ss->root, 0, s, 0);
}

void string_storage_enumerate_recursive(const struct string_struct *ss, string_storage_enumerate_callback cb, void *user_data, const char *parent, size_t user_data_size)
{
	char buf[256];
	size_t i;
	for (i = 0; i < ss->child_count; i++)
	{
		uint8_t prev = *(uint8_t*)ss->children[i].value;
		memcpy(buf, parent, prev);
		strncpy(buf + prev, ss->children[i].value + 1 + user_data_size, 255 - prev);
		buf[255] = 0;
		cb(user_data, buf, ss->children[i].value + 1);
		string_storage_enumerate_recursive(&ss->children[i], cb, user_data, buf, user_data_size);
	}
}

void string_storage_enumerate(const string_storage *ss, string_storage_enumerate_callback cb, void *user_data)
{
	string_storage_enumerate_recursive(&ss->root, cb, user_data, 0, ss->sb.user_data_size);
}

