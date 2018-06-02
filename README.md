## Overview
This site is associated with our 2018 Genetic Programming Theory and Practice (GPTP) workshop contribution, citation pending. 

### Contribution Authors:
- [Alexander Lalejini](lalejini.com)
- [Charles Ofria](ofria.com)

### Abstract
> What else is in an evolved name? In this chapter, we continue to explore tag-based naming with SignalGP. 
> Tags are evolvable labels that provide genetic programming (GP) a flexible mechanism for specification. 
> Tags are used to label and refer to programmatic elements (e.g. code fragments). 
> However, tags differ from traditional, more rigid methods for labeling programmatic elements because they allow for _inexact_ references (i.e. a referring tag need not _exactly_ match its referent). 
> SignalGP is a GP technique designed to give evolution direct access to the event-driven paradigm where programs are organized as sets of modules, and signals (e.g. from the environment or from other agents) can trigger the execution of modules. SignalGP uses tag-based naming to determine which module is triggered in response to an event. 
> We use SignalGP to explore the importance of inexactness in tag-based referencing. Additionally, we discuss broadened applications of tag-based naming in the context of SignalGP. 

## Some Background 
From the paper:
> In Chapter 1 of Genetic Programming Theory and Practice IX (Spector et al, 2011a), Spector et al. explored the use of tag-based naming in evolving modular programs. 
> In this chapter, we continue exploring tag-based naming with SignalGP (Lalejini and ofria, 2018); we investigate the importance of inexactness when making tag-based references: How important is imprecision when calling an evolvable name? Additionally, we discuss possible broadened applications of tag-based naming in the context of SignalGP. 
> 
> What's in an evolved name?
> How should modules (e.g. functions, sub-routines, data-objects, etc) be referenced in evolving programs? 
> In traditional software development, the programmer hand-labels modules and subsequently refers to them using their assigned label. This technique for referencing modules is intentionally rigid, requiring programmers to precisely name the module they aim to reference; imprecision often results in syntactic incorrectness. 
> Requiring evolving programs to follow traditional approaches to module referencing is not ideal: mutation operators must do extra work to guarantee label-correctness, else mutated programs are likely to make use of undefined labels, resulting in syntactic invalidity (Spector et al, 2011a).  
> Instead, is genetic programming (GP) better off relying on more flexible, less exacting referencing schemes? 

**Enter: tag-based referencing**. 
Spector et [CITE] Spector et al., 2011b,a, 2012) introduced and demonstrated a tag-based naming scheme for GP where tags are used to name and reference program modules. Tags are evolvable labels that are mutable, and the similarity (or dissimilarity) between any two possible tags is quantifiable. Tags allow for inexact referencing. Because the similarity between tags can be calculated, a referring tag can always link to the program module with the most similar (i.e. closest matching) tag; further, this ensures that all possible tags are valid references. Because tags are mutable, evolution can incrementally shape tag-based references within evolving code.

How important is that _inexactness_ when evolving programs that make use of evolvable names (i.e. tag-based referencing)? And, are there scenarios where requiring _some_ exactness is necessary when making tag-based references? 

## What did we do?
In this work, we make two primary contributions:
1. We use SignalGP to explore the role of inexactness in tag-based referencing. How important is imprecision when evolving programs that make use of evolvable names (i.e. tag-based referencing)? And, are there scenarios where requiring _some_ precision is necessary when making tag-based references? 
2. We discuss three major extensions to SignalGP that are facilitated by tag-based referencing. 

### 1) Exploring the role of inexactness in tag-based referencing.
To explore role of inexactness in tag-based referencing, we performed performed experiments where we evolved populations of SignalGP agents to solve the changing environment problem and the distracting environment problem. 

**For more details, including data visualizations and analyses, see [our data analysis write-up](analysis/stats.html).**


### 2) What else is in an evolved name? Extensions to SignalGP made possible by the evolvable specificity afforded by tag-based referencing. 
We discuss three major extensions to SignalGP:
1. SignalGP Function Regulation
    - We can incorporate regulatory instructions that can down- or up-regulate SignalGP functions, altering regulated functions' chances of being referenced by tags. 
2. Multi-representation SignalGP
    - SignalGP functions are black boxes that can contain arbitrary computational substrates, allowing multiple representations to be employed within a single agent.
3. Major transitions in SignalGP (i.e. hierarchically organized SignalGP programs)
    - SignalGP functions can contain entire SignalGP agents!


### References
TODO