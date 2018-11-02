#include "iobin.h"

namespace loon{

BinWriter::BinWriter(const std::string& fname /* = std::string() */)
{
    if(!fname.empty())      open_file(fout, fname, true);
}

BinWriter::~BinWriter()
{
    if(fout.is_open())
        fout.close();
}

void BinWriter::open(const std::string& fname)
{
    open_file(fout, fname, true);
}

void BinWriter::close()
{
    fout.close();
}

void BinWriter::seek(size_t offset)
{
    fout.seekp(offset, std::ios::beg);
}

void BinWriter::write_int(long long num)
{
    unsigned char bytes[9]={0};
    unsigned char is_negative = 0;
    unsigned long long tmp = num;
    if(num < 0)
    {
        is_negative = 0x80u;
        tmp = -num;
    }
    if(tmp < 64u)
    {
        bytes[0] = (tmp & 0x3Fu) | is_negative;
        fout.write(reinterpret_cast<char*>(bytes), 1);
    }
    else
    {
        short bytes_cnt = 0;
        while(tmp > 0)
        {
            bytes[ ++bytes_cnt ] = tmp & 0xFFu;
            tmp >>= 8;
        }
        bytes[0] = (bytes_cnt & 0x3Fu) | is_negative | 0x40u;
        fout.write(reinterpret_cast<char*>(bytes), bytes_cnt + 1);
    }
}

void BinWriter::write_int(const char *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_int(num[i]);
}
void BinWriter::write_int(const std::vector<char>& num)
{
    for(std::vector<char>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_int( *it );
}
void BinWriter::write_int(const short int* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_int(num[i]);
}
void BinWriter::write_int(const std::vector<short int>& num)
{
    for(std::vector<short int>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_int( *it );
}
void BinWriter::write_int(const int* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_int(num[i]);
}
void BinWriter::write_int(const std::vector<int>& num)
{
    for(std::vector<int>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_int( *it );
}
void BinWriter::write_int(const long long* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_int(num[i]);
}
void BinWriter::write_int(const std::vector<long long>& num)
{
    for(std::vector<long long>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_int( *it );
}



void BinWriter::write_int8(char num)
{
    fout.write(reinterpret_cast<const char*>(&num), 1);
}

void BinWriter::write_int8(const char* num, size_t n)
{
    fout.write(num, n);
}

void BinWriter::write_int8(const std::vector<char>& num)
{
    fout.write(num.data(), num.size());
}



void BinWriter::write_int16(short int num)
{
    fout.write(reinterpret_cast<const char*>(&num), 2);
}

void BinWriter::write_int16(const short int* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 1);
}

void BinWriter::write_int16(const std::vector<short int>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 1);
}


void BinWriter::write_int32(int num)
{
    fout.write(reinterpret_cast<const char*>(&num), 4);
}

void BinWriter::write_int32(const int* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 2);
}

void BinWriter::write_int32(const std::vector<int>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 2);
}


void BinWriter::write_int64(long long num)
{
    fout.write(reinterpret_cast<const char*>(&num), 8);
}

void BinWriter::write_int64(const long long* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 3);
}

void BinWriter::write_int64(const std::vector<long long>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 3);
}



void BinWriter::write_uint(unsigned long long num)
{
    unsigned char bytes[9] = {0};
    if(num < 128)
    {
        bytes[0] = num & 0x7Fu;
        fout.write( reinterpret_cast<char*>(bytes), 1 );
    }
    else
    {
        short bytes_cnt = 0;
        while(num > 0)
        {
            bytes[ ++bytes_cnt ] = num & 0xFFu;
            num >>= 8;
        }
        bytes[0] = (bytes_cnt & 0x7Fu) | 0x80u;
        fout.write(reinterpret_cast<char*>(bytes), bytes_cnt + 1);
    }
}

void BinWriter::write_uint(const unsigned char* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_uint(num[i]);
}
void BinWriter::write_uint(const std::vector<unsigned char>& num)
{
    for(std::vector<unsigned char>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_uint( *it );
}
void BinWriter::write_uint(const unsigned short int* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_uint(num[i]);
}
void BinWriter::write_uint(const std::vector<unsigned short int>& num)
{
    for(std::vector<unsigned short int>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_uint( *it );
}
void BinWriter::write_uint(const unsigned int* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_uint(num[i]);
}
void BinWriter::write_uint(const std::vector<unsigned int>& num)
{
    for(std::vector<unsigned int>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_uint( *it );
}

void BinWriter::write_uint(const unsigned long long* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        write_uint(num[i]);
}

void BinWriter::write_uint(const std::vector<unsigned long long>& num)
{
    for(std::vector<unsigned long long>::const_iterator it = num.begin(); it != num.end(); ++it)
        write_uint( *it );
}


void BinWriter::write_uint8(unsigned char num)
{
    fout.write(reinterpret_cast<const char*>(&num), 1);
}

void BinWriter::write_uint8(const unsigned char* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n);
}

void BinWriter::write_uint8(const std::vector<unsigned char>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size());
}

void BinWriter::write_uint16(unsigned short int num)
{
    fout.write(reinterpret_cast<const char*>(&num), 2);
}

void BinWriter::write_uint16(const unsigned short int* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 1);
}

void BinWriter::write_uint16(const std::vector<unsigned short int>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 1);
}

void BinWriter::write_uint32(unsigned int num)
{
    fout.write(reinterpret_cast<const char*>(&num), 4);
}

void BinWriter::write_uint32(const unsigned int* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 2);
}

void BinWriter::write_uint32(const std::vector<unsigned int>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 2);
}

void BinWriter::write_uint64(unsigned long long num)
{
    fout.write(reinterpret_cast<const char*>(&num), 8);
}

void BinWriter::write_uint64(const unsigned long long* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 3);
}

void BinWriter::write_uint64(const std::vector<unsigned long long>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 1);
}

void BinWriter::write_size_t(size_t num)
{
    fout.write(reinterpret_cast<const char*>(&num), sizeof(size_t));
}

void BinWriter::write_size_t(const size_t* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n * sizeof(size_t));
}

void BinWriter::write_size_t(const std::vector<size_t>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() * sizeof(size_t));
}


void BinWriter::write_float(float num)
{
    fout.write(reinterpret_cast<const char*>(&num), 4);
}

void BinWriter::write_float(const float* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 2);
}

void BinWriter::write_float(const std::vector<float>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 2);
}


void BinWriter::write_double(double num)
{
    fout.write(reinterpret_cast<const char*>(&num), 8);
}

void BinWriter::write_double(const double* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n << 3);
}

void BinWriter::write_double(const std::vector<double>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() << 3);
}


void BinWriter::write_longdouble(long double num)
{
    fout.write(reinterpret_cast<const char*>(&num), sizeof(long double));
}

void BinWriter::write_longdouble(const long double* num, size_t n)
{
    fout.write(reinterpret_cast<const char*>(num), n * sizeof(long double));
}

void BinWriter::write_longdouble(const std::vector<long double>& num)
{
    fout.write(reinterpret_cast<const char*>(num.data()), num.size() * sizeof(long double));
}

void BinWriter::write_string(const std::string& str)
{
    fout.write(reinterpret_cast<const char*>(str.data()), str.length());
}

void BinWriter::write_bytes(const void* bytes, size_t n)
{
    fout.write(reinterpret_cast<const char*>(bytes), n);
}

}
