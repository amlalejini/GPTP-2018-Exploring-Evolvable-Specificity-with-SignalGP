#ifndef ALIFE2018_EXPERIMENT_H
#define ALIFE2018_EXPERIMENT_H

// @includes
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <functional>

#include "base/Ptr.h"
#include "base/vector.h"
#include "control/Signal.h"
#include "Evolve/World.h"
#include "Evolve/Resource.h"
#include "Evolve/SystematicsAnalysis.h"
#include "Evolve/World_output.h"
#include "games/Othello.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/InstLib.h"
#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

#include "dol-config.h"
#include "SGPDeme.h"
#include "TaskSet.h"

// TODO's:
// [ ] ActivateFacing instruction
// [ ] Submit function

constexpr size_t RUN_ID__EXP = 0;
constexpr size_t RUN_ID__ANALYSIS = 1;

constexpr size_t TAG_WIDTH = 16;

constexpr uint32_t MIN_TASK_INPUT = 1;
constexpr uint32_t MAX_TASK_INPUT = 1000000000;
constexpr size_t MAX_TASK_NUM_INPUTS = 2;

constexpr size_t TASK_CNT = 9;

constexpr size_t TRAIT_ID__ACTIVE = 0;
constexpr size_t TRAIT_ID__LAST_TASK = 1;
constexpr size_t TRAIT_ID__DEME_ID = 2;
constexpr size_t TRAIT_ID__UID = 3;
constexpr size_t TRAIT_ID__DIR = 4;


/// Class to manage ALIFE2018 changing environment (w/logic 9) experiments.
class Experiment {
public:
  // Forward declarations.
  struct Agent;
  // Hardware/agent aliases.
  using hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using program_t = hardware_t::Program;
  using state_t = hardware_t::State;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using event_t = hardware_t::event_t;
  using event_lib_t = hardware_t::event_lib_t;
  using memory_t = hardware_t::memory_t;
  using tag_t = hardware_t::affinity_t;
  using exec_stk_t = hardware_t::exec_stk_t;

  // World alias
  using world_t = emp::World<Agent>;
  // Task aliases
  using task_io_t = uint32_t;

  /// Agent to be evolved.
  struct Agent {
    size_t agent_id;
    program_t program;

    Agent(const program_t & _p) : agent_id(0), program(_p) { ; }
    Agent(const Agent && in) : agent_id(in.GetID()), program(in.program) { ; }
    Agent(const Agent & in): agent_id(in.GetID()), program(in.program) { ; }

    size_t GetID() const { return agent_id; }
    void SetID(size_t id) { agent_id = id; }

    program_t & GetGenome() { return program; }

  };

  // TODO: switch over to DOLDeme
  /// Wrapper around SGPDeme that includes useful propagule/activation functions.
  class DOLDeme : public SGPDeme {
  public:
    using grid_t = SGPDeme::grid_t;
    using hardware_t = SGPDeme::hardware_t;
  protected:

    emp::Signal<void(hardware_t &)> on_propagule_activate_sig; // Triggered when a propagule is activated.

  public:
    DOLDeme(size_t _w, size_t _h, emp::Ptr<emp::Random> _rnd, emp::Ptr<inst_lib_t> _ilib, emp::Ptr<event_lib_t> _elib)
    : SGPDeme(_w, _h, _rnd, _ilib, _elib)
    {
      for (size_t i = 0; i < grid.size(); ++i) {
        grid[i].SetTrait(TRAIT_ID__ACTIVE, 0);
        grid[i].SetTrait(TRAIT_ID__DEME_ID, i);
      }
    }

    emp::SignalKey OnPropaguleActivation(const std::function<void(hardware_t &)> & fun) { return on_propagule_activate_sig.AddAction(fun); }

    bool IsActive(size_t id) const { return (bool)grid[id].GetTrait(TRAIT_ID__ACTIVE); }
    void Activate(size_t id) { grid[id].SetTrait(TRAIT_ID__ACTIVE, 1); }
    void Deactivate(size_t id) { grid[id].SetTrait(TRAIT_ID__ACTIVE, 0); }

