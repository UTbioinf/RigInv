#!/usr/bin/env python

import random
import argparse
import json

def gen_rects(n):
    coords = range(4)
    ret = []
    for i in xrange(n):
        tmp = [0, 0, 0, 0]
        symbols = random.randint(0, 0xF)
        for j in coords:
            tmp[j] = random.random()
            if (symbols >> j) & 1 == 0:
                tmp[j] = -tmp[j]
        if tmp[0] > tmp[1]: tmp[0], tmp[1] = tmp[1], tmp[0]
        if tmp[2] > tmp[3]: tmp[2], tmp[3] = tmp[3], tmp[2]
        ret.append(tmp)
    return ret

def parse_args():
    parser = argparse.ArgumentParser(description = "Generate random rects")
    parser.add_argument('-r', type=int, required=True, help="number of rectangles each cluster")
    parser.add_argument("-c", type=int, default=1, help="number of clusters (default: %(default)s)")
    return parser.parse_args()

def main():
    args = parse_args()
    data = {'min_left': -1,
            "max_right": 1,
            "min_bottom": -1,
            "max_top": 1}

    if args.c == 1:
        data['type'] = 'rects'
        data['rects'] = gen_rects( args.r )
    else:
        data['type'] = 'clustered'
        data['clusters'] = [gen_rects(args.r) for i in xrange(args.c)]
    with open('rects.json', "wb") as fout:
        json.dump(data, fout)


if __name__ == "__main__":
    main()
