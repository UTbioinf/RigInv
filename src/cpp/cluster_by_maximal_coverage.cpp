#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iterator>
#include <array>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>
#include <loonutil/logger.h>
#include <alglib/cpp/src/specialfunctions.h>

using namespace std;
using loon::global_logger;

//#define LOON_SHOW_DEBUG
#ifdef LOON_SHOW_DEBUG
#include <iomanip>
#endif

/*=========================== typedef and declarations ==========================*/
typedef long long LL;
typedef array<LL, 4> RectBase;
typedef unsigned int UInteger;
class LocalMaximal;
class Rect;

/*=========================== command line args ==========================*/
char* infile;
string outdir;
double p;
double delta;
size_t min_cluster;
LL prediction_size;
double remove_portion; // remove this proportion of largest rectangles 
bool compute_inital;
bool singletons_only; // if true, keep only isolated clusters
size_t min_brace;
double min_brace_ratio;

/*=========================== global variables ===========================*/
vector<Rect> rects;
size_t M;
vector<LL> x_axis, y_axis;
vector<LocalMaximal> local_maximal;
vector<long double> P;
size_t cluster_id = 0;
ofstream fout_predictions, fout_spec;

/*=========================== class Rect_T ===========================*/
class Rect: public RectBase
{
public:
    char brace;
    LL dist;
public:
    Rect(): brace(0)
    {}
    void swap(Rect& rhs)
    {
        ::swap(brace, rhs.brace);
        ::swap(dist, rhs.dist);
        RectBase::swap(rhs);
    }
};

/*=========================== class LocalMaximal =========================*/

class LocalMaximal
{
public:
    Rect rect; // the representative rect of the set of rects
    vector<size_t> rect_indices; // indices of the set of rects
public:
    void set_rect(const Rect& r); // set the representative rect as `r`
    void add_rect_index(size_t index); // append the index of a rect to the `rect_indices`
    size_t size() const;    // get the size of `rect_indices`
    void swap(LocalMaximal& rhs); // swap two LocalMaximal objects
    void do_union(const LocalMaximal& rhs); // append `rhs.rect_indices` to the `this->rect_indices`, nothing else
};

void LocalMaximal::set_rect(const Rect& r)
{
    rect = r;
}

void LocalMaximal::add_rect_index(size_t index)
{
    rect_indices.push_back( index );
}

size_t LocalMaximal::size() const
{
    return rect_indices.size();
}

void LocalMaximal::swap(LocalMaximal& rhs)
{
    rect.swap( rhs.rect );
    rect_indices.swap( rhs.rect_indices );
}

void LocalMaximal::do_union(const LocalMaximal& rhs)
{
    rect_indices.insert( rect_indices.end(), rhs.rect_indices.begin(), rhs.rect_indices.end() );
}

/*=========================== functions ===========================*/
template<typename basic_type>
void sort_and_unique(vector<basic_type>& a)
{
    if(a.empty())   return;

    size_t n_a = a.size();
    sort(a.begin(), a.end());
    // remove repeats
    size_t tail = 0;
    for(size_t i = 1; i < n_a; ++i)
    {
        if(a[i] != a[tail])
            a[++tail] = a[i];
    }
    if((++tail) < n_a)
    {
        a.resize(tail);
        a.shrink_to_fit();
    }
}

// return true if two rects `r1` and `r2` intersect
inline bool is_connected(const Rect& r1, const Rect& r2)
{
    if(r1[1] <= r2[0])   return false;
    if(r2[1] <= r1[0])   return false;
    if(r1[3] <= r2[2])   return false;
    if(r2[3] <= r1[2])   return false;
    return true;
}

size_t estimate_k(size_t n, double p, double delta)
{
    if(n == 0)  return 0;
    size_t min_k = 1, max_k = n + 1;
    while(min_k <= max_k)
    {
        size_t mid_k = (max_k + min_k) >> 1;
        double cdf = alglib::binomialdistribution(n - mid_k, n, p);
        if(cdf >= delta)    min_k = mid_k + 1;
        else    max_k = mid_k - 1;
    }
    return min_k - 1;
}

