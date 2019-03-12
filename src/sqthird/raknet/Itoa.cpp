#include "EmptyHeader.h"
#include <stdlib.h> 

#ifdef __cplusplus
extern "C" {
#endif

	// Fast itoa from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for Linux since it seems like Linux doesn't support this function.
	// I modified it to remove the std dependencies.
	char* Itoa( int value, char* result, int base )
	{
		return _itoa(value,result,base);
	}

#ifdef __cplusplus
}
#endif
