#ifndef __RDEV_STAT_H__
#define __RDEV_STAT_H__

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Creates nested directories up to last '/'. For example,
   'a/b/c' will create 'a' and 'a/b', whereas
   'a/b/c/' will create 'a' and 'a/b' and 'a/b/c'.
*/
int rdev_pmkdir_nz(char *pathname, size_t pathsize, mode_t mode);
int rdev_pmkdir(char *pathname, mode_t mode);

/* The same as functions above but doesn't required modifiable pathname.
   Copy passed pathname with alloca internally and call rdev_pmkdir_nz.
*/
int rdev_pmkdir_nz_const(const char *pathname, size_t pathsize, mode_t mode);
int rdev_pmkdir_const(const char *pathname, mode_t mode);

/* rdev_mapped_* */

struct rdev_mapped
{
    int o_flag;
    int o_mode;
    int m_prot;
    int m_flag;
    off_t ofst;
};

typedef struct rdev_mapped rdev_mapped_t;
typedef int (* rdev_mapped_fp) (void *my_data, void *mem, size_t sz_mem);

int rdev_mapped_readfn(const char *fn, void *my_data, rdev_mapped_fp my_cb);
int rdev_mapped_custom(const char *fn, void *my_data, rdev_mapped_fp my_cb, const rdev_mapped_t *mcfg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __RDEV_STAT_H__ */