// return true if there's a representative for the list of rectangles represented by points
bool real_check_rect(vector<LL>* points, Rect& res, size_t m)
{
    size_t k = estimate_k(m, p, delta);
    if(m > k)
    {
        nth_element(points[0].begin(), points[0].end() - k - 1, points[0].end());
        nth_element(points[1].begin(), points[1].begin() + k, points[1].end());
        nth_element(points[2].begin(), points[2].end() - k - 1, points[2].end());
        nth_element(points[3].begin(), points[3].begin() + k, points[3].end());

        res[0] = points[0][m - k - 1];
        res[1] = points[1][k];
        res[2] = points[2][m - k - 1];
        res[3] = points[3][k];

    #ifdef LOON_SHOW_DEBUG
        global_logger.debug("predicted rectangle: %lld, %lld, %lld, %lld; k = %lld", res[0], res[1], res[2], res[3], k);
    #endif
        return (res[0] <= res[1] && res[2] <= res[3] && res[1] - res[0] <= prediction_size && res[3] - res[2] <= prediction_size);
    }
    return false;
}

inline bool check_brace_cnt(size_t cnt_lbrace, size_t cnt_rbrace)
{
    if(cnt_lbrace < min_brace || cnt_rbrace < min_brace)    return false;
    if(cnt_lbrace < cnt_rbrace * min_brace_ratio || cnt_rbrace < cnt_lbrace * min_brace_ratio)  return false;
    return true;
}

// return true if there's a representative rect for the list of rectangles w.r.t. rect_indices
bool check_rect(const vector<size_t>& rect_indices, Rect& res)
{
    size_t cnt_lbrace = 0;
    vector<LL> points[4];
    size_t m = rect_indices.size();
    points[0].reserve( m );
    points[1].reserve( m );
    points[2].reserve( m );
    points[3].reserve( m );
    for(vector<size_t>::const_iterator it = rect_indices.begin(); it != rect_indices.end(); ++it)
    {
        points[0].push_back( rects[ *it ][0] );
        points[1].push_back( rects[ *it ][1] );
        points[2].push_back( rects[ *it ][2] );
        points[3].push_back( rects[ *it ][3] );
        if(rects[*it].brace == '[') ++cnt_lbrace;
    }
    // make decision based on [ and ]
    if(not check_brace_cnt(cnt_lbrace, m - cnt_lbrace)) return false;
    return real_check_rect(points, res, m);
}

// return true if there's a representative rect for all the rects in global `rects`
bool check_rect(Rect& res)
{
    size_t cnt_lbrace = 0;
    vector<LL> points[4];
    points[0].reserve( M );
    points[1].reserve( M );
    points[2].reserve( M );
    points[3].reserve( M );
    for(size_t i = M; i > 0;)
    {
        --i;
        points[0].push_back( rects[ i ][0] );
        points[1].push_back( rects[ i ][1] );
        points[2].push_back( rects[ i ][2] );
        points[3].push_back( rects[ i ][3] );
        if(rects[ i ].brace == '[') ++cnt_lbrace;
    }
    if(not check_brace_cnt(cnt_lbrace, M - cnt_lbrace)) return false;
    return real_check_rect(points, res, M);
}

// print one rect in a line, separated by a space
inline void print_rect(ostream& fout, const Rect& t, bool print_brace = false)
{
    fout << t[0] << ' '
         << t[1] << ' '
         << t[2] << ' '
         << t[3];
    if(print_brace)     fout << ' ' << t.brace << endl;
    else    fout << endl;
}

// save the cluster in `t`
void print_cluster(const LocalMaximal& t)
{
    // save the representative rectangle
    print_rect(fout_predictions, t.rect);

    string prefix = outdir + loon::int2path(cluster_id); // the path to the cluster
    loon::mkdir_p( prefix ); // make directory for the path to the cluster
    fout_spec << cluster_id << ' ' << prefix << endl; // save the prefix name to the spec file
    prefix += to_string( cluster_id ); // the prefix name of the cluster file

    // save the cluster to its file
    ofstream fout;
    loon::open_file(fout, prefix + ".txt");
    for(vector<size_t>::const_iterator it = t.rect_indices.begin(); it != t.rect_indices.end(); ++it)
    {
        print_rect(fout, rects[ *it ], true);
    }
    fout.close();

    ++cluster_id;
}

