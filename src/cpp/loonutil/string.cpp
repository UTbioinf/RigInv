#include "string.h"

namespace loon{

/*static*/ int String::compare(const char* s1, const char* s2, size_t n) const
{
    if(n == 0)  return 0;
    return memcmp(s1, s2, n);
}

/*static*/ char* String::realloc(char* ptr, size_t new_size, size_t keep_n/* = 0*/)
{
    if(new_size == 0)
    {
        if(nullptr != ptr)  delete[] ptr;
        return nullptr;
    }
    if(nullptr != ptr)
    {
        char* new_p = new char[ new_size ];
        if(keep_n > 0)  memmove(new_p, ptr, keep_n);
        delete[] ptr;
        return new_p;
    }
    return (new char[ new_size ]);
}

/*static*/ size_t String::safe_double_size(size_t s, bool use_max/* = true */)
{
    if(s == 0)  return 32;
    if(s < (~s))    return (s << 1);
    else
    {
        if(s == (~static_cast<size_t>(0)))
            throw std::length_error("Size has already reached it's maximum and cannot be larger");
        if(use_max)     return ~static_cast<size_t>(0);
        else    throw std::length_error("Size is too large to double");
    }
}

/*static*/ size_t String::safe_size_sum(size_t s1, size_t s2) const
{
    size_t s = s1 + s2;
    if(s < s1 || s < s2)
        throw std::length_error("The sum of two sizes are too large");
    return s;
}

String::String():
    p(nullptr), cap(0), n(0)
{}

String::String(const String& str)
{
    if(nullptr != str.p)
    {
        p = new char[ str.cap ];
        memmove(p, str.p, str.n);
    }
    else    p = nullptr;
    cap = str.cap;
    n = str.n;
}

String::String(const String& str, size_t pos, size_t len/* = std::string::npos*/)
{
    if(pos >= str.n)
    {
        p = nullptr;
        cap = n = 0;
    }
    else
    {
        if(str.n - pos < len) len = str.n - pos;
        cap = std::max(len, static_cast<size_t>(32));
        p = new char[ cap ];
        memmove(p, str.p + pos, len);
        n = len;
    }
}

String::String(const std::string& str)
{
    n = str.length();
    if(n == 0)
    {
        p = nullptr;
        cap = 0;
    }
    else
    {
        cap = std::max(static_cast<size_t>(32), n);
        p = new char[ cap ];
        memmove(p, str.data(), n);
    }
}

String::String(const std::string& str, size_t pos, size_t len/* = std::string::npos*/)
{
    if(pos >= str.length())
    {
        p = nullptr;
        cap = n = 0;
    }
    else
    {
        if(str.length() - pos < len)    len = str.length() - pos;
        cap = std::max(len, static_cast<size_t>(32));
        p = new char[ cap ];
        memmove(p, str.c_str() + pos, len);
        n = len;
    }
}

String::String(const char* s)
{
    if(s)
    {
        n = strlen(s);
        if(n > 0)
        {
            cap = std::max(static_cast<size_t>(32), n);
            p = new char[ cap ];
            memmove(p, s, n);
        }
        else
        {
            cap = 0;
            p = nullptr;
        }
    }
    else
    {
        p = nullptr;
        cap = n = 0;
    }
}

String::String(const char* s, size_t n)
{
    if(s && n > 0)
    {
        this->cap = this->n = n;
        p = new char[n];
        memmove(p, s, n);
    }
    else
    {
        this->cap = this->n = 0;
        p = nullptr;
    }
}

String::String(char* s, size_t sublen, size_t length)
{
    if(s)   p = s;
    else    p = nullptr;
    cap = length; 
    this->n = sublen;
}

String::String(size_t n, char c)
{
    if(n > 0)
    {
        cap = this->n = n;
        p = new char[ n ];
        memset(p, c, n);
    }
    else
    {
        cap = this->n = 0;
        p = nullptr;
    }
}

String::String(String&& str) noexcept
{
    p = str.p;
    cap = str.cap;
    n = str.n;

    if(str.cap > 0)
    {
        str.cap = str.n = 0;
        str.p = nullptr;
    }
}

String::~String()
{
    if(cap > 0)
        delete[] p;
}

String& String::operator=(const String& str)
{
    if(this == &str)    return *this;
    n = str.n;

    if(cap < str.n)
    {
        cap = str.cap;
        p = realloc(p, cap, 0);
    }
    if(n > 0)    memmove(p, str.p, n);
    return *this;
}

String& String::operator=(const std::string& str)
{
    n = str.length();
    if(cap < n)
    {
        cap = str.capacity();
        p = realloc(p, cap, 0);
    }
    if(n > 0)    memmove(p, str.c_str(), n);
    return *this;
}

String& String::operator=(const char* s)
{
    if(s)
    {
        n = strlen(s);
        if(cap < n)
        {
            cap = std::max(n, safe_double_size(cap));
            p = realloc(p, cap, 0);
        }
        if(n > 0)    memmove(p, s, n);
    }
    else
    {
        n = 0;
    }
    return *this;
}

String& String::operator=(char c)
{
    if(nullptr != p)
    {
        p[0] = c;
        n = 1;
    }
    else
    {
        p = new char[32];
        cap = 32;
        n = 1;
        p[0] = c;
    }
    return *this;
}

String& String::operator=(String&& str)
{
    if(nullptr != p)
        delete[] p;
    p = str.p;
    cap = str.cap;
    n = str.n;

    str.p = nullptr;
    str.cap = str.n = 0;
    return *this;
}

size_t String::size() const noexcept
{
    return n;
}

size_t String::length() const noexcept
{
    return n;
}

size_t String::capacity() const noexcept
{
    return cap;
}

String& String::resize(size_t n)
{
    if(cap < n)
    {
        p = realloc(p, n, this->n);
        cap = n;
    }
    this->n = n;
    return *this;
}

String& String::resize(size_t n, char c)
{
    if(cap < n)
    {
        p = realloc(p, n, this->n);
        cap = n;
    }
    if(this->n < n)
        memset(p + this->n, c, n - this->n);
    this->n = n;
    return *this;
}

String& String::reserve(size_t n)
{
    if(cap < n)
    {
        p = realloc(p, n, this->n);
        cap = n;
    }
    return *this;
}

String& String::clear() noexcept
{
    n = 0;
    return *this;
}

bool String::empty() const noexcept
{
    return n == 0;
}

char& String::operator[](size_t pos)
{
#ifdef DEBUG
    if(pos >= n) throw std::out_of_range("String[] access out of range");
#endif
    return p[pos];
}

const char& String::operator[](size_t pos) const
{
#ifdef DEBUG
    if(pos >= n) throw std::out_of_range("String[] access out of range");
#endif
    return p[pos];
}

char& String::at(size_t pos)
{
    if(pos >= n)    throw std::out_of_range("String[] access out of range");
    return p[pos];
}

const char& String::at(size_t pos) const
{
    if(pos >= n)    throw std::out_of_range("String[] access out of range");
    return p[pos];
}

char& String::back()
{
#ifdef DEBUG
    if(n == 0)    throw std::out_of_range("Empty String, failed to access back()");
#endif
    return p[n - 1];
}

const char& String::back() const
{
#ifdef DEBUG
    if(n == 0)    throw std::out_of_range("Empty String, failed to access back()");
#endif
    return p[n - 1];
}

char& String::front()
{
#ifdef DEBUG
    if(n == 0)  throw std::out_of_range("Empty String, failed to access front()");
#endif
    return p[0];
}

const char& String::front() const
{
#ifdef DEBUG
    if(n == 0)  throw std::out_of_range("Empty String, failed to access front()");
#endif
    return p[0];
}

String& String::operator+=(const String& str)
{
    if(nullptr != p)
    {
        if(cap - n < str.n)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str.n, n));
            p = realloc( p, cap, this->n);
        }
        memmove( p + n, str.p, str.n);
        n += str.n;
    }
    else
    {
        cap = str.cap;
        n = str.n;
        p = new char[cap];
        memmove( p, str.p, n );
    }
    return *this;
}

