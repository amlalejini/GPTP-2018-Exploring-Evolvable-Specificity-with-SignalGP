---
title: "Data Analysis"
output: 
  html_document: 
    keep_md: yes
    toc: true
    toc_float: true
    collapsed: false
    theme: default
  pdf_document: default
---
## Overview
Here, we analyze our experimental results from our 2018 GPTP contribution "What's in an evolved name? Exploring evolvable specificity with SignalGP" ([citation to come]). In this work, we use SignalGP to explore how important allowing for inexact matching is in tag-based referencing. 

## Dependencies & General Setup
We used R version 3.3.2 (2016-10-31) for statistical analyses (R Core Team, 2016).

In addition to R, we use Python 3 for data manipulation and visualization (because Python!). To get Python and R to play nice, we use the [reticulate](https://rstudio.github.io/reticulate/index.html) R package. 

```r
library(reticulate)
use_python("/anaconda3/bin/python")
```

Make sure R markdown is using the reticulate Python engine. 

```r
knitr::knit_engines$set(python = reticulate::eng_python)
```

All visualizations use the seaborn Python package (Waskom et al, 2017), which sits on top of Python's matplotlib, providing a "high-level interface for drawing attractive statistical graphics". 

```python
import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
```

To handle data on the python end of things, we'll use the [pandas](https://pandas.pydata.org/) Python library, which provides "high-performance, easy-to-use data structures and data analysis tools for the Python programming langauge". 

```python
import pandas as pd
```

## A little bit of background. 
Again, see the paper (citation to come) for context on these data analyses, but here's some high-level background. 

### Briefly, tag-based referencing. 
What's in an evolved name? How should modules (e.g. functions, sub-routines, data-objects, etc) be referenced in evolving programs? In traditional software development, the programmer hand-labels modules and subsequently refers to them using their assigned label. This technique for referencing modules is intentionally rigid, requiring programmers to precisely name the module they aim to reference; imprecision often results in syntactic incorrectness. Requiring evolving programs to follow traditional approaches to module referencing is not ideal: mutation operators must do extra work to guarantee label-correctness, else mutated programs are likely to make use of undefined labels, resulting in syntactic invalidity (Spector et al., 2011a). Instead, is genetic programming (GP) better off relying on more flexible, less exacting referencing schemes?

**Enter: tag-based referencing.** Spector et al (Spector et al., 2011a, 2011b, and 2012) introduced and demonstrated a tag-based naming scheme for GP where tags are used to name and reference program modules. Tags are evolvable labels that are mutable, and the similarity (or dissimilarity) between any two possible tags is quantifiable. Tags allow for inexact referencing. Because the similarity between tags can be calculated, a referring tag can always link to the program module with the most similar (_i.e._ closest matching) tag; further, this ensures that all possible tags are valid references. Because tags are mutable, evolution can incrementally shape tag-based references within evolving code.

In Spector et al.'s original conception of tag-based referencing, if any tagged entity (program module, etc) existed, a referring tag would always have _something_ to reference; there was no minimum similarity requirement for tags to successfully match. In other words, references allowed for _complete_ imprecision. We can, however, put a minimum threshold of similarity on tag-based references: for two tags to be successfully matched, they must be at least X% similar. Reminder: we represent tags as bit strings in our work, using the [simple matching coefficient](https://en.wikipedia.org/wiki/Simple_matching_coefficient) to define the similarity between two tags. 

### Just tell me what questions you're asking. 
Okay, okay. Here, we explore the effects of adjusting the minimum required similarity for tags to successfully match for a reference. How important is imprecision when doing tag-based referencing? Is exact name matching (what human programmers usually do) really all that bad for evolution? (**spoiler**: yes!) Is it important to allow for _complete_ inexactness as in Spector et al.'s original work? Or, is it fine to require a minimum threshold of similarity? Are there situations where requiring _some_ amount of precision (_i.e._ a minimum match similarity) is necessary? 

We use evolving populations of SignalGP agents to address these questions. See (Lalejini and Ofria, 2018) for details on exactly how SignalGP makes use of tag-based referencing. In short, SignalGP labels program modules with tags, and externally-generated or internally-generated can refer to SignalGP program modules via tag-based referencing. 

## Experiment 1: How important is inexactness in tag-based referencing?
How important is imprecision when calling an evolvable name? As discussed above, tag-based referencing has built-in flexibility, not requiring tags to _exactly_ match to successfully reference one another. To explore the importance of inexactness in tag-based referencing, we evolved populations (30 replicates) of SignalGP agents to solve the changing environment problem under nine different conditions where each condition required tags to have a different minimum similarity in order to successfully be matched: 0% (_i.e._, no restrictions on tag-similarity), 12.5%, 25.0%, 37.5%, 50.0%, 62.5%, 75.0%, 87.5%, and 100.0% (_i.e._, tags needed to match exactly). 

### The Changing Environment Problem
See (Lalejini and Ofria, 2018, and our contribution - citation coming) for a detailed description of the changing environment problem. The changing environment problem is a toy problem to test GP programs' capacity to respond appropriately to environmental signals. 

The changing environment problem requires agents to coordinate their behavior with a randomly changing environment. In these experiments, the environment can be in one of sixteen states at any one time. At the beginning of a run, the environment is initialized to a random state, and at every subsequent time step, the environment has a 12.5% chance of randomly changing. To be successful, agents must match their internal state to the current state of the environment; thus, agents must monitor the environment for changes, adjusting their internal state as appropriate. 

Environmental changes produce signals (events) with environment-specific tags, which can trigger SignalGP functions (program modules); in this way, SignalGP agents can monitor for and respond to environmental changes. In this experiment, we limit SignalGP programs to a maximum of 16 functions, which means that optimal agents _must_ have sixteen functions, _each_ function's tag must ensure that it is only triggered by a single environment change, and each function must adjust an agent's internal state appropriately.  

### Statistical Methods
For each condition, we have 30 replicates, and for each replicate, we extract the dominant (highest-fitness) agent at generation 1,000 and generation 10,000. To account for stochasticity in the environment, we test each extracted agent 100 times and record its average performance over those 100 trials; this performance score is used as the agent's fitness in our analyses. 

For each generation analyzed, we compared the performances of evolved, dominant programs across treatments. Let's set our significance level, $\alpha$, to...

```r
sig_level <- 0.05
```

To determine if any of the treatments were significant (_p_ < `sig_level`) for a given generation, we performed a [Kruskal-Wallis rank sum test](https://en.wikipedia.org/wiki/Kruskal%E2%80%93Wallis_one-way_analysis_of_variance). For a generation in which the Kruskal-Wallis test was significant, we performed a post-hoc pairwise Wilcoxon rank-sum test, applying a Bonferroni correction for multiple comparisons.

### Results
Finally, enough with all of those long-winded explanations! Results! 

Well, first we need to actually load in some data. Once we do this, we don't have to do it again.
We'll go ahead and load evolution data with R. 

```r
evo_data <- read.csv("../data/evo_dom.csv")
```

That evo_dom file has spoiler data in it! I.e., it has the data for both Experiment 1 and Experiment 2 described in this document. Let's filter down to just the data relevant to Experiment 1. 

```r
exp1_data <- evo_data[evo_data$distraction_sigs == 0,]
exp1_data$sim_thresh <- as.factor(exp1_data$sim_thresh)
```

#### Results after 1,000 generations of evolution.
Let's take a look at how things are doing after 1,000 generations of evolution. First, we'll grab only data relevent to generation 1,000. 

```r
exp1_1000_data <- exp1_data[exp1_data$update == 1000,]
```

**Visualize!** To keep things succinct, I'll keep the Python code under the hood (check out the .Rmd file if you're curious). 

![](stats_files/figure-html/unnamed-chunk-8-1.png)<!-- -->

**At a glance:**<br>
As expected, _exact_ name matching (100.0%) is **bad**. Much worse than any other treatment. And, up to a certain point, the value of the minimum similarity threshold doesn't matter much (_i.e._, 0.0% isn't any different than requiring 50.0%). 

