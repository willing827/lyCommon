#ifndef SQ_BASIC_TYPES_H
#define SQ_BASIC_TYPES_H

#include <WinSock2.h>
#include <windows.h>

#include <string>
#include <memory>
#include <functional>
#include <vector>

using namespace std;
typedef unsigned char boolean;
typedef unsigned long long uint64;
typedef int int32;
typedef unsigned int uint32;

#define SAFE_DELETE(ptr) if (ptr != NULL) { delete ptr; ptr = NULL; }
#define SAFE_DELETE_ARRAY(ptr) if (ptr != NULL) { delete [] ptr; ptr = NULL; }
#endif //SQ_BASIC_TYPES_H