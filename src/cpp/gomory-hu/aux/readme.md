# generate_gomory_hu_input

Convert the rectangles to a graph in the format that `ho_ghg` requires

**Input format**:

* Each line contains a rectangle `<left> <right> <bottom> <top>`

**Output format**

* line 1: `p cut <n> <m>`
* line 2 to m + 1: `a <u> <v> 1`
    * the vertices start from 0


```
Usage: generate_gomory_hu_input <required parameters>

Please provide the following parameters in order:
    1. Input file name
    2. Output file name
```

# prob_rect_prediction_gomory_hu

Taking the rectangles, and the Gomory-Hu as input, it outputs a set of files each forming a cluster (as well as the prediction of the representative rectangle).

**Input format**:

1. The rectangle file as above
2. Gomory-Hu Tree

**Output format**:
Many files in the output directory, maybe also in the subdirectory

For each cluster, it has a cluster file `<integer>.txt` in the rectangle file format, and a solution file `<integer>.sol` that contains a rectangle, also in rectangle file format.

There is also a spec file `clusters.txt` that contains, at each line, `<integer> /path/to/the/cluster/or/solution/<integer>` (i.e., without the extension).


```
Usage: prob_rect_prediction <required parameters>

Please provide the following parameters in order:
    1. Input file name of a set of rectangles
    2. Input file name of the Gomory-Hu tree
    3. Output directory of the clusters and their predictions
    4. p: Lower bound of the probability of one rectangle containing the target 
       point
    5. delta: confidence parameter (close to 0 is better)
    6. minimum size of a cluster
```
