
# SignalGP Instruction Set Details
Here, we provide details about the SignalGP instruction set used in the experiments contained within this repository. For a detailed description of the SignalGP virtual hardware, please refer to (Lalejini and Ofria, 2018)

Throughout this document, we use the following abbreviations in our descriptions:
- 'ARG1', 'ARG2', 'ARG3': The first, second, and third arguments for an instruction. 
- 'WM': Indicates working memory. WM[i] indicates accessing location i in working memory. For example, WM[ARG1] refers to value stored in the location in working memory specified by an instruction's first argument.
- 'IM': Indicates input memory. IM[i] indicates accessing location i in input memory.
- 'OM': Indicates output memory. OM[i] indicates accessing location i in output memory. 
- 'SM': Indicates shared memory. SM[i] indicates accessing shared memory at location i. 
- 'EOF': Indicates the end of a function.

## Default Instruction Set
The instructions 

| Instruction | Arguments*   | Uses Tag?** | Description |
| ----------- |:-----------: | ----------- | ----------- |
| Inc         | 1            | No          | WM[ARG1] = WM[ARG1] + 1                                        |   
| Dec         | 1            | No          | WM[ARG1] = WM[ARG1] \- 1                                       |
| Not         | 1            | No          | WM[ARG1] = !WM[ARG1] (logically toggle WM[ARG1])               |   
| Add         | 3            | No          | WM[ARG3] = WM[ARG1] + WM[ARG2]                                 |   
| Sub         | 3            | No          | WM[ARG3] = WM[ARG1] - WM[ARG2]                                 |
| Mult        | 3            | No          | WM[ARG3] = WM[ARG1] \* WM[ARG2]                                |
| Div         | 3            | No          | WM[ARG3] = WM[ARG1] / WM[ARG2]                                 |
| Mod         | 3            | No          | WM[ARG3] = WM[ARG1] % WM[ARG2]                                 |
| TestEqu     | 3            | No          | WM[ARG3] = (WM[ARG1] == WM[ARG2])                              |
| TestNEqu    | 3            | No          | WM[ARG3] = (WM[ARG1] != WM[ARG2])                              |
| TestLess    | 3            | No          | WM[ARG3] = (WM[ARG1] < WM[ARG2])                               |
| If          | 1            | No          | If WM[ARG1] != 0, proceed; else, skip until next Close or EOF          |
| While       | 1            | No          | If WM[ARG1] != 0, loop; else, skip until next Close or EOF             |
| Countdown   | 1            | No          | Same as While, but decrements WM[ARG1]                                 |
| Close       | 0            | No          | Indicates end of looping or conditional instruction block              |
| Break       | 0            | No          | Break out of current loop                                              |
| Call        | 0            | Yes         | Call function referenced by tag                                        |
| Return      | 0            | No          | Return from current function            |
| Fork        | 0            | Yes         | Generates an internal event where the instruction's tag defines the generated event's tag. More simply, a Fork is essentially a Call where the referenced function is called on a new thread. |
| Terminate   | 0            | No          | Kills the currently executing thread            |
| SetMem      | 2            | No          | WM[ARG1] = ARG2            |
| CopyMem     | 2            | No          | WM[ARG1] = WM[ARG2]            |
| SwapMem     | 2            | No          | Swap values in WM[ARG1] and WM[ARG2]            |
| Input       | 2            | No          | WM[ARG2] = IM[ARG1]            |
| Output      | 2            | No          | OM[ARG2] = WM[ARG1]            |
| Commit      | 2            | No          | SM[ARG2] = WM[ARG1]            |
| Pull        | 2            | No          | WM[ARG2] = SM[ARG1]            |
| Nop         | 0            | No          | No-operation            |

\* The number of arguments that modify the effect of the instruction. 

\*\* Does the instruction's tag modify the effect of the instruction? 



# References

Lalejini, A., & Ofria, C. (2018). Evolving Event-driven Programs with SignalGP. In Proceedings of the Genetic and Evolutionary Computation Conference. ACM. [Pre-print here.](https://arxiv.org/pdf/1804.05445.pdf)