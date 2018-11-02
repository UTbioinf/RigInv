#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*=============== typedef and declarations ==================*/
typedef unsigned long long ULL;
typedef array<ULL, 2> IntPoint;
typedef array<double, 2> DoublePoint;

const double ZERO = 1e-7;


/*=============== command line args ================*/
char* infile;
char* outfile;
double epsilon;
double delta;


class SquareBracket
{
public:
    double p, top, bottom;
    bool is_left;
public:
    SquareBracket(double pp = 0, double t = 0, double b = 0, bool is_L = false):
            p(pp), top(t), bottom(b), is_left(is_L)
    {}

    bool operator<(const SquareBracket& rhs) const
    {
        if(p < rhs.p - ZERO) return true;
        else if(p > rhs.p + ZERO) return false;
        if(is_left)
        {
            if(rhs.is_left)
                return top < rhs.top;
            else
                return true;
        }
        else
        {
            if(rhs.is_left) return false;
            else    return top < rhs.top;
        }
    }
    
};

/*============= global variables ================*/
vector<IntPoint> horizontals, verticals;

/*=============== functions ===============*/

inline DoublePoint rotate_point(ULL x, ULL y)
{
    // left     -> (left, bottom)
    // right    -> (right, top)
    // top      -> (left, top)
    // bottom   -> (right, bottom)
    const static double sqrt2 = sqrt(2) / 2;
    DoublePoint t;
    t[0] = sqrt2 * (x - y);
    t[1] = sqrt2 * (x + y);
    return t;
}

bool predict_segment(const vector<IntPoint>& points, IntPoint& res)
{
    size_t min_n = static_cast<size_t>( ceil( (1 - delta) * points.size() ) );
    size_t n = points.size();
    vector<SquareBracket> sq_brackets;
    sq_brackets.reserve( n << 1 );
    // rotate each squares and convert to left/right brackets
    for(size_t i = 0; i < n; ++i)
    {
        DoublePoint left_bottom = rotate_point( points[i][0] - epsilon, points[i][1] );
        DoublePoint right_top = rotate_point( points[i][0] + epsilon, points[i][1] );

        sq_brackets.push_back( SquareBracket(left_bottom[0], right_top[1], left_bottom[1], true) );
        sq_brackets.push_back( SquareBracket(right_top[0], right_top[1], left_bottom[1], false) );
    }
    
    sort(sq_brackets.begin(), sq_brackets.end());

    // TODO
    // do plane sweeping on the left/right brackets
    // using interval-tree

}

void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    // input format: left, right, top, bottom

    IntPoint tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1];
        --tmp[1];
        horizontals.push_back( tmp );

        fin >> tmp[0] >> tmp[1];
        --tmp[1];
        verticals.push_back( tmp );
    }

    fin.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("segment_prediction <required parameters>");

    help.add_argument("Input file of the set of rectangles");
    help.add_argument("Output file of the predicted rectangle");
    help.add_argument("epsilon");
    help.add_argument("delta");

    help.check( argc, argv );

    infile      = argv[1];
    outfile     = argv[2];
    epsilon     = stod(argv[3]);
    delta       = stod(argv[4]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();

    IntPoint h, v;
    if(! predict_segment( horizontals, h ) )
        return 0;
    if(! predict_segment( verticals, v ) )
        return 0;
    
    ofstream fout;
    loon::open_file(fout, outfile);
    fout << h[0] << ' ' << h[1] << ' ' << w[0] << ' ' << w[1] << endl;
    fout.close();
    return 0;
}
