#!/usr/bin/env python

import argparse
import json
import os
import sys

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

def load_predictions(fname):
    ret = {}
    cnt = 0
    with open(fname) as fin:
        for line in fin:
            line = line.strip().split()
            if not line:    continue
            ret[cnt] = [int(each) for each in line]
            cnt += 1
    return ret

def parse_args():
    parser = argparse.ArgumentParser(description = "Convert the result of Heuristic Local Maximal Clustering to json file")
    parser.add_argument("infile", help="The input file. The solution should be in the corresponding '_sol' directory")
    parser.add_argument("-P", "--padding", type=int, default=10, help="padding to the min-max values (default: %(default)s)")    
    return parser.parse_args()

def main():
    args = parse_args()
    data = {}
    prefix, ext = os.path.splitext( args.infile )
    sol_dir = prefix + "_sol"

    predictions = load_predictions( os.path.join(sol_dir, "predictions.sol") )

    spec_fname = os.path.join(sol_dir, "spec.txt")

    if os.path.isfile(spec_fname):
        if predictions:
            data['representative_clusters'] = predictions
            if len(predictions) == 1:
                data['type'] = "rects"
                with open(spec_fname) as fin:
                    line = fin.read().strip().split()
                    data["rects"] = load_rects( os.path.join(line[1], line[0]) + ".txt" )
            else:
                data['type'] = 'clustered'
                data['clusters'] = []
                with open(spec_fname) as fin:
                    for line in fin:
                        line = line.strip().split()
                        if not line:    continue
                        data['clusters'].append( load_rects( os.path.join(line[1], line[0]) + ".txt" ) )
        else:
            print "No results!!!"
            sys.exit(0)
    else:
        data['type'] = 'rects'
        data['rects'] = load_rects( args.infile )
        if predictions:
            data['representative_clusters'] = predictions

    data['min_left']    = min_left - args.padding
    data['max_right']   = max_right + args.padding
    data['min_bottom']  = min_bottom - args.padding
    data['max_top']     = max_top + args.padding
    with open("rects.json", "wb") as fout:
        json.dump(data, fout)

if __name__ == "__main__":
    main()
