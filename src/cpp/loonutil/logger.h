#ifndef __UTIL_LOGGER_H
#define __UTIL_LOGGER_H

/*! \file */
#include<iostream>
#include<string>
#include<ctime>
#include<cstdarg>

namespace loon
{
#ifndef LOGGER_LEVEL
/*!\ingroup g_Constants
 * \brief Compile options of Logger Level
 */
    #define LOGGER_LEVEL 0
#endif

/*! \ingroup Aux
 * \brief Logger class with some good functionalities.
 *
 * When you compile the code, you can define the `LOGGER_LEVEL` by setting a compile
 * option or `#define LOGGER_LEVEL <level_number>` in logger.h. The `LOGGER_LEVEL`
 * are listed as follows:
 *  * 0: NOTSET
 *  * 1: DEBUG
 *  * 2: INFO
 *  * 3: WARNING
 *  * 4: ERROR
 *  * 5: CRITICAL
 *
 * Notice:
 *
 * Although the member functions `critical()`, `error()`, `warning()`, `info()`, and `debug()`
 * have a version that accept line number and file name, it is recommended that one should use
 * the macro `logger_print` to call these member functions instead, so that one don't have to
 * type `__FILE__` and `__LINE__` manually.
 *
 * \todo Refine the link in for critical(), error(), warning(), info(), and debug()
 */
class Logger
{
private:
    int log_level;
    FILE* log_fp;
    bool use_color;
    static const char* const color_scheme[6];

private:
    void print_time();
    void print_msg(const char* fmt, va_list arg, const char* level_name);
public:
    Logger(int level = 0, FILE* fp = stderr);//!< Constructor.
    void set_level(int level);//!< Set logger level (0-5).
    void color_on(); //!< Turn on color print for logging information
    void color_off(); //!< Turn off color print for logging information
    void critical(const char* fmt, ...); //!< Logging critical information (level = 5)
    void error(const char* fmt, ...); //!< Logging error information (level = 4)
    void warning(const char* fmt, ...); //!< Logging warning information (level = 3)
    void info(const char* fmt, ...); //!< logging info information (level = 2)
    void debug(const char* fmt, ...);//!< logging debug information (level = 1)

    void critical(unsigned int lineno, const char* fname, const char* fmt, ...); //!< Logging critical information (level = 5). Using of `logger_print` is recommended.
    void error(unsigned int lineno, const char* fname, const char* fmt, ...); //!< Logging error information (level = 4). Using of `logger_print` is recommended.
    void warning(unsigned int lineno, const char* fname, const char* fmt, ...); //!< Logging warning information (level = 3). Using of `logger_print` is recommended.
    void info(unsigned int lineno, const char* fname, const char* fmt, ...); //!< Logging info information (level = 2). Using of `logger_print` is recommended.
    void debug(unsigned int lineno, const char* fname, const char* fmt, ...);//!< Logging debug information (level = 1). Using of `logger_print` is recommended.
};

/*!\ingroup Aux
 * \brief A convenient way to print logging information with file name and line number.
 *
 * The MACRO is a convenient way to call \link loon::Logger::critical critical()\endlink, error(), warning(), info() and debug() without
 * manually write `__LINE__` and `__FILE__`.
 * 
 * \param[in] LOGGER_CALLBACK Not really a callback, it's just one of the member function critical(), error(), warning(), info(), debug().
 * \param[in] ... The message to be logged.
 * 
 * Example:
 * ```cpp
 * loon::Logger logger;
 * logger_print(logger.critical, "Print integer %d and string %s", 123, "hello, world");
 * ```
 *
 * \todo Refine the link in for critical(), error(), warning(), info(), and debug()
 */
#define logger_print(LOGGER_CALLBACK, ...)\
{\
    LOGGER_CALLBACK(__LINE__, __FILE__, __VA_ARGS__);\
}

}// namespace loon

#endif
