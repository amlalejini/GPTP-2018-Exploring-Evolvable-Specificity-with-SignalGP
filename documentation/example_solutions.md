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

In the changing environment problem, environmental changes produce signals that
have environment-specific tags. These tags determine which of a program's functions
(if any at all) are triggered in response to a signal. In this example, we'll assume
the environment tags given in the table below.

| Environment-state   | Tag (16-bit) |
| :---: | :---: |
| 0 | 0000000000000000 |
| 1 | 1111111111111111 |
| 2 | 1111000000001111 |
| 3 | 0000111111110000 |
| 4 | 1111000011110000 |
| 5 | 0000111100001111 |
| 6 | 0000000011111111 |
| 7 | 1111111100000000 |
| 8 | 0110011001100110 |
| 9 | 1001100110011001 |
| 10 | 1001011001101001 |
| 11 | 0110100110010110 |
| 12 | 0110011010011001 |
| 13 | 1001100101100110 |
| 14 | 1001011010010110 |
| 15 | 0110100101101001 |

An optimal program must have one function per environment state, each tagged
such that is triggered only by the appropriate environment state. In the example
program given below, each function's tag exactly matches a single environment state
tag, and when triggered, each function adjusts the agent's internal state appropriately
(using a `SetState` instruction).

**Optimal Program:**

```signalgp
Fn−0000000000000000:
  SetState0

Fn−1111111111111111:
  SetState1

Fn−1111000000001111:
  SetState2

Fn−0000111111110000:
  SetState3

Fn−1111000011110000:
  SetState4

Fn−0000111100001111:
  SetState5

Fn−0000000011111111:
  SetState6
```

An optimal program's function tags do not need to match *exactly* to environment
state tags. Tag-based referencing allows for some *inexactness* (above a minimum
similarity threshold). In the next example optimal program (below), we flip a single
bit (the first bit) in all function tags; this, however, does not change the program's
behavior.

```signalgp
Fn−1000000000000000:
  SetState0

Fn−0111111111111111:
  SetState1

Fn−0111000000001111:
  SetState2

Fn−1000111111110000:
  SetState3

Fn−0111000011110000:
  SetState4

Fn−1000111100001111:
  SetState5

Fn−1000000011111111:
  SetState6
```

## Distracting Environment Problem

### Distracting Environment Problem - Overview

Problem overview from our paper:

> The distracting environment problem is identical to the changing environment problem but with the addition of randomly occurring distraction signals.
> Like the changing environment problem, the environment can be in one of 16 states at any time with a 12.5% chance to change each update.
> Every time step there is also a 12.5% chance of a distraction event occurring, independent of environmental changes.
> Just as we randomly generate 16 distinct tags associated with each of the 16 environment states, we also generate 16 distinct distraction signal tags, which are guaranteed to not be identical to environment-state tags. Thus, to be successful, agents must monitor the environment for changes (adjusting their internal state as appropriate) while ignoring misleading distraction signals.

### Distracting Environment Problem - Example Solution(s)

We'll assume the same environment state tags as we did for our changing environment
problem example (above).

| Environment-state   | Tag (16-bit) |
| :---: | :---: |
| 0 | 0000000000000000 |
| 1 | 1111111111111111 |
| 2 | 1111000000001111 |
| 3 | 0000111111110000 |
| 4 | 1111000011110000 |
| 5 | 0000111100001111 |
| 6 | 0000000011111111 |
| 7 | 1111111100000000 |
| 8 | 0110011001100110 |
| 9 | 1001100110011001 |
| 10 | 1001011001101001 |
| 11 | 0110100110010110 |
| 12 | 0110011010011001 |
| 13 | 1001100101100110 |
| 14 | 1001011010010110 |
| 15 | 0110100101101001 |

The distracting environment problem also requires a set of tagged distraction signals:

| Distraction signal   | Tag (16-bit) |
| :---: | :---: |
| 0 |  0000000000000001 |
| 1 |  0000000000000010 |
| 2 |  0000000000000100 |
| 3 |  0000000000001000 |
| 4 |  0000000000010000 |
| 5 |  0000000000100000 |
| 6 |  0000000001000000 |
| 7 |  0000000010000000 |
| 8 |  0000000100000000 |
| 9 |  0000001000000000 |
| 10 | 0000010000000000 |
| 11 | 0000100000000000 |
| 12 | 0001000000000000 |
| 13 | 0010000000000000 |
| 14 | 0100000000000000 |
| 15 | 1000000000000000 |

An optimal program must have a function for every true environment state signal,
and those functions must be tagged such that none of the distraction signals trigger
an internal-state-altering function. Here, I'll give an example 32-function solution.
I leave the 16-function solution as an exercise for the reader (depending on the
assumed minimum similarity threshold, is it even possible with the given environment/distraction
signal tags?).

```signalgp
Fn−0000000000000000:
  SetState0

Fn−1111111111111111:
  SetState1

Fn−1111000000001111:
  SetState2

Fn−0000111111110000:
  SetState3

Fn−1111000011110000:
  SetState4

Fn−0000111100001111:
  SetState5

Fn−0000000011111111:
  SetState6

Fn-0000000000000001:
  Nop

Fn-0000000000000010:
  Nop

Fn-0000000000000100:
  Nop

Fn-0000000000001000:
  Nop

Fn-0000000000010000:
  Nop

Fn-0000000000100000:
  Nop

Fn-0000000001000000:
  Nop

Fn-0000000010000000:
  Nop

Fn-0000000100000000:
  Nop

Fn-0000001000000000:
  Nop

Fn-0000010000000000:
  Nop

Fn-0000100000000000:
  Nop

Fn-0001000000000000:
  Nop

Fn-0010000000000000:
  Nop

Fn-0100000000000000:
  Nop

Fn-1000000000000000:
  Nop
```

## References

Lalejini, A., & Ofria, C. (2018). Evolving Event-driven Programs with SignalGP. In Proceedings of the Genetic and Evolutionary Computation Conference. ACM. [https://doi.org/10.1145/3205455.3205523](https://doi.org/10.1145/3205455.3205523)