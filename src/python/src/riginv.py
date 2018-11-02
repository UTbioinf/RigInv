#!/usr/bin/env python

import sys
import os
import argparse
import multiprocessing
import subprocess
import logging

with open(os.path.join( os.path.dirname(os.path.abspath(os.path.realpath(__file__))), "version.txt" )) as f_version:
    __version__ = f_version.read().strip()

all_steps = ["extract", "val_seg", "type2", "cluster2"]
all_steps_key = dict(zip(all_steps, range(len(all_steps))))

boolTo01 = {True: "1", False: "0"}

def id_path_iter(args):
    with open(os.path.join(args.working_dir, "intermediate_results", "spec.txt")) as fin:
        for line in fin:
            line = line.strip().split()
            if not line: continue
            yield {"id": line[0], # ref_id, str()
                   "ref_name": line[1],
                   "ref_len": line[2], # str()
                   "id_path": line[3]
                   }

def part_id_path_iter(contig, analysis_type):
    with open(os.path.join(contig["id_path"], contig["id"] + "_" + analysis_type + "_parts", "spec.txt")) as fin:
        for line in fin:
            line = line.strip().split()
            if not line: continue
            yield {"part_id": line[0],
                   "part_id_path": line[1]}

def run_extract_bam(args):
    """Extract bam file and sort it
    1. Extract the bam file
        Input file: bam file
        Output files:
            <working_directory>
                |- intermediate_results/
                    |- spec.txt
                    |- inc.mk
                    |- <id> paths ... /  (for each <id>)
                        |- <id>.txt

    2. Sort each <id>.txt as <id>.sorted.txt
        Input file:     each <id path>/<id>.txt
        Output file:    each <id path>/<id>.txt
    """

    sys.path.append(args.aux_dir)
    import bamExtractor
    # 1. Extract the bam file to the directory `<working_dir>/brief_alignments/`
    args.logger.info("Extract BAM")
    bamExtractor.main(['-b', args.bam, "-d", args.working_dir, "-m", "--max-nfiles", str(args.max_nfiles), "--cache-size", str(args.cache_size)])

    # 2. Sort each `<id>.txt` as `<id>.sorted.txt`
    args.logger.info("Sort extracted BAM")
    for contig in id_path_iter(args):
        subprocess.check_call([os.path.join(args.aux_dir, "sort_brief_alignment"), 
                    os.path.join( contig["id_path"],  contig["id"] + ".txt" ),
                    os.path.join( contig["id_path"],  contig["id"] + ".sorted.txt")
                ])
    ## subprocess.check_call(["make", "-f", os.path.join(args.makefile_dir, "intermediate_results.make"),
    ##         "-j", args.nproc,
    ##         "INC_FNAME={}".format(os.path.join(args.working_dir, "intermediate_results", "inc.mk")),
    ##         "ALN_SORTBIN={}".format(os.path.join(args.aux_dir, "intermediate_results"))
    ##         ])

def run_concordant_analysis(args):
    """Run concordant analysis for each contigs/scaffolds
    
    Input file:     each <id path>/<id>.sorted.txt
    Output file:    each <id path>/<id>.concordant.txt
    """
    args.logger.info("Concordant analysis")
    for contig in id_path_iter(args):
        subprocess.check_call([os.path.join(args.aux_dir, "concordant_aln_analysis"),
                    contig["ref_len"], str(args.ca_min_cutoff), str(args.ca_min_overlap), 
                    str(args.ca_min_length), str(args.min_quality),
                    os.path.join(contig["id_path"], contig["id"] + ".sorted.txt"),
                    os.path.join(contig["id_path"], contig["id"] + ".concordant.txt"),
                    str(args.ca_min_coverage), str(args.ca_min_coverage_ratio)
                ])