String& String::operator+=(const std::string& str)
{
    if(nullptr != p)
    {
        if(cap - n < str.length())
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str.length(), n));
            p = realloc(p, cap, this->n);
        }
        memmove( p + n, str.c_str(), str.length() );
        n += str.length();
    }
    else
    {
        cap = str.capacity();
        n = str.length();
        p = new char[cap];
        memmove(p, str.c_str(), n);
    }
    return *this;
}

String& String::operator+=(const char* str)
{
    if(nullptr != p)
    {
        size_t str_len = strlen( str );
        if(cap - n < str_len)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str_len, n));
            p = realloc(p, cap, this->n);
        }
        memmove(p + n, str, str_len);
        n += str_len;
    }
    else
    {
        cap = strlen( str );
        n = cap;
        p = new char[cap];
        memmove(p, str, n);
    }
    return *this;
}

String& String::operator+=(char c)
{
    if(cap <= n)
    {
        cap = safe_double_size( cap );
        p = realloc(p, cap, this->n);
    }
    p[n++] = c;
    return *this;
}

String& String::append(const String& str)
{
    if(nullptr != p)
    {
        if(cap - n < str.n)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str.n, n));
            p = realloc(p, cap, this->n);
        }
        memmove( p + n, str.p, str.n);
        n += str.n;
    }
    else
    {
        cap = str.cap;
        n = str.n;
        p = new char[cap];
        memmove( p, str.p, n );
    }
    return *this;
}

