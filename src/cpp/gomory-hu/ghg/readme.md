An algorithm for computing Gomory-Hu tree.

The running time is O(n H(n, m)), where n is the number of vertices, m is the number of edges, and H(n, m) the running time for computing the maximum flow using Hao-Orlin subroutine. This one has the best average performance comparing with the other algorithms:

* GH, GHs, GUS: O(n, S(n, m))
    * where S(n, m) is the running time for computing the maximum flow using pre-flow push

**N.B.** The code has been modified so that the program can read the graph from a file, and write the Gomory-Hu tree to a file

```
Usage: ho_ghg <infile> <outfile>
```

Input file format:

* line 1: `p cut <n> <m>`
    * `n` is the number of vertices in the graph
    * `m` is the number of edges in the graph
* line 2 to m + 1: `a <u> <v> <w>`:
    * `u`: vertex index of u
    * `v`: vertex index of v
    * `w`: edge weight
    * The vertices should be numbered from 0 to n-1, so that in the output, the vertices of the tree would have exactly the same number
    * Edge weight should be integer.
        * One can set MACRO to use double format. But that requires modifying the `CMakeLists.txt`


Output file format

* line 1: `<m>`
    * `m` number of edges. So the number of vertices is `m + 1`
* line 2 to m + 1: `<u> <v> <w>`
    * `u` and `v` are the same as above
    * `w` the min-cut between `u` and `v`
