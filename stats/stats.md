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
Here, we analyze our experimental results from our 2018 GPTP contribution "What's in an evolved name? Exploring evolvable specificity with SignalGP ([citation to come]). In this work, we use SignalGP to explore how important allowing for inexact matching is in tag-based referencing. 

## Dependencies & General Setup
We used R version 3.3.2 (2016-10-31) for statistical analyses.

All Dunn's tests were performed using the FSA package (Ogle, 2017). 

```r
library(FSA)
```

```
## ## FSA v0.8.20. See citation('FSA') if used in publication.
## ## Run fishR() for related website and fishR('IFAR') for related book.
```

In addition to R, we use Python 3 for data manipulation and visualization (because Python!). To get Python and R to play nice, we use the [reticulate](https://rstudio.github.io/reticulate/index.html) R package. 

```r
library(reticulate)
use_python("/anaconda3/bin/python")
```

Make sure R markdown is using the reticulate Python engine. 

```r
knitr::knit_engines$set(python = reticulate::eng_python)
```

All visualizations use the seaborn Python package ([CITE]), which sits on top of Python's matplotlib, providing a "high-level interface for drawing attractive statistical graphics". 

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
Again, see the paper [CITE] for context on these data analyses, but here's some high-level background. 

### Briefly, tag-based referencing. 
What's in an evolved name? How should modules (e.g. functions, sub-routines, data-objects, etc) be referenced in evolving programs? In traditional software development, the programmer hand-labels modules and subsequently refers to them using their assigned label. This technique for referencing modules is intentionally rigid, requiring programmers to precisely name the module they aim to reference; imprecision often results in syntactic incorrectness. Requiring evolving programs to follow traditional approaches to module referencing is not ideal: mutation operators must do extra work to guarantee label-correctness, else mutated programs are likely to make use of undefined labels, resulting in syntactic invalidity [(Spector et al., 2011a)]. Instead, is genetic programming (GP) better off relying on more flexible, less exacting referencing schemes?

**Enter: tag-based referencing.** Spector et [CITE] Spector et al., 2011b,a, 2012) introduced and demonstrated a tag-based naming scheme for GP where tags are used to name and reference program modules. Tags are evolvable labels that are mutable, and the similarity (or dissimilarity) between any two possible tags is quantifiable. Tags allow for inexact referencing. Because the similarity between tags can be calculated, a referring tag can always link to the program module with the most similar (_i.e._ closest matching) tag; further, this ensures that all possible tags are valid references. Because tags are mutable, evolution can incrementally shape tag-based references within evolving code.

In Spector et al.'s original conception of tag-based referencing, if any tagged entity (program module, etc) existed, a referring tag would always have _something_ to reference; there was no minimum similarity requirement for tags to successfully match. In other words, references allowed for _complete_ imprecision. We can, however, put a minimum threshold of similarity on tag-based references: for two tags to be successfully matched, they must be at least X% similar. Reminder: we represent tags as bit strings in our work, using the [simple matching coefficient](https://en.wikipedia.org/wiki/Simple_matching_coefficient) to define the similarity between two tags. 

### Just tell me what questions you're asking. 
Okay, okay. Here, we explore the effects of adjusting the minimum required similarity for tags to successfully match for a reference. How important is imprecision when doing tag-based referencing? Is exact name matching (what human programmers usually do) really all that bad for evolution? (**spoiler**: yes!) Is it important to allow for _complete_ inexactness as in Spector et al.'s original work? Or, is it fine to require a minimum threshold of similarity? Are there situations where requiring _some_ amount of precision (_i.e._ a minimum match similarity) is necessary? 

We use evolving populations of SignalGP agents to address these questions. See [CITE] for details on exactly how SignalGP makes use of tag-based referencing. In short, SignalGP labels program modules with tags, and externally-generated or internally-generated can refer to SignalGP program modules via tag-based referencing. 

