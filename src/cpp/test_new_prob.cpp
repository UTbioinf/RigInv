#include <iostream>
#include <string>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>
#include <alglib/cpp/src/specialfunctions.h>

using namespace std;

long long n, k;
double p, delta;

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("test_new_prob n p delta");
    help.add_argument("n");
    help.add_argument("p");
    help.add_argument("delta");
    help.check(argc, argv);

    n = stoll( argv[1] );
    p = stod( argv[2] );
    delta = stod(argv[3]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    cout << "format:  k:  2F(n, (1-p)/2, k)   F(n, 1-p, k)" << endl;
    for(long long k = 0; k < n; ++k)
    {
        double p1 = alglib::binomialcdistribution(k - 1, n, (1 - p)/2) * 2;
        double p2 = alglib::binomialcdistribution(k - 1, n, (1-p));
        cout << k << ":  " << p1 << "  " << p2 << endl;
        if(p1 <= delta && p2 <= delta)
            break;
    }
    return 0;
}