    void ActivateDemePropagule(size_t prop_size=1, bool clumpy=false) {
      emp_assert(prop_size <= grid.size());
      if (clumpy) {
        size_t hw_id = random->GetUInt(0, grid.size());
        size_t prop_cnt = 0;
        size_t dir = 0;
        while (prop_cnt < prop_size) {
          if (!IsActive(hw_id)) {
            Activate(hw_id); prop_cnt += 1;
            on_propagule_activate_sig.Trigger(grid[hw_id]);
          } else {
            size_t r_dir = (dir + 1) % SGPDeme::NUM_DIRS;
            size_t r_id = GetNeighborID(hw_id, r_dir);
            if (!IsActive(r_id)) {
              dir = r_dir; hw_id = r_id;
            } else {
              hw_id = GetNeighborID(hw_id, dir);
            }
          }
        }
      } else {
        // We need to activate a number of hardwares equal to DEME_PROP_SIZE
        emp::Shuffle(*random, schedule);
        for (size_t i = 0; i < prop_size; ++i) {
          Activate(schedule[i]);
          on_propagule_activate_sig.Trigger(grid[schedule[i]]);
        }
      }
    }

    void PrintActive(std::ostream & os=std::cout) {
      os << "-- Deme Active/Inactive --\n";
      for (size_t x = 0; x < width; ++x) {
        for (size_t y = 0; y < height; ++y) {
          os << (int)IsActive(GetID(x,y)) << " ";
        } os << "\n";
      }
    }
  };

  struct Phenotype {
    emp::vector<size_t> deme_tasks_cnts;        ///< Deme-wide task completion counts (indexed by task).
    emp::vector<size_t> indiv_tasks_cnts;       ///< Task completion broken down by individual and task.
    emp::vector<size_t> indiv_total_tasks_cnts; ///< Total task completion broken down by individual.
    emp::vector<size_t> task_switches;          ///< Task switches broken down by individual.
    size_t task_total;
    double score;
    Phenotype()
      : deme_tasks_cnts(0),
        indiv_tasks_cnts(0),
        indiv_total_tasks_cnts(0),
        task_switches(0),
        task_total(0),
        score(0)
    { ; }

    /// Given hw id and task id, return appropriate index.
    size_t IndivTaskIndex(size_t hw_id, size_t task_id) {
      return (hw_id*TASK_CNT) + task_id;
    }

    void Reset() {
      score = 0;
      task_total = 0;
      for (size_t i = 0; i < deme_tasks_cnts.size(); ++i) deme_tasks_cnts[i] = 0;
      for (size_t i = 0; i < indiv_tasks_cnts.size(); ++i) indiv_tasks_cnts[i] = 0;
      for (size_t i = 0; i < indiv_total_tasks_cnts.size(); ++i) indiv_total_tasks_cnts[i] = 0;
      for (size_t i = 0; i < task_switches.size(); ++i) task_switches[i] = 0;
    }

    double GetScore() const { return score; }

  };


protected:
  // == Configurable experiment parameters ==
  size_t RUN_MODE;
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t EVAL_TIME;
  size_t TRIAL_CNT;
  std::string ANCESTOR_FPATH;
  size_t DEME_WIDTH;
  size_t DEME_HEIGHT;
  size_t PROPAGULE_SIZE;
  bool PROPAGULE_CLUMPY;
  size_t TOURNAMENT_SIZE;
  size_t SELECTION_METHOD;
  size_t ELITE_SELECT__ELITE_CNT;
  size_t SGP_PROG_MAX_FUNC_CNT;
  size_t SGP_PROG_MIN_FUNC_CNT;
  size_t SGP_PROG_MAX_FUNC_LEN;
  size_t SGP_PROG_MIN_FUNC_LEN;
  size_t SGP_PROG_MAX_TOTAL_LEN;
  size_t SGP_HW_MAX_CORES;
  size_t SGP_HW_MAX_CALL_DEPTH;
  double SGP_HW_MIN_BIND_THRESH;
  size_t SGP__PROG_MAX_ARG_VAL;
  double SGP__PER_BIT__TAG_BFLIP_RATE;
  double SGP__PER_INST__SUB_RATE;
  double SGP__PER_INST__INS_RATE;
  double SGP__PER_INST__DEL_RATE;
  double SGP__PER_FUNC__SLIP_RATE;
  double SGP__PER_FUNC__FUNC_DUP_RATE;
  double SGP__PER_FUNC__FUNC_DEL_RATE;
  size_t SYSTEMATICS_INTERVAL;
  size_t FITNESS_INTERVAL;
  size_t POP_SNAPSHOT_INTERVAL;
  std::string DATA_DIRECTORY;

