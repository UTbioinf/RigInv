#ifndef __BIN_WRITER_H
#define __BIN_WRITER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "util.h"

namespace loon
{

class BinWriter
{
protected:
    std::ofstream fout;
public:
    BinWriter(const std::string& fname = std::string());
    ~BinWriter();
    void open(const std::string& fname);
    void close();
    void seek(size_t offset);

    void write_int(long long num);
    void write_int(const char *num, size_t n);
    void write_int(const std::vector<char>& num);
    void write_int(const short int* num, size_t n);
    void write_int(const std::vector<short int>& num);
    void write_int(const int* num, size_t n);
    void write_int(const std::vector<int>& num);
    void write_int(const long long *num, size_t n);
    void write_int(const std::vector<long long>& num);

    void write_int8(char num);
    void write_int8(const char *num, size_t n);
    void write_int8(const std::vector<char>& num);

    void write_int16(short int num);
    void write_int16(const short int *num, size_t n);
    void write_int16(const std::vector<short int>& num);

    void write_int32(int num);
    void write_int32(const int* num, size_t n);
    void write_int32(const std::vector<int>& num);

    void write_int64(long long num);
    void write_int64(const long long* num, size_t n);
    void write_int64(const std::vector<long long>& num);

    void write_uint(unsigned long long num);
    void write_uint(const unsigned char* num, size_t n);
    void write_uint(const std::vector<unsigned char>& num);
    void write_uint(const unsigned short int* num, size_t n);
    void write_uint(const std::vector<unsigned short int>& num);
    void write_uint(const unsigned int* num, size_t n);
    void write_uint(const std::vector<unsigned int>& num);
    void write_uint(const unsigned long long* num, size_t n);
    void write_uint(const std::vector<unsigned long long>& num);

    void write_uint8(unsigned char num);
    void write_uint8(const unsigned char* num, size_t n);
    void write_uint8(const std::vector<unsigned char>& num);

    void write_uint16(unsigned short int num);
    void write_uint16(const unsigned short int* num, size_t n);
    void write_uint16(const std::vector<unsigned short int>& num);

    void write_uint32(unsigned int num);
    void write_uint32(const unsigned int* num, size_t n);
    void write_uint32(const std::vector<unsigned int>& num);

    void write_uint64(unsigned long long num);
    void write_uint64(const unsigned long long* num, size_t n);
    void write_uint64(const std::vector<unsigned long long>& num);

    void write_size_t(size_t num);
    void write_size_t(const size_t* num, size_t n);
    void write_size_t(const std::vector<size_t>& num);

    void write_float(float num);
    void write_float(const float* num, size_t n);
    void write_float(const std::vector<float>& num);

    void write_double(double num);
    void write_double(const double* num, size_t n);
    void write_double(const std::vector<double>& num);

    void write_longdouble(long double num);
    void write_longdouble(const long double* num, size_t n);
    void write_longdouble(const std::vector<long double>& num);

    void write_string(const std::string& str);
    void write_bytes(const void* bytes, size_t n);
};

class BinReader
{
protected:
    std::ifstream fin;
public:
    BinReader(const std::string& fname = std::string());
    ~BinReader();
    void open(const std::string& fname);
    void close();
    void seek(size_t offset);
    unsigned long long file_size();

    long long read_int();
    void read_int(char *num, size_t n);
    void read_int(std::vector<char>& num, size_t n);
    void read_int(short int *num, size_t n);
    void read_int(std::vector<short int>& num, size_t n);
    void read_int(int *num, size_t n);
    void read_int(std::vector<int>& num, size_t n);
    void read_int(long long *num, size_t n);
    void read_int(std::vector<long long>& num, size_t n);

    char read_int8();
    void read_int8(char *num, size_t n);
    void read_int8(std::vector<char>& num, size_t n);

    short int read_int16();
    void read_int16(short int *num, size_t n);
    void read_int16(std::vector<short int>& num, size_t n);

    int read_int32();
    void read_int32(int* num, size_t n);
    void read_int32(std::vector<int>& num, size_t n);

    long long read_int64();
    void read_int64(long long* num, size_t n);
    void read_int64(std::vector<long long>& num, size_t n);

    unsigned long long read_uint();
    void read_uint(unsigned char *num, size_t n);
    void read_uint(std::vector<unsigned char>& num, size_t n);
    void read_uint(unsigned short int *num, size_t n);
    void read_uint(std::vector<unsigned short int>& num, size_t n);
    void read_uint(unsigned int *num, size_t n);
    void read_uint(std::vector<unsigned int>& num, size_t n);
    void read_uint(unsigned long long* num, size_t n);
    void read_uint(std::vector<unsigned long long>& num, size_t n);

    unsigned char read_uint8();
    void read_uint8(unsigned char* num, size_t n);
    void read_uint8(std::vector<unsigned char>& num, size_t n);

    unsigned short int read_uint16();
    void read_uint16(unsigned short int* num, size_t n);
    void read_uint16(std::vector<unsigned short int>& num, size_t n);

    unsigned int read_uint32();
    void read_uint32(unsigned int* num, size_t n);
    void read_uint32(std::vector<unsigned int>& num, size_t n);

    unsigned long long read_uint64();
    void read_uint64(unsigned long long* num, size_t n);
    void read_uint64(std::vector<unsigned long long>& num, size_t n);

    size_t read_size_t();
    void read_size_t(size_t* num, size_t n);
    void read_size_t(std::vector<size_t>& num, size_t n);

    float read_float();
    void read_float(float* num, size_t n);
    void read_float(std::vector<float>& num, size_t n);

    double read_double();
    void read_double(double* num, size_t n);
    void read_double(std::vector<double>& num, size_t n);

    long double read_longdouble();
    void read_longdouble(long double* num, size_t n);
    void read_longdouble(std::vector<long double>& num, size_t n);

    void read_string(std::string& str, size_t n);
    void read_bytes(void* bytes, size_t n);
};


}// namespace loon


#endif
