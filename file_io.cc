#include "file_io.h"

#include "utility.h"
#include "parse.h"
#include "response.h"

bool HTTPFile::read(Request &request) {
    if (!pfile_) return false;

    size_t file_size = size();
    request.body = new char[file_size];
    fseek(pfile_, 0, SEEK_SET);
    request.body_size = file_size * fread(request.body, file_size, 1, pfile_);

    return true;
}


