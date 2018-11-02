#ifndef __UTIL_EXCEPTION_H
#define __UTIL_EXCEPTION_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdarg>

namespace loon
{

class Exception: public std::exception
{
protected:
    std::string message;
    int ERRNO;

    std::string convert_msg(const char* fmt, va_list arg);
public:
    explicit Exception(const std::string& what_arg);
    explicit Exception(const char* fmt, ...);
    explicit Exception(int err_code, const std::string& what_arg);
    explicit Exception(int err_code, const char* fmt, ...);
    explicit Exception(int err_code, int lineno, const char* fname, const std::string& what_arg);
    explicit Exception(int err_code, int lineno, const char* fname, const char* fmt, ...);
    virtual ~Exception() throw();
    virtual const char* what() const throw();
    virtual int error_code() const throw();
};

} // namespace loon

#endif
