#ifndef __CODA_SOCKET_H__
#define __CODA_SOCKET_H__

#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

static inline
int coda_set_nonblk(int s, int value)
{
	return ioctl(s, FIONBIO, &value);
}

static inline
int coda_set_sckopt(int s, int level, int key, int value)
{
	return setsockopt(s, level, key, (void *) &value, sizeof(value));
}

static inline
int coda_unix_send(int fd, void *data, size_t size)
{
	return send(fd, data, size, MSG_NOSIGNAL);
}

static inline
int coda_unix_recv(int fd, void *data, size_t size)
{
	int rc;

	for (;;)
	{
		rc = recv(fd, data, size, 0);
		if (0 <= rc) break;

		if (EINTR != errno)
		{
			return -1;
		}
	}

	return rc;
}

static inline
in_addr_t coda_inet_addr(const char *data, size_t size)
{
	const char *p;
	in_addr_t addr;
	uintptr_t octet, n;

	addr = 0;
	octet = 0;
	n = 0;

	for (p = data; p < data + size; ++p)
	{
		char c = *p;

		if (c >= '0' && c <= '9')
		{
			octet = octet * 10 + (c - '0');
			continue;
		}

		if (c == '.' && octet < 256)
		{
			addr = (addr << 8) + octet;
			octet = 0;
			n++;
			continue;
		}

		return INADDR_NONE;
	}

	if (n != 3)
	{
		return INADDR_NONE;
	}

	if (octet < 256)
	{
		addr = (addr << 8) + octet;
		return htonl(addr);
	}

	return INADDR_NONE;
}

static inline
int coda_listen(const char *host, const char *port, int backlog, int nonblock)
{
	int sd, rc;
	struct sockaddr_in addr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sd) return -1;

	if (nonblock && 0 > (rc = coda_set_nonblk(sd, nonblock)))
	{
		close(sd);
		return -1;
	}

	if (0 > (rc = coda_set_sckopt(sd, SOL_SOCKET, SO_REUSEADDR, 1)))
	{
		close(sd);
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_port = htons(strtoul(port, NULL, 10));

	if (0 > (rc = bind(sd, (struct sockaddr *) &addr, sizeof(addr))))
	{
		close(sd);
		return -1;
	}

	if (0 > (rc = listen(sd, backlog)))
	{
		close(sd);
		return -1;
	}

	return sd;
}

static inline
int coda_accept(int fd, struct in_addr *ip, int nonblock)
{
	int sd, rc;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	sd = accept(fd, (struct sockaddr *) &addr, &addrlen);
	if (0 > sd) return -1;

	if (0 > (rc = coda_set_nonblk(sd, nonblock)))
	{
		close(sd);
		return -1;
	}

	*ip = addr.sin_addr;

	return sd;
}

static inline
int coda_connect(const char *host, const char *port, int nonblock)
{
	int sd, rc;
	struct sockaddr_in addr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sd) return -1;

	if (nonblock && 0 > (rc = coda_set_nonblk(sd, nonblock)))
	{
		close(sd);
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_port = htons(strtoul(port, NULL, 10));

	if (0 > (rc = connect(sd, (struct sockaddr *) &addr, sizeof(addr))) && EINPROGRESS != errno)
	{
		close(sd);
		return -1;
	}

	return sd;
}

#endif /* __CODA_SOCKET_H__ */
