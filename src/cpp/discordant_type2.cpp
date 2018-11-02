#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*==================== ====================*/

/*==================== typedef and declarations ====================*/
typedef unsigned long long ULL;
typedef long long LL;
class OneAln;
class ReadAln;

/*==================== command line args ====================*/
char* infile;
char* outfile;
ULL min_MAPQ = 0;
LL ksi = 0;
LL allowed_distance = 1000;
LL min_extension = 100;
LL allowed_overlap=50;

/*==================== global variables ====================*/
map<string, ReadAln > forward_reads, reverse_reads;

/*==================== class OneAln ===================*/
class OneAln
{
public:
    LL ref_start, ref_end;
    LL qry_start, qry_end;
};

class ReadAln: public vector<OneAln>
{
public:
    LL qry_len;
};

// TODO: can be changed to also output the corresponding reads
void analyze_Lneighbor_covered(LL qlen, const OneAln& left_aln, const OneAln& right_aln, ofstream& fout)
{
    if(left_aln.ref_end + min_extension > right_aln.ref_end)  return;
    if(left_aln.qry_end + allowed_distance < qlen - right_aln.qry_end)  return;

    if(left_aln.qry_end > qlen - right_aln.qry_end)
    {// split reads overlap.
        LL d = left_aln.qry_end - (qlen - right_aln.qry_end);
        if(left_aln.qry_end - left_aln.qry_start < d + min_extension)   return;
        if(right_aln.qry_end -right_aln.qry_start < d + min_extension)  return;
        if(left_aln.ref_end + min_extension > right_aln.ref_end - d)    return;
        if(d > allowed_overlap)    return;

        fout << (left_aln.ref_end - d - ksi) << ' '
            << (left_aln.ref_end + ksi) << ' '
            << (right_aln.ref_end - d - ksi) << ' '
            << (right_aln.ref_end + ksi) << ' '
            << "[ -" << d << endl; // negative means the two alignment w.r.t. reads overlap
    }
    else
    {
        LL d = qlen - right_aln.qry_end - left_aln.qry_end;
        fout << (left_aln.ref_end - ksi) << ' '
             << (left_aln.ref_end + d + ksi) << ' '
             << (right_aln.ref_end - ksi) << ' '
             << (right_aln.ref_end + d + ksi) << ' '
             << " [ " << d << endl;
    }
}

// TODO: can be changed to also output the corresponding reads
void analyze_Rneighbor_covered(LL qlen, const OneAln& left_aln, const OneAln& right_aln, ofstream& fout)
{
    if(left_aln.ref_start + min_extension > right_aln.ref_start)    return;
    if(qlen - right_aln.qry_start + allowed_distance < left_aln.qry_start)    return;

    if(qlen - right_aln.qry_start > left_aln.qry_start)
    {// split reads overlap.
        LL d = qlen - right_aln.qry_start - left_aln.qry_start;
        if(left_aln.qry_end - left_aln.qry_start < d + min_extension)   return;
        if(right_aln.qry_end - right_aln.qry_start < d + min_extension) return;
        if(left_aln.ref_start + d + min_extension > right_aln.ref_start)    return;
        if(d > allowed_overlap)    return;
        fout << (left_aln.ref_start - ksi) << ' '
             << (left_aln.ref_start + d + ksi) << ' '
             << (right_aln.ref_start - ksi) << ' '
             << (right_aln.ref_start + d + ksi) << ' '
             << "] -" << d << endl; // negative means the two alignments w.r.t. reads overlap
    }
    else
    {
        LL d = left_aln.qry_start - (qlen - right_aln.qry_start);
        fout << (left_aln.ref_start - d - ksi) << ' '
             << (left_aln.ref_start + ksi) << ' '
             << (right_aln.ref_start - d - ksi) << ' '
             << (right_aln.ref_start + ksi) << ' '
             << "] " << d << endl;
    }
}

void do_analysis()
{
    ofstream fout;
    loon::open_file(fout, outfile);
    for(map<string, ReadAln>::iterator fit = forward_reads.begin(); fit != forward_reads.end(); ++fit)
    {
        map<string, ReadAln>::iterator rit = reverse_reads.find( fit->first );
        if(rit == reverse_reads.end())  continue;

        // TODO: consider only adjacent alignments instead of ALL alignments
        // sort(fit->second.begin(), fit->second.end());
        // sort(rit->second.rbegin(), rit->second.rend());
        for(vector<OneAln>::const_iterator aln_fit = fit->second.cbegin(); aln_fit != fit->second.cend(); ++aln_fit)
            for(vector<OneAln>::const_iterator aln_rit = rit->second.cbegin(); aln_rit != rit->second.cend(); ++aln_rit)
            {
                analyze_Lneighbor_covered(fit->second.qry_len, *aln_fit, *aln_rit, fout);
                analyze_Lneighbor_covered(fit->second.qry_len, *aln_rit, *aln_fit, fout);

                analyze_Rneighbor_covered(fit->second.qry_len, *aln_fit, *aln_rit, fout);
                analyze_Rneighbor_covered(fit->second.qry_len, *aln_rit, *aln_fit, fout);
            }
    }
    fout.close();
}

void read_alignments()
{
    ifstream fin;
    loon::open_file(fin, infile);

    OneAln tmp;
    ULL mapping_quality;
    LL qry_len;
    string name, line, qry_name;
    char orientation;
    while(fin >> tmp.ref_start)
    {
        fin >> tmp.ref_end >> tmp.qry_start >> tmp.qry_end
            >> qry_name >> line >> qry_len >> mapping_quality >> orientation;
        if(mapping_quality < min_MAPQ)
            continue;
        map<string, ReadAln>::iterator it;
        if(orientation == 'F')
        {
            it = forward_reads.find( qry_name );
            if(it == forward_reads.end())
            {
                forward_reads[ qry_name ].qry_len = qry_len;
                it = forward_reads.find( qry_name );
            }
        }
        else
        {
            it = reverse_reads.find( qry_name );
            if(it == reverse_reads.end())
            {
                reverse_reads[ qry_name ].qry_len = qry_len;
                it = reverse_reads.find( qry_name );
            }
        }
        it->second.push_back( tmp );
    }

    fin.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("discordant_type2 <required parameters>");

    help.add_argument("Input file of sorted extracted brief alignment file");
    help.add_argument("Output file of type 2 alignments");
    help.add_argument("min_MAPQ: Minimum allowed MAPQ value");
    help.add_argument("min_extension: Minimum extended length when two split reads overlap");
    help.add_argument("Ksi: Allowed error for assessing break points");
    help.add_argument("Maximum allowed distance between adjacent aligned piece of the reads");

    help.check(argc, argv);

    infile = argv[1];
    outfile = argv[2];
    min_MAPQ = stoull(argv[3]);
    min_extension = stoull(argv[4]);
    ksi = stoull(argv[5]);
    allowed_distance = stoull(argv[6]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_alignments();
    do_analysis();
    return 0;
}
