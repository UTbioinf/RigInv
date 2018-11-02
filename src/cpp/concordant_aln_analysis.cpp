#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>
#include <loonalgorithm/LCA/RMQnlogn.hpp>

using namespace std;

/*====================== typedef and class declarations ==============================*/
typedef unsigned long long ULL;
class OneAln;
class SegEndpoint;

/*====================== command line parameters ================*/
ULL ref_len = 0;
ULL min_cutoff = 0;// A ref base is covered by an alignment if it's left `min_cutoff` number of bases and right `min_cutoff` number bases are covered in the alignment
ULL min_overlap = 0;// Two alignment are consecutive if they overlap at least `min_overlap` bases
ULL min_aln_len = 0;// Discard alignments whose length is < min_aln_len (min_aln_len > 2 * min_cutoff + 1)
ULL min_mapping_quality = 0;// Discard alignments whose mapping quality is < min_mapping_quality
char* infile;// Input file name
char* outfile;// Output file name
int min_cvg = 0;// if the max coverage of an alignment is < min_cvg, remove it
double min_cvg_percent = 0;// if the max coverage ratio of an alignment is < min_cvg_percent, remove it

/*====================== global variables =======================*/
vector<OneAln> alignments;
vector<SegEndpoint> segEndpoints;
double total_covered_length = 0;

/*====================== class SegEndpoint ======================*/
class SegEndpoint
{
public:
    ULL loc;
    size_t seg_id;
    bool is_start;
public:
    SegEndpoint(ULL position = -1, size_t idx = -1, bool is_left = false);
    bool operator<(const SegEndpoint& se) const;
};

SegEndpoint::SegEndpoint(ULL position, size_t idx, bool is_left):
        loc(position), seg_id(idx), is_start(is_left)
{}

bool SegEndpoint::operator<(const SegEndpoint& se) const
{
    if(loc < se.loc)    return true;
    if(loc > se.loc)    return false;
    return (!is_start && se.is_start);
}

/*======================= class OneAln ==========================*/

class OneAln
{
public:
    ULL ref_start, ref_end;
    string qname;
    ULL mapping_quality;
public:
    void invalidate();
    bool is_valid() const;
    bool satisfies(ULL m_aln_len, ULL m_map_quality) const;
    friend istream& operator>>(istream& in, OneAln& obj);
};

void OneAln::invalidate()
{
    ref_end = 0;
}

bool OneAln::is_valid() const
{
    return ref_end > 0;
}

bool OneAln::satisfies(ULL m_aln_len, ULL m_map_quality) const
{
    if(ref_end - ref_start < m_aln_len)
        return false;
    if(mapping_quality < m_map_quality)
        return false;
    return true;
}

istream& operator>>(istream& in, OneAln& obj)
{
    std::string tmp;
    if(in >> obj.ref_start)
    {
        in >> obj.ref_end;
        in >> tmp >> tmp; // filter out `qry_start` and `qry_end`
        in >> obj.qname;
        in >> tmp >> tmp; // filter out `ref_len` and `qry_len`
        in >> obj.mapping_quality;
        getline(in, tmp); // filter out orientation
    }
    return in;
}


/*=========================== functions =========================*/

void read_alignments()
{
    ifstream fin;
    loon::open_file(fin, infile);
    OneAln tmp_aln;
    while( fin >> tmp_aln)
    {
        if(tmp_aln.satisfies(min_aln_len, min_mapping_quality))
        {
            segEndpoints.emplace_back(tmp_aln.ref_start + min_cutoff,
                    alignments.size(),
                    true);
            segEndpoints.emplace_back(tmp_aln.ref_end - min_cutoff,
                    alignments.size(),
                    false);
            alignments.push_back( tmp_aln );
            total_covered_length = tmp_aln.ref_end - tmp_aln.ref_start - (min_cutoff << 1);
        }
    }
    fin.close();
}