String& String::append(const String& str, size_t subpos, size_t sublen)
{
    if(subpos >= str.n) return *this;
    if(sublen > str.n - subpos) sublen = str.n - subpos;
    if(nullptr != p)
    {
        if(cap - n < sublen)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(n, sublen));
            p = realloc(p, cap, this->n);
        }
        memmove(p + n, str.p + subpos, sublen);
        n += sublen;
    }
    else
    {
        cap = n = sublen;
        p = new char[ n ];
        memmove(p, str.p + subpos, sublen);
    }
    return *this;
}

String& String::append(const std::string& str)
{
    if(nullptr != p)
    {
        if(cap - n < str.length())
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str.length(), n));
            p = realloc(p, cap, this->n);
        }
        memmove(p + n, str.c_str(), str.length());
        n += str.length();
    }
    else
    {
        cap = str.capacity();
        n = str.length();
        p = new char[cap];
        memmove(p, str.c_str(), n);
    }
    return *this;
}

String& String::append(const std::string& str, size_t subpos, size_t sublen)
{
    if(subpos >= str.length()) return *this;
    if(sublen > str.length() - subpos) sublen = str.length() - subpos;
    if(nullptr != p)
    {
        if(cap - n < sublen)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(n, sublen));
            p = realloc(p, cap, this->n);
        }
        memmove(p + n, str.c_str() + subpos, sublen);
        n += sublen;
    }
    else
    {
        cap = n = sublen;
        p = new char[ n ];
        memmove(p, str.c_str() + subpos, sublen);
    }
    return *this;
}

String& String::append(const char* s)
{
    if(nullptr != p)
    {
        size_t str_len = strlen( s );
        if(cap - n < str_len)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(str_len, n));
            p = realloc(p, cap, this->n);
        }
        memmove(p + n, s, str_len);
        n += str_len;
    }
    else
    {
        n = strlen( s );
        cap = std::max(static_cast<size_t>(32), n);
        p = new char[cap];
        memmove(p, s, n);
    }
    return *this;
}

String& String::append(const char* s, size_t n)
{
    if(nullptr != p)
    {
        if(cap - this->n < n)
        {
            cap = std::max(safe_double_size(cap), safe_size_sum(this->n, n));
            p = realloc(p, cap, this->n);
        }
        memmove(p + this->n, s, n);
        this->n += n;
    }
    else
    {
        this->n = n;
        cap = std::max(static_cast<size_t>(32), n);
        p = new char[ cap ];
        memmove(p, s, n);
    }
    return *this;
}

String& String::push_back(char c)
{
    if(cap <= n)
    {
        cap = safe_double_size(cap);
        p = realloc( p, cap, this->n );
    }
    p[n++] = c;
    return *this;
}

