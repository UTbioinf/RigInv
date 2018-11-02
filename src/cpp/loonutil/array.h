#ifndef __LOON_ARRAY_H
#define __LOON_ARRAY_H

#include <iostream>
#include <array>

// Usage:
// using loonarray::operator<<;
// using loonarray::operator>>;
namespace loonarray{

template<class T, size_t N>
std::istream& operator>>(std::istream& iss, std::array<T, N>& obj)
{
    for(typename std::array<T, N>::iterator it = obj.begin(); it != obj.end(); ++it)
    {
        if(!(iss >> (*it)))
            return iss;
    }
    return iss;
}

template<class T, size_t N>
std::ostream& operator<<(std::ostream& oss, const std::array<T, N>& obj)
{
    for(typename std::array<T, N>::const_iterator it = obj.begin(); it != obj.end(); ++it)
        oss << (*it) << ' ';
    return oss;
}

}// namespace loonarray

#endif