  size_t DEME_SIZE;

  emp::Ptr<emp::Random> random;
  emp::Ptr<world_t> world;

  emp::Ptr<inst_lib_t> inst_lib;
  emp::Ptr<event_lib_t> event_lib;
  emp::Ptr<DOLDeme> eval_deme;

  using taskset_t = TaskSet<std::array<task_io_t,MAX_TASK_NUM_INPUTS>,task_io_t>;
  taskset_t task_set;
  std::array<task_io_t,MAX_TASK_NUM_INPUTS> task_inputs; ///< Current task inputs.
  size_t input_load_id;

  size_t update;
  size_t eval_time;

  size_t dom_agent_id;
  tag_t propagule_start_tag;

  emp::vector<Phenotype> agent_phen_cache;

  // Run signals.
  emp::Signal<void(void)> do_begin_run_setup_sig;   ///< Triggered at begining of run. Shared between AGP and SGP
  emp::Signal<void(void)> do_pop_init_sig;          ///< Triggered during run setup. Defines way population is initialized.
  emp::Signal<void(void)> do_evaluation_sig;        ///< Triggered during run step. Should trigger population-wide agent evaluation.
  emp::Signal<void(void)> do_selection_sig;         ///< Triggered during run step. Should trigger selection (which includes selection, reproduction, and mutation).
  emp::Signal<void(void)> do_world_update_sig;      ///< Triggered during run step. Should trigger world->Update(), and whatever else should happen right before/after population turnover.
  emp::Signal<void(void)> do_analysis_sig;
  // Systematics signals.
  emp::Signal<void(size_t)> do_pop_snapshot_sig;    ///< Triggered if we should take a snapshot of the population (as defined by POP_SNAPSHOT_INTERVAL). Should call appropriate functions to take snapshot.
  // Agent signals.
  emp::Signal<void(Agent &)> begin_agent_eval_sig;
  emp::Signal<void(Agent &)> record_cur_phenotype_sig;

  size_t GetCacheIndex(size_t agent_id, size_t trial_id) {
    return (agent_id * TRIAL_CNT) + trial_id;
  }

