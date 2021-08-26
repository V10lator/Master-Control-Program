#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static inline bool fileExists(const char *path)
{
	struct stat fs;
	return stat(path, &fs) == 0 && S_ISREG(fs.st_mode);
}

static inline off_t getFilesize(FILE *fp)
{
	off_t i = ftello(fp);
	if(fseek(fp, 0, SEEK_END) != 0)
		return -1;

	off_t fileSize = ftello(fp);

	fseeko(fp, i, SEEK_SET);
	return fileSize;
}
