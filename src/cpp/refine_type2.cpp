#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

//#define LOON_DEBUG
#ifdef LOON_DEBUG
  #include <loonutil/logger.h>
#endif

using namespace std;

/*====================== typedef and declarations ===========================*/
typedef unsigned long long ULL;
typedef array<ULL, 4> Rect;
typedef array<ULL, 2> ValidatedSeg;
typedef array<ULL, 2> IdxPair;
typedef unsigned int UInteger;
class Type2;

/*====================== command line args ==================================*/
char* infile1;
string type2_dir;
char* outfile;

/*====================== global variables ==============================*/
#ifdef LOON_DEBUG
  loon::Logger logger(1);
#endif
vector<ValidatedSeg> validated_segments;
vector<UInteger> mark_vs; // 1 means perfect; 2 means bad; 3 almost perfect; 4 means almost bad
size_t n_vs;
vector<Type2> type2_rects;

/*========================= class Type2 =======================*/
class Type2
{
public:
    Rect rect;
    UInteger label; // 0b11 means perfect;  0b101 means almost perfect
    IdxPair valid_range;
public:
    Type2();
    void set_rect(const Rect& r);
    void set_start(size_t start_id);
    void set_end(size_t end_id);
};

Type2::Type2(): label(0)
{}

void Type2::set_rect(const Rect& r)
{
    rect = r;
}

void Type2::set_start(size_t start_id)
{
    valid_range[0] = start_id;
}

void Type2::set_end(size_t end_id)
{
    valid_range[1] = end_id;
}

/*========================= functions =========================*/
void print_rect(ostream& out, const Rect& r)
{
    out << r[0] << ' '
        << r[1] << ' '
        << r[2] << ' '
        << r[3] << endl;
}

