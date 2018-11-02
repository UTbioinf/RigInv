#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include "timer.h"
#include "global.h"

namespace loon
{

#if !defined(__APPLE__) && !defined(_MSC_VER)
    #define UTIL_TIMER_POSIX_TIMER
    #include <ctime>
    #ifdef CLOCK_MONOTONIC
        #define UTIL_TIMER_CLOCKID CLOCK_MONOTONIC
    #else
        #define UTIL_TIMER_CLOCKID CLOCK_REALTIME
    #endif
#endif

#ifdef __APPLE__
    #define UTIL_TIMER_MACH_TIMER
    #include <mach/mach_time.h>
#endif

#ifdef _MSC_VER
    #define UTIL_TIMER_WIN32_TIMER
    #include <windows.h>
    #include <Strsafe.h>
    #define Guarded_Assert_WinErr(expression) \
        if(!(expression)) \
        { \
            ReportLastWindowsError(); \
            logger_print(global_logger.error, "Windows call failed: %s", #expression);\
        }

    void ReportLastWindowsError(void)
    {
        //EnterCriticalSection( _get_reporting_critical_section());
        {// Retrieve the system error message for the last error code
            LPVOID lpMsgBuf;
            LPVOID lpDisplayBuf;
            DWORD dw = GetLastError(); 

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

            // Display the error message and exit the process

            lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
                (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
            StringCchPrintf((LPTSTR)lpDisplayBuf, 
                LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                TEXT("Failed with error %d: %s"), 
                dw, lpMsgBuf); 
            
            // spam formated string to listeners
            { 
              fprintf(stderr,"%s",lpDisplayBuf);
            }

            LocalFree(lpMsgBuf);
            LocalFree(lpDisplayBuf);
        }
        // LeaveCriticalSection( _get_reporting_critical_section());
    }
#endif

uint64_t Timer::get_now() const
{
    uint64_t now;

#ifdef UTIL_TIMER_POSIX_TIMER
    struct timespec t;
    clock_gettime( UTIL_TIMER_CLOCKID, &t);
    now = t.tv_sec * 1000000000LL+t.tv_nsec;
#endif
#ifdef UTIL_TIMER_MACH_TIMER
    now = mach_absolute_time();
#endif
#ifdef UTIL_TIMER_WIN32_TIMER
    {
        LARGE_INTEGER tmp;
        Guarded_Assert_WinErr( QueryPerformanceCounter( &tmp ) );
        now = tmp.QuadPart;
    }
#endif
    return now;
}

Timer::Timer(char precision/* = 's' */): last(0), rate(0)
{
    set_precision(precision);

    if(rate == 0)
    {
    #ifdef UTIL_TIMER_POSIX_TIMER
        struct timespec r;
        clock_getres(UTIL_TIMER_CLOCKID, &r);
        rate = 1000000000LL*r.tv_nsec; // in Hz
    #endif
    #ifdef UTIL_TIMER_MACH_TIMER
        mach_timebase_info_data_t rate_nsec;
        mach_timebase_info( &rate_nsec );
        rate = 1000000000LL * rate_nsec.numer / rate_nsec.denom;
    #endif
    #ifdef UTIL_TIMER_WIN32_TIMER
        {
            LARGE_INTEGER tmp;
            Guarded_Assert_WinErr( QueryPerformanceFrequency(&tmp) );
            rate = tmp.QuadPart;
        }
    #endif
    }
    reset();
}

void Timer::set_precision(char precision/* = 's'*/)
{
    min_level = 0; // second level
    if(precision == 's' || precision == 'S')    min_level = 0; // second level
    else if(precision == 'm' || precision == 'M')   min_level = 1; // minute level
    else if(precision == 'h' || precision == 'H')   min_level = 2; // hour level
    else if(precision == 'd' || precision == 'D')   min_level = 3; // day level
    else
        global_logger.warning("Unknown Timer precision [%c], use seconds instead", precision);
}

void Timer::reset()
{
    last = get_now();
}

double Timer::get_duration() const
{
    return (get_now() - last) / double(rate);
}

void Timer::print_duration() const
{
    std::cerr << "Time elapsed: " << (*this) << std::endl;
}

std::ostream& operator<<(std::ostream& oss, const Timer& timer)
{
    double delta = (timer.get_now() - timer.last)/double(timer.rate);
    uint64_t delta_int = uint64_t( delta );

    int largest_print = 0;
    double secs = delta_int % 60 + delta - delta_int;

    delta_int /= 60; // minute

    int print_level = 0;
    if(timer.min_level == 3 || (timer.min_level < 3 && delta_int > 1440))
    {
        if(timer.min_level == 3 && delta_int % 1440 >= 720)
            oss << (delta_int / 1440) + 1 << "d";
        else
            oss << (delta_int / 1440) << "d";
        delta_int %= 1440;
        print_level |= 8;
    }
    if(timer.min_level == 2 || (timer.min_level < 2 && (delta_int > 60 || print_level & 8)))
    {
        if(timer.min_level == 2 && delta_int % 60 >= 30)
            oss << (delta_int / 60) + 1 << "h";
        else
            oss << (delta_int / 60) << "h";
        delta_int %= 60;
        print_level |= 4;
    }
    if(timer.min_level == 1 || (timer.min_level < 1 && (delta_int > 0 || print_level & 0xc)))
    {
        if(timer.min_level == 1 && secs >= 30)
            oss << delta_int + 1 << "m";
        else
            oss << delta_int << "m";
        print_level |= 2;
    }
    if(timer.min_level == 0)
    {
        std::streamsize p = oss.precision();
        oss.precision( 9 );
        oss << secs << "s";
        oss.precision( p );
    }
    return oss;
}

}// namespace loon
