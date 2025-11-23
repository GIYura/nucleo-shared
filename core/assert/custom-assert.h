#ifndef CUSTOM_ASSERT_H
#define CUSTOM_ASSERT_H

#include <stdint.h>

void ErrorHandler(const char* file, int line, const char* expr);

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            ErrorHandler(__FILE__, __LINE__, #expr); \
        } \
    } while (0)

#endif /* CUSTOM_ASSERT_H */

