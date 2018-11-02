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
class Tree;
class Edge;

/*========================== command line args =======================*/
char* infile;
char* treefile;
string outdir;
double p;   // probability of one rectangle containing the target point
double delta;   // confidence parameter (close to 0 is better)
int min_cluster; // minimum size of a cluster

/*========================== global variables =======================*/
vector<ULL> p_left, p_right, p_bottom, p_top;
size_t k;
vector<double> P;
int cluster_id = 0;
ofstream fout_spec;

/*===================== classes =======================*/
class Edge
{
public:
    int u, v, w;
public:
    bool operator<(const Edge& rhs) const;
};

bool Edge::operator<(const Edge& rhs) const
{
    return w < rhs.w;
}

class Tree
{
public:
    int m;
    vector<vector<int> > nodes;
    vector<Edge> edges;
    int min_active;
public:
    void read_edges(ifstream& fin);
    void build_tree();
    void remove_edge(int node, int edge_id);
    bool is_edge_active( int id );
    void active_inc();
};

void Tree::read_edges(ifstream& fin)
{
    fin >> m;
    edges.reserve(m);
    nodes.resize(m + 1);

    Edge e;
    for(int i = 0; i < m; ++i)
    {
        fin >> e.u >> e.v >> e.w;
        edges.push_back( e );
    }
    sort(edges.begin(), edges.end());
    min_active = 0;
}

void Tree::build_tree()
{
    for(int i = 0; i < m; ++i)
    {
        nodes[ edges[i].u ].push_back( i );
        nodes[ edges[i].v ].push_back( i );
    }
}

void Tree::remove_edge(int node, int edge_id)
{
    nodes[ node ][ edge_id ] = nodes[node].back();
    nodes[ node ].pop_back();
}

bool Tree::is_edge_active(int id)
{
    return id >= min_active;
}

void Tree::active_inc()
{
    if(min_active < m)
    {
        int cur_weight = edges[ min_active ].w;
        cerr << "[DEBUG]: current min edge weight = " << cur_weight << endl;
        int debug_cnt = 1;
        while((++min_active) < m && cur_weight == edges[ min_active ].w)
            ++debug_cnt;
        cerr << "[DEBUG]: remove " << debug_cnt << " edges" << endl;
    }
}

/*===================== functions =====================*/
Tree tree;

void read_tree()
{
    ifstream fin;
    loon::open_file( fin, treefile );
    tree.read_edges(fin);
    fin.close();

    tree.build_tree();
}

void compute_P(size_t m)
{
    P.resize( m + 1);
    P[0] = 1;
    for(size_t i = 0; i < m; ++i)
        P[i + 1] = P[i] * p;
}

void estimate_k(size_t m)
{
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

bool run_check_cluster(vector<ULL>& p_left, vector<ULL>& p_right,
        vector<ULL>& p_bottom, vector<ULL>& p_top)
{
    size_t m = p_left.size();
    estimate_k( m );
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
            string directory = outdir + loon::int2path( cluster_id );
            loon::mkdir_p( directory );
            string tmp_prefix = directory + to_string( cluster_id );
            fout_spec << cluster_id << ' ' << tmp_prefix << endl;

            ofstream fout;
            string path = tmp_prefix + ".txt";
            loon::open_file(fout ,path);
            for(size_t i = m; i > 0;)
            {
                --i;
                fout << p_left[i] << ' ' << p_right[i] << ' ' << p_bottom[i] << ' ' << p_top[i] << endl;
            }
            fout.close();
            

            path = tmp_prefix + ".sol";
            loon::open_file(fout, path);
            fout << ll << ' ' << rr << ' ' << bb << ' ' << tt << endl;
            fout.close();

            ++cluster_id;
            cerr << "[DEBUG]: save a cluster: m = " << m << ", k = " << k << ", ratio = " << double(k)/m << endl;
            return true;
        }
        else    return false;
    }
    else    return false;
}

bool check_cluster(const vector<int>& nodes)
{
    cerr << "[DEBUG]: check cluster with " << nodes.size() << " nodes" << endl;
    vector<ULL> t_l;
    vector<ULL> t_r;
    vector<ULL> t_b;
    vector<ULL> t_t;
    t_l.reserve( nodes.size() );
    t_r.reserve( nodes.size() );
    t_b.reserve( nodes.size() );
    t_t.reserve( nodes.size() );
    for(size_t i = nodes.size(); i > 0; )
    {
        --i;
        t_l.push_back( p_left[ nodes[i] ] );
        t_r.push_back( p_right[ nodes[i] ] );
        t_b.push_back( p_bottom[ nodes[i] ] );
        t_t.push_back( p_top[ nodes[i] ] );
    }
    return run_check_cluster( t_l, t_r, t_b, t_t );
}

