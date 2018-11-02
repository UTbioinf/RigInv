#!/usr/bin/env python

import argparse
import sys
import os
import numpy as np
import sklearn
import sklearn.cluster

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "lib", "python", "riginv_lib"))

import util

def rect_jaccard(rect1, rect2):
    x1 = max(rect1[0], rect2[0])
    x2 = min(rect1[1], rect2[1])
    if x1 > x2: return 0
    y1 = max(rect1[2], rect2[2])
    y2 = min(rect1[3], rect2[3])
    if y1 > y2: return 0
    area_a = (rect1[1] - rect1[0]) * (rect1[3] - rect1[2])
    area_b = (rect2[1] - rect2[0]) * (rect2[3] - rect2[2])
    area_int = (x2 - x1) * (y2 - y1)
    return float(area_int) / (area_a + area_b - area_int)

def rect_min_jaccard(rect1, rect2):
    x1 = max(rect1[0], rect2[0])
    x2 = min(rect1[1], rect2[1])
    if x1 > x2: return 0
    y1 = max(rect1[2], rect2[2])
    y2 = min(rect1[3], rect2[3])
    if y1 > y2: return 0
    area_a = (rect1[1] - rect1[0]) * (rect1[3] - rect1[2])
    area_b = (rect2[1] - rect2[0]) * (rect2[3] - rect2[2])
    area_int = (x2 - x1) * (y2 - y1)
    return float(area_int) / min(area_a, area_b)

def clustering_score(X, centroid, labels, score_func):
    s = 0
    for i, label in enumerate(labels):
        s += score_func(X[i], centroid[label])
    return s

def k_means_score(centroid, score_func):
    s = 0
    n = len(centroid)
    for i in xrange(n):
        for j in xrange(i + 1, n):
            s += score_func(centroid[i], centroid[j])
    return s

def run_and_write_k_means(X, directory):
    best_centorid = []
    best_labels = []
    best_score = float('-inf')
    N = X.shape[0] + 1
    for k in xrange(1, N):
        centroid, labels, intertia = sklearn.cluster.k_means(X, k)

        score_func = rect_min_jaccard
        score1 = clustering_score(X, centroid, labels, score_func)
        score2 = k_means_score(centroid, score_func)
        score = score1 / len(labels) - score2 / len(centroid)
        if score > best_score:
            best_score = score
            best_centorid = centroid
            best_labels = labels
    K = len(best_centorid)
    with open(os.path.join(directory, "k.txt"), "w") as fout:
        fout.write("{}\n".format(K))
    files = [open(os.path.join(directory, "{}.in".format(i)), "w") for i in xrange(K)]
    for i in xrange(N-1):
        files[ best_labels[i] ].write("{} {} {} {}\n".format(X[i][0], X[i][1], X[i][2], X[i][3]))
    for each_fp in files:
        each_fp.close()

def run_and_write_k_means_with_k(X, directory, k):
    if k >= X.shape[0]:
        with open(os.path.join(directory, "k.txt"), "w") as fout:
            fout.write("0\n")
        return
    centroid, labels, intertia = sklearn.cluster.k_means(X, k)
    with open(os.path.join(directory, "k.txt"), "w") as fout:
        fout.write("{}\n".format(k))
    files = [open(os.path.join(directory, "{}.in".format(i)), "w") for i in xrange(k)]
    for i in xrange(X.shape[0]):
        files[ labels[i] ].write("{} {} {} {}\n".format(X[i][0], X[i][1], X[i][2], X[i][3]))
    for each_fp in files:
        each_fp.close()

def run(args):
    X = np.loadtxt(args.input_file, np.uint64) # each row is Lx Ly Rx Ry, like the "left right bottom top" of a rectangle
    if args.k:
        run_and_write_k_means_with_k(X, args.directory, args.k)
    else:
        run_and_write_k_means(X, args.directory)

def parse_args( argv = None ):
    parser = argparse.ArgumentParser(prog="k_means", description = "k means clustering algorithm")
    parser.add_argument("-k", type=int, help = "Set k for k-means")
    parser.add_argument("-i", "--input-file", help="Input file name")
    parser.add_argument("-d", "--directory", default=".", help="directory of the output file. If not set, it will be the current directory")
    return parser.parse_args(argv)

def main(argv = None):
    args = parse_args( argv )
    run(args)

if __name__ == "__main__":
    main()