bool all_local_maximal(size_t min_raw_coverage, size_t min_lm_coverage)
{
    size_t m = x_axis.size() - 1;
    size_t n = y_axis.size() - 1;

    vector<LocalMaximal> new_local_maximal;

    vector<vector<UInteger> > matrix( m, vector<UInteger>(n, 0) );
    vector<vector<size_t> > label_matrix(m, vector<size_t>(n, 0));
    size_t N = local_maximal.size();
    size_t next_id = N+1; // according to the following algorithm, next_id can be upper bounded by $O(N^3)$.

#ifdef LOON_SHOW_DEBUG
    size_t cur_clu_size = 0;
    global_logger.debug("m = %lld, n = %lld", m, n);
    //bool print_this_round = (m <= 52) && (n <= 38);
    bool print_this_round = (m <= 46) && (n <= 41);
    if(print_this_round)
    {
        cerr << "[DEBUG]: x_axis: ";
        for(size_t mmm = 0; mmm < m; ++mmm)
            cerr << x_axis[ mmm ] << ' ';
        cerr << endl;
        cerr << "[DEBUG]: y_axis: ";
        for(size_t nnn = 0; nnn < n; ++nnn)
            cerr << y_axis[ nnn ] << ' ';
        cerr << endl;
    }
#endif

    // label the matrix
    for(size_t rect_i = N; rect_i > 0;)
    {
        const Rect& cur_rect = local_maximal[ --rect_i ].rect;

        size_t x_index = distance(x_axis.begin(), upper_bound( x_axis.begin(), x_axis.end(), cur_rect[0] ) - 1);
        size_t y_index = distance(y_axis.begin(), upper_bound( y_axis.begin(), y_axis.end(), cur_rect[2] ) - 1);

        map<size_t, size_t> id_to_nextid;
        map<size_t, size_t>::iterator id_to_nextid_iter;
        for(size_t ii = x_index; x_axis[ ii ] < cur_rect[1]; ++ii)
            for(size_t jj = y_index; y_axis[ jj ] < cur_rect[3]; ++jj)
            {   // count the coverage of each cell in the matrix
                ++matrix[ ii ][ jj ];
                // label each cell such that the cells with the same label represent that they are covered by the same group of rectangles
                if(label_matrix[ ii ][ jj ] == 0)
                    label_matrix[ ii ][ jj ] = rect_i+1;
                else
                {
                    id_to_nextid_iter = id_to_nextid.find( label_matrix[ ii ][ jj ] );
                    if(id_to_nextid_iter != id_to_nextid.end())
                    {
                        label_matrix[ ii ][ jj ] = id_to_nextid_iter->second;
                    }
                    else
                    {
                        id_to_nextid[ label_matrix[ ii ][ jj ] ] = next_id;
                        label_matrix[ ii ][ jj ] = next_id;
                        ++next_id;
                    }
                }
            }
    }
#ifdef LOON_SHOW_DEBUG
    if(print_this_round)
    {
        global_logger.debug("matrix:");
        for(size_t ii = 0; ii < m; ++ii)
        {
            for(size_t jj = 0; jj < n; ++jj)
                cerr << "(" << setw(2) << matrix[ii][jj] << "," << setw(3) << label_matrix[ii][jj] << ")\t";
            cerr << endl;
        }
        global_logger.debug("list all rep matrices:");
        for(size_t rect_i = N; rect_i > 0;)
        {
            print_rect(cerr, local_maximal[--rect_i].rect);
        }
    }
#endif

    vector<bool> used_labels(next_id, false);
    vector<bool> used_maximals( N, false );
    for(size_t i = m; i > 0;)
    {
        --i;
        for(size_t j = n; j > 0;)
        {
            --j;
            if(label_matrix[ i ][ j ] == 0) continue; // label = 0 means that no coverage
            if(matrix[ i ][ j ] < min_lm_coverage)  continue; // ignore the local maximal if the coverage is too low
            if(used_labels[ label_matrix[i][j] ]) continue;

            bool is_local_maximal = true;
            if(i > 0 && matrix[i-1][j] > matrix[i][j])
                is_local_maximal = false;
            else if(j > 0 && matrix[i][j-1] > matrix[i][j])
                is_local_maximal = false;
            else if(i + 1 < m && matrix[i+1][j] > matrix[i][j])
                is_local_maximal = false;
            else if(j + 1 < n && matrix[i][j+1] > matrix[i][j])
                is_local_maximal = false;

            if(is_local_maximal)
            {
                new_local_maximal.push_back( LocalMaximal() );
                LocalMaximal& tmp_lm = new_local_maximal.back();

                vector<size_t> used_lm_idx;
                for(size_t rect_i = N; rect_i > 0;)
                {
                    const Rect& cur_rect = local_maximal[ --rect_i ].rect;
                    if(x_axis[ i ] >= cur_rect[0] && x_axis[ i ] < cur_rect[1] &&
                            y_axis[j] >= cur_rect[2] && y_axis[ j ] < cur_rect[3])
                    {
                        tmp_lm.do_union( local_maximal[ rect_i ] );
                        if(not used_maximals[ rect_i ])
                            used_lm_idx.push_back( rect_i );
                    }
                }
                sort_and_unique( tmp_lm.rect_indices);
            #ifdef LOON_SHOW_DEBUG
                if(print_this_round)
                {
                    global_logger.debug("local maximal: %lld", matrix[i][j]);
                    global_logger.debug("tmp_lm.size() = %lld", tmp_lm.size());
                }
            #endif
                // check raw coverage and whether the new collection has a representative rectangle w.r.t. the input rectangles
                if(tmp_lm.size() >= min_raw_coverage && check_rect( tmp_lm.rect_indices, tmp_lm.rect ))
                {
                    for(vector<size_t>::const_iterator rect_idx_it = used_lm_idx.begin(); rect_idx_it != used_lm_idx.end(); ++rect_idx_it)
                        used_maximals[ *rect_idx_it ] = true;
                }
                else
                {
                    new_local_maximal.pop_back();
                }
                used_labels[ label_matrix[i][j] ] = true;
            }
        }
    }
    if(new_local_maximal.empty())   return true;
    for(size_t rect_i = N; rect_i > 0; )
    {
        if(!used_maximals[ --rect_i ] and local_maximal[ rect_i ].rect_indices.size() >= min_raw_coverage)
        {
            new_local_maximal.push_back( LocalMaximal() );
            new_local_maximal.back().swap( local_maximal[ rect_i ] );
        }
    }
    local_maximal.swap( new_local_maximal );
    return false;
}

