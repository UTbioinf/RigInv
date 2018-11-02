#include "iobin.h"

namespace loon{

BinReader::BinReader(const std::string& fname/* = std::string()*/)
{
    if(!fname.empty())  open_file(fin, fname, true);
}

BinReader::~BinReader()
{
    if(fin.is_open())
        fin.close();
}

void BinReader::open(const std::string& fname)
{
    open_file(fin, fname, true);
}

void BinReader::close()
{
    fin.close();
}

void BinReader::seek(size_t offset)
{
    fin.seekg(offset, std::ios::beg);
}

unsigned long long BinReader::file_size()
{
    unsigned long long pre_pos = fin.tellg();
    fin.seekg(0, std::ios::end);
    unsigned long long ret = fin.tellg();
    fin.seekg(pre_pos, std::ios::beg);
    return ret;
}

long long BinReader::read_int()
{
    unsigned long long ret = 0;
    unsigned char bytes[9] = {0};
    fin.read(reinterpret_cast<char*>(bytes), 1);
    if(bytes[0] & 0x40u)
    {
        short bytes_cnt = bytes[0] & 0x3Fu;
        fin.read(reinterpret_cast<char*>(bytes + 1), bytes_cnt);

        ret = bytes[ bytes_cnt-- ];
        while(bytes_cnt > 0)
        {
            ret = (ret << 8) | bytes[ bytes_cnt-- ];
        }
    }
    else    ret = bytes[0] & 0x3Fu;
    if(bytes[0] & 0x80u) return -static_cast<long long>(ret);
    return static_cast<long long>(ret);
}

void BinReader::read_int(char *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_int();
}
void BinReader::read_int(std::vector<char>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_int() );
}
void BinReader::read_int(short int *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_int();
}
void BinReader::read_int(std::vector<short int>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_int() );
}
void BinReader::read_int(int *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_int();
}
void BinReader::read_int(std::vector<int>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_int() );
}

void BinReader::read_int(long long *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_int();
}

void BinReader::read_int(std::vector<long long>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_int() );
}

char BinReader::read_int8()
{
    char ret;
    fin.read(&ret, 1);
    return ret;
}

void BinReader::read_int8(char *num, size_t n)
{
    fin.read(num, n);
}

void BinReader::read_int8(std::vector<char>& num, size_t n)
{
    char* tmp = new char[n];
    fin.read(tmp, n);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

short int BinReader::read_int16()
{
    short int ret;
    fin.read(reinterpret_cast<char*>(&ret), 2);
    return ret;
}

void BinReader::read_int16(short int *num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 1);
}

void BinReader::read_int16(std::vector<short int>& num, size_t n)
{
    short int* tmp = new short int[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 1);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

int BinReader::read_int32()
{
    int ret;
    fin.read(reinterpret_cast<char*>(&ret), 4);
    return ret;
}

void BinReader::read_int32(int* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 2);
}

void BinReader::read_int32(std::vector<int>& num, size_t n)
{
    int* tmp = new int[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 2);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

long long BinReader::read_int64()
{
    long long ret;
    fin.read(reinterpret_cast<char*>(&ret), 8);
    return ret;
}

void BinReader::read_int64(long long* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 3);
}

void BinReader::read_int64(std::vector<long long>& num, size_t n)
{
    long long* tmp = new long long[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 3);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

unsigned long long BinReader::read_uint()
{
    unsigned long long ret = 0;
    unsigned char bytes[9] = {0};
    fin.read(reinterpret_cast<char*>(bytes), 1);
    if(bytes[0] & 0x80u)
    {
        short bytes_cnt = bytes[0] & 0x7Fu;
        fin.read(reinterpret_cast<char*>(bytes + 1), bytes_cnt);

        ret = bytes[ bytes_cnt-- ];
        while(bytes_cnt > 0)
            ret = (ret << 8) | bytes[ bytes_cnt-- ];
        return ret;
    }
    else
        return static_cast<unsigned long long>( bytes[0] & 0x7Fu );
}

void BinReader::read_uint(unsigned char *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_uint();
}
void BinReader::read_uint(std::vector<unsigned char>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_uint() );
}
void BinReader::read_uint(unsigned short int *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_uint();
}
void BinReader::read_uint(std::vector<unsigned short int>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_uint() );
}
void BinReader::read_uint(unsigned int *num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_uint();
}
void BinReader::read_uint(std::vector<unsigned int>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_uint() );
}

