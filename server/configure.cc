#include "configure.h"
#include "http_io.h"

void AnalyzeConfigue(const char *filename) {
    size_t text_size = 0;
    char *configure_text = ReadFileData(filename, &text_size);
}