void prepare_axes()
{
    size_t m = local_maximal.size();
    x_axis.clear();
    y_axis.clear();
    x_axis.reserve( m << 1 );
    y_axis.reserve( m << 1 );
    for(size_t i = m; i > 0;)
    {
        --i;
        x_axis.push_back( local_maximal[i].rect[0] );
        x_axis.push_back( local_maximal[i].rect[1] );

        y_axis.push_back( local_maximal[i].rect[2] );
        y_axis.push_back( local_maximal[i].rect[3] );
    }
    sort_and_unique( x_axis );
    sort_and_unique( y_axis );
}

void run_clustering()
{
    if(M == 0)  return;

    bool stop = false;
    
#ifdef LOON_SHOW_DEBUG
    size_t debug_iter_cnt = 0;
#endif
    size_t min_lm_coverage = max(static_cast<size_t>(min_cluster), static_cast<size_t>(2));;
    while(not stop)
    {
        prepare_axes();
        if(x_axis.empty() or y_axis.empty())    break;
        stop = all_local_maximal(min_cluster, min_lm_coverage);
        min_lm_coverage = 2;
    #ifdef LOON_SHOW_DEBUG
        global_logger.debug("iter = %lld: %lld clusters", (debug_iter_cnt++), local_maximal.size());
    #endif
    }

    if(local_maximal.size() < rects.size())
    {
    #ifdef LOON_SHOW_DEBUG
        global_logger.info("Done! %lld clusters after %lld iterations", local_maximal.size(), debug_iter_cnt);
    #endif
        if(singletons_only)
        {
            size_t lm_n = local_maximal.size();
            for(size_t lm_i = 0; lm_i < lm_n; ++lm_i)
            {
                bool is_singleton = true;
                for(size_t lm_j = 0; lm_j < lm_n; ++lm_j)
                    if(lm_i != lm_j && is_connected(local_maximal[lm_i].rect, local_maximal[lm_j].rect))
                    {
                        is_singleton = false;
                        break;
                    }
                if(is_singleton)    print_cluster( local_maximal[lm_i] );
            }
                    
        }
        else
        {
            for(vector<LocalMaximal>::const_iterator it = local_maximal.begin(); it != local_maximal.end(); ++it)
                print_cluster( *it );
        }
    }
    #ifdef LOON_SHOW_DEBUG
    else
    {
        global_logger.info("Done! No clusters after %lld iterations");
    }
    #endif
}

