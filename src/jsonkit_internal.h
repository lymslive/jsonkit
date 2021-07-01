#ifndef JSONKIT_INTERNAL_H__
#define JSONKIT_INTERNAL_H__

#include "jsonkit_config.h"

namespace jsonkit
{

void log_report(LOCATION_PAR, const char* buffer, size_t size);

void log_printf(LOCATION_PAR, const char* format, ...);

} /* jsonkit */ 

#ifndef NO_JSONKIT_LOG

// log as printf style
#define LOGF(format, ...) jsonkit::log_printf(LOCATION_SRC, format, ## __VA_ARGS__)
// log a single std::string
#define LOGS(str) jsonkit::log_report(LOCATION_SRC, str.c_str(), str.size())

#ifdef _DEBUG
#define LOGD(format, ...) jsonkit::log_printf(LOCATION_SRC, format, ## __VA_ARGS__)
#else
#define LOGD(format, ...)
#endif

#endif

#endif /* end of include guard: JSONKIT_INTERNAL_H__ */
