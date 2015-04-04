#include "file_io.h"

#include "utility.h"
#include "response.h"

bool HTTPFile::read(Response &response) {
    if (!pfile_) return false;

    size_t file_size = size();
    response.body = new char[file_size];
    fseek(pfile_, 0, SEEK_SET);
    response.data_size = file_size * fread(response.body, file_size, 1, pfile_);

    return true;
}