String& String::assign(const String& str)
{
    if(this == &str)    return *this;
    n = str.n;
    if(cap < n)
    {
        cap = str.cap;
        p = realloc(p, cap, 0);
    }
    if(n > 0)    memmove(p, str.p, n);
    return *this;
}

String& String::assign(const String& str, size_t subpos, size_t sublen)
{
    if(subpos >= str.n)
    {
        n = 0;
        return *this;
    }
    if(sublen > str.n - subpos) n = str.n - sublen;
    if(cap < n)
    {
        cap = str.cap;
        p = realloc(p, cap, 0);
    }
    if(n > 0)    memmove(p, str.p + subpos, n);
    return *this;
}

String& String::assign(const std::string& str)
{
    n = str.length();
    if(cap < n)
    {
        cap = str.capacity();
        p = realloc(p, cap, 0);
    }
    if(n > 0)   memmove(p, str.data(), n);
    return *this;
}

String& String::assign(const std::string& str, size_t subpos, size_t sublen)
{
    if(subpos >= str.length())
    {
        n = 0;
        return *this;
    }
    if(sublen > str.length() - subpos)  n = str.length() - subpos;
    if(cap < n)
    {
        cap = str.capacity();
        p = realloc(p, cap, 0);
    }
    if(n > 0)   memmove(p, str.data() + subpos, n);
    return *this;
}

String& String::assign(const char* s)
{
    if(s)
    {
        size_t s_n = strlen(s);
        n = s_n;
        if(cap < n)
        {
            cap = std::max(n, safe_double_size(cap));
            p = realloc(p, cap);
        }
        if(n > 0)   memmove(p, s, n);
    }
    else
    {
        n = 0;
    }
    return *this;
}

String& String::assign(const char* s, size_t n)
{
    this->n = n;
    if(cap < n)
    {
        cap = std::max(n, safe_double_size(cap));
        p = realloc(p, cap);
    }
    if(n > 0)   memmove(p, s, n);
    return *this;
}

String& String::assign(size_t n, char c)
{
    this->n = n;
    if(cap < n)
    {
        cap = std::max(n, safe_double_size(cap));
        p = realloc(p, cap);
    }
    if(n > 0)   memset(p, c, n);
    return *this;
}

String& String::assign(String&& str) noexcept
{
    p = str.p;
    cap = str.cap;
    n = str.n;

    if(str.cap > 0)
    {
        str.cap = str.n = 0;
        str.p = nullptr;
    }
}

void String::plant(char* s, size_t sublen, size_t length)
{
    if(nullptr != p)    delete[] p;
    if(s)   p = s;
    else    p = nullptr;
    n = sublen;
    cap = length;
}

char* String::rob()
{
    char* pp = p;
    p = nullptr;
    n = cap = 0;
    return pp;
}

String& String::erase(size_t pos/* = 0*/, size_t len/* = std::string::npos*/)
{
    if(pos >= n)    return *this;
    if(n - pos < len)   len = n - pos;
    memmove(p + pos, p + pos + len, len);
    n -= len;
    return *this;
}

String& String::swap(String& str)
{
    std::swap(p, str.p);
    std::swap(n, str.n);
    std::swap(cap, str.cap);
    return *this;
}

String& String::swap(char*& s, size_t length)
{
    std::swap(p, s);
    cap = n = length;
    if(! p) p = nullptr;
    return *this;
}

char String::pop_back()
{
#ifdef DEBUG
    if(n == 0)
        throw std::out_of_range("Empty String, failed to pop_back()");
#endif
    return p[--n];
}

char* String::c_str()
{
    if(n == cap)
    {
        cap = safe_double_size( cap );
        p = realloc(p, cap, this->n);
    }
    p[ this->n ] = 0;
    return p;
}

const char* String::c_str() const noexcept
{
    return const_cast<String*>(this)->c_str();
}

char* String::data()
{
    return p;
}

const char* String::data() const noexcept
{
    return p;
}

size_t String::find(const String& str, size_t pos/* = 0*/) const noexcept
{
    return find(str.p, pos, str.n);
}

size_t String::find(const std::string& str, size_t pos /*= 0*/) const noexcept
{
    return find(str.c_str(), pos, str.length());
}

