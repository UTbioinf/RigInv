#ifndef __LOONLIB_LOONUTIL_STRING_H
#define __LOONLIB_LOONUTIL_STRING_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

namespace loon{

class String{
private:
    char* p;
    size_t cap;
    size_t n;

    static int compare(const char* s1, const char* s2, size_t n) const;
    static char* realloc(char* ptr, size_t new_size, size_t keep_n = 0);
    static size_t safe_double_size(size_t s, bool use_max = true);
    static size_t safe_size_sum(size_t s1, size_t s2) const;
public:
    String();
    String(const String& str);
    String(const String& str, size_t pos, size_t len = std::string::npos);
    String(const std::string& str);
    String(const std::string& str, size_t pos, size_t len = std::string::npos);
    String(const char* s);
    String(const char* s, size_t n);
    String(char* s, size_t sublen, size_t length);
    String(size_t n, char c);
    String(String&& str) noexcept;

    ~String();

    String& operator=(const String& str);
    String& operator=(const std::string& str);
    String& operator=(const char* s);
    String& operator=(char* s);
    String& operator=(char c);
    String& operator=(String&& str);

    size_t size() const noexcept;
    size_t length() const noexcept;
    size_t capacity() const noexcept;
    String& resize(size_t n);
    String& resize(size_t n, char c);
    String& reserve(size_t n);
    String& clear() noexcept;
    bool empty() const noexcept;

    char& operator[](size_t pos);
    const char& operator[](size_t pos) const;
    char& at(size_t pos);
    const char& at(size_t pos) const;
    char& back();
    const char& back() const;
    char& front();
    const char& front() const;

    String& operator+=(const String& str);
    String& operator+=(const std::string& str);
    String& operator+=(const char* str);
    String& operator+=(char c);

    String& append(const String& str);
    String& append(const String& str, size_t subpos, size_t sublen);
    String& append(const std::string& str);
    String& append(const std::string& str, size_t subpos, size_t sublen);
    String& append(const char* s);
    String& append(const char* s, size_t n);

    String& push_back(char c);

    String& assign(const String& str);
    String& assign(const String& str, size_t subpos, size_t sublen);
    String& assign(const std::string& str);
    String& assign(const std::string& str, size_t subpos, size_t sublen)
    String& assign(const char* s);
    String& assign(const char* s, size_t n);
    String& assign(size_t n, char c);
    String& assign(String&& str) noexcept;

    void plant(char* s, size_t sublen, size_t length);
    char* rob();
    // insert()

    String& erase(size_t pos = 0, size_t len = std::string::npos);

    // replace()

    String& swap(String& str);
    String& swap(char*& s, size_t length);

    char pop_back();

    char* c_str();
    const char* c_str() const noexcept; // the string is null-terminated

    char* data();
    const char* data() const noexcept; // the string is not null-terminated

    size_t find(const String& str, size_t pos = 0) const noexcept;
    size_t find(const std::string& str, size_t pos = 0) const noexcept;
    size_t find(const char* s, size_t pos = 0) const;
    size_t find(const char* s, size_t pos, size_t n) const;
    size_t find(char c, size_t pos = 0) const noexcept;

    String substr(size_t pos = 0, size_t len = std::string::npos) const;

    int compare(const String& str) const noexcept;
    int compare(const std::string& str) const noexcept;
    int compare(const char* s) const;
    int compare(size_t pos, size_t len, const String& str) const;
    int compare(size_t pos, size_t len, const std::string& str) const;
    int compare(size_t pos, size_t len, const String& str,
            size_t subpos, size_t sublen) const;
    int compare(size_t pos, size_t len, const std::string& str,
            size_t subpos, size_t sublen) const;
    int compare(size_t pos, size_t len, const char* s) const;
    int compare(size_t pos, size_t len, const char* s, size_t n) const;

    // TODO: begin
    static String to_string(float val);
    static String to_string(double val);
    static String to_string(long double val);
    static String to_string(short val);
    static String to_string(int val);
    static String to_string(long val);
    static String to_string(long long val);
    static String to_string(unsigned short val);
    static String to_string(unsigned int val);
    static String to_string(unsigned long val);
    static String to_string(unsigned long long val);

    float stof() const;
    double stod() const;
    long double stold() const;
    short stos() const;
    int stoi() const;
    long stol() const;
    long long stoll() const;
    unsigned short stous() const;
    unsigned int stoui() const;
    unsigned long stoul() const;
    unsigned long long stoull() const;
    size_t stosize_t() const;
    // TODO: end


    // operator+

    // relational operators
    friend bool operator==(const String& lhs,       const String& rhs);
    friend bool operator==(const char* lhs,         const String& rhs);
    friend bool operator==(const String& lhs,       const char* rhs);
    friend bool operator==(const std::string& lhs,  const String& rhs);
    friend bool operator==(const String& lhs,       const std::string& rhs);

    friend bool operator!=(const String& lhs,       const String& rhs);
    friend bool operator!=(const char* lhs,         const String& rhs);
    friend bool operator!=(const String& lhs,       const char* rhs);
    friend bool operator!=(const std::string& lhs,  const String& rhs);
    friend bool operator!=(const String& lhs,       const std::string& rhs);

    friend bool operator<(const String& lhs,       const String& rhs);
    friend bool operator<(const char* lhs,         const String& rhs);
    friend bool operator<(const String& lhs,       const char* rhs);
    friend bool operator<(const std::string& lhs,  const String& rhs);
    friend bool operator<(const String& lhs,       const std::string& rhs);

    friend bool operator<=(const String& lhs,       const String& rhs);
    friend bool operator<=(const char* lhs,         const String& rhs);
    friend bool operator<=(const String& lhs,       const char* rhs);
    friend bool operator<=(const std::string& lhs,  const String& rhs);
    friend bool operator<=(const String& lhs,       const std::string& rhs);

    friend bool operator>(const String& lhs,       const String& rhs);
    friend bool operator>(const char* lhs,         const String& rhs);
    friend bool operator>(const String& lhs,       const char* rhs);
    friend bool operator>(const std::string& lhs,  const String& rhs);
    friend bool operator>(const String& lhs,       const std::string& rhs);

    friend bool operator>=(const String& lhs,       const String& rhs);
    friend bool operator>=(const char* lhs,         const String& rhs);
    friend bool operator>=(const String& lhs,       const char* rhs);
    friend bool operator>=(const std::string& lhs,  const String& rhs);
    friend bool operator>=(const String& lhs,       const std::string& rhs);

    // swap
    friend void swap(String& x, String& y);
    friend void swap(String& x, char*& y);
    friend void swap(char*& x, String& y);

    // >>
    friend std::istream& operator>>(std::istream& is, String& str);
    // <<
    friend std::ostream& operator<<(std::ostream& os, const String& str);

    // getline
    friend std::istream& getline(std::istream& is,  String& str, char delim);
    friend std::istream& getline(std::istream&& is, String& str, char delim);
    friend std::istream& getline(std::istream& is,  String& str);
    friend std::istream& getline(std::istream&& is, String& str);
};

} // namespace loon

#endif
