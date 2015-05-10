/*
 * Author: DY.HUST
 * Date: 2015-05-09
 * Email: ml_143@sina.com
 */

#pragma once

struct GlobalConfigure {
    int max_workers; 
    int max_fd;
    // FILE *log_file;
};

void AnalyzeConfigue(const char *filename);