## Experiment 1: How important is inexactness in tag-based referencing?
How important is imprecision when calling an evolvable name? As discussed above, tag-based referencing has built-in flexibility, not requiring tags to _exactly_ match to successfully reference one another. To explore the importance of inexactness in tag-based referencing, we evolved populations (30 replicates) of SignalGP agents to solve the changing environment problem under nine different conditions where each condition required tags to have a different minimum similarity in order to successfully be matched: 0% (_i.e._, no restrictions on tag-similarity), 12.5%, 25.0%, 37.5%, 50.0%, 62.5%, 75.0%, 87.5%, and 100.0% (_i.e._, tags needed to match exactly). 

### The Changing Environment Problem
See [CITE, CITE] for a detailed description of the changing environment problem. The changing environment problem is a toy problem to test GP programs' capacity to respond appropriately to environmental signals. 

The changing environment problem requires agents to coordinate their behavior with a randomly changing environment. In these experiments, the environment can be in one of sixteen states at any one time. At the beginning of a run, the environment is initialized to a random state, and at every subsequent time step, the environment has a 12.5% chance of randomly changing. To be successful, agents must match their internal state to the current state of the environment; thus, agents must monitor the environment for changes, adjusting their internal state as appropriate. 

Environmental changes produce signals (events) with environment-specific tags, which can trigger SignalGP functions (program modules); in this way, SignalGP agents can monitor for and respond to environmental changes. In this experiment, we limit SignalGP programs to a maximum of 16 functions, which means that optimal agents _must_ have sixteen functions, _each_ function's tag must ensure that it is only triggered by a single environment change, and each function must adjust an agent's internal state appropriately.  

### Statistical Methods
For each condition, we have 30 replicates, and for each replicate, we extract the dominant (highest-fitness) agent at generation 1,000 and generation 10,000. To account for stochasticity in the environment, we test each extracted agent 100 times and record its average performance over those 100 trials; this performance score is used as the agent's fitness in our analyses. 

For each generation analyzed, we compared the performances of evolved, dominant programs across treatments. Let's set our significance level, $\alpha$, to...

```r
sig_level <- 0.05
```

