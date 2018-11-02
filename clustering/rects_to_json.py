#!/usr/bin/env python

import argparse
import json
import os

min_left = float('inf')
max_right = float('-inf')
min_bottom = float('inf')
max_top = float('-inf')

def load_rects(fname):
    global min_left
    global max_right
    global min_bottom
    global max_top
    rects = []
    with open(fname) as fin:
        for line in fin:
            line = line.strip().split()
            if not line:
                continue
            tmp = [int(each) for each in line]
            rects.append(tmp)

            if tmp[0] < min_left:   min_left = tmp[0]
            if tmp[1] > max_right:  max_right = tmp[1]
            if tmp[2] < min_bottom: min_bottom = tmp[2]
            if tmp[3] > max_top:    max_top = tmp[3]
    return rects

def load_cluster(fname):
    prefix, ext = os.path.splitext(fname)
    with open(prefix + ".sol") as fin:
        return [int(each) for each in fin.read().strip().split()]

def parse_args():
    parser = argparse.ArgumentParser(description = "Convert a list of txt files of rectangles to a json file")
    parser.add_argument("rects", nargs="+", help="a list of txt files")
    parser.add_argument("-P", "--padding", type=int, default=10, help="padding to the min-max values (default: %(default)s)")
    parser.add_argument("--end-index", type=int, default=0, help="End index, assuming that the files are \"0-<end_index>.<suffix>\". In this case, the index starts with 0. The first parameter \"rects\" will be the directory, and the second parameter \"rects\" will be the suffix w/o \".\".")
    return parser.parse_args()

def main():
    args = parse_args()
    data = {}
    if len(args.rects) == 1 or args.end_index == 1:
        data['type'] = 'rects'
        fname = os.path.join(args.rects[0], str(0) + "." + args.rects[1]) if args.end_index == 1 else args.rects[0]
        data['rects'] = load_rects( fname )
        try:
            clu = load_cluster( fname )
            if 'representative_clusters' not in data:
                data['representative_clusters'] = {}
            data['representative_clusters'][0] = clu;
        except IOError:
            pass
    else:
        data['type'] = 'clustered'
        data['clusters'] = []
        if args.end_index > 1:
            fnames = [os.path.join(args.rects[0], str(ii) + "." + args.rects[1]) for ii in xrange( args.end_index )]
        else:
            fnames = args.rects
        for i, fname in enumerate(fnames):
            data['clusters'].append( load_rects(fname) )
            try:
                clu = load_cluster( fname )
                if 'representative_clusters' not in data:
                    data['representative_clusters'] = {}
                data['representative_clusters'][i] = clu
            except IOError:
                pass
    data['min_left']    = min_left - args.padding
    data['max_right']   = max_right + args.padding
    data['min_bottom']  = min_bottom - args.padding
    data['max_top']     = max_top + args.padding
    with open("rects.json", "wb") as fout:
        json.dump(data, fout)

if __name__ == "__main__":
    main()