def run_discordant_type1(args):
    """Run discordant type1 analysis

    Input file:     each <id path>/<id>.sorted.txt
    Output file:    each <id path>/<id>.type1.txt
    """
    args.logger.info("Find type 1 inversions")
    for contig in id_path_iter(args):
        subprocess.check_call([os.path.join(args.aux_dir, "discordant_type1"),
                    os.path.join(contig["id_path"], contig["id"] + ".sorted.txt"),
                    os.path.join(contig["id_path"], contig["id"] + ".type1.txt"),
                    str(args.t1_delta), str(args.t1_percent), str(args.min_quality), str(args.max_allowed_overlap)
                ])

def run_discordant_type2(args):
    """Run discordant type2 analysis, partition them into connected components, and do clustering
    1. Discordant type2 analysis
        Input file:     each <id path>/<id>.sorted.txt
        Output file:    each <id path>/<id>.type2.txt

    2. Partition into connected components
        Input file:     each <id path>/<id>.type2.txt
        Output files:
            <id_path>/<id>_type2_parts/
                |- spec.txt
                |- <part_id> paths ... /  (for each <part_id>)
                    |- <part_id>.txt

    3. Clustering:
        Input file:     each <part_id_path>/<part_id>.txt
        Output file:
            <part_id_path>/
                |- <part_id>_sol/
                    |- predictions.sol
                    |- spec.txt (may exist)
                    |- <sub_part_id> paths ... /
                        |- <sub_part_id>.txt

    4. Refine the results
        Input file:         each <id>.concordant.txt
        Input directory:    each <id>_type2_parts/
        Output file:        each <id>_type2.sol
    """
    args.logger.info("Find Type 2 inversions")
    import util
    for contig in id_path_iter(args):
        if "type2" in args.run_steps:
            # 1. Discordant type2 analysis
            args.logger.info("    Analyze contig {}".format(contig["id"]))
            args.logger.info("        Find all type 2 rectangles")
            subprocess.check_call([os.path.join(args.aux_dir, "discordant_type2"),
                        os.path.join(contig["id_path"], contig["id"] + ".sorted.txt"),
                        os.path.join(contig["id_path"], contig["id"] + ".type2.txt"),
                        str(args.min_quality), str(args.t2_min_extension),
                        str(args.t2_ksi), str(args.max_adj_distance)
                    ])
        if "cluster2" in args.run_steps:
            # 2. Partition into connected components
            args.logger.info("        Partition into connected components")
            type2_parts_dir = os.path.join(contig["id_path"], contig["id"] + "_type2_parts")
            util.makedir( type2_parts_dir )
            subprocess.check_call([os.path.join(args.aux_dir, "partition_disconnected_rects"),
                        os.path.join( contig["id_path"], contig["id"] + ".type2.txt"),
                        os.path.join( type2_parts_dir ),
                        str(args.min_rectangles), str(args.min_rect_sides)
                    ])
            # 3. Clustering
            args.logger.info("        Clustering")
            for component in part_id_path_iter(contig, "type2"):
                sol_dir = os.path.join(component["part_id_path"], component["part_id"] + "_sol")
                util.makedir( sol_dir )
                subprocess.check_call(["rm", "-f", os.path.join(sol_dir, "spec.txt"), os.path.join(sol_dir, "predictions.sol")])
                subprocess.check_call([os.path.join(args.aux_dir, "cluster_by_maximal_coverage"),
                            os.path.join(component["part_id_path"], component["part_id"] + ".txt"), 
                            sol_dir,
                            str(args.prob_contain_rect), str(args.confidence),
                            str(args.min_rectangles), str(args.min_rect_sides),
                            str(args.remove_portion), boolTo01[ args.compute_rect_first ],
                            boolTo01[ not args.keep_overlapping_predictions ],
                            str(args.min_brace_coverage),
                            str(args.min_brace_imbalance_ratio)
                        ])
            # 4. Refinement
            if args.t2_no_refine:
                args.logger.info("        Merge predictions to a single file")
                subprocess.check_call([os.path.join(args.aux_dir, "merge_files"),
                            os.path.join(contig["id_path"], contig["id"] + "_type2_parts"),
                            os.path.join(contig["id_path"], contig["id"] + "_type2.sol"),
                            "2", "0", "1", os.path.join("_sol", "predictions.sol")
                        ])
            else:
                args.logger.info("        Refinement")
                subprocess.check_call([os.path.join(args.aux_dir, "refine_type2"),
                            os.path.join(contig["id_path"], contig["id"] + ".concordant.txt"),
                            os.path.join(contig["id_path"], contig["id"] + "_type2_parts"),
                            os.path.join(contig["id_path"], contig["id"] + "_type2.sol")
                        ])