To determine if any of the treatments were significant (_p_ < `sig_level`) for a given generation, we performed a [Kruskal-Wallis rank sum test](https://en.wikipedia.org/wiki/Kruskal%E2%80%93Wallis_one-way_analysis_of_variance). For a generation in which the Kruskal-Wallis test was significant, we performed a post-hoc Dunn's test, applying a Bonferroni correction for multiple comparisons. 

### Results
Finally, enough with all of those long-winded explanations! Results! 

Well, first we need to actually load in some data. Once we do this, we don't have to do it again.
We'll go ahead and load evolution data with R. 

```r
evo_data <- read.csv("../data/evo_dom.csv")
```

That evo_dom file has spoiler data in! I.e., it has the data for both Experiment 1 and Experiment 2 described in this document. Let's filter down to just the data relevant to Experiment 1. 

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

![](stats_files/figure-html/unnamed-chunk-9-1.png)<!-- -->

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

As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc Dunn's test to analyze which treatments are significantly different from one another. 

```r
exp1_1000_dt <- dunnTest(fitness ~ sim_thresh, data=exp1_1000_data, method="bonferroni")
exp1_1000_dt
```

```
## Dunn (1964) Kruskal-Wallis multiple comparison
```

```
##   p-values adjusted with the Bonferroni method.
```

```
##       Comparison           Z      P.unadj        P.adj
## 1      0 - 0.125 -0.75959350 4.474976e-01 1.000000e+00
## 2       0 - 0.25 -0.29859299 7.652506e-01 1.000000e+00
## 3   0.125 - 0.25  0.46100051 6.447982e-01 1.000000e+00
## 4      0 - 0.375 -1.26288764 2.066296e-01 1.000000e+00
## 5  0.125 - 0.375 -0.50329413 6.147575e-01 1.000000e+00
## 6   0.25 - 0.375 -0.96429465 3.348982e-01 1.000000e+00
## 7        0 - 0.5 -0.49652715 6.195225e-01 1.000000e+00
## 8    0.125 - 0.5  0.26306635 7.924994e-01 1.000000e+00
## 9     0.25 - 0.5 -0.19793416 8.430966e-01 1.000000e+00
## 10   0.375 - 0.5  0.76636048 4.434618e-01 1.000000e+00
## 11     0 - 0.625 -0.27490856 7.833865e-01 1.000000e+00
## 12 0.125 - 0.625  0.48468494 6.278999e-01 1.000000e+00
## 13  0.25 - 0.625  0.02368443 9.811043e-01 1.000000e+00
## 14 0.375 - 0.625  0.98797907 3.231629e-01 1.000000e+00
## 15   0.5 - 0.625  0.22161859 8.246108e-01 1.000000e+00
## 16      0 - 0.75  2.00894718 4.454274e-02 1.000000e+00
## 17  0.125 - 0.75  2.76854068 5.630796e-03 2.027086e-01
## 18   0.25 - 0.75  2.30754017 2.102473e-02 7.568901e-01
## 19  0.375 - 0.75  3.27183481 1.068520e-03 3.846672e-02
## 20    0.5 - 0.75  2.50547433 1.222873e-02 4.402342e-01
## 21  0.625 - 0.75  2.28385574 2.238001e-02 8.056804e-01
## 22     0 - 0.875  5.93548730 2.929740e-09 1.054706e-07
## 23 0.125 - 0.875  6.69508080 2.155525e-11 7.759890e-10
## 24  0.25 - 0.875  6.23408029 4.544400e-10 1.635984e-08
## 25 0.375 - 0.875  7.19837494 6.093440e-13 2.193638e-11
## 26   0.5 - 0.875  6.43201446 1.259238e-10 4.533255e-09
## 27 0.625 - 0.875  6.21039586 5.285129e-10 1.902646e-08
## 28  0.75 - 0.875  3.92654012 8.617652e-05 3.102355e-03
## 29         0 - 1  7.45805779 8.780716e-14 3.161058e-12
## 30     0.125 - 1  8.21765130 2.075273e-16 7.470984e-15
## 31      0.25 - 1  7.75665078 8.720162e-15 3.139258e-13
## 32     0.375 - 1  8.72094543 2.758884e-18 9.931982e-17
## 33       0.5 - 1  7.95458495 1.797330e-15 6.470389e-14
## 34     0.625 - 1  7.73296636 1.050691e-14 3.782487e-13
## 35      0.75 - 1  5.44911062 5.062232e-08 1.822404e-06
## 36     0.875 - 1  1.52257049 1.278662e-01 1.000000e+00
```

Woah! That's a big wall of stats! Let's write a little bit of code to pull out the significant relationships. Because I like Python, I'll do the pulling in Python, but first, some R to make things easier on the Python side. 

```r
exp1_1000_dt_comp <- exp1_1000_dt$res$Comparison
exp1_1000_dt_padj <- exp1_1000_dt$res$P.adj
exp1_1000_dt_z <- exp1_1000_dt$res$Z
```
Now, Python!

```python
# Combine all of the dunn test results into a more useful data structure.
exp1_1000_dt_results = [{"comparison": r.exp1_1000_dt_comp[i], "p-value": r.exp1_1000_dt_padj[i], "z": r.exp1_1000_dt_z[i], "sig": r.exp1_1000_dt_padj[i] < r.sig_level} for i in range(0, len(r.exp1_1000_dt_comp))]
```
Here are the treatments with significant differences:

```python
for comp in exp1_1000_dt_results:
  if comp["sig"]:
    print(comp["comparison"])
```

```
## 0.375 - 0.75
## 0 - 0.875
## 0.125 - 0.875
## 0.25 - 0.875
## 0.375 - 0.875
## 0.5 - 0.875
## 0.625 - 0.875
## 0.75 - 0.875
## 0 - 1
## 0.125 - 1
## 0.25 - 1
## 0.375 - 1
## 0.5 - 1
## 0.625 - 1
## 0.75 - 1
```
Here are all the treatments with no significant differences:

```python
for comp in exp1_1000_dt_results:
  if not comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.125
## 0 - 0.25
## 0.125 - 0.25
## 0 - 0.375
## 0.125 - 0.375
## 0.25 - 0.375
## 0 - 0.5
## 0.125 - 0.5
## 0.25 - 0.5
## 0.375 - 0.5
## 0 - 0.625
## 0.125 - 0.625
## 0.25 - 0.625
## 0.375 - 0.625
## 0.5 - 0.625
## 0 - 0.75
## 0.125 - 0.75
## 0.25 - 0.75
## 0.5 - 0.75
## 0.625 - 0.75
## 0.875 - 1
```

In general, the two treatments with the highest minimum similarity thresholds (87.5% and 100.0%) seem to be worse than the others, while (in general) there aren't real differences between the others. Let's move on to the end of the run. 

#### Results after 10,000 generations of evolution.
Let's take a look at how things are doing at the end of the runs: generation 10,000! Let's grab only data relevant to generation 10,000.

```r
exp1_10000_data <- exp1_data[exp1_data$update == 10000,]
```

**Visualize!**

![](stats_files/figure-html/unnamed-chunk-17-1.png)<!-- -->

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

As expected, the Kruskal-Wallis test comes out significant. Let's do a post-hoc Dunn's test to analyze which treatments are significantly different from one another. 

```r
exp1_10000_dt <- dunnTest(fitness ~ sim_thresh, data=exp1_10000_data, method="bonferroni")
exp1_10000_dt
```

```
## Dunn (1964) Kruskal-Wallis multiple comparison
```

```
##   p-values adjusted with the Bonferroni method.
```

```
##       Comparison         Z      P.unadj        P.adj
## 1      0 - 0.125  0.283976 7.764288e-01 1.000000e+00
## 2       0 - 0.25  0.000000 1.000000e+00 1.000000e+00
## 3   0.125 - 0.25 -0.283976 7.764288e-01 1.000000e+00
## 4      0 - 0.375  0.000000 1.000000e+00 1.000000e+00
## 5  0.125 - 0.375 -0.283976 7.764288e-01 1.000000e+00
## 6   0.25 - 0.375  0.000000 1.000000e+00 1.000000e+00
## 7        0 - 0.5  0.000000 1.000000e+00 1.000000e+00
## 8    0.125 - 0.5 -0.283976 7.764288e-01 1.000000e+00
## 9     0.25 - 0.5  0.000000 1.000000e+00 1.000000e+00
## 10   0.375 - 0.5  0.000000 1.000000e+00 1.000000e+00
## 11     0 - 0.625  0.000000 1.000000e+00 1.000000e+00
## 12 0.125 - 0.625 -0.283976 7.764288e-01 1.000000e+00
## 13  0.25 - 0.625  0.000000 1.000000e+00 1.000000e+00
## 14 0.375 - 0.625  0.000000 1.000000e+00 1.000000e+00
## 15   0.5 - 0.625  0.000000 1.000000e+00 1.000000e+00
## 16      0 - 0.75  0.000000 1.000000e+00 1.000000e+00
## 17  0.125 - 0.75 -0.283976 7.764288e-01 1.000000e+00
## 18   0.25 - 0.75  0.000000 1.000000e+00 1.000000e+00
## 19  0.375 - 0.75  0.000000 1.000000e+00 1.000000e+00
## 20    0.5 - 0.75  0.000000 1.000000e+00 1.000000e+00
## 21  0.625 - 0.75  0.000000 1.000000e+00 1.000000e+00
## 22     0 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 23 0.125 - 0.875  4.278572 1.880964e-05 6.771470e-04
## 24  0.25 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 25 0.375 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 26   0.5 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 27 0.625 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 28  0.75 - 0.875  4.562548 5.053661e-06 1.819318e-04
## 29         0 - 1 10.828951 2.510133e-27 9.036478e-26
## 30     0.125 - 1 10.544975 5.358688e-26 1.929128e-24
## 31      0.25 - 1 10.828951 2.510133e-27 9.036478e-26
## 32     0.375 - 1 10.828951 2.510133e-27 9.036478e-26
## 33       0.5 - 1 10.828951 2.510133e-27 9.036478e-26
## 34     0.625 - 1 10.828951 2.510133e-27 9.036478e-26
## 35      0.75 - 1 10.828951 2.510133e-27 9.036478e-26
## 36     0.875 - 1  6.266404 3.694817e-10 1.330134e-08
```

Let's write a little bit of code to pull out the significant relationships. Again, some R code then some Python code.

```r
exp1_10000_dt_comp <- exp1_10000_dt$res$Comparison
exp1_10000_dt_padj <- exp1_10000_dt$res$P.adj
exp1_10000_dt_z <- exp1_10000_dt$res$Z
```

```python
# Combine all of the dunn test results into a more useful data structure.
exp1_10000_dt_results = [{"comparison": r.exp1_10000_dt_comp[i], "p-value": r.exp1_10000_dt_padj[i], "z": r.exp1_10000_dt_z[i], "sig": r.exp1_10000_dt_padj[i] < r.sig_level} for i in range(0, len(r.exp1_10000_dt_comp))]
```
Here are the treatments with significant differences: [TODO: make this a heatmap]

```python
for comp in exp1_10000_dt_results:
  if comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.875
## 0.125 - 0.875
## 0.25 - 0.875
## 0.375 - 0.875
## 0.5 - 0.875
## 0.625 - 0.875
## 0.75 - 0.875
## 0 - 1
## 0.125 - 1
## 0.25 - 1
## 0.375 - 1
## 0.5 - 1
## 0.625 - 1
## 0.75 - 1
## 0.875 - 1
```
Here are all the treatments with no significant differences:

```python
for comp in exp1_10000_dt_results:
  if not comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.125
## 0 - 0.25
## 0.125 - 0.25
## 0 - 0.375
## 0.125 - 0.375
## 0.25 - 0.375
## 0 - 0.5
## 0.125 - 0.5
## 0.25 - 0.5
## 0.375 - 0.5
## 0 - 0.625
## 0.125 - 0.625
## 0.25 - 0.625
## 0.375 - 0.625
## 0.5 - 0.625
## 0 - 0.75
## 0.125 - 0.75
## 0.25 - 0.75
## 0.375 - 0.75
## 0.5 - 0.75
## 0.625 - 0.75
```

These data support the idea that exact name matching is not very evolvable. In fact, requiring too much precision when calling an evolvable name (doing tag-based referencing) is not ideal. On the flip side, requiring _some_ precision for successful tag-based references is fine (_i.e._, 12.5% through 75% minimum thresholds are not significantly different than a 0.0% threshold). 

### Exploring the Importance of Inexactness with MAP-Elites
To further illuminate the space of similarity thresholds for SignalGP in the changing environment problem, we can use the MAP-Elites algorithm [CITE]. In MAP-Elites, a population is structured based on a set of chosen phenotypic traits of evolving solutions. Each chosen phenotypic trait defines an axis on a grid of cells where each cell represents a distinct combination of phenotypic traits; further, each cell maintains only the most fit solution 'discovered' with that cell's associated combination of phenotypic traits. We initialize the map in MAP-Elites by randomly generating a solution and placing it into its appropriate cell in the grid (based on its phenotypic characteristics). From here, we repeatedly select an occupied cell in the grid at random, generate a mutated offspring for that cell's occupant, and figure out where the offspring belongs in the grid; if that cell is empty, place the offspring there, otherwise compare the offspring's fitness to the current occupants, keeping the fitter of the two. We repeat this process for as long as we want, and we eventually end up with a grid of prospective solutions that span the range of the phenotypic traits we defined as our axes. Neat! 

We can apply this algorithm to GP (in fact, we have an entire paper on this idea! [CITE]) where phenotypic traits are program characteristics (_e.g._, program length/size, module count, instruction entropy, etc.). Here, we apply MAP-Elites to illuminate the relationship between function count and similarity threshold for SignalGP agents in the context of the changing environment problem. 

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
![](stats_files/figure-html/unnamed-chunk-26-1.png)<!-- -->

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
![](stats_files/figure-html/unnamed-chunk-29-1.png)<!-- -->

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

As expected, the Kruskal-Wallis test is significant. Next, let's do a post-hoc Dunn's test to analyze which treatments are statistically different. 

```r
exp2_1000_dt <- dunnTest(fitness ~ sim_thresh, data=exp2_1000_data, method="bonferroni")
exp2_1000_dt
```

```
## Dunn (1964) Kruskal-Wallis multiple comparison
```

```
##   p-values adjusted with the Bonferroni method.
```

```
##       Comparison           Z      P.unadj        P.adj
## 1      0 - 0.125 -0.92927718 3.527455e-01 1.000000e+00
## 2       0 - 0.25 -0.66210999 5.079007e-01 1.000000e+00
## 3   0.125 - 0.25  0.26716719 7.893404e-01 1.000000e+00
## 4      0 - 0.375 -0.90272640 3.666711e-01 1.000000e+00
## 5  0.125 - 0.375  0.02655078 9.788180e-01 1.000000e+00
## 6   0.25 - 0.375 -0.24061641 8.098524e-01 1.000000e+00
## 7        0 - 0.5 -0.04812328 9.616180e-01 1.000000e+00
## 8    0.125 - 0.5  0.88115389 3.782345e-01 1.000000e+00
## 9     0.25 - 0.5  0.61398671 5.392241e-01 1.000000e+00
## 10   0.375 - 0.5  0.85460312 3.927709e-01 1.000000e+00
## 11     0 - 0.625 -1.09024126 2.756069e-01 1.000000e+00
## 12 0.125 - 0.625 -0.16096408 8.721217e-01 1.000000e+00
## 13  0.25 - 0.625 -0.42813127 6.685556e-01 1.000000e+00
## 14 0.375 - 0.625 -0.18751486 8.512570e-01 1.000000e+00
## 15   0.5 - 0.625 -1.04211798 2.973570e-01 1.000000e+00
## 16      0 - 0.75 -6.63935354 3.150619e-11 1.134223e-09
## 17  0.125 - 0.75 -5.71007636 1.129255e-08 4.065318e-07
## 18   0.25 - 0.75 -5.97724355 2.269448e-09 8.170012e-08
## 19  0.375 - 0.75 -5.73662714 9.658056e-09 3.476900e-07
## 20    0.5 - 0.75 -6.59123025 4.361968e-11 1.570308e-09
## 21  0.625 - 0.75 -5.54911228 2.871237e-08 1.033645e-06
## 22     0 - 0.875 -1.42392250 1.544689e-01 1.000000e+00
## 23 0.125 - 0.875 -0.50255421 6.152777e-01 1.000000e+00
## 24  0.25 - 0.875 -0.76744759 4.428154e-01 1.000000e+00
## 25 0.375 - 0.875 -0.52887901 5.968894e-01 1.000000e+00
## 26   0.5 - 0.875 -1.37620878 1.687570e-01 1.000000e+00
## 27 0.625 - 0.875 -0.34296006 7.316285e-01 1.000000e+00
## 28  0.75 - 0.875  5.15892488 2.483719e-07 8.941390e-06
## 29         0 - 1  5.23216238 1.675385e-07 6.031385e-06
## 30     0.125 - 1  6.16143956 7.208658e-10 2.595117e-08
## 31      0.25 - 1  5.89427237 3.763360e-09 1.354810e-07
## 32     0.375 - 1  6.13488878 8.521879e-10 3.067876e-08
## 33       0.5 - 1  5.28028567 1.289826e-07 4.643375e-06
## 34     0.625 - 1  6.32240364 2.575256e-10 9.270922e-09
## 35      0.75 - 1 11.87151592 1.664244e-32 5.991277e-31
## 36     0.875 - 1  6.61155503 3.803037e-11 1.369093e-09
```
As before, let's write some R/Python code to pull out significant vs. not significant relationships. 

```r
exp2_1000_dt_comp <- exp2_1000_dt$res$Comparison
exp2_1000_dt_padj <- exp2_1000_dt$res$P.adj
exp2_1000_dt_z <- exp2_1000_dt$res$Z
```

```python
# Combine all the dunn test results into a useful python data structure.
exp2_1000_dt_results = [{"comparison": r.exp2_1000_dt_comp[i], "p-value": r.exp2_1000_dt_padj[i], "z": r.exp2_1000_dt_z[i], "sig": r.exp2_1000_dt_padj[i] < r.sig_level} for i in range(0, len(r.exp2_1000_dt_comp))]
```
Treatments with significant differences:

```python
for comp in exp2_1000_dt_results:
  if comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.75
## 0.125 - 0.75
## 0.25 - 0.75
## 0.375 - 0.75
## 0.5 - 0.75
## 0.625 - 0.75
## 0.75 - 0.875
## 0 - 1
## 0.125 - 1
## 0.25 - 1
## 0.375 - 1
## 0.5 - 1
## 0.625 - 1
## 0.75 - 1
## 0.875 - 1
```

Treatments that are not significantly different:

```python
for comp in exp2_1000_dt_results:
  if not comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.125
## 0 - 0.25
## 0.125 - 0.25
## 0 - 0.375
## 0.125 - 0.375
## 0.25 - 0.375
## 0 - 0.5
## 0.125 - 0.5
## 0.25 - 0.5
## 0.375 - 0.5
## 0 - 0.625
## 0.125 - 0.625
## 0.25 - 0.625
## 0.375 - 0.625
## 0.5 - 0.625
## 0 - 0.875
## 0.125 - 0.875
## 0.25 - 0.875
## 0.375 - 0.875
## 0.5 - 0.875
## 0.625 - 0.875
```

#### Results after 10,000 generations of evolution.
Let's take a look at how things are doing at the end of the runs: generation 10,000! 

```r
exp2_10000_data <- exp2_data[exp2_data$update == 10000,]
```

**Lumos, results!**
![](stats_files/figure-html/unnamed-chunk-37-1.png)<!-- -->

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

As expected from looking at the visualiztion, the Kruskal-Wallis test is significant. Next let's do a post-hoc Dunn's test to analyze which treatments are different. 

```r
exp2_10000_dt <- dunnTest(fitness ~ sim_thresh, data=exp2_10000_data, method="bonferroni")
exp2_10000_dt
```

```
## Dunn (1964) Kruskal-Wallis multiple comparison
```

```
##   p-values adjusted with the Bonferroni method.
```

```
##       Comparison           Z      P.unadj        P.adj
## 1      0 - 0.125 -0.73882920 4.600107e-01 1.000000e+00
## 2       0 - 0.25  0.26926958 7.877222e-01 1.000000e+00
## 3   0.125 - 0.25  1.00809878 3.134070e-01 1.000000e+00
## 4      0 - 0.375 -0.08393898 9.331050e-01 1.000000e+00
## 5  0.125 - 0.375  0.65489022 5.125384e-01 1.000000e+00
## 6   0.25 - 0.375 -0.35320856 7.239321e-01 1.000000e+00
## 7        0 - 0.5  0.34074237 7.332975e-01 1.000000e+00
## 8    0.125 - 0.5  1.07957157 2.803330e-01 1.000000e+00
## 9     0.25 - 0.5  0.07147279 9.430215e-01 1.000000e+00
## 10   0.375 - 0.5  0.42468135 6.710690e-01 1.000000e+00
## 11     0 - 0.625 -1.89652219 5.789103e-02 1.000000e+00
## 12 0.125 - 0.625 -1.15769299 2.469893e-01 1.000000e+00
## 13  0.25 - 0.625 -2.16579178 3.032710e-02 1.000000e+00
## 14 0.375 - 0.625 -1.81258322 6.989613e-02 1.000000e+00
## 15   0.5 - 0.625 -2.23726457 2.526905e-02 9.096859e-01
## 16      0 - 0.75 -6.78243541 1.181667e-11 4.254001e-10
## 17  0.125 - 0.75 -6.04360621 1.507071e-09 5.425455e-08
## 18   0.25 - 0.75 -7.05170500 1.767386e-12 6.362590e-11
## 19  0.375 - 0.75 -6.69849644 2.105749e-11 7.580695e-10
## 20    0.5 - 0.75 -7.12317779 1.054666e-12 3.796799e-11
## 21  0.625 - 0.75 -4.88591322 1.029505e-06 3.706219e-05
## 22     0 - 0.875 -5.75891986 8.465388e-09 3.047540e-07
## 23 0.125 - 0.875 -5.02637868 4.998284e-07 1.799382e-05
## 24  0.25 - 0.875 -6.02589774 1.681735e-09 6.054246e-08
## 25 0.375 - 0.875 -5.67569527 1.381265e-08 4.972553e-07
## 26   0.5 - 0.875 -6.09676224 1.082383e-09 3.896579e-08
## 27 0.625 - 0.875 -3.87853857 1.050859e-04 3.783091e-03
## 28  0.75 - 0.875  0.96579165 3.341484e-01 1.000000e+00
## 29         0 - 1  4.88591322 1.029505e-06 3.706219e-05
## 30     0.125 - 1  5.62474242 1.857850e-08 6.688259e-07
## 31      0.25 - 1  4.61664364 3.899961e-06 1.403986e-04
## 32     0.375 - 1  4.96985219 6.700396e-07 2.412143e-05
## 33       0.5 - 1  4.54517085 5.489071e-06 1.976066e-04
## 34     0.625 - 1  6.78243541 1.181667e-11 4.254001e-10
## 35      0.75 - 1 11.66834863 1.849790e-31 6.659244e-30
## 36     0.875 - 1 10.60325008 2.878011e-26 1.036084e-24
```

Again, some Python/R code to pull out significantly different treatments.

```r
exp2_10000_dt_comp <- exp2_10000_dt$res$Comparison
exp2_10000_dt_padj <- exp2_10000_dt$res$P.adj
exp2_10000_dt_z <- exp2_10000_dt$res$Z
```

```python
# Combine all the dunn test results into a useful python data structure.
exp2_10000_dt_results = [{"comparison": r.exp2_10000_dt_comp[i], "p-value": r.exp2_10000_dt_padj[i], "z": r.exp2_10000_dt_z[i], "sig": r.exp2_10000_dt_padj[i] < r.sig_level} for i in range(0, len(r.exp2_10000_dt_comp))]
```
Treatments with significant differences:

```python
for comp in exp2_10000_dt_results:
  if comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.75
## 0.125 - 0.75
## 0.25 - 0.75
## 0.375 - 0.75
## 0.5 - 0.75
## 0.625 - 0.75
## 0 - 0.875
## 0.125 - 0.875
## 0.25 - 0.875
## 0.375 - 0.875
## 0.5 - 0.875
## 0.625 - 0.875
## 0 - 1
## 0.125 - 1
## 0.25 - 1
## 0.375 - 1
## 0.5 - 1
## 0.625 - 1
## 0.75 - 1
## 0.875 - 1
```

Treatments that are not significantly different:

```python
for comp in exp2_10000_dt_results:
  if not comp["sig"]:
    print(comp["comparison"])
```

```
## 0 - 0.125
## 0 - 0.25
## 0.125 - 0.25
## 0 - 0.375
## 0.125 - 0.375
## 0.25 - 0.375
## 0 - 0.5
## 0.125 - 0.5
## 0.25 - 0.5
## 0.375 - 0.5
## 0 - 0.625
## 0.125 - 0.625
## 0.25 - 0.625
## 0.375 - 0.625
## 0.5 - 0.625
## 0.75 - 0.875
```

#### Exploring the importance of _some_ precision with MAP-Elites.
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

![](stats_files/figure-html/unnamed-chunk-45-1.png)<!-- -->


## References

Ogle, D.H. 2017. FSA: Fisheries Stock Analysis. R package version 0.8.17.

R Core Team (2016). R: A language and environment for statistical computing. R Foundation for Statistical Computing, Vienna, Austria. URL https://www.R-project.org/.

Michael Waskom, Olga Botvinnik, Drew O'Kane, Paul Hobson, Saulius Lukauskas, David C Gemperline, … Adel Qalieh. (2017, September 3). mwaskom/seaborn: v0.8.1 (September 2017) (Version v0.8.1). Zenodo. http://doi.org/10.5281/zenodo.883859
