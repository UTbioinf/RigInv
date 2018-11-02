#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <limits>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*========================== typedef and declarations =====================*/
//typedef unsigned long long ULL;
typedef long long LL;
typedef array<LL, 4> RectBase;
class Rect;

/*========================= command line args ==========================*/
char* infile;
string outdir;
LL max_side_length;
//double confidence;

/*========================= global variables ===========================*/
vector<Rect> rects;
vector<size_t> label_left;
LL min_component;

/*========================= class Rect =======================*/
class Rect: public RectBase
{
public:
    char brace;
};

/*========================= functions =======================*/
bool is_connected(const Rect& r1, const Rect& r2)
{
    if(r1[1] < r2[0])   return false;
    if(r2[1] < r1[0])   return false;
    if(r1[3] < r2[2])   return false;
    if(r2[3] < r1[2])   return false;
    return true;
}

void do_partition()
{
    size_t n = rects.size();
    label_left.push_back( n );
    for(size_t i = n; i > 0;)
    {
        --i;
        if(i < label_left.back())
            label_left.push_back(i);
        for(size_t j = label_left.back(); j > 0;)
        {
            --j;
            if(is_connected(rects[j], rects[i]))
            {
                --label_left.back();
                if(j < label_left.back())
                    swap( rects[j], rects[ label_left.back() ]);
            }
        }
    }
}

void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    Rect tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3] >> tmp.brace;
        if(tmp[1] - tmp[0] > max_side_length || tmp[3] - tmp[2] > max_side_length)
            continue;
        rects.push_back( tmp );
    }
    fin.close();
}

/*
void confidence_analysis()
{
    if(label_left.empty())  return;
    size_t K = label_left.size() - 1;

    vector<size_t> counts;
    counts.reserve( K );
    for(size_t k = K; k > 0;)
    {
        --k;
        counts.push_back( label_left[k] - label_left[k + 1] );
    }
    sort(counts.begin(), counts.end());

    double margin = (1 - confidence ) * rects.size();
    size_t s = 0;
    for(size_t i = 0; i < counts.size(); ++i)
    {
        s += counts[i];
        if(s > margin)
        {
            min_component = counts[ i ];
            return;
        }
    }
}
*/

void write_rectangles()
{
    ofstream fout_k;
    loon::open_file(fout_k, outdir + "spec.txt");
    if(label_left.empty())
    {
        return;
    }
    size_t K = label_left.size() - 1;
    for(size_t k = 0; k < K; ++k)
    {
        if(label_left[k] - label_left[k+1] < min_component)
            continue;
        ofstream fout;
        string directory = outdir + loon::int2path(k);
        loon::mkdir_p(directory);
        string path = directory + to_string(k) + ".txt";
        fout_k << k << ' ' << directory << ' '<< (label_left[k] - label_left[k+1]) << endl;

        loon::open_file(fout, path);
        for(size_t j = label_left[k+1]; j < label_left[k]; ++j)
            fout << rects[j][0] << ' ' << rects[j][1] << ' ' << rects[j][2] << ' ' << rects[j][3] << ' ' << rects[j].brace<< endl;
        fout.close();
    }
    fout_k.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("partition_disconnected_rects <required parameters>");
    help.add_argument("Input file of a set of rectangles");
    help.add_argument("Output directory");
    help.add_argument("Keep only the connected component that has at least this number of rectangles");
    help.add_argument("Keep only the rectangles whose side lengths are smaller than this value (set as 0 if one wants to keep all the rectangles)");

    help.check(argc, argv);
    infile  = argv[1];
    outdir  = string(argv[2]) + loon::directory_delimiter;
    //confidence = stod( argv[3] );
    min_component = stoll( argv[3] );
    max_side_length = stoll( argv[4] );
    if(max_side_length == 0)    max_side_length = numeric_limits<LL>::max();
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();
    do_partition();
    //confidence_analysis();
    write_rectangles();
    return 0;
}