  void Evaluate(Agent & agent) {
    begin_agent_eval_sig.Trigger(agent);
    for (eval_time = 0; eval_time < EVAL_TIME; ++eval_time) {
      eval_deme->SingleAdvance();
    }
    // Record everything we want to store about trial phenotype:
    record_cur_phenotype_sig.Trigger(agent); // TODO: might not need this!
  }

  

public:
  Experiment(const DOLConfig & config)
    : DEME_SIZE(0), input_load_id(0), update(0),
      eval_time(0), dom_agent_id(0), propagule_start_tag()
  {
    RUN_MODE = config.RUN_MODE();
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    EVAL_TIME = config.EVAL_TIME();
    TRIAL_CNT = config.TRIAL_CNT();
    DEME_WIDTH = config.DEME_WIDTH();
    DEME_HEIGHT = config.DEME_HEIGHT();
    PROPAGULE_SIZE = config.PROPAGULE_SIZE();
    PROPAGULE_CLUMPY = config.PROPAGULE_CLUMPY();
    ANCESTOR_FPATH = config.ANCESTOR_FPATH();
    TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    SELECTION_METHOD = config.SELECTION_METHOD();
    ELITE_SELECT__ELITE_CNT = config.ELITE_SELECT__ELITE_CNT();
    SGP_PROG_MAX_FUNC_CNT = config.SGP_PROG_MAX_FUNC_CNT();
    SGP_PROG_MIN_FUNC_CNT = config.SGP_PROG_MIN_FUNC_CNT();
    SGP_PROG_MAX_FUNC_LEN = config.SGP_PROG_MAX_FUNC_LEN();
    SGP_PROG_MIN_FUNC_LEN = config.SGP_PROG_MIN_FUNC_LEN();
    SGP_PROG_MAX_TOTAL_LEN = config.SGP_PROG_MAX_TOTAL_LEN();
    SGP_HW_MAX_CORES = config.SGP_HW_MAX_CORES();
    SGP_HW_MAX_CALL_DEPTH = config.SGP_HW_MAX_CALL_DEPTH();
    SGP_HW_MIN_BIND_THRESH = config.SGP_HW_MIN_BIND_THRESH();
    SGP__PROG_MAX_ARG_VAL = config.SGP__PROG_MAX_ARG_VAL();
    SGP__PER_BIT__TAG_BFLIP_RATE = config.SGP__PER_BIT__TAG_BFLIP_RATE();
    SGP__PER_INST__SUB_RATE = config.SGP__PER_INST__SUB_RATE();
    SGP__PER_INST__INS_RATE = config.SGP__PER_INST__INS_RATE();
    SGP__PER_INST__DEL_RATE = config.SGP__PER_INST__DEL_RATE();
    SGP__PER_FUNC__SLIP_RATE = config.SGP__PER_FUNC__SLIP_RATE();
    SGP__PER_FUNC__FUNC_DUP_RATE = config.SGP__PER_FUNC__FUNC_DUP_RATE();
    SGP__PER_FUNC__FUNC_DEL_RATE = config.SGP__PER_FUNC__FUNC_DEL_RATE();
    SYSTEMATICS_INTERVAL = config.SYSTEMATICS_INTERVAL();
    FITNESS_INTERVAL = config.FITNESS_INTERVAL();
    POP_SNAPSHOT_INTERVAL = config.POP_SNAPSHOT_INTERVAL();
    DATA_DIRECTORY = config.DATA_DIRECTORY();

    DEME_SIZE = DEME_WIDTH*DEME_HEIGHT;

    // Make the random number generator.
    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    // Make the world!
    world = emp::NewPtr<world_t>(random, "World");

    // Build phenotype cache.
    agent_phen_cache.resize(POP_SIZE);
    for (size_t i = 0; i < agent_phen_cache.size(); ++i) {
      Phenotype & phen = agent_phen_cache[i];
      phen.deme_tasks_cnts.resize(DEME_SIZE);
      phen.indiv_tasks_cnts.resize(DEME_SIZE*TASK_CNT);
      phen.indiv_total_tasks_cnts.resize(DEME_SIZE);
      phen.task_switches.resize(DEME_SIZE);
      phen.Reset();
    }

    // Make inst/event libraries.
    inst_lib = emp::NewPtr<inst_lib_t>();
    event_lib = emp::NewPtr<event_lib_t>();
    // Propagule start tag will be all 0s
    propagule_start_tag.Clear();

    // Configure tasks.
    Config_Tasks();

    // Configure hardware/instruction libs.
    Config_HW();

    // Configure given run mode.
    switch (RUN_MODE) {
      case RUN_ID__EXP:
        Config_Run();
        break;
      case RUN_ID__ANALYSIS:
        Config_Analysis();
        break;
    }

  }

  void Run() {
    switch (RUN_MODE) {
      case RUN_ID__EXP:
        do_begin_run_setup_sig.Trigger();
        for (update = 0; update <= GENERATIONS; ++update) {
          RunStep();
          if (update % POP_SNAPSHOT_INTERVAL == 0) do_pop_snapshot_sig.Trigger(update);
        }
        break;
      case RUN_ID__ANALYSIS:
        std::cout << "Analysis mode not implemented yet..." << std::endl;
        exit(-1);
        do_analysis_sig.Trigger();
        break;
      default:
        std::cout << "Unrecognized run mode! Exiting..." << std::endl;
        exit(-1);
    }
  }

  void RunStep() {
    do_evaluation_sig.Trigger();
    do_selection_sig.Trigger();
    do_world_update_sig.Trigger();
  }

  void Config_Tasks();
  void Config_HW();
  void Config_Run();
  void Config_Analysis();

  size_t Mutate(Agent & agent, emp::Random & rnd);
  double CalcFitness(Agent & agent) { return agent_phen_cache[agent.GetID()].GetScore(); } ;

  void InitPopulation_FromAncestorFile();

  // Instructions
  static void Inst_Fork(hardware_t & hw, const inst_t & inst);
  static void Inst_Nand(hardware_t & hw, const inst_t & inst);
  static void Inst_Terminate(hardware_t & hw, const inst_t & inst);

  void Inst_Load1(hardware_t & hw, const inst_t & inst);
  void Inst_Load2(hardware_t & hw, const inst_t & inst);
  void Inst_Submit(hardware_t & hw, const inst_t & inst);

};

