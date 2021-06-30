#include "jsonkit_config.h"
#include <stdarg.h>

namespace jsonkit
{

FILE* g_default_file = stderr;

FILE* set_logreport(FILE* fp)
{
    FILE* old = g_default_file;
    g_default_file = fp;
    return old;
}

void log_to_default_file(LOCATION_PAR, const char* buffer, size_t size)
{
    if (g_default_file != NULL)
    {
        fwrite(buffer, 1, size, g_default_file);
    }
}

fn_logreport_t g_default_log_handle = log_to_default_file;

fn_logreport_t set_logreport(fn_logreport_t fn)
{
    fn_logreport_t old = g_default_log_handle;
    g_default_log_handle = fn;
    return old;
}

void log_report(LOCATION_PAR, const char* buffer, size_t size)
{
    if (g_default_log_handle != NULL)
    {
        g_default_log_handle(LOCATION_ARG, buffer, size);
    }
}

#define MAX_PRINT_BUFFER 1024
void log_printf(LOCATION_PAR, const char* format, ...)
{
    char buffer[MAX_PRINT_BUFFER] = {0};

    int n = 0;

    va_list vlist;
    va_start(vlist, format);
    n = vsnprintf(buffer, sizeof(buffer), format, vlist);
    va_end(vlist);

    if (n > 0)
    {
        if (n >= MAX_PRINT_BUFFER)
        {
            n = MAX_PRINT_BUFFER - 1;
        }
        log_report(LOCATION_ARG, buffer, n);
    }
}
#undef MAX_PRINT_BUFFER

} /* jsonkit */ 
