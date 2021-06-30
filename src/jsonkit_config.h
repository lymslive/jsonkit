#ifndef JSONKIT_CONFIG_H__
#define JSONKIT_CONFIG_H__

#include <stdio.h>

#define LOCATION_PAR const char* file, int line, const char* func
#define LOCATION_ARG file, line, func
#define LOCATION_SRC __FILE__, __LINE__, __FUNCTION__

namespace jsonkit
{

// set log handle function, return the old handle
// can also pass FILE*, indicate log to that file
// not thread safe
typedef void (*fn_logreport_t)(LOCATION_PAR, const char* buffer, size_t size);
fn_logreport_t set_logreport(fn_logreport_t fn);
FILE* set_logreport(FILE* fp);

} /* jsonkit */ 

// to disable log in jsonkit lib
// #define NO_JSONKIT_LOG

#endif /* end of include guard: JSONKIT_CONFIG_H__ */
