/*
 * Author:  DY.HUST
 * Date:    2015-04-15
 * Email:   ml_143@sina.com
 */

#pragma once

#include <cstdio>

class Request;
class strpair;

// Interface

// Parameter:   Request&, the request to be filled in with raw information
// Return:      int, the status code
int ParseHeader(const char *header_text, Request& req);

// Parameter:   Request&, the request which extract from and fill in with the final information
// Return:      int, the status code
int ExtractInformation(Request &req);

/*----------------------------------------------------------------------------*/
// Inner function

inline const char *skip_space(const char *p);

inline const char *ParseToken(const char *p, char tag, strpair &sp);
inline const char *ParseToken(const char *p, const char *tag_s, strpair *sp);
inline const char *ParseUntilCRLF(const char *p, strpair &sp);

inline int ExtractPath(const strpair &sp, strpair &path);
inline int ExtractMethod(const strpair &method);
inline int ExtractVersion(const strpair &version, size_t *pmajor, size_t *pminor);
size_t ExtractContentLength(Request &req);

