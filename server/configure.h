/*
 * Author: DY.HUST
 * Date: 2015-05-09
 * Email: ml_143@sina.com
 */

#pragma once

#include <vector>
#include <cstdio>

class strpair;
class MemoryPool;

/******************************************************************************/
/* Interface. Load default configure, and read user configure from "configure"
 * file. If returning false, the program will use part of default and uesr
 * configure. In fact, it's a good choice to exit program when returning false
 */

bool InitConfigure();
void ReleaseConfigureResource();

/******************************************************************************/

bool LoadDefaultConfigure();
void ReadConfigure(const char *filename, MemoryPool &pool);
bool ParseConfigureCommand(const strpair &key, const std::vector<strpair> &values);

void LoadLogFile(FILE **ppf, const strpair &key);