Let's be responsible and do some actual statistical analyses. 

First, we'll do a Kruskal-Wallis test to confirm that at least one of the treatments within the set is different from the others. 

```r
exp1_1000_kt <- kruskal.test(fitness ~ sim_thresh, data=exp1_1000_data)
exp1_1000_kt
```

```
## 
## 	Kruskal-Wallis rank sum test
## 
## data:  fitness by sim_thresh
## Kruskal-Wallis chi-squared = 161.27, df = 8, p-value < 2.2e-16
```

As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc pairwise Wilcoxon rank-sum test to analyze which treatments are significantly different from one another.

```r
exp1_1000_wt <- pairwise.wilcox.test(x=exp1_1000_data$fitness, g=exp1_1000_data$sim_thresh, p.adjust.method="bonferroni", exact=FALSE)
exp1_1000_wt
```

```
## 
## 	Pairwise comparisons using Wilcoxon rank sum test 
## 
## data:  exp1_1000_data$fitness and exp1_1000_data$sim_thresh 
## 
##       0       0.125   0.25    0.375   0.5     0.625   0.75    0.875  
## 0.125 1.0000  -       -       -       -       -       -       -      
## 0.25  1.0000  1.0000  -       -       -       -       -       -      
## 0.375 1.0000  1.0000  1.0000  -       -       -       -       -      
## 0.5   1.0000  1.0000  1.0000  1.0000  -       -       -       -      
## 0.625 1.0000  1.0000  1.0000  1.0000  1.0000  -       -       -      
## 0.75  0.3559  0.0109  0.0956  0.0013  0.0654  0.2005  -       -      
## 0.875 8.6e-10 7.6e-10 8.1e-10 5.2e-10 7.0e-10 7.6e-10 1.1e-09 -      
## 1     8.6e-10 7.6e-10 8.1e-10 5.2e-10 7.0e-10 7.6e-10 1.1e-09 1.1e-09
## 
## P value adjustment method: bonferroni
```

