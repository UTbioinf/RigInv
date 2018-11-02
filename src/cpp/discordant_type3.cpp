#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

/*==================== ====================*/

/*==================== typedef and declarations ====================*/
typedef unsigned long long ULL;
typedef long long LL;
class OneAln;

/*==================== command line args  ====================*/
char* infile[2];
char* outfile_prefix;
ULL min_MAPQ = 0;
LL allowed_overlap = 5;
LL ksi = 0;

/*==================== globals variables  ====================*/
vector<OneAln> alns[2];
map<string, LL> qry_lens;
map<string, vector<OneAln*>* > forward_reads[2], reverse_reads[2];

/*==================== class OneAln ====================*/
class OneAln
{
public:
    LL ref_start, ref_end;
    LL qry_start, qry_end;
    string qry_name;
    char orientation;
};

/*==================== functions  ====================*/

// 1) ref: F F ; [ ]
void analyze_ref_FF(const OneAln& A_L, const OneAln& A_R, LL A_len,
        const OneAln& B_L, const OneAln& B_R, LL B_len,
        ofstream& fout)
{
    if(A_L.ref_end > B_L.ref_start + allowed_overlap)   return;
    if(A_R.ref_end > B_R.ref_start + allowed_overlap)   return;

    LL d1 = A_len - A_R.qry_end - A_L.qry_end;
    LL d2 = B_R.qry_start + B_L.qry_start - B_len;

    if(d1 + allowed_overlap < 0)    return;
    if(d2 + allowed_overlap < 0)    return;

    LL lbp_l = max(A_L.ref_end, B_L.ref_start - d2) - ksi;
    LL lbp_r = min(A_L.ref_end + d1, B_L.ref_start) + ksi;
    if(lbp_l > lbp_r)   return;

    LL rbp_l = max(A_R.ref_end, B_R.ref_start - d2) - ksi;
    LL rbp_r = min(A_R.ref_end + d1, B_R.ref_start) + ksi;
    if(rbp_l > rbp_r)   return;

    fout << lbp_l << ' ' << (lbp_r + 1) << ' ' << rbp_l << ' ' << (rbp_r + 1) << " []" << endl;
}

// 2) ref: F R ; [ [
void analyze_ref_FR(const OneAln& A_L, const OneAln& A_R, LL A_len,
        const OneAln& B_L, const OneAln& B_R, LL B_len,
        ofstream& fout)
{
    if(A_L.ref_end > B_L.ref_start + allowed_overlap)   return;
    if(B_R.ref_end > A_R.ref_start + allowed_overlap)   return;

    LL d1 = A_R.qry_start - A_L.qry_end;
    LL d2 = B_R.qry_end - B_L.qry_start;

    if(d1 + allowed_overlap < 0)    return;
    if(d2 + allowed_overlap < 0)    return;

    LL lbp_l = max(A_L.ref_end, B_L.ref_start - d2) - ksi;
    LL lbp_r = min(A_L.ref_end + d1, B_L.ref_start) + ksi;
    if(lbp_l > lbp_r)   return;

    LL rbp_l = max(B_R.ref_end, A_R.ref_start - d1) - ksi;
    LL rbp_r = min(B_R.ref_end + d2, A_R.ref_start) + ksi;
    if(rbp_l > rbp_r)   return;

    fout << lbp_l << ' ' << (lbp_r + 1) << ' ' << rbp_l << ' ' << (rbp_r + 1) << " [[" << endl;
}

