#include <stddef.h>
#include <sys/types.h>


__attribute__((weak))
int _write(int file, const void * ptr, size_t len)
{
	return (int)len;
}

__attribute__((weak))
void _exit(int status)
{
	while (1)
	{
		;
	}
}

__attribute__((weak))
int _sbrk(int incr)
{
	return 0;
}

__attribute__((weak))
int _close(int file)
{
	return -1;
}

__attribute__((weak))
int _read(int file, char * ptr, int len)
{
	return 0;
}

__attribute__((weak))
int _lseek(int file, int ptr, int dir)
{
	return 0;
}