def run(args):
    if "extract" in args.run_steps:
        run_extract_bam(args)

    
    if "val_seg" in args.run_steps:
        run_concordant_analysis(args)
    # run_discordant_type1(args) # not useful for now

    if "type2" in args.run_steps or "cluster2" in args.run_steps:
        run_discordant_type2(args)
    # run_discordant_type3(args) # not implemented yet

def parse_args():
    parser = argparse.ArgumentParser(description = "RigInv: Rigorous Inversion Detection (Slow version)")
    parser.add_argument("--start-from", default="extract", choices = all_steps, help="Start from this chosen step. (default: %(default)s)")
    parser.add_argument("--steps", choices = all_steps, action='append', help="If set, --step-from will be ignored. And only these steps will run")
    parser.add_argument("-d", "--working-dir", default="riginv", help="Working directory (default: %(default)s)")
    parser.add_argument("-j", "--nproc", default=multiprocessing.cpu_count(), type=int, help="Number of processes (default: %(default)s)")
    parser.add_argument("--max-nfiles", default=512, type=int, help="Keep at most this number of files open (default: %(default)s)")
    parser.add_argument("--cache-size", default=1048576, type=int, help="Hold at most this number of bytes for each file before flushing to the disk (default: %(default)s)")

    # extract
    parser.add_argument("-b", "--bam", help="BAM/SAM file to be extracted. This is only used in the 'extract' step")
    
    # for concordant analysis, and all discordant types analysis
    parser.add_argument("--min-quality", default=0, type=int, help="Minimum mapping quality in consideration (default: %(default)s)")
    # for all discordant types analysis
    parser.add_argument("--max-allowed-overlap", default=5, type=int, help="Maximum allowed overlap for inversion analysis (used for comparison) (default: %(default)s)")
    parser.add_argument("--max-adj-distance", default=1000, type=int, help="Maximum allowed distance between two adjacent alignments that determine a candidate inversion (default: %(default)s)")

    # concordant analysis
    parser.add_argument("--ca-min-cutoff", default=3, type=int, help="[concordant analysis]: Minimum cutoff for identifying one coverage for a nucleotide (default: %(default)s)")
    parser.add_argument("--ca-min-overlap", default=10, type=int, help="[concordant analysis]: Minimum overlap for identifying consecutive validated segment (default: %(default)s)")
    parser.add_argument("--ca-min-length", default=100, type=int, help="[concordant analysis]: Minimum length of an alignment in consideration (default: %(default)s)")
    parser.add_argument("--ca-min-coverage", default=10, type=int, help="[concordant analysis]: Minimum coverage for an alignment to be kept (default: %(default)s)")
    parser.add_argument("--ca-min-coverage-ratio", default=0.1, type=float, help="[concordant analysis]: Minimum coverage ratio for an alignment to be kept (default: %(default)s)")
    
    # discordant type 1
    parser.add_argument("--t1-delta", default=100, type=int, help="[discordant type 1]: Maximum allowed difference between the inversions of REF and QRY (default: %(default)s)")
    parser.add_argument("--t1-percent", default=0.8, type=float, help="[discordant type 1]: Minimum required percentage of coverage for the inversions of ref REF QRY. Value [0, 1]. (default: %(default)s)")

    # discordant type 2
    parser.add_argument("--t2-ksi", default=10, type=int, help="[discordant type 2]: Allowed error for assessing break points (used for output breakpoints) (default: %(default)s)")
    parser.add_argument("--t2-min-extension", default=100, type=int, help="[discordant type 2]: Minimum required non-overlapping length when two split reads overlap w.r.t. their input sequence (default: %(default)s)")

    # refine type 2
    parser.add_argument("--t2-no-refine", action="store_true", help="Don't refine type 2 predictions.")

    # discordant type 3
    # skpped

    # partition step
    parser.add_argument("--min-rectangles", default=10, type=int, help="Keep only the connected components that has at least this number of rectangles (default: %(default)s)")
    parser.add_argument("--min-rect-sides", default=0, type=int, help="Keep only the rectangles whose side lengths are both smaller than this value. (Set as 0 if one wants to keep all the rectangles) (default: %(default)s)")

    # clustering step
    parser.add_argument("--prob-contain-rect", default=0.8, type=float, help="The lower bound of the probability of one rectangle containing the target point in a cluster (default: %(default)s)")
    parser.add_argument("--confidence", default=0.001, type=float, help="How much confidence one wants the prediction to be correct (We use 1 - <confidence> in the implementation. So lower value means higher confidence). Lower confidence smaller range of the prediction, but higher risk of missing the breakpoints). (default: %(default)s)")
    parser.add_argument("--remove-portion", default=0.0, type=float, help="Remove this portion of largest (w.r.t. the largest side) rectangles before doing clustering (default: %(default)s)")
    parser.add_argument("--compute-rect-first", action="store_true", help="If set, before doing clustering, the first step is to try to compute a representative rectangle for all rectangles. If such a representative rectangle exists, then stop. Otherwise, do clustering")
    parser.add_argument("--keep-overlapping-predictions", action="store_true", help="If set, retain prediction that overlap with one another. Otherwise, keep singletons only")
    parser.add_argument("--min-brace-coverage", type=int, default=5, help="If the number of alignments less than this number, the inversion will not be considered (default: %(default)s)")
    parser.add_argument("--min-brace-imbalance-ratio", type=float, default=0, help="If min(lbrace/rbrace, rbrace/lbrace) is less than this value, the inversion will not be considered (default: %(default)s)")

    parser.add_argument("-V", "--version", action="version", version="%(prog)s " + __version__)
    parser.add_argument("--log", action="store_true", help="save log to file [%(prog)s.log] instead of printing in the console")
    parser.add_argument("--log-level", default="info", choices=["debug", "info", "warning", "error", "critical"], help="Set log level. (default: %(default)s)")
    return parser.parse_args()

