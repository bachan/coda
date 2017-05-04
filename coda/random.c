#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "random.h"

static volatile int coda_random_inited = 0;

int coda_random_init()
{
	if (coda_random_inited)
	{
		return 0;
	}

	int fd = open("/dev/urandom", O_RDONLY);
	if (0 > fd) return -1;

	uint32_t rnd_from_device;

	int rc = read(fd, &rnd_from_device, sizeof(rnd_from_device));
	if (0 > rc) return -1;

	close(fd);

	srandom(rnd_from_device);
	coda_random_inited = 1;

	return 0;
}

