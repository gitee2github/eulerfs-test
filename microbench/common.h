#ifndef COMMON_H
#define COMMON_H

#include <sys/stat.h>
#include <fcntl.h>

typedef unsigned long u32;
typedef unsigned long long u64;
typedef long s32;
typedef long long s64;

#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_thread_error(msg, ...) \
	do { fprintf(stderr, "[%s:%d:func=%s:tid=%lld] %s "msg"\n", \
		     __FILE__, __LINE__, __func__, \
		     pthread_self(), strerror(errno), ##__VA_ARGS__); \
		pthread_exit((void *)EXIT_FAILURE); } while (0)


#define fsync_pair(dir, file)                     \
({                                                \
	int r = 0;                                \
	do {                                      \
		int fd = open(file, O_RDONLY);    \
		if ((r = fsync(fd)))              \
			break;                    \
		close(fd);                        \
		fd = open(dir, O_RDONLY);         \
		if ((r = fsync(fd)))              \
			break;                    \
		close(fd);                        \
	} while (0);                              \
	r;                                        \
})

#define fsync_single(dir)                         \
({                                                \
	int r = 0;                                \
	do {                                      \
		int fd = open(dir, O_RDONLY);     \
		if ((r = fsync(fd)))              \
			break;                    \
		close(fd);                        \
	} while (0);                              \
	r;                                        \
})

#endif