In general, the two treatments with the highest minimum similarity thresholds (87.5% and 100.0%) seem to be worse than the others, while (in general) there aren't real differences between the others. Let's move on to the end of the run. 

#### Results after 10,000 generations of evolution.
Let's take a look at how things are doing at the end of the runs: generation 10,000! Let's grab only data relevant to generation 10,000.

```r
exp1_10000_data <- exp1_data[exp1_data$update == 10000,]
```

**Visualize!**

![](stats_files/figure-html/unnamed-chunk-12-1.png)<!-- -->

Exact name matching (100.0%) is still **bad**. Much worse than any other treatment. Everything else, except the 87.5% treatment solved the problem pretty much optimally in all replicates. 

First, we'll do a Kruskal-Wallis test to confirm that at least one of the treatments within the set is different from the others. 

```r
exp1_10000_kt <- kruskal.test(fitness ~ sim_thresh, data=exp1_10000_data)
exp1_10000_kt
```

```
## 
## 	Kruskal-Wallis rank sum test
## 
## data:  fitness by sim_thresh
## Kruskal-Wallis chi-squared = 221.72, df = 8, p-value < 2.2e-16
```

As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc pairwise Wilcoxon rank-sum test to analyze which treatments are significantly different from one another.

```r
exp1_10000_wt <- pairwise.wilcox.test(x=exp1_10000_data$fitness, g=exp1_10000_data$sim_thresh, p.adjust.method="bonferroni", exact=FALSE)
exp1_10000_wt
```