def main():
    args = parse_args()
    
    
    if args.steps:
        args.run_steps = set(args.steps)
    else:
        args.run_steps = set(all_steps[ all_steps_key[ args.start_from ]: ])

    riginv_rootdir= os.path.normpath( os.path.join( os.path.dirname(os.path.abspath(os.path.realpath(__file__))), "..") )
    args.aux_dir = os.path.join(riginv_rootdir, "libexec", "bin")
    args.makefile_dir = os.path.join(riginv_rootdir, "libexec", "makefiles")
    args.pylib_dir = os.path.join(riginv_rootdir, "lib", "python", "riginv_lib")

    sys.path.append( args.pylib_dir )

    log_levels = {"debug": logging.DEBUG,
                  "info": logging.INFO,
                  "warning": logging.WARNING,
                  "error": logging.ERROR,
                  "critical": logging.CRITICAL}
    if args.log:
        import util
        util.makedir( args.working_dir )
        logging.basicConfig(filename = os.path.join(args.working_dir, "riginv.log"), format="[%(asctime)s] [%(levelname)s]: %(message)s", level=log_levels[ args.log_level ])
    else:
        logging.basicConfig(format="[%(asctime)s] [%(levelname)s]: %(message)s", level=log_levels[ args.log_level ])

    args.logger = logging.getLogger()

    args.logger.info("Start")
    args.logger.debug("Run steps: {}".format( ", ".join(args.run_steps) ))
    run(args)
    args.logger.info("Done!")

if __name__ == "__main__":
    main()