bool check_cluster()
{
    vector<ULL> t_l = p_left;
    vector<ULL> t_r = p_right;
    vector<ULL> t_b = p_bottom;
    vector<ULL> t_t = p_top;
    return run_check_cluster(t_l, t_r, t_b, t_t);
}

void debug_clustering()
{
    int remove_times = 0;
    cout << "Input remove times: ";
    cin >> remove_times;

    while(remove_times > 0)
    {
        tree.active_inc();
        --remove_times;
    }

    // do BFS to find clusters
    vector<bool> visited(tree.m + 1, false);
    vector<int> queue;
    for(int i = 0; i <= tree.m; ++i)
    {
        if(!visited[ i ])
        {
            visited[ i ] = true;
            queue.clear();
            int queue_head = 0;
            queue.push_back( i );
            while(queue_head < queue.size())
            {
                int u = queue[ queue_head ];
                for(int j = 0; j < tree.nodes[ u ].size(); ++j)
                {
                    if(tree.is_edge_active( tree.nodes[u][j] ))
                    {
                        const Edge& te = tree.edges[ tree.nodes[u][j] ];
                        int v = te.u;
                        if(v == u)  v = te.v;
                        if(visited[ v ])    continue;
                        queue.push_back( v );
                        visited[v] = true;
                    }
                }
                ++queue_head;
            }
            if(queue_head < min_cluster)    continue;
            string path = outdir + to_string(cluster_id) + ".txt";
            ofstream fout_clu;
            loon::open_file(fout_clu, path);
            while(queue_head > 0)
            {
                --queue_head;
                int idx = queue[ queue_head ];
                fout_clu << p_left[ idx ] << ' ' << p_right[ idx ] << ' ' << p_bottom[ idx ] << ' ' << p_top[ idx ] << endl;
            }
            fout_clu.close();
            ++cluster_id;
        }
    }
    cout << "cluster_id = " << cluster_id << endl;

}

void do_clustering()
{
    if(check_cluster())   return;

    vector<bool> node_is_active(tree.m + 1, true);
    int active_cnt = tree.m + 1;
    
    vector<int> bfs_queue;
    while(active_cnt > 0)
    {
        cerr << "\n[DEBUG]: Once in while loop" << endl;
        tree.active_inc();
        vector<bool> visited(tree.m + 1, false);

        for(int i = 0; i <= tree.m; ++i)
        {
            if(node_is_active[ i ] and !visited[ i ])
            {
                visited[ i ] = true;
                bfs_queue.clear();
                int queue_head = 0;
                bfs_queue.push_back( i );
                while(queue_head < bfs_queue.size())
                {
                    int u = bfs_queue[ queue_head ];
                    for(int j = 0; j < tree.nodes[ u ].size(); ++j)
                    {
                        if(tree.is_edge_active( tree.nodes[u][j] ))
                        {
                            const Edge& te = tree.edges[ tree.nodes[u][j] ];
                            int v = te.u;
                            if(v == u)
                                v = te.v;
                            if(visited[ v ])    continue;
                            if(node_is_active[ v ])
                            {
                                bfs_queue.push_back( v );
                            }
                            else
                            {
                                tree.remove_edge( u, j );
                                --j;
                            }
                            visited[ v ] = true;
                        }
                        else
                        {
                            tree.remove_edge(u, j);
                            --j;
                        }
                    }
                    ++queue_head;
                }
                if(bfs_queue.size() < min_cluster || check_cluster( bfs_queue ))
                {// de-activate the nodes
                    for(queue_head = bfs_queue.size(); queue_head > 0;)
                        node_is_active[ bfs_queue[ --queue_head ] ] = false;
                    active_cnt -= bfs_queue.size();
                    if(bfs_queue.size() < min_cluster)
                    {
                        cerr << "[DEBUG]: de-activate nodes in small clusters: " << bfs_queue.size() << ", active_cnt = " << active_cnt << endl;
                    }
                }
            }
        }
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
    help.add_argument("Input file name of the Gomory-Hu tree");
    help.add_argument("Output directory of the clusters and their predictions");
    help.add_argument("p: Lower bound of the probability of one rectangle containing the target point");
    help.add_argument("delta: confidence parameter (close to 0 is better)");
    help.add_argument("minimum size of a cluster");

    help.check(argc, argv);
    
    infile      = argv[1];
    treefile    = argv[2];
    outdir      = string( argv[3] ) + "/";
    p           = stod( argv[4] );
    delta       = stod( argv[5] );
    min_cluster = stoi( argv[6] );
}


int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_rectangles();
    read_tree();

    size_t m = p_left.size();
    compute_P(m);
    
    loon::open_file(fout_spec, outdir + "clusters.txt");
    // do_clustering();
    debug_clustering();
    fout_spec.close();
    return 0;
}
