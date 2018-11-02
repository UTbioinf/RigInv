#!/usr/bin/env python2

import tqdm
import json
import argparse
import sklearn
import scipy
import scipy.cluster
from sklearn.cluster import spectral_clustering
import numpy as np
from scipy.sparse import dok_matrix
from css_colors import css_colors

colors = css_colors
n_colors = len(colors)

def compute_similarty(a, b):
    x1 = max(a["rect"]["min_x"], b["rect"]["min_x"])
    x2 = min(a["rect"]["max_x"], b["rect"]["max_x"])
    if x1 > x2:
        return -1
    y1 = max(a["rect"]["min_y"], b["rect"]["min_y"])
    y2 = min(a["rect"]["max_y"], b["rect"]["max_y"])
    if y1 > y2:
        return -1
    area_a = (a["rect"]["max_x"] - a["rect"]["min_x"]) * (a["rect"]["max_y"] - a["rect"]["min_y"])
    area_b = (b["rect"]["max_x"] - b["rect"]["min_x"]) * (b["rect"]["max_y"] - b["rect"]["min_y"])
    area_int = (x2 - x1) * (y2 - y1)
    # return area_int / (area_a + area_b - area_int)
    return area_int / min(area_a, area_b)

def gen_similarity_matrix( data ):
    n = len(data)
    S = dok_matrix((n, n), dtype=np.float64)
    for i in xrange(n):
        for j in xrange(i+1, n):
            score = compute_similarty(data[i], data[j])
            if score > -0.5:
                S[i, j] = S[j, i] = score
    return S

def gen_sample_feature_matrix( data ):
    n = len(data)
    S = np.zeros((n, 4))
    for i, each in enumerate(data):
        r = each["rect"]
        x1, x2, y1, y2 = r["min_x"], r["max_x"], r["min_y"], r["max_y"]
        S[i, :] = [x1, x2, y1, y2]
    return S

def rect_jaccard(x1, x2, y1, y2, xx1, xx2, yy1, yy2):
    x_1 = max(x1, xx1)
    x_2 = min(x2, xx2)
    if x_1 > x_2:   return 0
    y_1 = max(y1, yy1)
    y_2 = min(y2, yy2)
    if y_1 > y_2:   return 0
    area_a = (x2 - x1) * (y2 - y1)
    area_b = (xx2 - xx1) * (yy2 - yy1)
    area_int = (x_2 - x_1) * (y_2 - y_1)
    return area_int / (area_a + area_b - area_int)

def rect_min_jaccard(x1, x2, y1, y2, xx1, xx2, yy1, yy2):
    x_1 = max(x1, xx1)
    x_2 = min(x2, xx2)
    if x_1 > x_2:   return 0
    y_1 = max(y1, yy1)
    y_2 = min(y2, yy2)
    if y_1 > y_2:   return 0
    area_a = (x2 - x1) * (y2 - y1)
    area_b = (xx2 - xx1) * (yy2 - yy1)
    area_int = (x_2 - x_1) * (y_2 - y_1)
    return area_int / min(area_a, area_b)

def rect_max_jaccard(x1, x2, y1, y2, xx1, xx2, yy1, yy2):
    x_1 = max(x1, xx1)
    x_2 = min(x2, xx2)
    if x_1 > x_2:   return 0
    y_1 = max(y1, yy1)
    y_2 = min(y2, yy2)
    if y_1 > y_2:   return 0
    area_a = (x2 - x1) * (y2 - y1)
    area_b = (xx2 - xx1) * (yy2 - yy1)
    area_int = (x_2 - x_1) * (y_2 - y_1)
    return area_int / max(area_a, area_b)

def clustering_score(data, centroid, labels, score_func):
    s = 0
    for i, label in enumerate(labels):
        s += score_func( data[i]["rect"]["min_x"], data[i]["rect"]["max_x"],
                         data[i]["rect"]["min_y"], data[i]["rect"]["max_y"],
                         centroid[ label ][0], centroid[ label ][1],
                         centroid[ label ][2], centroid[ label ][3])
    return s

def k_means_score(centroid, score_func):
    s = 0
    n = len(centroid)
    for i in xrange(n):
        for j in xrange(i+1, n):
            s += score_func( centroid[i][0], centroid[i][1], centroid[i][2], centroid[i][3],
                             centroid[j][0], centroid[j][1], centroid[j][2], centroid[j][3])
    return s

def k_means(data, args):
    X = gen_sample_feature_matrix( data )
    if args.num_cluster:
        best_centroid, best_labels, intertia = sklearn.cluster.k_means(X, args.num_cluster)
    else:
        best_centroid = []
        best_labels = []
        best_score = float('-inf') 
        if args.max_cluster and args.max_cluster <= len(data):
            N = args.max_cluster + 1
        else:
            N = len(data) + 1
        for k in tqdm.tqdm(xrange(1, N)):
            centroid, labels, intertia = sklearn.cluster.k_means(X, k)

            score_func = rect_min_jaccard
            score1 = clustering_score(data, centroid, labels, score_func)
            score2 = k_means_score( centroid, score_func)
            score = score1 / len(labels) - score2 / len(centroid)
            if score > best_score:
                best_score = score
                best_centroid = centroid
                best_labels = labels
    best_centroid = [{"min_x": each[0],
                     "max_x": each[1],
                     "min_y": each[2],
                     "max_y": each[3]} for each in best_centroid] 
    print "{} clusters".format( len(best_centroid) )
    return best_labels, best_centroid

def spectral(data, args):
    similarity_matrix = gen_similarity_matrix( data )
    labels = spectral_clustering(similarity_matrix, args.num_cluster, eigen_solver=args.eigen_solver, n_init=args.number_k_means)
    return labels, []

def dbscan(data, args):
    # not good enough
    X = gen_sample_feature_matrix( data )
    core_samples, labels = sklearn.cluster.dbscan(X, 0.05, 3)
    print core_samples
    print labels
    return labels, []

def parse_args():
    parser = argparse.ArgumentParser(description = "Do Spectral Clustering")
    subparsers = parser.add_subparsers(title="clustering algorithm", dest = "subcmd")

    parser_kmeans = subparsers.add_parser("kmeans", help="Use kmeans")
    parser_kmeans.add_argument("-c", "--num-cluster", type=int, help="Set number of clusters (if not set, run adaptive k-means)")
    parser_kmeans.add_argument("-m", "--max-cluster", type=int, help="Set the maximum number of clusters")

    parser_spectral = subparsers.add_parser("spectral", help="Use Spectral Clustering")
    parser_spectral.add_argument("-c", "--num-cluster", type=int, default=10, help="Set number of clusters (default: %(default)s)")
    parser_spectral.add_argument("-m", "--number-k-means", type=int, default=10, help="Set the number of times to run k-means (default: %(default)s)")
    # 'lobpcg' crashed too many times
    parser_spectral.add_argument("-s", "--eigen-solver", choices=['arpack', 'lobpcg'], help="Choose eigen solver (lobpcg is good but vulnerable to the number of clusters)")
    return parser.parse_args()

def main():
    args = parse_args()
    with open("data.json", "rb") as fin:
        data = json.load(fin)
    data = data["rects"]

    algorithms = {"kmeans": k_means,
                  "spectral": spectral}
    
    labels, centroid = algorithms[args.subcmd](data, args)
    
    for i, each in enumerate(data):
        each["color"] = colors[ labels[i] % (n_colors - 2) + 2 ]
    with open("cluster.json", "wb") as fout:
        json.dump({"data": data, "centroid": centroid, "labels": labels.tolist()}, fout)
    


if __name__ == "__main__":
    main()
