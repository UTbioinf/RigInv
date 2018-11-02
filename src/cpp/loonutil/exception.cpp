#include "exception.h"

namespace loon
{

std::string Exception::convert_msg(const char* fmt, va_list arg)
{
    va_list arg2;
    va_copy(arg2, arg);
    int buf_size = std::vsnprintf(NULL, 0, fmt, arg2) + 1;
    va_end( arg2 );
    char* buf = new char[buf_size];
    std::vsnprintf(buf, buf_size, fmt, arg);
    buf[buf_size - 1] = 0;
    std::string ret( buf );
    delete[] buf;
    return ret;
}

Exception::Exception(const std::string& what_arg): message(what_arg), ERRNO(0)
{}

Exception::Exception(const char* fmt, ...): ERRNO(0)
{
    va_list arg;
    va_start( arg, fmt );
    message = convert_msg(fmt, arg);
    va_end( arg );
}

Exception::Exception(int err_code, const std::string& what_arg): ERRNO(err_code), message(what_arg)
{}

Exception::Exception(int err_code, const char* fmt, ...): ERRNO(err_code)
{
    va_list arg;
    va_start( arg, fmt );
    message = convert_msg(fmt, arg);
    va_end(arg);
}

Exception::Exception(int err_code, int lineno, const char* fname, const std::string& what_arg): ERRNO(err_code)
{
    message = std::string("[") + std::string(fname) + std::string(": ") + std::to_string(lineno) + std::string("]: ") + what_arg;
}

Exception::Exception(int err_code, int lineno, const char* fname, const char* fmt, ...): ERRNO(err_code)
{
    va_list arg;
    va_start(arg, fmt);
    message = std::string("[") + std::string(fname) + std::string(": ") + std::to_string(lineno) + std::string("]: ") + convert_msg(fmt, arg);
    va_end(arg);
}

Exception::~Exception() throw()
{}

const char* Exception::what() const throw()
{
    return message.c_str();
}

int Exception::error_code() const throw()
{
    return ERRNO;
}

}// namespace loon
