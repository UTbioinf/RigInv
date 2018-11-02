#ifndef __UTIL_TIMER_H
#define __UTIL_TIMER_H

#include <iostream>
#include <cstdint>

namespace loon
{

/*!\ingroup Aux
 * \brief Cross platform timer, correct result for multi-threading
 *
 * This timer is better than simply calling `clock()` because `clock()` counts
 * the ticks of the CPU. If the program is multi-threaded, then the ticks of
 * CPU is a few times larger because it has higher utilization. This is the
 * reason why we need this Timer class. This Timer class also deal with, to
 * some degree, the network time calibration. So the accuracy can be guaranteed.
 * In some systems, the precision can be nanoseconds. Although the least unit
 * is seconds in the output, the corresponding precision is kept.
 *
 * \todo Test on linux and windows
 */
class Timer
{
private:
    uint64_t rate; // counts per second
    uint64_t last;  // last observation on the counter
    short min_level; // set level for output
private:
    /*!\brief Get the current time*/
    uint64_t get_now() const;
public:
    /*!\brief Constructor
     *
     * Set minimum time unit and init timer.
     *
     * \param [in] precision Set the minimum time unit. See set_precision() for details.
     */
    Timer(char precision = 's');

    /*!\brief Set minimum time unit
     *
     * The time unit can be set as follows:
     *  * 's' or 'S': seconds
     *  * 'm' or 'M': minute
     *  * 'h' or 'H': hour
     *  * 'd' or 'D': day
     */
    void set_precision(char precision = 's');

    void reset();//!< init timer
    double get_duration() const;//!< get elapsed time (in secs)
    void print_duration() const;//!< print elapsed time with the format &nbsp;&nbsp;`Time elapsed: <time>`
    friend std::ostream& operator<<(std::ostream& oss, const Timer& timer);//!< the stream to print elapsed time, only the time!!!
};

}// namespace loon

#endif
