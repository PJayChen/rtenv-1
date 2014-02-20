#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
size_t strlen(const char *s);


#endif