```
## 
## 	Pairwise comparisons using Wilcoxon rank sum test 
## 
## data:  exp1_10000_data$fitness and exp1_10000_data$sim_thresh 
## 
##       0       0.125   0.25    0.375   0.5     0.625   0.75    0.875  
## 0.125 1.00000 -       -       -       -       -       -       -      
## 0.25  -       1.00000 -       -       -       -       -       -      
## 0.375 -       1.00000 -       -       -       -       -       -      
## 0.5   -       1.00000 -       -       -       -       -       -      
## 0.625 -       1.00000 -       -       -       -       -       -      
## 0.75  -       1.00000 -       -       -       -       -       -      
## 0.875 0.00027 0.00079 0.00027 0.00027 0.00027 0.00027 0.00027 -      
## 1     2.5e-11 3.6e-11 2.5e-11 2.5e-11 2.5e-11 2.5e-11 2.5e-11 4.4e-10
## 
## P value adjustment method: bonferroni
```

These data support the idea that exact name matching is not very evolvable. In fact, requiring too much precision when calling an evolvable name (doing tag-based referencing) is not ideal. On the flip side, requiring _some_ precision for successful tag-based references is fine (_i.e._, 12.5% through 75% minimum thresholds are not significantly different than a 0.0% threshold). 

### Exploring the Importance of Inexactness with MAP-Elites
To further illuminate the space of similarity thresholds for SignalGP in the changing environment problem, we can use the MAP-Elites algorithm (Mauret and Clune, 2015). In MAP-Elites, a population is structured based on a set of chosen phenotypic traits of evolving solutions. Each chosen phenotypic trait defines an axis on a grid of cells where each cell represents a distinct combination of phenotypic traits; further, each cell maintains only the most fit solution 'discovered' with that cell's associated combination of phenotypic traits. We initialize the map in MAP-Elites by randomly generating a solution and placing it into its appropriate cell in the grid (based on its phenotypic characteristics). From here, we repeatedly select an occupied cell in the grid at random, generate a mutated offspring for that cell's occupant, and figure out where the offspring belongs in the grid; if that cell is empty, place the offspring there, otherwise compare the offspring's fitness to the current occupants, keeping the fitter of the two. We repeat this process for as long as we want, and we eventually end up with a grid of prospective solutions that span the range of the phenotypic traits we defined as our axes. Neat! 

We can apply this algorithm to GP (in fact, we have an entire paper on this idea! [citation to come]) where phenotypic traits are program characteristics (_e.g._, program length/size, module count, instruction entropy, etc.). Here, we apply MAP-Elites to illuminate the relationship between function count and similarity threshold for SignalGP agents in the context of the changing environment problem. 

In our evolution experiment, we locked in the minimum required similarity threshold for each treatment, comparing the performance of agents evolved with different similarity thresholds. For this, we allow the minimum required similarity threshold to evolve between 0.0% and 100.0%. Minimum similarity threshold is one axis of our MAP-Elites grid. In our evolution experiment we capped the allowed number of functions in a SignalGP program at 16. For this, we allow SignalGP programs to have function counts ranging from 0 to 32. The number of functions an agent uses (i.e., actually gets executed) is our second axis for our MAP-Elites grid. 

We initialized our MAP-Elites grid with 1,000 randomly generated programs. We ran the MAP-Elite algorithm for 100,000 'generations' where each 'generation' represents 1,000 births. We ran 50 replicate MAP-Elites runs, giving us 50 grids of diverse solutions for the changing environment problem. 

We can use the grids of solutions to illuminate where solutions exist in our search space. How many functions do agents need to solve the problem? What similarity thresholds are most likely to produce solutions? How do these 'phenotypic' traits interact? 

To look into these questions, we collect all of the grids generated by all 50 replicate MAP-Elites algorithms. We test all solutions 100 times (to account for environmental stochasticity), and we filter the solutions by fitness, keeping only optimal solutions capable of perfectly matching their internal state with the environment state. Next, we toss all of those optimal solutions into a heatmap where similarity threshold and function count are our x and y axes and the number of solutions in a particular region give the heat!

Bippity boppity boo! We'll load these data in and filter out all non-optimal solutions. 

```r
mape_data <- read.csv("../data/mape.csv")
max_fit <- 256
mape_data <- mape_data[mape_data$fitness >= max_fit,]
```
Again, we need to filter out Experiment 2 spoilers.

```r
mape1_data <- mape_data[mape_data$distraction_sigs == 0,]
```