void BinReader::read_uint(unsigned long long* num, size_t n)
{
    for(size_t i = 0; i < n; ++i)
        num[i] = read_uint();
}

void BinReader::read_uint(std::vector<unsigned long long>& num, size_t n)
{
    num.reserve( n );
    for(size_t i = 0; i < n; ++i)
        num.push_back( read_uint() );
}

unsigned char BinReader::read_uint8()
{
    unsigned char ret;
    fin.read(reinterpret_cast<char*>(&ret), 1);
    return ret;
}

void BinReader::read_uint8(unsigned char* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n);
}

void BinReader::read_uint8(std::vector<unsigned char>& num, size_t n)
{
    unsigned char* tmp = new unsigned char[n];
    fin.read(reinterpret_cast<char*>(tmp), n);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

unsigned short int BinReader::read_uint16()
{
    unsigned short int ret;
    fin.read(reinterpret_cast<char*>(&ret), 2);
    return ret;
}

void BinReader::read_uint16(unsigned short int* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 1);
}

void BinReader::read_uint16(std::vector<unsigned short int>& num, size_t n)
{
    unsigned short int* tmp = new unsigned short int[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 1);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

unsigned int BinReader::read_uint32()
{
    unsigned int ret;
    fin.read(reinterpret_cast<char*>(&ret), 4);
    return ret;
}

void BinReader::read_uint32(unsigned int* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 2);
}

void BinReader::read_uint32(std::vector<unsigned int>& num, size_t n)
{
    unsigned int* tmp = new unsigned int[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 2);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

unsigned long long BinReader::read_uint64()
{
    unsigned long long ret;
    fin.read(reinterpret_cast<char*>(&ret), 8);
    return ret;
}

void BinReader::read_uint64(unsigned long long* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 3);
}

void BinReader::read_uint64(std::vector<unsigned long long>& num, size_t n)
{
    unsigned long long* tmp = new unsigned long long[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 3);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

size_t BinReader::read_size_t()
{
    size_t ret;
    fin.read(reinterpret_cast<char*>(&ret), sizeof(size_t));
    return ret;
}

void BinReader::read_size_t(size_t* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n * sizeof(size_t));
}
void BinReader::read_size_t(std::vector<size_t>& num, size_t n)
{
    size_t* tmp = new size_t[n];
    fin.read(reinterpret_cast<char*>(tmp), n * sizeof(size_t));
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

float BinReader::read_float()
{
    float ret;
    fin.read(reinterpret_cast<char*>(&ret), 4);
    return ret;
}
void BinReader::read_float(float* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 2);
}
void BinReader::read_float(std::vector<float>& num, size_t n)
{
    float* tmp = new float[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 2);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

double BinReader::read_double()
{
    double ret;
    fin.read(reinterpret_cast<char*>(&ret), 8);
    return ret;
}
void BinReader::read_double(double* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n << 3);
}
void BinReader::read_double(std::vector<double>& num, size_t n)
{
    double* tmp = new double[n];
    fin.read(reinterpret_cast<char*>(tmp), n << 3);
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

long double BinReader::read_longdouble()
{
    long double ret;
    fin.read(reinterpret_cast<char*>(&ret), sizeof(long double));
    return ret;
}
void BinReader::read_longdouble(long double* num, size_t n)
{
    fin.read(reinterpret_cast<char*>(num), n * sizeof(long double));
}
void BinReader::read_longdouble(std::vector<long double>& num, size_t n)
{
    long double* tmp = new long double[n];
    fin.read(reinterpret_cast<char*>(tmp), n * sizeof(long double));
    num.assign(tmp, tmp + n);
    delete[] tmp;
}

void BinReader::read_string(std::string& str, size_t n)
{
    char* tmp = new char[n];
    fin.read(tmp, n);
    str.assign( tmp, n );
    delete[] tmp;
}

void BinReader::read_bytes(void* bytes, size_t n)
{
    fin.read(reinterpret_cast<char*>(bytes), n);
}

}// namespace loon