void init()
{
    local_maximal.reserve(M);
    LocalMaximal tmp;
    for(size_t i = M; i > 0; )
    {
        --i;
        local_maximal.push_back( LocalMaximal() );
        local_maximal.back().set_rect( rects[i] );
        local_maximal.back().add_rect_index( i );
    }
}

bool initial_check()
{
    Rect ans;
    if(compute_inital && check_rect(ans))
    {
        print_rect(fout_predictions, ans);
        return true;
    }
    return false;
}

bool compare_rect(const Rect& lhs, const Rect& rhs)
{
    return max(lhs[1] - lhs[0], lhs[3] - lhs[2]) < max(rhs[1] - rhs[0], rhs[3] - rhs[2]);
}
void read_rects()
{
    ifstream fin;
    loon::open_file(fin, infile);
    Rect tmp;
    while(fin >> tmp[0])
    {
        fin >> tmp[1] >> tmp[2] >> tmp[3] >> tmp.brace >> tmp.dist;
        rects.push_back( tmp );

    }
    fin.close();
    sort(rects.begin(), rects.end(), compare_rect);// sort in ascending order, w.r.t. the longest sides of the rect
    rects.resize( static_cast<size_t>(rects.size() * (1 - remove_portion)) );// keep only a proportion of the rectangles
    M = rects.size();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("cluster_by_maximal_coverage <required parameters>");
    help.add_argument("Input file name of a set of rectangles");
    help.add_argument("Output directory");
    help.add_argument("p: lower bound of the probability of one rectangle containing the target point");
    help.add_argument("delta: confidence parameter (close to 0 is better)");
    help.add_argument("Minimum number of rectangles required to estimate the representative rectangle");
    help.add_argument("Set maximum allowed side length of the predicted representative rectangles (0 if ignore this parameter)");
    help.add_argument("Remove this portion of large rectangles");
    help.add_argument("Set to 1 if the first step is to try to compute a representative rectangle for all rectangles");
    help.add_argument("Set to 1 if want to remove representative rectangles that overlap one another");
    help.add_argument("If the number of alignments less than this number, the inversion will not be considered");
    help.add_argument("if min(lbrace/rbrace, rbrace/lbrace) is less than this number, the inversion will not be considered");

    help.check(argc, argv);

    infile      = argv[1];
    outdir      = string(argv[2]) + loon::directory_delimiter;
    p           = stod(argv[3]);
    delta       = stod(argv[4]);
    min_cluster = stoll(argv[5]);
    prediction_size = stoll( argv[6] );
    remove_portion  = stod( argv[7] );
    if(prediction_size == 0)
        prediction_size = numeric_limits<LL>::max();
    compute_inital  = string(argv[8]) == "1";
    singletons_only = string(argv[9]) == "1";
    min_brace       = stoll(argv[10]);
    min_brace_ratio = stod(argv[11]);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
#ifdef LOON_SHOW_DEBUG
    global_logger.color_on();
#endif

    read_rects();
    if(rects.empty())
    {
    #ifdef LOON_SHOW_DEBUG
        global_logger.debug("no rectangles");
    #endif
        return 0;
    }
#ifdef LOON_SHOW_DEBUG
    else    global_logger.debug("%lld rectangles in consideration", rects.size());
#endif

    loon::open_file( fout_predictions, outdir + "predictions.sol" );
    loon::open_file( fout_spec, outdir + "spec.txt" );
    if(initial_check())
    {
        fout_predictions.close();
        fout_spec.close();
        return 0;
    }

    init();
    run_clustering();
    fout_predictions.close();
    fout_spec.close();
    return 0;
}
