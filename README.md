# What else is in an evolved name? Exploring Evolvable Specificity with SignalGP


# Changing Environment Task/Environment Coordination Task
The changing environment problem is designed to be a simple toy problem that tests the capacity for SignalGP agents to coordinate with external signals. 

The changing environment problem requires agents to coordinate their behavior with a randomly changing environment. The environment can be in one of *K* possible states; to maximize fitness, agents must match their internal state to the current state of their environment. The environment is initialized to a random state and has a configurable chance of changing to a random state at every subsequent time step. Successful agents must adjust their internal state whenever an environmental change occurs. 

The problem scales in difficulty as *K* increases. Agents adjust their internal state by executing one of *K* state-altering instructions. For each possible environmental state, there is an associated SetState instruction. Being required to execute a distinct instruction for each environment represents performing a behavior unique to that environment. 

Agents can monitor environmental changes in a two ways: 1) by responding to signals produced by environmental changes and 2) by using sensory instructions to poll the environment for changes.

I've used this or a variant of this problem in the following papers: (Lalejini and Ofria, 2018). 

# [Data Analyses](stats/stats.html)


# References
Lalejini, A., & Ofria, C. (2018). Evolving Event-driven Programs with SignalGP. In Proceedings of the Genetic and Evolutionary Computation Conference. ACM. https://doi.org/10.1145/3205455.3205523