#ifndef __LOONUTIL_SIMPLE_HELP_H
#define __LOONUTIL_SIMPLE_HELP_H

#include <vector>
#include <string>

namespace loon
{

class SimpleHelp
{
private:
    std::string usage;
    std::vector<std::string> parameters;

    void print_description();
    int count_digits(size_t num);
    void print_index(size_t idx, int max_width);
    void print_parameter(const std::string& param, int max_width);
    void print_usage(int exit_code);
public:
    SimpleHelp(const std::string& description = "");
    void set_description(const std::string& description);
    void add_argument(const std::string& param);
    void check(int argc, char* argv[]);
};

}//namespace loon

#endif
