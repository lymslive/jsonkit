/** 
 * @file jsonkit_config.h
 * @author lymslive
 * @date 2021-10-27 
 * @brief some configuration for this jsonkit lib
 * */
#ifndef JSONKIT_CONFIG_H__
#define JSONKIT_CONFIG_H__

#include <stdio.h>

/// macros for log location parameter
#define LOCATION_PAR const char* file, int line, const char* func
#define LOCATION_ARG file, line, func
#define LOCATION_SRC __FILE__, __LINE__, __FUNCTION__

namespace jsonkit
{

/** log function callback
 * @param LOCATION_ARG The three common macor information when log: file, line, func
 * @param buffer The message buffer to log, pointer to the first byte
 * @param size The size of the message buffer
 * */
typedef void (*fn_logreport_t)(LOCATION_PAR, const char* buffer, size_t size);

/** set log handle function, return the old handle 
 * @param fn set log handle to a function
 * @param fp set log handle to a file pointer
 * @return the old log function or file.
 * */
fn_logreport_t set_logreport(fn_logreport_t fn);
FILE* set_logreport(FILE* fp);

} /* jsonkit */ 

// to disable log in jsonkit lib
// #define NO_JSONKIT_LOG

#endif /* end of include guard: JSONKIT_CONFIG_H__ */