// --- Instruction implementations ---
/// Instruction: Fork
/// Description: Fork thread with local memory as new thread's input buffer.
void Experiment::Inst_Fork(hardware_t & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  hw.SpawnCore(inst.affinity, hw.GetMinBindThresh(), state.local_mem);
}

void Experiment::Inst_Nand(hardware_t & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  const task_io_t a = (task_io_t)state.GetLocal(inst.args[0]);
  const task_io_t b = (task_io_t)state.GetLocal(inst.args[1]);
  state.SetLocal(inst.args[2], ~(a&b));
}

void Experiment::Inst_Terminate(hardware_t & hw, const inst_t & inst) {
  // Pop all the call states from current core.
  exec_stk_t & core = hw.GetCurCore();
  core.resize(0);
}

void Experiment::Inst_Load1(hardware_t & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], task_inputs[input_load_id]); // Load input.
  input_load_id += 1;
  if (input_load_id >= task_inputs.size()) input_load_id = 0; // Update load ID.
}

void Experiment::Inst_Load2(hardware_t & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], task_inputs[0]);
  state.SetLocal(inst.args[1], task_inputs[1]);
}

void Experiment::Inst_Submit(hardware_t & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  // Submit! --> Did hw complete a task?
  task_io_t sol = (task_io_t)state.GetLocal(inst.args[0]);
  size_t hw_id = hw.GetTrait(TRAIT_ID__DEME_ID);
  // TODO: what do about multiple hits? --> Don't care; just give credit for first one?
  for (size_t task_id = 0; task_id < task_set.GetSize(); ++task_id) {
    if (task_set.CheckTask(task_id, sol)) {
      // Just give credit to first one.
      // Submit(task_id, hw_id);
      // TODO: stick information somewhere
      //  - In the deme? Or, in the phenotype?
      //  - If in the deme: need to add proper structures to deme class
      //  - If in the phenotype cache, need to pull cache ID from somewhere (eval_deme seems like a good place)
      break;
    }
  }
  // task_set.Submit((task_io_t)state.GetLocal(inst.args[0]), eval_time);
}