**Visualize!**
![](stats_files/figure-html/unnamed-chunk-17-1.png)<!-- -->

From this heat map, we can see that solutions exist at 16+ (looking directly at the data, the lowest functions used is \infty{}). Remember, that value is the average functions used over 100 trials. Because of environmental stochasticity, a few of that agent's trials likely didn't feature _all_ of the environments, which resulted in not all 16 necessary functions being called. Nonetheless, our MAP-Elites results are comfirming that programs need 16 functions to solve the problem, and it's hard to generate optimal solutions with a similarity threshold above 0.874657 (the maximum similarity threshold of all solutions represented in our heat map). 

## Experiment 2: Is it ever important to allow for _some_ exactness in tag-based referencing? 
It's a whole new experiment! In the previous experiments, we confirm that inexactness is an important feature of evolvable names. But, are there times where we need _some_ amount of exactness when calling evolvable names (doing tag-based referencing)? Here, we demonstrate yes! Sometimes requiring some precision in tag-based referencing is necessary for solving a problem optimally. To do this, we'll repeat our first experiment except will extend the changing environment to include distraction signals. 

### Distracting Environment Problem
The distracting environment problem is identical to the changing environment problem except with distraction signals. As in the changing environment problem, the environment can be in any one of sixteen states at any given time, and every time the environment changes, it emits a signal. However, in the distracting environment problem, the environment also has a 12.5% chance to emit a random one of sixteen meaningless distraction signals at every time step. To be successful in the distracting environment problem, populations need to evolve to respond to the true environment change signals and ignore the distraction signals. 

There are two ways SignalGP agents can ignore distraction signals: 1) consume them with functions that do nothing; if a distraction signal triggers a function that adjusts the agent's internal state, it could cause the agent's internal state to mismatch with the state of the environment. Or, 2) ensure that all internal state adjusting functions have tags that are dissimilar enough to the distraction signal tags that the distraction signals do not trigger any of the SignalGP agent's functions. Of course, there are successful strategies that employ combinations of these two ways of ignoring distraction signals. 

As in the first experiment, we'll limit agents to a maximum of sixteen functions. With this restriction, agents cannot consume distraction signals by having do-nothing functions that are triggered in response to the distraction signals. Programs _must_ rely on the minimum similarity threshold to ignore distraction signals; in other words, function tags must evolve to respond to true environmental change signals and ignore distraction signals. Thus, the 0.0% similarity threshold treatment is doomed to fail when the function count is limited to 16 in the distracting environment problem. 

### Statitical Methods
We'll use the exact same statistical methods as we did for Experiment 1. 

### Results
We've already loaded all of the data for this experiment; we just need to filter down to only data relevant to this experiment.

```r
exp2_data <- evo_data[evo_data$distraction_sigs == 1,]
exp2_data$sim_thresh <- as.factor(exp2_data$sim_thresh)
```

#### Results after 1,000 generations of evolution.
Let's take a look at how things are doing after 1,000 generations of evolution. 

```r
exp2_1000_data <- exp2_data[exp2_data$update == 1000,]
```

**Accio, visualization!** 
![](stats_files/figure-html/unnamed-chunk-20-1.png)<!-- -->

Things are looking quite a bit different than in the first experiment with the vanilla changing environment problem! Exact name matching is still bad, but the 75% similarity threshold treatment seems to be doing better than all other treatments.

Let's do some statistics! First, we'll do a Kruskal-Wallis test to confirm that at least one of the treatments in the set is different than the others.

```r
exp2_1000_kt <- kruskal.test(fitness ~ sim_thresh, data=exp2_1000_data)
exp2_1000_kt
```

```
## 
## 	Kruskal-Wallis rank sum test
## 
## data:  fitness by sim_thresh
## Kruskal-Wallis chi-squared = 144.3, df = 8, p-value < 2.2e-16
```


As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc pairwise Wilcoxon rank-sum test to analyze which treatments are significantly different from one another.

