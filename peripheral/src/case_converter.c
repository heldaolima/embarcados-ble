#include "case_converter.h"

int islower(char c) {
    return (c >= 'a' && c <= 'z');
}

void convert_to_uppercase(const uint8_t *src, uint8_t *dest, size_t len) {
    for (size_t i = 0; i < len; i++) {
        dest[i] = islower(src[i]) ? toupper(src[i]) : src[i];
    }
    dest[len] = '\0';
}