/* Author:  DY
 * Date:    2015-04-15
 * Email:   ml_143@sina.com
 */

#pragma once

#include <cstdio>

class strpair;
class MemoryPool;

/******************************************************************************/
/* Socket IO */

size_t ReadRequestHeader(int sockfd, char *buff, size_t buff_size);
size_t ReadRequestBody(int sockfd, char *buff, size_t read_size);
size_t SendSocketData(int sockfd, const char *buff, size_t write_size);

/******************************************************************************/
/* FILE IO */

char *ReadFileData(const char *path, size_t *read_size, MemoryPool &pool);
char *ReadFileData(const strpair &sp_path, size_t *read_size, MemoryPool &pool);

bool IsFileExist(const char *path);
bool IsFileExist(const strpair &sp_path);

/******************************************************************************/