```r
exp2_1000_wt <- pairwise.wilcox.test(x=exp2_1000_data$fitness, g=exp2_1000_data$sim_thresh, p.adjust.method="bonferroni", exact=FALSE)
exp2_1000_wt
```

```
## 
## 	Pairwise comparisons using Wilcoxon rank sum test 
## 
## data:  exp2_1000_data$fitness and exp2_1000_data$sim_thresh 
## 
##       0       0.125   0.25    0.375   0.5     0.625   0.75    0.875  
## 0.125 1       -       -       -       -       -       -       -      
## 0.25  1       1       -       -       -       -       -       -      
## 0.375 1       1       1       -       -       -       -       -      
## 0.5   1       1       1       1       -       -       -       -      
## 0.625 1       1       1       1       1       -       -       -      
## 0.75  1.1e-09 1.3e-09 1.1e-09 1.5e-09 1.2e-09 1.5e-09 -       -      
## 0.875 1       1       1       1       1       1       3.3e-09 -      
## 1     1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.6e-09
## 
## P value adjustment method: bonferroni
```


In general, the 75% treatment is significantly better than all other treatments, and the 100.0% (exact matching) treatment is significantly worse than all other treatments. 

#### Results after 10,000 generations of evolution.
Let's take a look at how things are doing at the end of the runs: generation 10,000! 

```r
exp2_10000_data <- exp2_data[exp2_data$update == 10000,]
```

**Lumos, results!**
![](stats_files/figure-html/unnamed-chunk-24-1.png)<!-- -->

Only the 75% and 87.5% treatments produce any optimal solutions. Exact name matching is _still_ bad. Before we get too much further, I want to put this out there: these particular results (i.e. which treatments are able to produce optimal solutions) are entirely dependent on how I am representing tags, the number of environment states, and the number of distraction signals. My point here is a more general one: sometimes _some_ precision is important! The amount of necessary precision will vary problem to problem. 

Okay, now we do some statistics. First a Kruskal-Wallis test.

```r
exp2_10000_kt <- kruskal.test(fitness ~ sim_thresh, data=exp2_10000_data)
exp2_10000_kt
```

```
## 
## 	Kruskal-Wallis rank sum test
## 
## data:  fitness by sim_thresh
## Kruskal-Wallis chi-squared = 193, df = 8, p-value < 2.2e-16
```


As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc pairwise Wilcoxon rank-sum test to analyze which treatments are significantly different from one another.

```r
exp2_10000_wt <- pairwise.wilcox.test(x=exp2_10000_data$fitness, g=exp2_10000_data$sim_thresh, p.adjust.method="bonferroni", exact=FALSE)
exp2_10000_wt
```

```
## 
## 	Pairwise comparisons using Wilcoxon rank sum test 
## 
## data:  exp2_10000_data$fitness and exp2_10000_data$sim_thresh 
## 
##       0       0.125   0.25    0.375   0.5     0.625   0.75    0.875  
## 0.125 1.000   -       -       -       -       -       -       -      
## 0.25  1.000   1.000   -       -       -       -       -       -      
## 0.375 1.000   1.000   1.000   -       -       -       -       -      
## 0.5   1.000   1.000   1.000   1.000   -       -       -       -      
## 0.625 0.240   1.000   0.072   0.183   0.065   -       -       -      
## 0.75  4.4e-11 4.4e-11 4.4e-11 4.4e-11 4.4e-11 4.4e-11 -       -      
## 0.875 1.4e-09 1.4e-09 1.4e-09 1.4e-09 1.4e-09 2.7e-09 7.2e-06 -      
## 1     1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.1e-09 1.1e-09 4.4e-11 1.4e-09
## 
## P value adjustment method: bonferroni
```

The statistical results confirm what we were seeing in the box plot. The 75% treatment and 87.5% treatment produce significantly better performing agents than all other treatments, and exact name matching is still terrible. In fact, only 75% and 87.5% treatments produced _any_ optimal programs. 

This supports the idea that _some_ amount of precision when making tag-based referencing can be important for certain classes of problems; though, allowing for _some_ inexactness is still important, requiring exact name matching is not particularly evolvable. 

