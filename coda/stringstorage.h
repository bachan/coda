#ifndef __STRINGSTORAGE_H__
#define __STRINGSTORAGE_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct storage_blocks_struct
{
	size_t user_data_size;
	size_t used_bytes;
	char *current;
	size_t block_count;
	char **blocks;
};

typedef struct storage_blocks_struct storage_blocks;

struct string_struct
{
	char *value;
	size_t child_count;
	struct string_struct *children;
};

struct string_storage_struct
{
	storage_blocks sb;
	struct string_struct root;
};

typedef struct string_storage_struct string_storage;

void string_storage_init(string_storage *ss, size_t udbs);
void string_storage_free(string_storage *ss);
void *string_storage_add(string_storage *ss, const char *s);

typedef void (*string_storage_enumerate_callback)(void *user_data, const char *key, void *value);
void string_storage_enumerate(const string_storage *ss, string_storage_enumerate_callback cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __STRINGSTORAGE_H__ */
