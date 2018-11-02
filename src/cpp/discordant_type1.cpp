#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*=========== typedef and declarations =====*/
typedef unsigned long long ULL;
class OneAln;

/*========== command line args ============*/
char* infile;
char* outfile;
ULL delta1 = 0;
double ptg1 = 0;
ULL min_MAPQ = 0;
ULL allowed_overlap = 5;

/*========== global variables ============*/
vector<vector<OneAln> > alns[2]; // 0 for forward, and 1 for backward
vector<ULL> qry_lens;            // query length
map<string, size_t> alns_ids;    // TODO: use Trie

/*========== debug variables =============*/

/*========== class OneAln ================*/

class OneAln
{
public:
    ULL ref_start, ref_end;
    ULL qry_start, qry_end;
};

/*========== functions ====================*/

void save_inversion(ULL x1, ULL x2, ULL y1, ULL y2,
        const vector<OneAln>& backward, size_t b,
        ofstream& fout, ULL len)
{
    ULL min_x = x2 + allowed_overlap, max_x = 0;
    ULL last_x = x1, last_y = y1;
    ULL total_cvg = 0;
    for(; b < backward.size(); ++b)
    {// TODO: a better analysis for this part should be a Longest Increasing Sequence
        if(backward[b].ref_start > x2 + allowed_overlap)    break;
        if(backward[b].ref_end >= allowed_overlap + x2) continue;

        if(len-backward[b].qry_end <= y2 + allowed_overlap && len - backward[b].qry_start + allowed_overlap >= y1)
        {
            min_x = min(min_x, backward[b].ref_start);
            max_x = max(max_x, backward[b].ref_end);

            if(last_x < backward[b].ref_start)
            {
                last_x = backward[b].ref_end;
                total_cvg += backward[b].ref_end - backward[b].ref_start;
            }
            else if(last_x < backward[b].ref_end)
            {
                total_cvg += backward[b].ref_end - last_x;
                last_x = backward[b].ref_end;
            }

            if(last_y < backward[b].qry_start)
            {
                last_y = backward[b].qry_end;
                total_cvg += backward[b].qry_end - backward[b].qry_start;
            }
            else if(last_y < backward[b].qry_end)
            {
                total_cvg += backward[b].qry_end - last_y;
                last_y = backward[b].qry_end;
            }
        }
    }
    if(total_cvg == 0)  return;
    if(total_cvg >= ((x2 - x1) + (y2 - y1)) * ptg1)
    {
        if(x1 > min_x)  swap(x1, min_x);
        if(max_x < x2)  swap(max_x, x2);
        fout << x1 << ' ' << min_x << ' ' << x2 << ' ' << max_x << endl;
    }
}

size_t bi_move_right(ULL x1, const vector<OneAln>& backward, size_t bi)
{
    for(; bi < backward.size(); ++bi)
    {
        if(backward[bi].ref_start + allowed_overlap >= x1)
            return bi;
    }
    return bi;
}

bool abs_smallerthan_delta1(ULL x1, ULL x2, ULL y1, ULL y2)
{
    if(x1 >= x2)    return false;
    if(y1 >= y2)    return false;
    ULL dx = x2 - x1;
    ULL dy = y2 - y1;
    if(dx < dy)     return (dy - dx <= delta1);
    return (dx - dy <= delta1);
}

void type1_inversion(const vector<OneAln>& forward, const vector<OneAln>& backward, ofstream& fout, ULL len)
{
    size_t n = forward.size();
    size_t backward_i = 0;
    for(size_t i = 0; i < n; ++i)
    {
        backward_i = bi_move_right(forward[i].ref_end, backward, backward_i);
        if(backward_i >= backward.size())    return;

        for(size_t j = i + 1; j < n; ++j)
        {
            if(abs_smallerthan_delta1( forward[i].ref_end, forward[j].ref_start,
                    forward[i].qry_end, forward[j].qry_end))
            {
                save_inversion(forward[i].ref_end, forward[j].ref_start,
                        forward[i].qry_end, forward[j].qry_start,
                        backward, backward_i,
                        fout, len);
            }
        }
    }
}

void do_analysis()
{
    ofstream fout;
    loon::open_file(fout, outfile);
    
    size_t n = alns[0].size();
    for(size_t i = 0; i < n; ++i)
        if(alns[0][i].size() > 0 && alns[1][i].size() > 0)
        {
            type1_inversion(alns[0][i], alns[1][i], fout, qry_lens[i]);
            type1_inversion(alns[1][i], alns[0][i], fout, qry_lens[i]);
        }

    fout.close();
}

void read_alignments()
{
    ifstream fin;
    loon::open_file(fin, infile);

    OneAln tmp;
    ULL mapping_quality, qry_len;
    string name;
    string line;
    char orientation;
    map<string, size_t>::iterator it;
    while(fin >> tmp.ref_start)
    {
        fin >> tmp.ref_end >> tmp.qry_start >> tmp.qry_end
            >> name >> line >> qry_len >> mapping_quality >> orientation;
        if(mapping_quality < min_MAPQ)
            continue;
        it = alns_ids.find( name );

        if(it == alns_ids.end())
        {
            alns_ids[ name ] = alns[0].size();
            alns[0].push_back( vector<OneAln>() );
            alns[1].push_back( vector<OneAln>() );
            qry_lens.push_back( qry_len );
            
            alns[ orientation == 'R' ].back().push_back( tmp );
        }
        else
        {
            alns[ orientation == 'R' ][ it->second ].push_back( tmp );
        }
    }
    fin.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("discordant_type1 <required parameters>");

    help.add_argument("Input file of sorted extracted brief alignment file");
    help.add_argument("Output file of type 1 alignments");
    help.add_argument("delta1: Maximum allowed difference between the inversions of ref and qry");
    help.add_argument("ptg1:    Minimum required percentage of coverage for the inversions of ref and qry");
    help.add_argument("min_MAP: Minium allowed MAPQ value");
    help.add_argument("Maximum allowed overlap for inversion analysis");

    help.check(argc, argv);

    infile  = argv[1];
    outfile = argv[2];
    delta1  = stoull(argv[3]);
    ptg1    = stod(argv[4]);
    min_MAPQ = stoull(argv[5]);
    allowed_overlap = stoull( argv[6] );
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_alignments();
    do_analysis();
    return 0;
}