void write_result()
{
    ofstream fout;
    loon::open_file(fout, outfile);
    for(vector<Type2>::const_iterator it = type2_rects.begin(); it != type2_rects.end(); ++it)
    {
        if(it->label == 0x3)
        {
        #ifdef LOON_DEBUG
            logger.info("[%llu %llu %llu %llu] perfect", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
        #endif
            print_rect(fout, it->rect);
        }
        else if(it->label < 3)
        {
            if(mark_vs[ it->valid_range[0] ] == 0)
            {
            #ifdef LOON_DEBUG
                logger.info("[%llu %llu %llu %llu] possible", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
            #endif
                print_rect(fout, it->rect);
            }
        #ifdef LOON_DEBUG
            else // for debug
            {
                string msg;
                if(mark_vs[ it->valid_range[0] ] == 1)
                    msg = "Partially in a perfect region";
                else if(mark_vs[ it->valid_range[0] ] == 2)
                    msg = "Contains bad region";
                else if(mark_vs[ it->valid_range[0] ] == 3)
                    msg = "Partially in an almost perfect region";
                else if(mark_vs[ it->valid_range[0] ] == 3)
                    msg = "Contains almost bad region";
                logger.warning("[%llu %llu %llu %llu]: %s", it->rect[0], it->rect[1], it->rect[2], it->rect[3], msg.c_str());
            }
        #endif
        }
        else
        {
            bool possibly_good = true;
            UInteger tmp_label = 0;
            for(size_t ii = it->valid_range[0]; ii <= it->valid_range[1]; ++ii)
            {
                if(mark_vs[ ii ] == 0)
                    tmp_label |= 1;
                else if(mark_vs[ii] == 3)
                    tmp_label |= 2;
                else // for debug
                {
                    possibly_good = false;
                #ifdef LOON_DEBUG
                    if(mark_vs[ ii ] == 2)
                    {
                        logger.warning("[%llu %llu %llu %llu]: Contains a bad region 2", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
                        break;
                    }
                    else if(mark_vs[ ii ] == 4)
                    {
                        logger.warning("[%llu %llu %llu %llu]: Contains an almost bad region 2", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
                        break;
                    }
                    else if(mark_vs[ ii ] == 1)
                    {
                        logger.warning("[%llu %llu %llu %llu]: Partially contains a perfect region 2", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
                        break;
                    }
                    else
                    {
                        logger.error("[%llu %llu %llu %llu]: mark_vs[ %llu ] = %u: unknown reason, but not good!!!", it->rect[0], it->rect[1], it->rect[2], it->rect[3], ii, mark_vs[ii]);
                        break;
                    }
                #else
                    break;
                #endif
                }
                if(tmp_label == 3)
                {
                    possibly_good = false;
                #ifdef LOON_DEBUG
                    logger.warning("[%llu %llu %llu %llu]: Partially contains an almost perfect region 2", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
                #endif
                    break;
                }
            }
            if(possibly_good)
            {
            #ifdef LOON_DEBUG
                logger.info("[%llu %llu %llu %llu] possible 2", it->rect[0], it->rect[1], it->rect[2], it->rect[3]);
            #endif
                print_rect(fout, it->rect);
            }
        }
    }
    fout.close();
}

bool comp_pairs(const IdxPair& lhs, const IdxPair& rhs)
{
    if(lhs[0] < rhs[0]) return true;
    if(lhs[0] > rhs[0]) return false;
    if(lhs[1] > rhs[0]) return true;
    return false;
}

void mark_almost_perfect()
{
    vector<IdxPair> pairs;
    IdxPair tmp_pair;
    for(vector<Type2>::const_iterator it = type2_rects.begin(); it != type2_rects.end(); ++it)
    {
        if(it->label & 0x5) // almost perfect
        {
            tmp_pair = it->valid_range;
            ++tmp_pair[ 1 ];
            pairs.push_back( tmp_pair );
        }
    }
    sort(pairs.begin(), pairs.end(), comp_pairs);
    tmp_pair[0] = 0;
    tmp_pair[1] = 0;
    size_t cnt = 0;
    for(vector<IdxPair>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
    {
        if((*it)[0] > tmp_pair[1])
        {// new contiguous segments
            if(cnt == 1)
            {// mark the last one as almost perfect
                bool all_zeros = true;
                for(size_t ii = tmp_pair[0]; ii < tmp_pair[1]; ++ii)
                {
                    if(mark_vs[ ii ] != 0)
                    {
                        all_zeros = false;
                        break;
                    }
                }
                if(all_zeros)
                {
                    for(size_t ii = tmp_pair[0]; ii < tmp_pair[1]; ++ii)
                        mark_vs[ ii ] = 3;
                }
            }
            tmp_pair = *it;
            cnt = 1;
        }
        else
        {
            if(cnt == 0)
                tmp_pair = *it;
            else if(tmp_pair[1] < (*it)[1])
                tmp_pair[1] = (*it)[1];
        }
    }
    if(cnt == 1)
    {
        // mark the last one as almost perfect
        bool all_zeros = true;
        for(size_t ii = tmp_pair[0]; ii < tmp_pair[1]; ++ii)
            if( mark_vs[ ii ] != 0 )
            {
                all_zeros = false;
                break;
            }
        if(all_zeros)
        {
            for(size_t ii = tmp_pair[0]; ii < tmp_pair[1]; ++ii)
                mark_vs[ ii ] = 3;
        }
    }
}

void mark_bad()
{
    for(size_t i = 0; i < n_vs; ++i)
    {
        if(mark_vs[ i ] == 0)
        {
            if(i > 0)
            {
                if(mark_vs[i - 1] == 1)
                    mark_vs[ i ] = 2;
                else if(mark_vs[i-1] == 3)
                    mark_vs[ i ] = 4;
            }
            if(i + 1 < n_vs)
            {
                if(mark_vs[ i + 1 ] == 1)
                    mark_vs[ i ] = 2;
                else if(mark_vs[i + 1] == 3)
                    mark_vs[ i ] = 4;
            }
        }
    }
}

void find_validated_segments(Type2& type2)
{
    bool found_left = false;
    for(size_t i = 0; i < n_vs; ++i)
    {
        if(!found_left)
        {
            if(validated_segments[i][1] >= type2.rect[1])
            {
                found_left = true;
                if(type2.rect[0] <= validated_segments[i][0] && type2.rect[1] > validated_segments[i][0])
                {
                    type2.label |= 0x1;
                }
                if(type2.rect[2] <= validated_segments[i][1] && type2.rect[3] > validated_segments[i][1])
                {
                    type2.label |= 0x2;
                }
                if(type2.label == 0x3) // label == 0b11 means perfect
                {
                    mark_vs[ i ] = 1; // 1 means perfect
                    return;
                }
                type2.set_start( i );
            }
        }
        else
        {
            if(validated_segments[i][0] <= type2.rect[2])
            {
                if(type2.rect[2] <= validated_segments[i][1] && type2.rect[3] > validated_segments[i][1])
                {
                    type2.label |= 0x4;
                }
                type2.label |= 0x8; // this means that there are at least two validated segments covered by this prediction
                type2.set_end(i);
            }
            else    return;
        }
    }
}

void read_predictions(const string& fname)
{
    ifstream fin;
    loon::open_file(fin, fname);
    Rect tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3];
        type2_rects.push_back( Type2() );
        type2_rects.back().set_rect( tmp );

        find_validated_segments( type2_rects.back() );
    }
    fin.close();
}

void read_type2()
{
    string spec = type2_dir + "spec.txt";
    ifstream fin;
    loon::open_file(fin, spec);

    string cp_id, cp_fname;
    string line;
    while(fin >> cp_id)
    {
        fin >> cp_fname;
        getline(fin, line);
        read_predictions(loon::get_directory(cp_fname) + loon::directory_delimiter + cp_id + "_sol" + loon::directory_delimiter+ "predictions.sol");
    }
    fin.close();
}

void read_validated_segments()
{
    ifstream fin;
    loon::open_file(fin, infile1);
    ValidatedSeg tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1];
        validated_segments.push_back( tmp );
    }
    fin.close();
    n_vs = validated_segments.size();
    mark_vs.assign(n_vs, 0);
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("refine_type2 <required parameters>");
    help.add_argument("Input file for the results of concordant analysis");
    help.add_argument("Root directory of the type2 results");
    help.add_argument("Output filename");

    help.check( argc, argv );

    infile1     = argv[1];
    type2_dir   = string( argv[2] ) + loon::directory_delimiter;
    outfile     = argv[3];
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
#ifdef LOON_DEBUG
    logger.color_on();
#endif

    read_validated_segments();
    read_type2();

    mark_bad();
    mark_almost_perfect();
    mark_bad();
    write_result();
    return 0;
}
