/* Author:  DY
 * Date:    2015-04-15
 * Email:   ml_143@sina.com
 */

#pragma once

class strpair;

size_t ReadRequestHeader(int sockfd, char *buff, size_t buff_size);

//size_t ReadRequestBody(int sockfd, char *buff, size_t buff_size, size_t read_size);
size_t ReadRequestBody(int sockfd, char *buff, size_t read_size);

size_t SendSocketData(int sockfd, const char *buff, size_t write_size);

char *ReadFileData(const char *path, size_t *read_size);
char *ReadFileData(const strpair &sp_path, size_t *read_size);

bool IsFileExist(const char *path);
bool IsFileExist(const strpair &sp_path);