// --- Utilities ---
void Experiment::InitPopulation_FromAncestorFile() {
  std::cout << "Initializing population from ancestor file!" << std::endl;
  // Configure the ancestor program.
  program_t ancestor_prog(inst_lib);
  std::ifstream ancestor_fstream(ANCESTOR_FPATH);
  if (!ancestor_fstream.is_open()) {
    std::cout << "Failed to open ancestor program file(" << ANCESTOR_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  ancestor_prog.Load(ancestor_fstream);
  std::cout << " --- Ancestor program: ---" << std::endl;
  ancestor_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;
  world->Inject(ancestor_prog, 1);    // Inject a bunch of ancestors into the population.
}

// --- Configuration/setup function implementations ---
void Experiment::Config_Run() {
  // Make data directory.
  mkdir(DATA_DIRECTORY.c_str(), ACCESSPERMS);
  if (DATA_DIRECTORY.back() != '/') DATA_DIRECTORY += '/';
  
  // Configure the world.
  world->Reset();
  world->SetWellMixed(true);
  world->SetFitFun([this](Agent & agent) { return this->CalcFitness(agent); });
  world->SetMutFun([this](Agent &agent, emp::Random &rnd) { return this->Mutate(agent, rnd); }, ELITE_SELECT__ELITE_CNT);

  // === Setup signals! ===
  // On population initialization:
  do_pop_init_sig.AddAction([this]() {
    this->InitPopulation_FromAncestorFile();
  });

  // On run setup:
  do_begin_run_setup_sig.AddAction([this]() {
    std::cout << "Doing initial run setup." << std::endl;
    // Setup systematics/fitness tracking.
    auto & sys_file = world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
    sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
    auto & fit_file = world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
    fit_file.SetTimingRepeat(FITNESS_INTERVAL);
    // TODO: add useful dominant phenotype tracking
    do_pop_init_sig.Trigger();
  });

  begin_agent_eval_sig.AddAction([this](Agent & agent) {
    // Do agent setup at the beginning of its evaluation.
    // - Here, the eval_deme has been reset.
    // - No agents in deme have active threads.
    // Setup propagule.
    eval_deme->ActivateDemePropagule(PROPAGULE_SIZE, PROPAGULE_CLUMPY);
  });

  // On evaluation:
  do_evaluation_sig.AddAction([this]() {
    double best_score = -32767;
    dom_agent_id = 0;
    for (size_t id = 0; id < world->GetSize(); ++id) {
      Agent & our_hero = world->GetOrg(id);
      our_hero.SetID(id);
      eval_deme->SetProgram(our_hero.GetGenome());
      agent_phen_cache[id].Reset();
      this->Evaluate(our_hero);
      // TODO: any phenotype crimping?
      if (agent_phen_cache[id].GetScore() > best_score) { best_score = agent_phen_cache[id].GetScore(); dom_agent_id = id; }
    }
  });
  // TODO: finish..

}

void Experiment::Config_Analysis() { ; }

void Experiment::Config_HW() {
  // - Setup the instruction set. -
  // Standard instructions:
  inst_lib->AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib->AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib->AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib->AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  inst_lib->AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  inst_lib->AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  inst_lib->AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  inst_lib->AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  inst_lib->AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  inst_lib->AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  inst_lib->AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib->AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib->AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib->AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.", emp::ScopeType::BASIC, 0, {"affinity"});
  inst_lib->AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib->AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib->AddInst("CopyMem", hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  inst_lib->AddInst("SwapMem", hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  inst_lib->AddInst("Input", hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  inst_lib->AddInst("Output", hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  inst_lib->AddInst("Commit", hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Pull", hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");
  inst_lib->AddInst("Fork", Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
  inst_lib->AddInst("Nand", Inst_Nand, 3, "WM[ARG3]=~(WM[ARG1]&WM[ARG2])");
  inst_lib->AddInst("Terminate", Inst_Terminate, 0, "Kill current thread.");

  // Add experiment-specific instructions.
  inst_lib->AddInst("Load-1", [this](hardware_t & hw, const inst_t & inst) { this->Inst_Load1(hw, inst); }, 1, "WM[ARG1] = TaskInput[LOAD_ID]; LOAD_ID++;");
  inst_lib->AddInst("Load-2", [this](hardware_t & hw, const inst_t & inst) { this->Inst_Load2(hw, inst); }, 2, "WM[ARG1] = TASKINPUT[0]; WM[ARG2] = TASKINPUT[1];");
  inst_lib->AddInst("Submit", [this](hardware_t & hw, const inst_t & inst) { this->Inst_Submit(hw, inst); }, 1, "Submit WM[ARG1] as potential task solution.");

  // TODO: experiment-specific instructions/events
  // --> Broadcast, SendMsg, Activate

  // Configure evaluation hardware.
  // Make eval deme.
  eval_deme = emp::NewPtr<DOLDeme>(DEME_WIDTH, DEME_HEIGHT, random, inst_lib, event_lib);
  eval_deme->SetHardwareMinBindThresh(SGP_HW_MIN_BIND_THRESH);
  eval_deme->SetHardwareMaxCores(SGP_HW_MAX_CORES);
  eval_deme->SetHardwareMaxCallDepth(SGP_HW_MAX_CALL_DEPTH);

  // TODO: what happens on hardware reset (in eval deme)
  eval_deme->OnHardwareReset([this](hardware_t & hw) {
    hw.SetTrait(TRAIT_ID__ACTIVE, 0);
    hw.SetTrait(TRAIT_ID__LAST_TASK, -1);
    hw.SetTrait(TRAIT_ID__UID, 0);
    hw.SetTrait(TRAIT_ID__DIR, 0);
  });

  eval_deme->OnHardwareAdvance([this](hardware_t & hw) {
    if ((bool)hw.GetTrait(TRAIT_ID__ACTIVE)) hw.SingleProcess();
  });

  eval_deme->OnPropaguleActivation([this](hardware_t & hw) {
    // Spawn thread!
    hw.SpawnCore(propagule_start_tag, 0.0);
  });
}

void Experiment::Config_Tasks() {
  // Zero out task inputs.
  for (size_t i = 0; i < MAX_TASK_NUM_INPUTS; ++i) task_inputs[i] = 0;
  // Add tasks to task set.
  // NAND
  task_set.AddTask("NAND", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(~(a&b));
  }, "NAND task");
  // NOT
  task_set.AddTask("NOT", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(~a);
    task.solutions.emplace_back(~b);
  }, "NOT task");
  // ORN
  task_set.AddTask("ORN", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back((a|(~b)));
    task.solutions.emplace_back((b|(~a)));
  }, "ORN task");
  // AND
  task_set.AddTask("AND", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(a&b);
  }, "AND task");
  // OR
  task_set.AddTask("OR", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(a|b);
  }, "OR task");
  // ANDN
  task_set.AddTask("ANDN", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back((a&(~b)));
    task.solutions.emplace_back((b&(~a)));
  }, "ANDN task");
  // NOR
  task_set.AddTask("NOR", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(~(a|b));
  }, "NOR task");
  // XOR
  task_set.AddTask("XOR", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(a^b);
  }, "XOR task");
  // EQU
  task_set.AddTask("EQU", [this](taskset_t::Task & task, const std::array<task_io_t, MAX_TASK_NUM_INPUTS> & inputs) {
    const task_io_t a = inputs[0], b = inputs[1];
    task.solutions.emplace_back(~(a^b));
  }, "EQU task");
}

