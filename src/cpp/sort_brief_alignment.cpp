#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

#include <loonutil/simpleHelp.h>

using namespace std;

typedef unsigned long long ULL;

class BriefAln
{
public:
    ULL ref_start, ref_end;
    string remaining;
public:
    bool operator<(const BriefAln& rhs) const
    {
        if(ref_start < rhs.ref_start)   return true;
        if(ref_start > rhs.ref_start)   return false;
        return ref_end > rhs.ref_end;
    }
    friend ostream& operator<<(ostream& out, const BriefAln& obj);
    friend istream& operator>>(istream& in, BriefAln& obj);
};

ostream& operator<<(ostream& out, const BriefAln& obj)
{
    return out << obj.ref_start << ' ' << obj.ref_end << obj.remaining;
}

istream& operator>>(istream& in, BriefAln& obj)
{
    if(in >> obj.ref_start)
    {
        in >> obj.ref_end;
        getline(in, obj.remaining);
    }
    return in;
}

int main(int argc, char* argv[])
{
    loon::SimpleHelp help("sort_brief_alignment <required parameter>");

    help.add_argument("Input file of unsorted brief alignments");
    help.add_argument("Output file of sorted brief alignments");

    help.check(argc, argv);

    vector<BriefAln> alns;
    BriefAln tmp;
    ifstream fin(argv[1]);
    ofstream fout(argv[2]);
    while(fin >> tmp)
        alns.push_back( tmp );
    fin.close();

    sort(alns.begin(), alns.end());
    for(vector<BriefAln>::const_iterator it = alns.begin(); it != alns.end(); ++it)
        fout << (*it) << endl;
    fout.close();
    return 0;
}
