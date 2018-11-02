#ifndef __UTIL_GLOBAL_H
#define __UTIL_GLOBAL_H

/*!
 * \defgroup Global Global constants and variables
 * All the global constants and variables
 */


/*! \defgroup Func_util Function utilities
 * Commonly used function utilities
 */

/*!
 * \defgroup Class_util Class utilities
 * Commonly used class utilities
 */

/*! \defgroup Aux Auxiliary information print
 * A module for printing auxiliary information
 */


#include <cstdio>
#include <limits>
#include "logger.h"

#ifndef _IN
    #define _IN
#endif

#ifndef _OUT
    #define _OUT
#endif

namespace loon
{

/*! \defgroup g_Constants Global constants
 * \ingroup Global
 * List of all the global constants
 * @{
 */

/*! \brief global constant ZERO */
const static double ZERO = 1e-7;
/*! \brief global constant INF_size_t */
const static size_t INF_size_t = std::numeric_limits<size_t>::max();
#if defined(_WIN32) || defined(_WIN64)
const static char directory_delimiter = '\\';
#else
const static char directory_delimiter = '/';
#endif

/*! @} */ // group g_Constants

/*! \defgroup g_Variables Global variables
 * \ingroup Global
 * List of all the global constants
 * @{
 */

/*! \brief A global logger used anywhere
 * 
 * This global logger is used by all the other classes for debugging, reporting errors, etc.
 * If color output is preferred, put `global_logger.color_on()` at the first line of `main()`
 * function.
 */
extern Logger global_logger;

/*! @} */ // group g_Variables
}// namespace loon



#endif
