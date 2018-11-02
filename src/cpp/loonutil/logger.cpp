#include "logger.h"

namespace loon
{
/*static*/ const char* const Logger::color_scheme[6] = {
        (char* const)"\e[0m",   // clear
        (char* const)"\e[34m",  // debug
        (char* const)"\e[32m",  // info
        (char* const)"\e[36m",  // warning
        (char* const)"\e[31m",  // error
        (char* const)"\e[1;35m"    // critical
    };

Logger::Logger(int level/* = 0*/, FILE* fp/* = stderr*/):
    log_level( level ), log_fp(fp), use_color(false)
{
}

void Logger::print_time()
{
    std::time_t raw_time;
    std::time(&raw_time);
    struct tm* time_info = std::localtime(&raw_time);
    fprintf(log_fp, "[%02d/%02d/%04d %02d:%02d:%02d]",
            time_info->tm_mon + 1, time_info->tm_mday, time_info->tm_year + 1900,
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
}

void Logger::print_msg(const char* fmt, va_list arg, const char* level_name)
{
    print_time();
    fprintf(log_fp, " [%s]: ", level_name);
    vfprintf(log_fp, fmt, arg);
    if(use_color)   fprintf(log_fp, "%s", color_scheme[0]);
    fprintf(log_fp, "\n");
}

void Logger::set_level(int level)
{
    log_level = level;
}

void Logger::color_on()
{
    use_color = true;
}

void Logger::color_off()
{
    use_color = false;
}

void Logger::critical(const char* fmt, ...)
{
#if LOGGER_LEVEL <= 5
    if(log_level <= 5)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[5]);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "CRITICAL");
        va_end(arg);
    }
#endif
}

void Logger::error(const char* fmt, ...)
{
#if LOGGER_LEVEL <= 4
    if(log_level <= 4)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[4]);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "ERROR");
        va_end(arg);
    }
#endif
}

void Logger::warning(const char* fmt, ...)
{
#if LOGGER_LEVEL <= 3
    if(log_level <= 3)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[3]);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "WARNING");
        va_end(arg);
    }
#endif
}

void Logger::info(const char* fmt, ...)
{
#if LOGGER_LEVEL <= 2
    if(log_level <= 2)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[2]);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "INFO");
        va_end(arg);
    }
#endif
}

void Logger::debug(const char* fmt, ...)
{
#if LOGGER_LEVEL <= 1
    if(log_level <= 1)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[1]);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "DEBUG");
        va_end(arg);
    }
#endif
}

void Logger::critical(unsigned int lineno, const char* fname, const char* fmt, ...)
{
#if LOGGER_LEVEL <= 5
    if(log_level <= 5)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[5]);
        fprintf(log_fp, "[%s %d] ", fname, lineno);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "CRITICAL");
        va_end(arg);
    }
#endif
}

void Logger::error(unsigned int lineno, const char* fname, const char* fmt, ...) // level = 4
{
#if LOGGER_LEVEL <= 4
    if(log_level <= 4)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[4]);
        fprintf(log_fp, "[%s %d] ", fname, lineno);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "ERROR");
        va_end(arg);
    }
#endif
}

void Logger::warning(unsigned int lineno, const char* fname, const char* fmt, ...) // level = 3
{
#if LOGGER_LEVEL <= 3
    if(log_level <= 3)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[3]);
        fprintf(log_fp, "[%s %d] ", fname, lineno);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "WARNING");
        va_end(arg);
    }
#endif
}

void Logger::info(unsigned int lineno, const char* fname, const char* fmt, ...) // level = 2
{
#if LOGGER_LEVEL <= 2
    if(log_level <= 2)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[2]);
        fprintf(log_fp, "[%s %d] ", fname, lineno);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "INFO");
        va_end(arg);
    }
#endif
}

void Logger::debug(unsigned int lineno, const char* fname, const char* fmt, ...)// level = 1
{
#if LOGGER_LEVEL <= 1
    if(log_level <= 1)
    {
        if(use_color)   fprintf(log_fp, "%s", color_scheme[1]);
        fprintf(log_fp, "[%s %d] ", fname, lineno);
        va_list arg;
        va_start(arg, fmt);
        print_msg(fmt, arg, "DEBUG");
        va_end(arg);
    }
#endif
}


}// namespace loon
