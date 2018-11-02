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
LL min_component;
LL max_side_length;
//double confidence;

/*========================= global variables ===========================*/
vector<Rect> rects;
vector<size_t> label_left;
LL component_id = 0;
ofstream fout_k;

/*========================= class Rect =======================*/
class Rect: public RectBase
{
public:
    char brace;
    LL dist;
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

bool x_smaller(const Rect& r1, const Rect& r2)
{
    return (r1[0] < r2[0] || (r1[0] == r2[0] && r1[1] < r2[1]));
}

bool y_smaller(const Rect& r1, const Rect& r2)
{
    return (r1[2] < r2[2] || (r1[2] == r2[2] && r1[3] < r2[3]));
}

void write_connected_component(const vector<Rect>::iterator& begin_it, const vector<Rect>::iterator& end_it)
{
    if(end_it - begin_it < min_component)  return;

    ofstream fout;
    string directory = outdir + loon::int2path(component_id);
    loon::mkdir_p(directory);
    string path = directory + to_string(component_id) + ".txt";
    fout_k << component_id << ' ' << directory << ' ' << (end_it - begin_it) << endl;
    loon::open_file(fout, path);

    for(vector<Rect>::iterator it = begin_it; it < end_it; ++it)
    {
        fout << (*it)[0] << ' '
             << (*it)[1] << ' '
             << (*it)[2] << ' '
             << (*it)[3] << ' '
             << (it->brace) << ' '
             << (it->dist)<< endl;
    }
    fout.close();
    ++component_id;
}

void find_connected_components(const vector<Rect>::iterator& bottom_it, const vector<Rect>::iterator& top_it)
{
    vector<Rect>::iterator smallest_it = bottom_it;
    vector<Rect>::iterator boundary_it = bottom_it + 1;

    for(vector<Rect>::iterator cur_it = bottom_it; cur_it != top_it; ++cur_it)
    {
        if(cur_it == boundary_it)
        {
            write_connected_component(smallest_it, boundary_it);
            smallest_it = boundary_it;
            ++boundary_it;
        }
        for(vector<Rect>::iterator cmp_it = boundary_it; cmp_it != top_it; ++cmp_it)
        {
            if(is_connected(*cur_it, *cmp_it))
            {
                swap(*boundary_it, *cmp_it);
                ++boundary_it;
            }
        }
    }
    write_connected_component(smallest_it, boundary_it);
}

void partition_by_y(const vector<Rect>::iterator& left_it, const vector<Rect>::iterator& right_it)
{
    vector<Rect>::iterator bottom_it = left_it, top_it = left_it;
    LL top_most = (*bottom_it)[3];
    for(; top_it != right_it; ++top_it)
    {
        if((*top_it)[2] <= top_most)
            top_most = max(top_most, (*top_it)[3]);
        else
        {
            if(top_it >= bottom_it + min_component)
                find_connected_components(bottom_it, top_it);
            bottom_it = top_it;
            top_most = (*bottom_it)[3];
        }
    }
    find_connected_components(bottom_it, top_it);
}

void do_partition()
{
    if(rects.empty() || rects.size() < min_component)   return;

    sort(rects.begin(), rects.end(), x_smaller);
    vector<Rect>::iterator left_it = rects.begin();
    vector<Rect>::iterator right_it = left_it + 1;
    LL right_most = (*left_it)[1];

    for(; right_it != rects.end(); ++right_it)
    {
        if((*right_it)[0] <= right_most)
            right_most = max(right_most, (*right_it)[1]);
        else
        {
            if(right_it >= left_it + min_component)
            {
                sort(left_it, right_it, y_smaller);
                partition_by_y(left_it, right_it);
            }
            left_it = right_it;
            right_most = (*left_it)[1];
        }
    }
    sort(left_it, right_it, y_smaller);
    partition_by_y(left_it, right_it);
}

void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    Rect tmp;

    string line;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3] >> tmp.brace >> tmp.dist;
        if(tmp[1] - tmp[0] > max_side_length || tmp[3] - tmp[2] > max_side_length)
            continue;
        rects.push_back( tmp );
    }
    fin.close();
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
    min_component = stoll( argv[3] );
    max_side_length = stoll( argv[4] );
    if(max_side_length == 0)    max_side_length = numeric_limits<LL>::max();
    min_component = max(1LL, min_component);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();
    loon::open_file(fout_k, outdir + "spec.txt");
    do_partition();
    fout_k.close();
    return 0;
}