### Exploring the importance of _some_ precision with MAP-Elites.
We'll use MAP-Elites for the distracting environment problem in the same way we used it for the changing enviroment problem. 

We've already loaded and filtered the relevant data by fitness. We just need to filter down to distracting environment problem MAPE-Elites data.

```r
mape2_data <- mape_data[mape_data$distraction_sigs == 1,]
```

**Visualization!**

```python
# A few constants
min_sim_thresh = 0.0
max_sim_thresh = 1.0
min_fun_used = 0
max_fun_used = 32
xy_label_fs = 14
xy_tick_fs = 12
cmap = sns.cubehelix_palette(as_cmap=True)
# Visualize!
fig = plt.figure(1)
#fig.set_size_inches(5,5)
with sns.axes_style("white"):
    g = sns.jointplot(data=r.mape2_data, x="sim_thresh", y="fun_used", kind="kde",
                      xlim=(min_sim_thresh, max_sim_thresh), ylim=(min_fun_used, max_fun_used),
                      stat_func=None, shade=True, cmap=cmap, shade_lowest=False, color="Grey")
    g.set_axis_labels("Similarity Threshold", "Functions Used")
    ax = g.ax_joint
    ax.xaxis.label.set_fontsize(xy_label_fs)
    ax.yaxis.label.set_fontsize(xy_label_fs)
    for tick in ax.get_xticklabels():
        tick.set_fontsize(xy_tick_fs)
    for tick in ax.get_yticklabels():
        tick.set_fontsize(xy_tick_fs)
plt.show()
```

![](stats_files/figure-html/unnamed-chunk-28-1.png)<!-- -->

This heatmap is very neat. It confirms our intuitions about the solution space in the distracting environment problem, showing that many strategies exist with around 32 functions where dummy do-nothing functions can consume distraction signals. There are solutions that use only 16 functions, but they exist only at high minimum similarity thresholds. And, there are a number of solutions that use a combination of extra do-nothing functions and referencing precision to successfully ignore distraction signals. 

## References
Lalejini, A., & Ofria, C. (2018). Evolving Event-driven Programs with SignalGP. In Proceedings of the Genetic and Evolutionary Computation Conference. ACM. https://doi.org/10.1145/3205455.3205523

Mouret, J., & Clune, J. (2015). Illuminating search spaces by mapping elites, 1–15. Retrieved from http://arxiv.org/abs/1504.04909

Ogle, D.H. 2017. FSA: Fisheries Stock Analysis. R package version 0.8.17.

R Core Team (2016). R: A language and environment for statistical computing. R Foundation for Statistical Computing, Vienna, Austria. URL https://www.R-project.org/.

Spector, L., Harringtion, K., Martin, B., & Helmuth, T. (2011a). What’s in an Evolved Name? The Evolution of Modularity via Tag-Based Reference. In R. Riolo, E. Vladislavleva, & J. H. Moore (Eds.), Genetic Programming Theory and Practice IX (pp. 1–16). New York, NY: Springer New York. https://doi.org/10.1007/978-1-4614-1770-5

Spector, L., Martin, B., Harrington, K., & Helmuth, T. (2011b). Tag-based modules in genetic programming. GECCO ’11: Proceedings of the 13th Annual Conference on Genetic and Evolutionary Computation, 1419–1426. https://doi.org/doi:10.1145/2001576.2001767

Spector, L., Harrington, K., & Helmuth, T. (2012). Tag-based modularity in tree-based genetic programming. GECCO ’12: Proceedings of the Fourteenth International Conference on Genetic and Evolutionary Computation Conference, 815–822. https://doi.org/doi:10.1145/2330163.2330276

Michael Waskom, Olga Botvinnik, Drew O'Kane, Paul Hobson, Saulius Lukauskas, David C Gemperline, … Adel Qalieh. (2017, September 3). mwaskom/seaborn: v0.8.1 (September 2017) (Version v0.8.1). Zenodo. http://doi.org/10.5281/zenodo.883859
