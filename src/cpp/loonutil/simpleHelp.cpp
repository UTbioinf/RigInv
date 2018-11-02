#include <iostream>
#include <cstdlib>
#include "simpleHelp.h"

namespace loon
{

void SimpleHelp::print_description()
{
    std::cerr << "Usage: ";
    std::string spaces(7, ' ');
    int string_len = 80 - 7;
    for(size_t i = 0; i < usage.length(); i += string_len)
    {
        if(i > 0)
            std::cerr << std::endl << spaces;
        std::cerr << usage.substr(i, string_len);
    }
    std::cerr << std::endl << std::endl;
}

int SimpleHelp::count_digits(size_t num)
{
    if(num == 0)    return 1;
    int ret = 0;
    while(num > 0)
    {
        num /= 10;
        ++ret;
    }
    return ret;
}

void SimpleHelp::print_index(size_t idx, int max_width)
{
    int n_spaces = max_width - count_digits(idx);
    std::cerr << "    ";
    while(n_spaces-- > 0)
        std::cerr << ' ';
    std::cerr << idx << ". ";
}

void SimpleHelp::print_parameter(const std::string& param, int max_width)
{
    int n_space = 6 + max_width;
    int string_len = 80 - n_space;
    std::string spaces(n_space, ' ');
    for(size_t i = 0; i < param.length(); i += string_len)
    {
        if(i > 0)
            std::cerr << std::endl << spaces;
        std::cerr << param.substr(i, string_len);
    }
    std::cerr << std::endl;
}

void SimpleHelp::print_usage(int exit_code)
{
    print_description();

    size_t n = parameters.size();
    if(n == 0)
    {
        std:: cerr << "No parameters are required!" << std::endl;
        exit( exit_code );
    }

    std::cerr << "Please provide the following parameters in order:" << std::endl;
    int max_width = count_digits( n + 1 );
    for(size_t i = 0; i < n; ++i)
    {
        print_index(i+1, max_width);
        print_parameter( parameters[i], max_width );
    }
    exit( exit_code );
}

SimpleHelp::SimpleHelp(const std::string& description/* = "" */):
        usage(description)
{}

void SimpleHelp::set_description(const std::string& description)
{
    usage = description;
}

void SimpleHelp::add_argument(const std::string& param)
{
    parameters.push_back(param);
}

void SimpleHelp::check(int argc, char* argv[])
{
    std::string help_option[] = {std::string("--help"),
            std::string("-help"),
            std::string("-h")};
    for(int i = 1; i < argc; ++i)
        for(int j = 0; j < 3; ++j)
            if(std::string(argv[i]) == help_option[j])
                print_usage(0);
    if(argc != parameters.size() + 1)
    {
        std::cerr << "ERROR: number of parameters doesn't match.\n       The required number of parameters is " << parameters.size() << std::endl << std::endl;
        print_usage(1);
    }
}

}// namespace loon
