#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <direct.h>  // _mkdir
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <errno.h>
#endif

#include "util.h"

namespace loon
{

void open_file(std::ifstream& fin, const std::string& fname, bool is_binary/* = false */)
{
    if(is_binary)   fin.open(fname.c_str(), std::ifstream::binary);
    else    fin.open(fname.c_str());
    check_file_open(fin, fname);
}

void open_file(std::ofstream& fout, const std::string& fname, bool is_binary/* = false */)
{
    if(is_binary)   fout.open(fname.c_str(), std::ofstream::binary);
    else    fout.open(fname.c_str());
    check_file_open(fout, fname);
}

void check_file_open(std::ifstream& fin, const std::string& fname)
{
    if(!fin.is_open())
        throw Exception(1, "Cannot open file [%s]", fname.c_str());
}

void check_file_open(std::ofstream& fout, const std::string& fname)
{
    if(!fout.is_open())
        throw Exception(2, "Cannot open file [%s]", fname.c_str());
}

void Permutation(bool* a, int n, int k, long long& cnt)
{
    if(k == n)
    {
        ++cnt;
        return;
    }
    for(int i = 0; i < n; ++i)
        if(!a[i])
        {
            a[i] = true;
            Permutation(a, n, k + 1, cnt);
            a[i] = false;
        }
}

int rand_bits(int bits)
{
    static uint64_t rand_store = 0;
    int extract_bits = (1 << bits) - 1;
    uint64_t cur_rand = rand_store;
    if(int(cur_rand) < extract_bits)
        cur_rand = uint64_t(RAND_MAX) | (uint64_t(rand()) << 32);
    rand_store = cur_rand >> bits;
    return (extract_bits & int(cur_rand) & int(cur_rand >> 32));
}

long long large_job(int n)
{
    bool* a = new bool[n];
    memset(a, false, sizeof(bool) * n);
    long long cnt = 0;
    Permutation(a, n, 0, cnt);
    delete[] a;
    return cnt;
}

void set_stdin_echo(bool enable/* = true*/)
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );
#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

bool file_exist(const std::string& fname)
{
    std::ifstream ifile( fname.c_str() );
    return (bool)ifile;
}

bool check_help(int argc, char* argv[])
{
    if(argc == 1)   return true;
    std::string help_option[] = {std::string("--help"), std::string("-h"), std::string("-help")};

    for(int i = 1; i < argc; ++i)
        for(int j = 0; j < 3; ++j)
            if(std::string(argv[i]) == help_option[j])
                return true;
    return false;
}

std::string file_basename(const std::string& fname)
{
    size_t slash_pos = fname.find_last_of(directory_delimiter);
    if(slash_pos == std::string::npos)  slash_pos = 0;
    else    ++slash_pos;
    if(slash_pos == fname.length())
        throw Exception(7, "Missing file basename in path [%s]", fname.c_str());
    return fname.substr( slash_pos );
}

std::string get_directory(const std::string& path)
{
    size_t slash_pos = path.find_last_of(directory_delimiter);
    return path.substr(0, slash_pos);
}

std::string int2path(unsigned long long num, short int digits/* = 1 */)
{
    const unsigned long long shift_digits = digits * 5;
    const unsigned long long mask = (1 << shift_digits) - 1;
    const static char digit_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz-_";
    std::string ret;
    do
    {
        ret.push_back(directory_delimiter);
        unsigned long long tmp_digits = num & mask;
        for(short int i = 0; i < digits; ++i)
        {
            ret.push_back( digit_char[ tmp_digits & 31 ] );
            tmp_digits >>= 5;
        }
        num >>= shift_digits;
    }while(num > 0);
    std::reverse(ret.begin(), ret.end());
    return ret;
}

bool is_dir_exist(const std::string& directory)
{
#if defined(_WIN32) || defined(_WIN64)
    struct _stat info;
    if(_stat(directory.c_str(), &info) != 0)
        return false;
    return (info.st_mode & _S_IFDIR) != 0
#else
    struct stat info;
    if(stat(directory.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

bool mkdir_p(const std::string& directory)
{
#if defined(_WIN32) || defined(_WIN64)
    int ret = _mkdir(directory.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(directory.c_str(), mode);
#endif
    if(ret == 0)    return true;

    switch(errno)
    {
        case ENOENT: // parent didn't exist , try to create it
            {
                int pos = directory.find_last_of(directory_delimiter);
                if(pos == std::string::npos)
                    return false;
                if(not mkdir_p( directory.substr(0, pos) ))
                    return false;
            }
            // now try to create again
        #if defined(_WIN32) || defined(_WIN64)
            return 0 == _mkdir(directory.c_str());
        #else
            return 0 == mkdir(directory.c_str(), mode);
        #endif
        case EEXIST: // done
            return is_dir_exist(directory);
        default:
            return false;
    }
}

}// namespace loon
