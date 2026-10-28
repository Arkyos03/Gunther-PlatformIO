/*
 * Gunther.ino Arduino core - C++ new / delete (on top of malloc / free)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include <stdlib.h>
#include <stddef.h>

void *operator new(size_t size)                  { return malloc(size ? size : 1); }
void *operator new[](size_t size)                { return malloc(size ? size : 1); }
void  operator delete(void *ptr) noexcept        { free(ptr); }
void  operator delete[](void *ptr) noexcept      { free(ptr); }
void  operator delete(void *ptr, size_t) noexcept   { free(ptr); }
void  operator delete[](void *ptr, size_t) noexcept { free(ptr); }

/* placement new is provided inline by <new> */