// 3) ref: R F ; ] ]
void analyze_ref_RF(const OneAln& A_L, const OneAln& A_R, LL A_len,
        const OneAln& B_L, const OneAln& B_R, LL B_len,
        ofstream& fout)
{
    if(B_L.ref_end > A_L.ref_start + allowed_overlap)   return;
    if(A_R.ref_end > B_R.ref_start + allowed_overlap)   return;

    LL d1 = A_L.qry_start - A_R.qry_end;
    LL d2 = B_R.qry_start - B_L.qry_end;

    if(d1 + allowed_overlap < 0)    return;
    if(d2 + allowed_overlap < 0)    return;

    LL lbp_l = max(B_L.ref_end, A_L.ref_start - d1) - ksi;
    LL lbp_r = min(B_L.ref_end + d2, A_L.ref_start) + ksi;
    if(lbp_l > lbp_r)   return;

    LL rbp_l = max(A_R.ref_end, B_R.ref_start - d2) - ksi;
    LL rbp_r = min(A_R.ref_end + d1, B_R.ref_start) + ksi;
    if(rbp_l > rbp_r)  return;

    fout << lbp_l << ' ' << (lbp_r + 1) << ' ' << rbp_l << ' ' << (rbp_r + 1) << " ]]" << endl;
}

// 4) ref: R R ; ] [
void analyze_ref_RR(const OneAln& A_L, const OneAln& A_R, LL A_len,
        const OneAln& B_L, const OneAln& B_R, LL B_len,
        ofstream& fout)
{
    if(B_L.ref_end > A_L.ref_start + allowed_overlap)   return;
    if(B_R.ref_end > A_R.ref_start + allowed_overlap)   return;

    LL d1 = A_L.qry_start + A_R.qry_start - A_len;
    LL d2 = B_len - B_L.qry_end - B_R.qry_end;

    if(d1 + allowed_overlap < 0)    return;
    if(d2 + allowed_overlap < 0)    return;

    LL lbp_l = max(B_L.ref_end, A_L.ref_start - d1) - ksi;
    LL lbp_r = min(B_L.ref_end + d2, A_L.ref_start) + ksi;
    if(lbp_l > lbp_r)   return;

    LL rbp_l = max(B_R.ref_end, A_R.ref_start - d1) - ksi;
    LL rbp_r = min(B_R.ref_end + d2, A_R.ref_start) + ksi;
    if(rbp_l > rbp_r)   return;

    fout << lbp_l << ' ' << (lbp_r + 1) << ' ' << rbp_l << ' ' << (rbp_r + 1) << " ][" << endl;
}