size_t String::find(const char* s, size_t pos/* = 0*/) const
{
    return find(s, pos, strlen(s));
}

size_t String::find(const char* s, size_t pos, size_t n) const
{
    switch(n)
    {
        case 0:
            return std::string::npos;
        case 1:
            return find(s[0], pos);
        default:
            if(n <= this->n)
            {
                size_t end_pos = this->n - n;
                while(pos <= end_pos)
                {
                    size_t j;
                    for(j = 0; j < n; ++j)
                    {
                        char *pp = (char*)memchr(p + pos + j, s[j], end_pos - pos + 1);
                        if(pp)
                        {
                            if(pp != p + pos + j)
                            {
                                pos = pp - p - j;
                                break;
                            }
                        }
                        else    return std::string::npos;
                    }
                    if(j == n)  return pos;
                }

                // for(; pos <= this->n - n; ++pos)
                // {
                //     if(s[0] == p[pos] && memcmp(p + pos + 1, s + 1, n - 1) == 0)
                //         return pos;
                // }
            }
            return std::string::npos;
    }
}

size_t String::find(char c, size_t pos/* = 0*/) const noexcept
{
    if(pos >= n)    return std::string::npos;
    char* pp = (char *)memchr(p + pos, c, n - pos);
    return pp ? pp - p : std::string::npos;
}

String String::substr(size_t pos/* = 0*/, size_t len/* = std::string::npos */) const
{
    if(nullptr == p || n == 0)    return String();

    if(len > n) len = n;
    String ret;
    ret.p = new char[len];
    memmove(ret.p, p + pos, len);
    ret.cap = ret.n = len;
    return ret;
}

int String::compare(const String& str) const noexcept
{
    int cmp = compare(p, str.p, std::min(n, str.n));
    return cmp ? cmp : (n < str.n ? -1 : n - str.n);
}

int String::compare(const std::string& str) const noexcept
{
    int cmp = compare(p, str.data(), std::min(n, str.length()));
    return cmp ? cmp : (n < str.length() ? -1 : n - str.length());
}

int String::compare(const char* s) const
{
    size_t len = strlen(s);
    int cmp = compare(p, s, std::min(len, n));
    return cmp ? cmp : (n < len ? -1 : n - len);
}

int String::compare(size_t pos, size_t len, const String& str) const
{
    if(pos >= n)    throw std::out_of_range("First string pos out of range in compare()");
    if(n - pos < len)   len = n - pos;

    int cmp = compare(p + pos, str.p, std::min(len, str.n) );
    return cmp ? cmp : ( len < str.n ? -1 : len - str.n );
}

int String::compare(size_t pos, size_t len, const std::string& str) const
{
    if(pos >= n)    throw std::out_of_range("First string pos out of range in compare()");
    if(n - pos < len)   len = n - pos;

    int cmp = compare(p + pos, str.data(), std::min(len, str.length()) );
    return cmp ? cmp : ( len < str.length() ? -1 : len - str.length() );
}

int String::compare(size_t pos, size_t len, const String& str,
        size_t subpos, size_t sublen) const
{
    if(pos >= n || subpos >= str.n) throw std::out_of_range("First or second string pos out of range in Compare()");
    if(n - pos < len)   len = n - pos;
    if(str.n - subpos < sublen) sublen = str.n - subpos;

    int cmp = compare(p + pos, str.p + subpos, std::min(len, sublen));
    return cmp ? cmp : ( len < sublen ? -1 : len - sublen );
}

int String::compare(size_t pos, size_t len, const std::string& str,
        size_t subpos, size_t sublen) const
{
    if(pos >= n || subpos >= str.length()) throw std::out_of_range("First or second string pos out of range in Compare()");
    if(n - pos < len)   len = n - pos;
    if(str.length() - subpos < sublen) sublen = str.length() - subpos;

    int cmp = compare(p + pos, str.data() + subpos, std::min(len, sublen));
    return cmp ? cmp : ( len < sublen ? -1 : len - sublen );
}

