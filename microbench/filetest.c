#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include "common.h"

char basebasedir[1024] = {"/mnt/ramdisk/"};
char basedir[1024];
char dir[1024];
int dirtest;

void work_dirtest(u64 arg, int sync)
{
	static const int nr_round = 100;
	int nr_work_per_thread = (u64) arg;
	assert(nr_work_per_thread > nr_round);
	int nr_work_per_round = nr_work_per_thread / nr_round;
	int i, j;
	int r;
	int r2;
	struct timespec t0, t1;
	u64 tsum1 = 0;
	u64 tsum2 = 0;
	sprintf(basedir, "/mnt/ramdisk/D_%lld", pthread_self());
	for (j = 0; j < nr_round; ++j) {
		r = mkdir(basedir, 0777);
		if (r != 0)
			handle_thread_error("create wd %s", basedir);
		r2 = fsync_pair(basebasedir, basedir);
		if (r2 != 0)
			handle_thread_error("fsync wd %s & %s", basebasedir, basedir);

		for (i = 0; i < nr_work_per_round; ++i) {
			sprintf(dir, "%s/D_%d", basedir, i);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			r = mkdir(dir, 0777);
			if (sync)
				r2 = fsync_pair(basedir, dir);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			tsum1 += (t1.tv_sec - t0.tv_sec) * (u64)1e9 +
				(t1.tv_nsec - t0.tv_nsec);
			if (r != 0)
				handle_thread_error("create dir %s", dir);
			if (r2 != 0)
				handle_thread_error("fsync %s & %s", basedir, dir);
		}

		for (i = 0; i < nr_work_per_round; ++i) {
			sprintf(dir, "%s/D_%d", basedir, i);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			r = rmdir(dir);
			if (sync)
				r2 = fsync_single(basedir);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			tsum2 += (t1.tv_sec - t0.tv_sec) * (u64)1e9 +
				(t1.tv_nsec - t0.tv_nsec);
			if (r != 0)
				handle_thread_error("remove dir %s", dir);
			if (r2 != 0)
				handle_thread_error("fsync %s", basedir);
		}

		r = rmdir(basedir);
		if (r != 0)
			handle_thread_error("remove wd %s", basedir);
		r2 = fsync_single("/mnt/ramdisk/");
		if (r2 != 0)
			handle_thread_error("fsync wd %s", "/mnt/ramdisk/");
	}

	printf("mkdir latency tot %lld ns %lf ns/op\n",
	       tsum1, ((double)tsum1) / nr_work_per_thread);
	printf("mkdir throughput #op %lld %lf op/s\n",
	       nr_work_per_thread, ((double)nr_work_per_thread) * 1e9 / tsum1);

	printf("rmdir latency tot %lld ns %lf ns/op\n",
	       tsum2, ((double)tsum2) / nr_work_per_thread);
	printf("rmdir throughput #op %lld %lf op/s\n",
	       nr_work_per_thread, ((double)nr_work_per_thread) * 1e9 / tsum2);
}

void
work_filetest(u64 arg, int sync)
{
	static const int nr_round = 100;
	int nr_work_per_thread = (u64) arg;
	assert(nr_work_per_thread > nr_round);
	int nr_work_per_round = nr_work_per_thread / nr_round;
	int i, j;
	int r;
	int r2;
	struct timespec t0, t1;
	u64 tsum1 = 0;
	u64 tsum2 = 0;
	sprintf(basedir, "/mnt/ramdisk/D_%lld", pthread_self());
	for (j = 0; j < nr_round; ++j) {
		r = mkdir(basedir, 0777);
		if (r != 0)
			handle_thread_error("create wd %s", basedir);
		r2 = fsync_pair(basebasedir, basedir);
		if (r2 != 0)
			handle_thread_error("fsync wd %s & %s", basebasedir, basedir);

		for (i = 0; i < nr_work_per_round; ++i) {
			sprintf(dir, "%s/D_%d", basedir, i);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			r = creat(dir, 0777);
			if (sync)
				r2 = fsync_pair(basedir, dir);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			tsum1 += (t1.tv_sec - t0.tv_sec) * (u64)1e9 +
				(t1.tv_nsec - t0.tv_nsec);
			if (r == -1)
				handle_thread_error("create file %s", dir);
			close(r);
			if (r2 != 0)
				handle_thread_error("fsync %s & %s", basedir, dir);
		}

		for (i = 0; i < nr_work_per_round; ++i) {
			sprintf(dir, "%s/D_%d", basedir, i);
			clock_gettime(CLOCK_MONOTONIC, &t0);
			r = unlink(dir);
			if (sync)
				r2 = fsync_single(basedir);
			clock_gettime(CLOCK_MONOTONIC, &t1);
			tsum2 += (t1.tv_sec - t0.tv_sec) * (u64)1e9 +
				(t1.tv_nsec - t0.tv_nsec);
			if (r != 0)
				handle_thread_error("remove file %s", dir);
			if (r2 != 0)
				handle_thread_error("fsync %s", basedir);
		}

		r = rmdir(basedir);
		if (r != 0)
			handle_thread_error("remove wd %s", basedir);
		r2 = fsync_single(basebasedir);
		if (r2 != 0)
			handle_thread_error("fsync wd %s", basebasedir);
	}

	printf("create latency tot %lld ns %lf ns/op\n",
	       tsum1, ((double)tsum1) / nr_work_per_thread);
	printf("create throughput #op %lld %lf op/s\n",
	       nr_work_per_thread, ((double)nr_work_per_thread) * 1e9 / tsum1);

	printf("unlink latency tot %lld ns %lf ns/op\n",
	       tsum2, ((double)tsum2) / nr_work_per_thread);
	printf("unlink throughput #op %lld %lf op/s\n",
	       nr_work_per_thread, ((double)nr_work_per_thread) * 1e9 / tsum2);
}


int
main(int argc, char *argv[])
{
	int opt;
	u64 nwork = 0;
	int sync = 0;

	while ((opt = getopt(argc, argv, "s:n:d:t:")) != -1) {
		switch(opt) {
			break;
		case 's':
			sync = atoi(optarg);
			break;
		case 'n':
			nwork = strtoull(optarg, NULL, 0);
			break;
		case 'd':
			strcpy(basebasedir, optarg);	
			break;
		case 't':
			dirtest = atoi(optarg);
			break;
		default:
			printf("invalid opt: %c\n", opt);
			exit(EXIT_FAILURE);
		}
	}

	if (0 == nwork)
		return 0;

	printf("testing nwork=%llu...\n", nwork);
	fflush(stdout);
	if (dirtest)
		work_dirtest(nwork, sync);
	else
		work_filetest(nwork, sync);
	puts("Done!");

	return 0;
}
