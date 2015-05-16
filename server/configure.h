/*
 * Author: DY.HUST
 * Date: 2015-05-09
 * Email: ml_143@sina.com
 */

#pragma once

#include <vector>
#include <cstdio>
#include "utility.h"

struct GlobalConfigure {
    size_t      workers,
                max_request_header,
                max_response_header,
                log_level;

    strpair     log_filename[8];
    FILE        *log_fp[8];
};

extern GlobalConfigure g_configure;

bool InitConfigure();

void LoadDefaultConfigure();
void ReadConfigure(const char *filename);
void ParseConfigureCommand(const strpair &key, const std::vector<strpair> &values);

