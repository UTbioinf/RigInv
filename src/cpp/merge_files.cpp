#include <iostream>
#include <fstream>
#include <string>
#include <loonutil/util.h>
#include <loonutil/simpleHelp.h>

using namespace std;

string infile_dir;
char* outfile;
int entries_per_line;
int id_index;
int path_index;
string suffix;

void read_and_write_predictions(const string& fname, ofstream& fout)
{
    ifstream fin;
    loon::open_file(fin, fname);
    string line;
    while(getline(fin, line))
        fout << line << endl;
    fin.close();
}

void read_and_write()
{
    string spec = infile_dir + "spec.txt";
    ifstream fin;
    loon::open_file(fin, spec);

    ofstream fout;
    loon::open_file(fout, outfile);

    int entry_cnt = 0;
    string fid, fpath;
    string line;
    while(fin >> line)
    {
        if(entry_cnt == id_index)   fid = line;
        else if(entry_cnt == path_index)    fpath = line;
        else if(entry_cnt + 1 >= entries_per_line)
        {
            entry_cnt = 0;
            getline(fin, line);
            read_and_write_predictions( fpath + fid + suffix, fout);
            continue;
        }
        ++entry_cnt;
    }

    fin.close();
    fout.close();
}

void parse_args(int argc, char* argv[])
{
    loon::SimpleHelp help("merge_files <required parameters>");
    help.add_argument("Root directory of the files to be merged");
    help.add_argument("Output filename");
    help.add_argument("There are at least this number of entries per line");
    help.add_argument("The index of the entries for the ID of the file");
    help.add_argument("The index of the entries for the PATH of the file");
    help.add_argument("The suffix of the filename. Set as $ if no suffix");
    help.check(argc, argv);

    infile_dir  = string( argv[1] ) + loon::directory_delimiter;
    outfile     = argv[2];
    entries_per_line    = stoi( argv[3] );
    id_index    = stoi( argv[4] );
    path_index  = stoi( argv[5] );
    suffix      = string( argv[6] );
    if(suffix == "$")   suffix = "";
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    read_and_write();
    return 0;
}
