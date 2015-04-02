#include "file_io.h"

bool is_exist(const char *filename) {
    return NULL != fopen(filename, 'r');
}



