#ifndef __UTIL_UTIL_H
#define __UTIL_UTIL_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "exception.h"
#include "global.h"

namespace loon
{
/*!\ingroup Func_util
 * @{
 */

/*! \brief Open file for read
 *
 * Open a file for read. This function will check whether the file is appropriately open. If not,
 * an error message will be printed by the loon::global_logger and the program will terminate.
 *
 * \param [in, out] fin The file stream that to be opened.
 * \param [in] fname The name of the file to be opened.
 * \param [in] is_binary If true, the file will be opened in binary mode.
 */
void open_file(std::ifstream& fin, const std::string& fname, bool is_binary = false);

/*! \brief Open file for write
 *
 * Open a file for write. This function will check whether the file is appropriately open. If not,
 * an error message will be printed by the loon::global_logger and the program will terminate.
 *
 * \param [in, out] fout The file stream that to be opened.
 * \param [in] fname The name of the file to be opened.
 * \param [in] is_binary If true, the file will be opened in binary mode.
 */
void open_file(std::ofstream& fout, const std::string& fname, bool is_binary = false);

/*! \brief Check whether the file is open appropriately.
 * 
 * Check whether the file for read is open appropriately. If not, an error message will be printed
 * by the loon::global_logger and the program will terminate.
 *
 * \param [in] fin The file stream that is to be checked.
 * \param [in] fname The name of the file that is to be checked. It's only used in the error message.
 */
void check_file_open(std::ifstream& fin, const std::string& fname);

/*! \brief Check whether the file is open appropriately.
 * 
 * Check whether the file for write is open appropriately. If not, an error message will be printed
 * by the loon::global_logger and the program will terminate.
 *
 * \param [in] fout The file stream that is to be checked.
 * \param [in] fname The name of the file that is to be checked. It's only used in the error message.
 */
void check_file_open(std::ofstream& fout, const std::string& fname);

/*! \brief Fast random bits generator
 *
 * Generate short random bits (<=31) in a faster fashion. This function is about 5 times faster than the system `rand()` function (tested on via clang on OS X with -O3 flag).
 *
 * \param [in] bits Number of bits to generate
 * \return The bits required to be generated
 */
int rand_bits(int bits);

/*! \brief Large job for test
 *
 * Run a large job. This job is to compute all the permutations of [`n`]. The following is the
 * approximate running time for different `n`
 *  * `n = 10`: 0.133496s
 *  * `n = 11`: 1.4579s
 *  * `n = 12`: 17.8554s
 *  * `n = 13`: 4m 1.66994s
 *
 * \param [in] n An integer `n` indicate the size of [`n`]
 * \return The number of permutations of [`n`]
 */
long long large_job(int n);
/*! @} */

void set_stdin_echo(bool enable = true);
bool file_exist(const std::string& fname);
bool check_help(int argc, char* argv[]);
std::string file_basename(const std::string& fname);
std::string get_directory(const std::string& path);

// every 5 bits in "num" will be used to build a character for the path, so digits should not be too large
// there will be "power(32, digits)" number of directory in one directory. So the recommended choices would be 1 or 2
std::string int2path(unsigned long long num, short int digits = 1);
bool is_dir_exist(const std::string& directory);
// recursively make directory. Like `mkdir -p` in linux/unix
bool mkdir_p(const std::string& directory);

}// namespace loon
#endif
