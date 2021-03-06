# rectangle viewer

## HTML: `rect_viewer.html`

Draw rectangles or clustered rectangles. If the representative rectangle of each cluster exists, also draw it.

### Input file

`rects.json` generated by `rand_rects.py`, `rects_to_json.py`, `local_maximal_clustering_result_to_json.py`

data format:

```
{
    'represensentative_clusters': ### Optional. If exits, draw representative rectangles as well
        {
            <id>: [left, right, bottom, top], ### <id> is the index of the 'clusters'
            ...
        },
    'type' 'clustered' | 'rects',
    'min_left': <min_left>,
    'max_right': <max_right>,
    'min_bottom': <min_bottom>,
    'max_top': <max_top>,
    'clusters': ### exists if 'type' is 'clustered'. In this case, draw rectangles in random colors
        [
            [
                [left, right, bottom, top],
                ...
            ], # one cluster
            ...
        ],
    'rects': ### exits if 'type' is 'rects'. In this case, draw all rectangles in one color
        [
            [left, right, bottom, top],
            ...
        ]
}
```

### viewer

* button `reload`: reload `rects.json`. If one was to use a new coloring for the clusters, they can also click this button
* button `show-cluster`: toggle the drawing of representative rectangles for the clusters

## HTML: `show_clustering.html`

TBA

# python scripts

## py script: `gen_json.py`

TBA...

### Usage

### Output

`data.json` file


## py script: `css_colors.py`

A list of optional colors in the variable `css_colors`. This library is imported by other scripts

## py script: `do_cluster.py`

Use k-means or spectral clustering to cluster rectangles

TBA...

## py script: `rand_rects.py`

Generate random rectangles and produce `rects.json` file. This file is the input file for `rect_viewer.html`

### Usage

```
usage: rand_rects.py [-h] -r R [-c C]

Generate random rects

optional arguments:
  -h, --help  show this help message and exit
  -r R        number of rectangles each cluster
  -c C        number of clusters (default: 1)
```

### Output file

`rects.json`

* if `args.c == 1`
    * generate `rects` type, and generate `args.r` rectangles
* if `args.c > 1`
    * generate `clustered` type, and generate exactly `args.r` rectangles for each cluster

## py script: `rects_to_json.py`

Convert a list of rectangle files (as well as their representative clusters if exists) to `rects.json`, as input to `rect_viewer.html`

### Usage

```
usage: rects_to_json.py [-h] [-P PADDING] [--end-index END_INDEX]
                        rects [rects ...]

Convert a list of txt files of rectangles to a json file

positional arguments:
  rects                 a list of txt files

optional arguments:
  -h, --help            show this help message and exit
  -P PADDING, --padding PADDING
                        padding to the min-max values (default: 10)
  --end-index END_INDEX
                        End index, assuming that the files are
                        "0-<end_index>.<suffix>". In this case, the index
                        starts with 0. The first parameter "rects" will be the
                        directory, and the second parameter "rects" will be
                        the suffix w/o ".".
```

### Input file

For each rectangle file, suppose that the file name is of the format `<prefix>.<ext>`

The rectangle file has the rectangle format, i.e., `<left> <right> <bottom> <top>` each line

The cluster file (if exists) has file name `<prefix>.sol`, and contains only one line of the rectangle format

### Output file

* if `end-index == 1` or `len(rects) == 1`: convert the rectangle file to `rects` type
    * load `representative_clusters` if it exists
* else, convert the rectangles files (in either of the two format) to `clustered` type
    * load `representative_clusters` for each rectangle file if it exists

## py script: `local_maximal_clustering_result_to_json.py`

Convert all the output of Heuristic Local Maximal Clustering to `rects.json` file, as input to `rect_viewer.html`

### Usage

```
usage: local_maximal_clustering_result_to_json.py [-h] [-P PADDING] infile

Convert the result of Heuristic Local Maximal Clustering to json file

positional arguments:
  infile                The input file. The solution should be in the
                        corresponding '_sol' directory

optional arguments:
  -h, --help            show this help message and exit
  -P PADDING, --padding PADDING
                        padding to the min-max values (default: 10)
```

### Input file

The input directory structure should be as follows

```
|- <infile>.<ext>:   # the input file of rectangles
|- <infile>_sol/
    |- predictions.sol:  # rep rects for each cluster. The file is in rectangle format
    |- spec.txt:         # format at each line: <sub_part_id> <sub_part_id_path>
    |- <sub_part_id_path> paths ... /
        |- <sub_part_id>.txt        # in rectangle format

```

### Output file

* if `spec.txt` exists
    * if there's only one prediction in `predictions.sol`
        * convert the unique `<sub_part_id>.txt` to `rects` type
    * else if there are more than one predictions
        * convert all the `<sub_part_id>.txt`s to `clustered` type.
    * else
        * print "No results!!!"
* else, convert `infile` to `rects` type.
    * if there are predictions in `predictions.sol` file, also generate that representative cluster

