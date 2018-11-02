#!/usr/bin/env python2

import json
import random
import argparse
import tqdm
from css_colors import css_colors

colors = css_colors
n_colors = len(colors)

disjoint_rect = []
rect = []

def append_rect(x1, x2, y1, y2, color_name):
    if x1 > x2:
        x1, x2 = x2, x1
    if x1 == x2:
        return
    if y1 > y2:
        y1, y2 = y2, y1
    if y1 == y2:
        return
    rect.append({"rect": {"min_x": x1, "max_x": x2, "min_y": y1, "max_y": y2}, "color": color_name})

def add_rect(x1, x2, y1, y2):
    color_id = len(rect) % (n_colors - 1) + 1
    append_rect(x1, x2, y1, y2, colors[ color_id ])

def gen_rect():
    min_width = 0.005
    while True:
        width = random.random() * 0.2
        if width > min_width:
            break
    while True:
        height = random.random() * 0.2
        if height > min_width:
            break
    x1 = random.random() * 0.8
    y1 = random.random() * 0.8
    return x1, x1 + width, y1, y1 + height

def check_epsilon(a, b, epsilon):
    x1 = max(a[0], b["min_x"])
    x2 = min(a[1], b["max_x"])
    if x1 > x2:
        return False
    y1 = max(a[2], b["min_y"])
    y2 = min(a[3], b["max_y"])
    if y1 > y2:
        return False

    area_a = (a[1] - a[0]) * (a[3] - a[2])
    area_b = (b["max_x"] - b["min_x"]) * (b["max_y"] - b["min_y"])
    area_intersect = (x2 - x1) * (y2 - y1)
    return area_intersect > epsilon * (area_a + area_b - area_intersect)

def gen_rect_around(k, epsilon):
    with tqdm.tqdm(total = len(disjoint_rect)) as pbar:
        for i, r in enumerate(disjoint_rect):
            color_name = colors[ i % (n_colors - 2) + 2 ]
            K = k
            while K > 0:
                tmp_rect = gen_rect()
                if check_epsilon(tmp_rect, r, epsilon):
                    append_rect(tmp_rect[0], tmp_rect[1], tmp_rect[2], tmp_rect[3], color_name)
                    K -= 1
            pbar.update(1)

def check_disjoint(tmp_rect):
    for each in disjoint_rect:
        if tmp_rect[1] <= each["min_x"]:
            continue
        if tmp_rect[0] >= each["max_x"]:
            continue
        if tmp_rect[3] <= each["min_y"]:
            continue
        if tmp_rect[2] >= each["max_y"]:
            continue
        return False
    return True

def gen_disjoint_rect(n):
    while n > 0:
        tmp_rect = gen_rect()
        if check_disjoint(tmp_rect):
            disjoint_rect.append({
                        "min_x": tmp_rect[0], 
                        "max_x": tmp_rect[1], 
                        "min_y": tmp_rect[2], 
                        "max_y": tmp_rect[3]
                    }
                )
            n -= 1

def write_disjoint(n):
    gen_disjoint_rect(n)
    with open("disjoint.json", "wb") as fout:
        json.dump(disjoint_rect, fout)

def read_disjoint():
    global disjoint_rect
    with open("disjoint.json", "rb") as fin:
        disjoint_rect = json.load(fin)

def parse_args():
    parser = argparse.ArgumentParser(description = "Generate random rectangles")
    parser.add_argument("-n", "--number-disjoint", type=int, help="Set the number of disjoint rectangles. If not set, load from 'disjoint.json'")
    parser.add_argument("-k", type=int, default=0, help="Set the number of rectangles for each disjoint rectangles (default: %(default)s)")
    parser.add_argument("-e", "--epsilon", type=float, default=0.5, help="Set epsilon (default: %(default)s)")
    return parser.parse_args()

def main():
    args = parse_args()
    if args.number_disjoint:
        write_disjoint( args.number_disjoint )
    else:
        read_disjoint()
    gen_rect_around(args.k, args.epsilon)

    with open("data.json", "wb") as fout:
        json.dump({"marker": disjoint_rect,
                "rects": rect}, fout)


if __name__ == "__main__":
    main()
