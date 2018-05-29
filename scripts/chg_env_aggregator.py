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
    parser.add_argument("-D", "--run_evo_dom_agg", action="store_true", help="Aggregate dominants from evolution runs.")
    parser.add_argument("-M", "--run_mape_agg", action="store_true", help="Aggregate dominants from MAPE runs.")
    parser.add_argument("-u", "--update", type=int, help="What update do we want to pull MAPE info from?")

    args = parser.parse_args()

    data_directory = args.data_directory
    benchmark = args.benchmark
    dump = os.path.join(aggregator_dump, benchmark)

    mape_update = args.update

    agg_doms = args.run_evo_dom_agg
    agg_mape = args.run_mape_agg

    print("Aggregate doms? " + str(agg_doms))
    print("Aggregate MAPE? " + str(agg_mape))

    # Get a list of all runs
    runs = [d for d in os.listdir(data_directory) if "RM" in d]
    runs.sort()

    evo_runs = [r for r in runs if "RM0" in r]
    mape_runs = [r for r in runs if "RM1" in r]

    print("EVO run cnt: " + str(len(evo_runs)))
    print("MAPE run cnt: " + str(len(mape_runs)))
    
    mkdir_p(dump)

    if (agg_doms):
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
                dom_evo_content += ",".join([run_id, sim_thresh, dist_sigs, update, str(avg_fit)]) + "\n"
        # Write out dom evo content
        with open(os.path.join(dump, "evo_dom.csv"), "w") as fp:
            fp.write(dom_evo_content)

    if (agg_mape):
        mape_content = "run_id,agent_id,update,distraction_sigs,fitness,fun_cnt,fun_used,inst_entropy,sim_thresh\n"
        print("Aggregating MAPE info from MAPE runs.")
        for run in mape_runs:
            print ("Run: {}".format(run))
            run_dir = os.path.join(data_directory, run)
            run_info = run.split("_")
            run_id = run_info[-1]
            dist_sigs = run_info[2].replace("DS", "")
            print("  RunID: " + run_id)
            print("  DistSigs: " + dist_sigs)
            output_dir = os.path.join(run_dir, "output")
            update = str(mape_update)
            pop_dir = os.path.join(output_dir, "pop_{}".format(update))
            mape_fpath = os.path.join(pop_dir, "map_{}.csv".format(update))
            file_content = None
            with open(mape_fpath, "r") as fp:
                file_content = fp.readlines()
            header = file_content[0].split(",")
            header_lu = {header[i].strip():i for i in range(0, len(header))}
            info_by_agent = {}
            file_content = file_content[1:]
            for line in file_content:
                line = line.split(",")
                agent_id = line[header_lu["agent_id"]]
                fitness = line[header_lu["fitness"]]
                func_cnt = line[header_lu["func_cnt"]]
                func_used = line[header_lu["func_used"]]
                inst_entropy = line[header_lu["inst_entropy"]]
                sim_thresh = line[header_lu["sim_thresh"]]
                if not agent_id in info_by_agent:
                    info_by_agent[agent_id] = {"trials": 0, "agg_fitness": 0, "agg_func_used": 0}
                info_by_agent[agent_id]["trials"] += 1
                info_by_agent[agent_id]["agg_fitness"] += float(fitness)
                info_by_agent[agent_id]["agg_func_used"] += float(func_used)
                info_by_agent[agent_id]["func_cnt"] = func_cnt
                info_by_agent[agent_id]["inst_entropy"] = inst_entropy
                info_by_agent[agent_id]["sim_thresh"] = sim_thresh
            for agent_id in info_by_agent:
                info = info_by_agent[agent_id]
                fitness = info["agg_fitness"] / info["trials"]
                func_used = info["agg_func_used"] / info["trials"]
                mape_content += ",".join([run_id, agent_id, update, dist_sigs, str(fitness), info["func_cnt"], str(func_used), info["inst_entropy"], info["sim_thresh"]]) + "\n"
        with open(os.path.join(dump, "mape.csv"), "w") as fp:
            fp.write(mape_content)


            

            
            

            


            



if __name__ == "__main__":
    main()