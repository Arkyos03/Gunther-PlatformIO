/*
 * Gunther.ino Arduino core - minimal newlib system calls
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>

extern char __end__;
extern char __HeapLimit;

static char *s_heap_end = 0;

/* malloc / new: heap between the end of .bss and the bottom of the stack area */
void *_sbrk(ptrdiff_t incr)
{
  if (s_heap_end == 0) s_heap_end = &__end__;
  char *prev = s_heap_end;
  if (s_heap_end + incr > &__HeapLimit) {
    errno = ENOMEM;
    return (void *)-1;
  }
  s_heap_end += incr;
  return prev;
}

/* Free RAM between heap and stack (handy to debug memory problems) */
int freeMemory(void)
{
  char top;
  char *heap = s_heap_end ? s_heap_end : &__end__;
  return (int)(&top - heap);
}

int _close(int fd)                    { (void)fd; return -1; }
int _fstat(int fd, struct stat *st)   { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)                   { (void)fd; return 1; }
int _lseek(int fd, int ptr, int dir)  { (void)fd; (void)ptr; (void)dir; return 0; }
int _read(int fd, char *ptr, int len) { (void)fd; (void)ptr; (void)len; return 0; }
int _write(int fd, const char *ptr, int len) { (void)fd; (void)ptr; return len; }
void _exit(int status)                { (void)status; while (1) { } }
int _kill(int pid, int sig)           { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void)                     { return 1; }

/* C++ ABI helpers (no exceptions, no RTTI) */
void __cxa_pure_virtual(void)         { while (1) { } }
