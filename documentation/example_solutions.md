# Example Program Solutions

Here, we give example program solutions for both the changing environment
and distracting environment problems.

## Changing Environment Problem

### Changing Environment Problem - Overview

Problem overview from our paper:

> The changing environment problem is a toy problem that we designed to test GP programs' capacity to respond appropriately to environmental signals. We have previously used this problem to demonstrate the value of the event-driven paradigm using SignalGP (Lalejini and Ofria, 2018).
>
> The changing environment problem requires agents to continually match their internal state with the current state of a stochastically changing environment.
The environment is initialized to a random state, and at every subsequent time step, the environment has a 12.5% chance of randomly changing to any of 16 possible states.
To be successful, agents must monitor the environment for changes, adjusting their internal state as appropriate.
>
>Environmental changes produce signals (events) with environment-specific tags that will trigger an appropriate SignalGP function; in this way, SignalGP agents can respond to environmental changes.
Each of the 16 environment states is associated with a distinct tag that is randomly generated at the beginning of a run. Agents adjust their internal state by executing one of 16 state-altering instructions (one for each possible environmental state). Thus, the optimal solution to this problem is a 16-function program where each function is triggered by a different environment signal, and functions, when triggered, adjust the agent's internal state appropriately.

### Changing Environment Problem - Example Solution(s)

## Distracting Environment Problem

### Distracting Environment Problem - Overview

Problem overview from our paper:

> The distracting environment problem is identical to the changing environment problem but with the addition of randomly occurring distraction signals.
> Like the changing environment problem, the environment can be in one of 16 states at any time with a 12.5% chance to change each update.
> Every time step there is also a 12.5% chance of a distraction event occurring, independent of environmental changes.
> Just as we randomly generate 16 distinct tags associated with each of the 16 environment states, we also generate 16 distinct distraction signal tags, which are guaranteed to not be identical to environment-state tags. Thus, to be successful, agents must monitor the environment for changes (adjusting their internal state as appropriate) while ignoring misleading distraction signals.

### Distracting Environment Problem - Example Solution(s)

## References

Lalejini, A., & Ofria, C. (2018). Evolving Event-driven Programs with SignalGP. In Proceedings of the Genetic and Evolutionary Computation Conference. ACM. [https://doi.org/10.1145/3205455.3205523](https://doi.org/10.1145/3205455.3205523)

---

## Perfect solution

## If we were to change this bit...this solution would work too

- if we made all bits off by one, as long as sim thresh was not... this solution would work too.