/// Mutation rules:
/// - Function Duplication (per-program rate):
///   - Result cannot allow program to exceed max function count
///   - Result cannot allow program to exceed max total instruction length.
/// - Function Deletion (per-program rate).
///   - Result cannot allow program to have no functions.
/// - Slip mutations (per-function rate)
///   - Result cannot allow function length to break the range [PROG_MIN_FUNC_LEN:PROG_MAX_FUNC_LEN]
/// - Instruction insertion/deletion mutations (per-instruction rate)
///   - Result cannot allow function length to break [PROG_MIN_FUNC_LEN:PROG_MAX_FUNC_LEN]
///   - Result cannot allow function length to exeed PROG_MAX_TOTAL_LEN
size_t Experiment::Mutate(Agent &agent, emp::Random &rnd)
{
  program_t &program = agent.GetGenome();
  size_t mut_cnt = 0;
  size_t expected_prog_len = program.GetInstCnt();

  // Duplicate a (single) function?
  if (rnd.P(SGP__PER_FUNC__FUNC_DUP_RATE) && program.GetSize() < SGP_PROG_MAX_FUNC_CNT)
  {
    const uint32_t fID = rnd.GetUInt(program.GetSize());
    // Would function duplication make expected program length exceed max?
    if (expected_prog_len + program[fID].GetSize() <= SGP_PROG_MAX_TOTAL_LEN)
    {
      program.PushFunction(program[fID]);
      expected_prog_len += program[fID].GetSize();
      ++mut_cnt;
    }
  }

  // Delete a (single) function?
  if (rnd.P(SGP__PER_FUNC__FUNC_DEL_RATE) && program.GetSize() > SGP_PROG_MIN_FUNC_CNT)
  {
    const uint32_t fID = rnd.GetUInt(program.GetSize());
    expected_prog_len -= program[fID].GetSize();
    program[fID] = program[program.GetSize() - 1];
    program.program.resize(program.GetSize() - 1);
    ++mut_cnt;
  }

  // For each function...
  for (size_t fID = 0; fID < program.GetSize(); ++fID)
  {

    // Mutate affinity
    for (size_t i = 0; i < program[fID].GetAffinity().GetSize(); ++i)
    {
      tag_t &aff = program[fID].GetAffinity();
      if (rnd.P(SGP__PER_BIT__TAG_BFLIP_RATE))
      {
        ++mut_cnt;
        aff.Set(i, !aff.Get(i));
      }
    }

    // Slip-mutation?
    if (rnd.P(SGP__PER_FUNC__SLIP_RATE))
    {
      uint32_t begin = rnd.GetUInt(program[fID].GetSize());
      uint32_t end = rnd.GetUInt(program[fID].GetSize());
      const bool dup = begin < end;
      const bool del = begin > end;
      const int dup_size = end - begin;
      const int del_size = begin - end;
      // If we would be duplicating and the result will not exceed maximum program length, duplicate!
      if (dup && (expected_prog_len + dup_size <= SGP_PROG_MAX_TOTAL_LEN) && (program[fID].GetSize() + dup_size <= SGP_PROG_MAX_FUNC_LEN))
      {
        // duplicate begin:end
        const size_t new_size = program[fID].GetSize() + (size_t)dup_size;
        hardware_t::Function new_fun(program[fID].GetAffinity());
        for (size_t i = 0; i < new_size; ++i)
        {
          if (i < end)
            new_fun.PushInst(program[fID][i]);
          else
            new_fun.PushInst(program[fID][i - dup_size]);
        }
        program[fID] = new_fun;
        ++mut_cnt;
        expected_prog_len += dup_size;
      }
      else if (del && ((program[fID].GetSize() - del_size) >= SGP_PROG_MIN_FUNC_LEN))
      {
        // delete end:begin
        hardware_t::Function new_fun(program[fID].GetAffinity());
        for (size_t i = 0; i < end; ++i)
          new_fun.PushInst(program[fID][i]);
        for (size_t i = begin; i < program[fID].GetSize(); ++i)
          new_fun.PushInst(program[fID][i]);
        program[fID] = new_fun;
        ++mut_cnt;
        expected_prog_len -= del_size;
      }
    }

    // Substitution mutations? (pretty much completely safe)
    for (size_t i = 0; i < program[fID].GetSize(); ++i)
    {
      inst_t &inst = program[fID][i];
      // Mutate affinity (even when it doesn't use it).
      for (size_t k = 0; k < inst.affinity.GetSize(); ++k)
      {
        if (rnd.P(SGP__PER_BIT__TAG_BFLIP_RATE))
        {
          ++mut_cnt;
          inst.affinity.Set(k, !inst.affinity.Get(k));
        }
      }

      // Mutate instruction.
      if (rnd.P(SGP__PER_INST__SUB_RATE))
      {
        ++mut_cnt;
        inst.id = rnd.GetUInt(program.GetInstLib()->GetSize());
      }

      // Mutate arguments (even if they aren't relevent to instruction).
      for (size_t k = 0; k < hardware_t::MAX_INST_ARGS; ++k)
      {
        if (rnd.P(SGP__PER_INST__SUB_RATE))
        {
          ++mut_cnt;
          inst.args[k] = rnd.GetInt(SGP__PROG_MAX_ARG_VAL);
        }
      }
    }

    // Insertion/deletion mutations?
    // - Compute number of insertions.
    int num_ins = rnd.GetRandBinomial(program[fID].GetSize(), SGP__PER_INST__INS_RATE);
    // Ensure that insertions don't exceed maximum program length.
    if ((num_ins + program[fID].GetSize()) > SGP_PROG_MAX_FUNC_LEN)
    {
      num_ins = SGP_PROG_MAX_FUNC_LEN - program[fID].GetSize();
    }
    if ((num_ins + expected_prog_len) > SGP_PROG_MAX_TOTAL_LEN)
    {
      num_ins = SGP_PROG_MAX_TOTAL_LEN - expected_prog_len;
    }
    expected_prog_len += num_ins;

    // Do we need to do any insertions or deletions?
    if (num_ins > 0 || SGP__PER_INST__DEL_RATE > 0.0)
    {
      size_t expected_func_len = num_ins + program[fID].GetSize();
      // Compute insertion locations and sort them.
      emp::vector<size_t> ins_locs = emp::RandomUIntVector(rnd, num_ins, 0, program[fID].GetSize());
      if (ins_locs.size())
        std::sort(ins_locs.begin(), ins_locs.end(), std::greater<size_t>());
      hardware_t::Function new_fun(program[fID].GetAffinity());
      size_t rhead = 0;
      while (rhead < program[fID].GetSize())
      {
        if (ins_locs.size())
        {
          if (rhead >= ins_locs.back())
          {
            // Insert a random instruction.
            new_fun.PushInst(rnd.GetUInt(program.GetInstLib()->GetSize()),
                             rnd.GetInt(SGP__PROG_MAX_ARG_VAL),
                             rnd.GetInt(SGP__PROG_MAX_ARG_VAL),
                             rnd.GetInt(SGP__PROG_MAX_ARG_VAL),
                             tag_t());
            new_fun.inst_seq.back().affinity.Randomize(rnd);
            ++mut_cnt;
            ins_locs.pop_back();
            continue;
          }
        }
        // Do we delete this instruction?
        if (rnd.P(SGP__PER_INST__DEL_RATE) && (expected_func_len > SGP_PROG_MIN_FUNC_LEN))
        {
          ++mut_cnt;
          --expected_prog_len;
          --expected_func_len;
        }
        else
        {
          new_fun.PushInst(program[fID][rhead]);
        }
        ++rhead;
      }
      program[fID] = new_fun;
    }
  }
  return mut_cnt;
}

#endif
