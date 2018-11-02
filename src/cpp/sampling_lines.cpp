#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*======================== typedef and declarations ==========================*/
/*======================== command line args ==========================*/
char* infile;
char* outfile;
int n_lines;
double p;
int seed_add; // this is used to avoid generating the same sequence of random numbers when running in parallel

/*======================== global variables ==========================*/
vector<string> a;

void sample_nlines()
{
    // read file
    ifstream fin;
    loon::open_file(fin, infile);
    string line;
    while(getline(fin, line))
        a.push_back(line);
    fin.close();

    if(a.size() > n_lines)
    {
        shuffle(a.begin(), a.end(), default_random_engine(chrono::system_clock::now().time_since_epoch().count() + seed_add));
        a.resize(n_lines);
    }

    ofstream fout;
    loon::open_file(fout, outfile);
    for(vector<string>::const_iterator it = a.cbegin(); it != a.cend(); ++it)
        fout << (*it) << endl;
    fout.close();
}

void sample_withp()
{
    srand(time(NULL) + seed_add);
    ifstream fin;
    loon::open_file(fin, infile);
    ofstream fout;
    loon::open_file(fout, outfile);

    string line;
    while(getline(fin, line))
    {
        if(rand() <= p * RAND_MAX)
            fout << line << endl;
    }

    fin.close();
    fout.close();
}


void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("sampling_lines <required parameters>");
    help.add_argument("Input file name");
    help.add_argument("Output file name");
    help.add_argument("Sample this number of lines (unless = 0)");
    help.add_argument("Sample with this probability (when n_lines = 0)");
    help.add_argument("An integer added to the seed");
    help.check(argc, argv);

    infile      = argv[1];
    outfile     = argv[2];
    n_lines     = stoi(argv[3]);
    p           = stod(argv[4]);
    seed_add    = stoi(argv[5]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    if(n_lines > 0) sample_nlines();
    else            sample_withp();
    return 0;
}