void remove_low_coverage_reads()
{
    min_cvg = max( min_cvg, int(total_covered_length * min_cvg_percent / ref_len) );
    sort(segEndpoints.begin(), segEndpoints.end());
    size_t nn = alignments.size();

    vector<size_t> q_segStart( nn ); // start index of RMQ
    vector<size_t> q_segEnd( nn );   // end index of RMQ
    size_t last_pos = 0;
    loon::RMQnlogn<size_t> rmq;
    size_t cur_cvg = 0, cur_index = 1;
    for(size_t i = 0; i < (nn << 1); ++i)
    {
        if(segEndpoints[i].is_start)
        {
            if(segEndpoints[i].loc == last_pos)
                q_segStart[ segEndpoints[i].seg_id ] = cur_index - 1;
            else
            {
                q_segStart[ segEndpoints[i].seg_id ] = cur_index;
                rmq.push_back( nn - cur_cvg );
                ++cur_index;
                last_pos = segEndpoints[i].loc;
            }
            ++cur_cvg;
        }
        else
        {
            if(last_pos == segEndpoints[i].loc)
                q_segEnd[ segEndpoints[i].seg_id ] = cur_index - 2;
            else
            {
                q_segEnd[ segEndpoints[i].seg_id ] = cur_index - 1;
                rmq.push_back( nn - cur_cvg );
                ++cur_index;
                last_pos = segEndpoints[i].loc;
            }
            --cur_cvg;
        }
    }
    rmq.push_back( nn - cur_cvg );
    // Range maximum query and remove low coverage reads
    rmq.preprocess();
    for(size_t i = 0; i < nn; ++i)
    {
        size_t max_cvg = nn - rmq.query( q_segStart[i], q_segEnd[i] );
        if(max_cvg < min_cvg)   alignments[i].invalidate();
    }
    segEndpoints.clear();
}

void generate_validated_segments()
{
    ofstream fout;
    loon::open_file(fout, outfile);

    ULL start_loc = 0, end_loc = 0;
    for(vector<OneAln>::const_iterator it = alignments.begin(); it != alignments.end(); ++it)
        if(it->is_valid())
        {
            if(end_loc == 0)
            { // the first segment
                start_loc = it->ref_start;
                end_loc = it->ref_end;
            }
            else if(end_loc <= (it->ref_start + min_overlap))
            { // new segment
                fout << start_loc << ' ' << end_loc << endl;
                start_loc = it->ref_start;
                end_loc = it->ref_end;
            }
            else if(end_loc < (it->ref_end))
            { // update right-end
                end_loc = it->ref_end;
            }
            // qname <--> count(start_loc, end_loc). This can be used to map the alignment to the validated segments
        }
    if(end_loc > 0)
        fout << start_loc << ' ' << end_loc << endl;
    fout.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("concordant_aln_analysis <required parameters>");

    help.add_argument("Length of the reference");
    help.add_argument("Minimum cutoff for identifying one coverage for a base");
    help.add_argument("Minimum overlap for identifying consecutive validated segment");
    help.add_argument("Minimum length of an alignment in consideration");
    help.add_argument("Minimum mapping quality in consideration");
    help.add_argument("Input file name");
    help.add_argument("Output file name");
    help.add_argument("Minimum coverage for an alignment to be kept");
    help.add_argument("Minimum coverage ratio for an alignment to be kept");
    
    help.check(argc, argv);

    ref_len             = stoull( argv[1] );
    min_cutoff          = stoull( argv[2] );
    min_overlap         = stoull( argv[3] );
    min_aln_len         = stoull( argv[4] );
    min_mapping_quality = stoull( argv[5] );
    infile = argv[6];
    outfile = argv[7];
    min_cvg             = stoi( argv[8] );
    min_cvg_percent     = stod( argv[9] );

    min_aln_len = max(min_aln_len, (min_cutoff << 1) + 1);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_alignments();
    remove_low_coverage_reads();
    generate_validated_segments();
    return 0;
}
