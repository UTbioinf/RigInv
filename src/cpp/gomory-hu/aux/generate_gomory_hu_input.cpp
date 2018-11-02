#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <array>

#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*===================== typedef and declaractions =======================*/
typedef unsigned long long ULL;
typedef array<ULL, 4> Rect;
typedef array<int, 2> Edge;

/*===================== command line args =======================*/
char* infile;
char* outfile;

/*===================== global variables ========================*/
vector<Rect> rects;
vector<Edge> edges;

/*===================== functions ====================*/
void read_rectangles()
{
    ifstream fin;
    loon::open_file(fin, infile);
    Rect tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3];
        rects.push_back( tmp );
    }
    fin.close();
}

bool is_connected(const Rect& r1, const Rect& r2)
{
    if(r1[1] < r2[0])   return false;
    if(r2[1] < r1[0])   return false;
    if(r1[3] < r2[2])   return false;
    if(r2[3] < r1[2])   return false;
    return true;    
}

void do_conversion()
{
    int n = rects.size();
    for(int i = 0; i < n; ++i)
        for(int j = i + 1; j < n; ++j)
            if(is_connected( rects[i], rects[j] ))
                edges.push_back( {i, j} );
    int m = edges.size();

    ofstream fout;
    loon::open_file(fout, outfile);
    fout << "p cut " << n << ' ' << edges.size() << endl;
    for(int e = 0; e < m; ++e)
        fout << "a " << edges[e][0] << ' ' << edges[e][1] << " 1" << endl;
    fout.close();
}


void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("generate_gomory_hu_input <required parameters>");
    help.add_argument("Input file name");
    help.add_argument("Output file name");
    help.check(argc, argv);

    infile  = argv[1];
    outfile = argv[2];
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();
    do_conversion();
    return 0;
}