void do_analysis()
{
    size_t n1 = alns[0].size();
    map<string, vector<OneAln*>*>::iterator A_R_forward_it, A_R_reverse_it,
                                            B_R_forward_it, B_R_reverse_it;
    ofstream fout_FF, fout_FR, fout_RF, fout_RR;
    loon::open_file(fout_FF, string(outfile_prefix) + string("_FF"));
    loon::open_file(fout_FR, string(outfile_prefix) + string("_FR"));
    loon::open_file(fout_RF, string(outfile_prefix) + string("_RF"));
    loon::open_file(fout_RR, string(outfile_prefix) + string("_RR"));
    for(size_t A_L = 0; A_L < n1; ++A_L)
    {
        A_R_forward_it = forward_reads[1].find( alns[0][A_L].qry_name );
        A_R_reverse_it = reverse_reads[1].find( alns[0][A_L].qry_name );

        if(A_R_forward_it == forward_reads[1].end() && A_R_reverse_it == reverse_reads[1].end())
            continue;

        LL A_len = qry_lens[ alns[0][A_L].qry_name ];

        for(size_t B_L = A_L + 1; B_L < n1; ++B_L)
        {
            if(alns[0][ A_L ].qry_name == alns[0][ B_L ].qry_name)
                continue;
            B_R_forward_it = forward_reads[1].find( alns[0][B_L].qry_name );
            B_R_reverse_it = reverse_reads[1].find( alns[0][B_L].qry_name );

            if(B_R_forward_it == forward_reads[1].end() && B_R_reverse_it == reverse_reads[1].end())
                continue;

            LL B_len = qry_lens[ alns[0][B_L].qry_name ];

            if(A_R_forward_it != forward_reads[1].end())
            {
                for(vector<OneAln*>::iterator A_R_pit = A_R_forward_it->second->begin();
                        A_R_pit != A_R_forward_it->second->end(); ++A_R_pit)
                {// a: - f
                    if(B_R_forward_it != forward_reads[1].end())
                    {
                        for(vector<OneAln*>::iterator B_R_pit = B_R_forward_it->second->begin();
                                B_R_pit != B_R_forward_it->second->end(); ++B_R_pit)
                        {// a: - f;  b: - f
                            if(alns[0][A_L].orientation == 'F')
                            {// a: f f; b: - f
                                if(alns[0][B_L].orientation == 'F')
                                {
                                    // a: f f
                                    // b: f f
                                    analyze_ref_FR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FR); // 2) 2
                                    analyze_ref_RF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RF); // 3) 3
                                }
                                // no ELSE case:  a: f f;  b: r f
                            }
                            else
                            {// a: r f;   b: - f
                                if(alns[0][B_L].orientation == 'R')
                                {
                                    // a: r f
                                    // b: r f
                                    analyze_ref_FF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FF); // 1) 3
                                    analyze_ref_RR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RR); // 4) 2
                                }
                                // no ELSE case:  a: r f;  b: f f
                            }
                        }
                    }

                    if(B_R_reverse_it != reverse_reads[1].end())
                    {
                        for(vector<OneAln*>::iterator B_R_pit = B_R_reverse_it->second->begin();
                                B_R_pit != B_R_reverse_it->second->end(); ++B_R_pit)
                        {// a: - f;  b: - r
                            if(alns[0][A_L].orientation == 'F')
                            {// a: f f;  b: - r
                                if(alns[0][B_L].orientation == 'R')
                                {
                                    // a: f f
                                    // b: r r
                                    analyze_ref_FR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FR); // 2) 1
                                    analyze_ref_RF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RF); // 3) 4
                                }
                                // no ELSE case:  a: f f;  b: f r
                            }
                            else
                            {// a; r f;  b: - r
                                if(alns[0][B_L].orientation == 'F')
                                {
                                    // a: r f
                                    // b: f r
                                    analyze_ref_FF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FF); // 1) 4
                                    analyze_ref_RR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RR); // 4) 1
                                }
                                // no ELSE case:  a: r f;  b: r r
                            }
                        }
                    }
                }
            }
            if(A_R_reverse_it != reverse_reads[1].end())
            {
                for(vector<OneAln*>::iterator A_R_pit = A_R_reverse_it->second->begin();
                        A_R_pit != A_R_reverse_it->second->end(); ++A_R_pit)
                {// a: - r
                    if(B_R_forward_it != forward_reads[1].end())
                    {
                        for(vector<OneAln*>::iterator B_R_pit = B_R_forward_it->second->begin();
                                B_R_pit != B_R_forward_it->second->end(); ++B_R_pit)
                        {// a: - r;  b: - f
                            if(alns[0][A_L].orientation == 'F')
                            {// a: f r;  b: - f
                                if(alns[0][B_L].orientation == 'R')
                                {
                                    // a: f r
                                    // b: r f
                                    analyze_ref_FF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FF); // 1) 1
                                    analyze_ref_RR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RR); // 4) 4
                                }
                                // no ELSE case:  a: f r;  b: f f
                            }
                            else
                            {// a: r r;  b: - f
                                if(alns[0][B_L].orientation == 'F')
                                {
                                    // a: r r
                                    // b: f f
                                    analyze_ref_FR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FR); // 2) 4
                                    analyze_ref_RF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RF); // 3) 1
                                }
                                // no ELSE case:  a: r r;  b: r f
                            }
                        }
                    }

                    if(B_R_reverse_it != reverse_reads[1].end())
                    {
                        for(vector<OneAln*>::iterator B_R_pit = B_R_reverse_it->second->begin();
                                B_R_pit != B_R_reverse_it->second->end(); ++B_R_pit)
                        {// a: - r; b: - r
                            if(alns[0][A_L].orientation == 'F')
                            {// a: f r;  b: - r
                                if(alns[0][B_L].orientation == 'F')
                                {
                                    // a: f r
                                    // b: f r
                                    analyze_ref_FF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FF); // 1) 2
                                    analyze_ref_RR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RR); // 4) 3
                                }
                                // no ELSE case:  a: f r;  b: r r
                            }
                            else
                            {// a: r r;  b: - r
                                if(alns[0][B_L].orientation == 'R')
                                {
                                    // a: r r
                                    // b: r r
                                    analyze_ref_FR(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_FR); // 2) 3
                                    analyze_ref_RF(alns[0][A_L], **A_R_pit, A_len,
                                            alns[0][B_L], **B_R_pit, B_len,
                                            fout_RF); // 3) 2
                                }
                                // no ELSE case:  a: r r;  b: f r
                            }
                        }
                    }
                }
            }
        }
    }
    fout_FF.close();
    fout_FR.close();
    fout_RF.close();
    fout_RR.close();
}

