#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

char dirname[1024] = {"/mnt/ramdisk/testdir"};
char filename[1024];
unsigned long size = 4096;
int bs = 4096;
int count = 10000;
bool async = false;

char *buf;

enum type {
	CREATE = 0,
	APPEND_WRITE = 1,
	COVER_WRITE = 2,
	READ = 3,
	DELETE = 4,
	STAT_TYPES,
};
unsigned long *stats[STAT_TYPES];

#define get_time(t0, t1) ((t1.tv_sec - t0.tv_sec) * (unsigned long)1000000000 +\
			  (t1.tv_nsec - t0.tv_nsec))

static void random_buf(char *buf)
{
	int *ptr = (int *)buf;

	for (int i = 0; i < bs / 4; ++i)
		buf[i] = rand();
}

static void free_mem()
{
	free(buf);
	for (int i = 0; i < STAT_TYPES; ++i)
		free(stats[i]);
}

static int alloc_mem()
{
	buf = malloc(bs);
	if (!buf)
		goto free;

	for (int i = 0; i < STAT_TYPES; ++i) {
		stats[i] = malloc(count * sizeof(unsigned long));
		if (!stats[i])
			goto free;

		memset(stats[i], 0, count * sizeof(unsigned long));
	}
	return 0;

free:
	free_mem();
	return -ENOMEM;
}

static int cmpfn(const void *p1, const void *p2)
{
	return *(unsigned long *)p1 > *(unsigned long *)p2 ? 1: -1;
}

static void report()
{
	/* report result */
	printf("---------- latency in ns:\n");
	printf("create         append-write   cover-write    read           delete\n");

	/* caculate latency from %10 to %90 */
	for (int i = 0; i < STAT_TYPES; ++i) {
		unsigned long sum = 0;
		int start = count / 10;
		int end = start * 9;

		qsort(stats[i], count, sizeof(unsigned long), cmpfn);
		for (int j = start; j < end; ++j)
			sum += stats[i][j];

		printf("%-15lu", sum / (end - start));
	}

	printf("\n");
}

/* only test single thread currently */
static void test()
{
	struct timespec t0, t1;
	int ret;

	if (alloc_mem()) {
		perror("malloc failed");
		return;
	}

	ret = mkdir(dirname, 0777);
	if (ret != 0) {
		perror("mkdir failed\n");
		free_mem();
		return;
	}

	for (int i = 0; i < count; ++i) {
		int fd;
		int err = 0;

		sprintf(filename, "%s/f_%d", dirname, i);

		/* create */
		clock_gettime(CLOCK_MONOTONIC, &t0);
		fd = creat(filename, 0777);
		if (!async)
			err = fsync(fd);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		stats[CREATE][i] = get_time(t0, t1);

		if (fd == -1 || err != 0) {
			perror("create failed");
			free_mem();
			return;
		}

		/* append write */
		for (unsigned long offset = 0; offset <= size; offset += bs) {
			random_buf(buf);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			ret = write(fd, buf, bs);
			if (!async)
				err = fsync(fd);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			stats[APPEND_WRITE][i] += get_time(t0, t1);
			if (ret != bs || err != 0) {
				perror("append write failed");
				free_mem();
				return;
			}
		}

		/* cover write */
		ret = lseek(fd, 0, SEEK_SET);
		if (ret != 0) {
			perror("lseek after append write failed");
			free_mem();
			return;
		}

		for (unsigned long offset = 0; offset < size; offset += bs) {
			random_buf(buf);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			ret = write(fd, buf, bs);
			if (!async)
				err = fsync(fd);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			stats[COVER_WRITE][i] += get_time(t0, t1);
			if (ret != bs) {
				perror("cover write failed");
				free_mem();
				return;
			}
		}

		/* read */
		/*
		 * try to delete inode cache, so that read will access file
		 * mapping from device.
		 */
		ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
		if (ret != 0) {
			perror("fadvise failed");
			free_mem();
			return;
		}
		close(fd);

		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			perror("open file failed");
			free_mem();
			return;
		}

		for (unsigned long offset = 0; offset < size; offset += bs) {
			random_buf(buf);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			ret = read(fd, buf, bs);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			stats[READ][i] += get_time(t0, t1);
			if (ret != bs) {
				perror("read failed");
				free_mem();
				return;
			}
		}
		close(fd);

		/* delete */
		if (!async) {
			fd = open(dirname, O_DIRECTORY);
			if (fd == -1) {
				perror("open dir failed");
				return;
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &t0);
		ret = unlink(filename);
		if (!async)
			err = fsync(fd);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		stats[DELETE][i] += get_time(t0, t1);

		if (!async)
			close(fd);

		if (ret == -1 || err == -1) {
			perror("unlink failed");
			return;
		}
	}

	rmdir(dirname);
	report();
	free_mem();
}

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "ad:s:b:c:")) != -1) {
		switch(opt) {
		case 'a':
			async = true;
			break;
		case 'd':
			strcpy(dirname, optarg);
			break;
		case 's':
			size = atol(optarg);
			break;
		case 'b':
			bs = atoi(optarg);
			break;
		case 'c':
			count = atoi(optarg);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (size % bs != 0) {
		errno = EINVAL;
		perror(argv[0]);
		return 1;
	}

	printf("---------- testing dir %s size %lu bs %d count %d sync? %d\n",
		dirname, size, bs, count, !async);
	test();
	printf("---------- test done!\n\n");
	return 0;
}
