#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*==================== typedef and declarations ====================*/
typedef unsigned long long ULL;
typedef array<ULL, 4> Rect;

const double ZERO = 1e-7;

/*==================== command line args ====================*/
char* infile;
char* outfile;
double epsilon;
double delta;

/*==================== global variables ====================*/
vector<Rect> rects;
Rect objective_boundary;

/*==================== functions ====================*/

bool check_symmetric_difference(const Rect& rect, ULL l, ULL r, ULL t, ULL b)
{
    ULL area1 = (r - l) * (t - b);
    ULL area2 = (rect[1] - rect[0]) * (rect[3] - rect[2]);
    ULL ll = std::max(l, rect[0]);
    ULL rr = std::min(r, rect[1]);
    ULL bb = std::max(b, rect[2]);
    ULL tt = std::min(t, rect[3]);
    ULL int_area = 0;
    if(ll < rr && bb < tt)
        int_area = (rr - ll) * (tt - bb);
    ULL final_area = area1 + area2 - (int_area << 1);
    return final_area <= epsilon;
}

Rect predict_rect()
{
    Rect res;
    size_t cnt = static_cast<size_t>(ceil( (1 - delta) * rects.size() )) - 1;
    bool found_one = false;
    for(ULL l = objective_boundary[0]; l <= objective_boundary[1]; ++l)
    {
        for(ULL r = l + 1; r <= objective_boundary[1]; ++r)
        { // no degenerated case because of the property of the input data!!!
            for(ULL b = objective_boundary[2]; b <= objective_boundary[3]; ++b)
            {
                for(ULL t = b + 1; t <= objective_boundary[3]; ++t)
                {
                    size_t tmp_cnt = 0;
                    for(size_t i = 0; i < rects.size(); ++i)
                    {
                        if(check_symmetric_difference(rects[i], l, r, t, b))
                            ++tmp_cnt;
                    }
                    if(tmp_cnt > cnt)
                    {
                        res[0] = l; res[1] = r; res[2] = b; res[3] = t;
                        cnt = tmp_cnt;
                        found_one = true;
                    }
                }
            }
        }
    }
    if(found_one)   return res;
    exit(0);
}

void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    // input format: left - 0, right - 1, bottom - 2, top - 3
    Rect tmp;
    double min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3];
        rects.push_back(tmp);
        double t_width = epsilon / (tmp[3] - tmp[2]);
        double t_height = epsilon / (tmp[1] - tmp[0]);

        if(rects.size() == 1)
        {
            min_x = std::max(0., tmp[0] - t_width);
            max_x = tmp[1] + t_width;
            min_y = std::max(0., tmp[2] - t_height);
            max_y = tmp[3] + t_height;
        }
        else
        {
            min_x = std::min( min_x, std::max(0., tmp[0] - t_width) );
            max_x = std::max( max_x, tmp[1] + t_width );
            min_y = std::min( min_y, std::max(0., tmp[2] - t_height) );
            max_y = std::max( max_y, tmp[3] + t_height );
        }
    }
    if(rects.empty())   exit(0);
    objective_boundary[0] = static_cast<ULL>(ceil( min_x ));
    objective_boundary[1] = static_cast<ULL>(floor( max_x ));
    objective_boundary[2] = static_cast<ULL>(ceil( min_y ));
    objective_boundary[3] = static_cast<ULL>(floor( max_y ));
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("brute_force_rect_prediction <required parameters>");

    help.add_argument("Input file of the set of rectangles");
    help.add_argument("Output file of the set of rectangle");
    help.add_argument("epsilon");
    help.add_argument("delta");

    help.check(argc, argv);

    infile      = argv[1];
    outfile     = argv[2];
    epsilon     = stod(argv[3]);
    delta       = stod(argv[4]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();

    Rect r = predict_rect();

    ofstream fout;
    loon::open_file(fout, outfile);
    fout << r[0] << ' ' << r[1] << ' ' << r[2] << ' ' << r[3] << endl;
    fout.close();
    return 0;
}