int String::compare(size_t pos, size_t len, const char* s) const
{
    if(pos >= n)    throw std::out_of_range("First string pos out of range in compare()");
    if(n - pos < len)   len = n - pos;
    size_t str_len = strlen(s);

    int cmp = compare(p + pos, s, std::min(len, str_len));
    return cmp ? cmp : ( len < str_len ? -1 : len - str_len);
}

int String::compare(size_t pos, size_t len, const char* s, size_t n) const
{
    if(pos >= this->n)  throw std::out_of_range("First string pos out of range in compare()");
    if(this->n - pos < len) len = this->n - pos;

    int cmp = compare(p + pos, s, std::min(len, n));
    return cmp ? cmp : (len < n ? -1 : len - n);
}

bool operator==(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) == 0;
}

bool operator==(const char* lhs, const String& rhs)
{
    return rhs.compare( lhs ) == 0;
}

bool operator==(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) == 0;
}

bool operator==(const std::string& lhs, const String& rhs)
{
    return rhs.compare( lhs ) == 0;
}

bool operator==(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) == 0;
}

bool operator!=(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) != 0;
}

bool operator!=(const char* lhs, const String& rhs)
{
    return rhs.compare( lhs ) != 0;
}

bool operator!=(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) != 0;
}

bool operator!=(const std::string& lhs, const String& rhs)
{
    return rhs.compare( lhs ) != 0;
}

bool operator!=(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) != 0;
}

bool operator<(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) < 0;
}

bool operator<(const char* lhs, const String& rhs)
{
    return 0 < rhs.compare( lhs );
}

bool operator<(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) < 0;
}

bool operator<(const std::string& lhs, const String& rhs)
{
    return 0 < rhs.compare( lhs );
}

bool operator<(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) < 0;
}

bool operator<=(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) <= 0;
}

bool operator<=(const char* lhs, const String& rhs)
{
    return  0 <= rhs.compare( lhs );
}

bool operator<=(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) <= 0;
}

bool operator<=(const std::string& lhs, const String& rhs)
{
    return 0 <= rhs.compare( lhs );
}

bool operator<=(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) <= 0;
}

bool operator>(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) > 0;
}

bool operator>(const char* lhs, const String& rhs)
{
    return  0 > rhs.compare( lhs );
}

bool operator>(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) > 0;
}

bool operator>(const std::string& lhs, const String& rhs)
{
    return 0 > rhs.compare( lhs );
}

bool operator>(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) > 0;
}

bool operator>=(const String& lhs, const String& rhs)
{
    return lhs.compare( rhs ) >= 0;
}

bool operator>=(const char* lhs, const String& rhs)
{
    return  0 >= rhs.compare( lhs );
}

bool operator>=(const String& lhs, const char* rhs)
{
    return lhs.compare( rhs ) >= 0;
}

bool operator>=(const std::string& lhs, const String& rhs)
{
    return 0 >= rhs.compare( lhs );
}

bool operator>=(const String& lhs, const std::string& rhs)
{
    return lhs.compare( rhs ) >= 0;
}

void swap(String& x, String& y)
{
    std::swap(x.p, y.p);
    std::swap(x.n, y.n);
    std::swap(x.cap, y.cap);
}

void swap(String& x, char*& y)
{
    x.swap(y, strlen(y));
}

void swap(char*& x, String& y)
{
    y.swap(x, strlen(x));
}

std::istream& operator>>(std::istream& is, String& str)
{
    std::string t;
    is >> t;
    str = t;
    return is;
}

std::ostream& operator<<(std::ostream& os, const String& str)
{
    for(size_t i = 0; i < str.n; ++i)
        os << str.p[i];
    return os;
}

std::istream& getline(std::istream& is, String& str, char delim)
{
    std::string line;
    getline(is, line, delim);
    str = line;
    return is;
}

std::istream& getline(std::istream&& is, String& str, char delim)
{
    std::string line;
    getline(is, line, delim);
    str = line;
    return is;
}

std::istream& getline(std::istream& is, String& str)
{
    std::string line;
    getline(is, line);
    str = line;
    return is;
}

std::istream& getline(std::istream&& is, String& str)
{
    std::string line;
    getline(is, line);
    str = line;
    return is;
}
} // namespace loon
