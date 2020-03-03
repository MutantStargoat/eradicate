#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "logger.h"

int init_logger(const char *fname)
{
	int fd;
	if((fd = open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
		fprintf(stderr, "init_logger: failed to open %s: %s\n", fname, strerror(errno));
		return -1;
	}

	close(1);
	close(2);
	dup(fd);
	dup(fd);
	return 0;
}
