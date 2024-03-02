#ifndef _case_converter_h_
#define _case_converter_h_

#include <zephyr/types.h>
#include <stddef.h>
#include <ctype.h>

void convert_to_uppercase(const uint8_t *src, uint8_t *dest, size_t len);

#endif