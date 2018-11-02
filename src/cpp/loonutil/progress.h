#ifndef __UTIL_PROGRESS_H
#define __UTIL_PROGRESS_H

#include <iostream>

namespace loon
{
/*!\ingroup Aux
 * \brief Show percentage of the progress
 * 
 * This utility can be used to show progress information, specifically, it will show the percentage
 * of the progress.
 *
 * \todo Refine the `back_cnt()` private member function.
 */
class Progress
{
private:
    double total;
    double precision;
    double cur;
    double accum_precision;
    int float_num;

    int back_cnt(double percent);
public:
    Progress();//!< Constructor
    /*!\brief Init the Progress object
     * \param [in] total Total number of discrete jobs to process.
     * \param [in] float_num Set the number of digits after the floating point, after adding "%"
     */
    void start(double total, int float_num = 0);
    /*!\brief Emit an progress.
     * \param [in] inc The number of discrete jobs that have been processed in the current step.
     */
    void progress(double inc = 1);
    /*!\brief Stop progress
     * \param [in] show100 If true, then "100%" will shown. Otherwise, the progressive information will be
     * erased.
     */
    void stop(bool show100 = false);
};

}// namespace loon

#endif
