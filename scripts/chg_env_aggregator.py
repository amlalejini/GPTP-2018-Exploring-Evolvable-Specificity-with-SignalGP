'''
aggregator.py

This script does the following:
    - Aggregate final fitness data for changing environment GPTP experiment.
    - Aggregate MAP-Elites information for changing environment GPTP experiment. 
'''

import argparse, os, copy, errno

aggregator_dump = "./aggregated_data"

def mkdir_p(path):
    """
    This is functionally equivalent to the mkdir -p [fname] bash command
    """
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def main():
    parser = argparse.ArgumentParser(description="Data aggregation script.")
    parser.add_argument("data_directory", type=str, help="Target experiment directory.")
    parser.add_argument("benchmark", type=str, help="What benchmark is this?")

    args = parser.parse_args()

    data_directory = args.data_directory
    benchmark = args.benchmark
    dump = os.path.join(aggregator_dump, benchmark)

    # Get a list of all runs
    runs = [d for d in os.listdir(data_directory) if "RM" in d]
    runs.sort()

    evo_runs = [r for r in runs if "RM0" in r]
    mape_runs = [r for r in runs if "RM1" in r]

    print("=== EVO runs: ===\n" + str(evo_runs))
    print("=== MAPE runs: ===\n" + str(mape_runs))
    
    mkdir_p(dump)

    # Aggregate EVO runs
    dom_evo_content = "run_id,sim_thresh,distraction_sigs,update,fitness\n"
    print("Aggregating dominant multi-trial fitness for evolution runs")
    for run in evo_runs:
        print("Run: {}".format(run))
        run_dir = os.path.join(data_directory, run)
        run_info = run.split("_")
        run_id = run_info[-1]
        sim_thresh = run_info[1].replace("BT", "")
        dist_sigs = run_info[2].replace("DS", "")
        print("  RunID: " + run_id)
        print("  SimThresh: " + sim_thresh)
        print("  DistSigs: " + dist_sigs)
        output_dir = os.path.join(run_dir, "output")
        # Collect pop snapshots
        pops = [d for d in os.listdir(output_dir) if "pop" in d]
        pops.sort()
        for pop in pops:
            print ("  Aggregating {}".format(pop))
            update = pop.split("_")[-1]
            dom_fpath = os.path.join(output_dir, pop, "dom_{}.csv".format(update))
            file_content = None
            with open(dom_fpath, "r") as fp:
                file_content = fp.readlines()
            header = file_content[0].split(",")
            header_lu = {header[i].strip():i for i in range(0, len(header))}
            file_content = file_content[1:]
            trials = 0
            fit_agg = 0
            for line in file_content:
                line = line.split(",")
                fit_agg += float(line[header_lu["fitness"]])
                trials += 1
            avg_fit = fit_agg / float(trials)
            dom_evo_content += ",".join([run_id, sim_thresh, dist_sigs, update, avg_fit]) + "\n"
    # Write out dom evo content
    with open(os.path.join(dump, "evo_dom.csv"), "w") as fp:
        fp.write(dom_evo_content)

        








    

    args = parser.parse_args()


if __name__ == "__main__":
    main()