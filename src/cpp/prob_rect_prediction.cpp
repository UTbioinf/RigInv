#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*========================== typedef and declarations ========================*/
typedef unsigned long long ULL;

/*========================== command line args =======================*/
char* infile;
char* outfile;
double p;   // probability of one rectangle containing the target point
double delta;   // confidence parameter (close to 0 is better)

/*========================== global variables =======================*/
vector<ULL> p_left, p_right, p_bottom, p_top;
size_t k;

void estimate_k()
{
    size_t m = p_left.size();
    vector<double> P;
    P.resize(m + 1);
    P[0] = 1;
    for(size_t i = 0; i < m; ++i)
        P[i + 1] = P[i] * p;
    double S = 0;
    double A = 1;
    double confidence = 1 - delta;
    for(k = 0; k <= m; ++k)
    {
        S += A * P[m - k];
        if(S >= confidence)
            return;
        A *= (m - k) * (1 - p) / (k + 1);
    }
}

void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    ULL l, r, b, t;
    while(fin >> l)
    {
        fin >> r >> b >> t;
        p_left.push_back(l);
        p_right.push_back(r);
        p_bottom.push_back(b);
        p_top.push_back(t);
    }
    fin.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("prob_rect_prediction <required parameters>");
    help.add_argument("Input file name of a set of rectangles");
    help.add_argument("Output file name of the predicted rectangle");
    help.add_argument("p: Lower bound of the probability of one rectangle containing the target point");
    help.add_argument("delta: confidence parameter (close to 0 is better)");

    help.check(argc, argv);
    
    infile = argv[1];
    outfile = argv[2];
    p = stod( argv[3] );
    delta = stod( argv[4] );
}


int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();
    estimate_k();

    size_t m = p_left.size();
    if(m > k)
    {
        nth_element(p_left.begin(), p_left.end() - k, p_left.end());
        nth_element(p_right.begin(), p_right.begin() + k - 1, p_right.end());
        nth_element(p_bottom.begin(), p_bottom.end() - k, p_bottom.end());
        nth_element(p_top.begin(), p_top.begin() + k - 1, p_top.end());

        ULL ll = p_left[ m - k];
        ULL rr = p_right[ k - 1];
        ULL bb = p_bottom[ m - k ];
        ULL tt = p_top[ k - 1 ];
        if(ll <= rr && bb <= tt)
        {
            ofstream fout;
            loon::open_file(fout, outfile);
            fout << ll << ' ' << rr << ' ' << bb << ' ' << tt << endl;
            fout.close();
        }
        else
        {
            cerr << "[WARNING]: Bad prediction: k = " << k << ", m = " << m << ", rect = (" 
                    << ll << ", " << rr << ", " << bb << ", " << tt << ")" << endl;
        }
    }
    else
    {
        cerr << "[WARNING]: k is too large. k = " << k << ", m = " << m << endl;
    }
    
    return 0;
}
