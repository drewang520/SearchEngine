#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RETURN_ERROR_CHECK(ret, num, msg)\
    {                                    \
        if (ret == num)                  \
            {                            \
                perror(msg);           \
                return -1;               \
            }                            \
    }                                    \