void read_alignment(char* fname, vector<OneAln>& alignments,
        map<string, vector<OneAln*>*>& fwd_reads, 
        map<string, vector<OneAln*>*>& rev_reads)
{
    ifstream fin;
    loon::open_file(fin, fname);
    
    OneAln tmp;
    ULL mapping_quality;
    LL qry_len;
    string name;
    string line;
    while(fin >> tmp.ref_start)
    {
        fin >> tmp.ref_end >> tmp.qry_start >> tmp.qry_end
            >> tmp.qry_name >> line >> qry_len >> mapping_quality >> tmp.orientation;
        if(mapping_quality < min_MAPQ)
            continue;
        alignments.push_back( tmp );
        qry_lens[ tmp.qry_name ] = qry_len;
    }
    map<string, vector<OneAln*>*>::iterator it;
    for(size_t i = 0; i < alignments.size(); ++i)
    {
        if(alignments[i].orientation == 'F')
        {
            it = fwd_reads.find( alignments[i].qry_name );
            if(it == fwd_reads.end())
            {
                fwd_reads[ alignments[i].qry_name ] = new vector<OneAln*>();
                it = fwd_reads.find( alignments[i].qry_name );
            }
            it->second->push_back( &alignments[i] );
        }
        else
        {
            it = rev_reads.find( alignments[i].qry_name );
            if(it == rev_reads.end())
            {
                rev_reads[ alignments[i].qry_name ] = new vector<OneAln*>();
                it = rev_reads.find( alignments[i].qry_name );
            }
            it->second->push_back( &alignments[i] );
        }
    }

    fin.close();
}

void deallocate_maps(map<string, vector<OneAln*>*>& reads)
{
    for(map<string, vector<OneAln*>*>::iterator it = reads.begin();
            it != reads.end(); ++it)
        delete it->second;
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("discordant_type3 <required parameters>");
    help.add_argument("Input file of sorted extracted brief alignment for scaffold 1");
    help.add_argument("Input file of sorted extracted brief alignment for scaffold 2");
    help.add_argument("Output file prefix of type 3 discordant alignment for scaffold 1 and 2");
    help.add_argument("min_MAPQ: Minimum allowed MAPQ value");
    help.add_argument("Maximum allowed overlap for inversion analysis");
    help.add_argument("Ksi: Allowed error for assessing break points");

    help.check(argc, argv);

    // do parsing
    infile[0] = argv[1];
    infile[1] = argv[2];
    outfile_prefix = argv[3];
    min_MAPQ = stoull( argv[4] );
    allowed_overlap = stoull( argv[5] );
    ksi = stoull( argv[6] );
}


int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_alignment(infile[0], alns[0], forward_reads[0], reverse_reads[0]);
    read_alignment(infile[1], alns[0], forward_reads[1], reverse_reads[1]);

    do_analysis();

    deallocate_maps(forward_reads[0]);
    deallocate_maps(forward_reads[1]);
    deallocate_maps(reverse_reads[0]);
    deallocate_maps(reverse_reads[1]);
    return 0;
}
