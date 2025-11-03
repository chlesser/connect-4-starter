# Connect 4
This is an implementation of Connect 4 for Graeme Devine's CMPM123 class. It is made with Dear ImGui, and written with C++ in visual studio code, on a windows machine.
## Methodology
The methodology of this assignment was relatively straightforward. I copied Graeme Devine's tic tac toe code and modified it to be appropriate for connect 4. I changed the grid size, and the ways it checks for victory. Additionally, I prevented the user clicking anywhere but the bottom row.
## AI Implementation
My AI is affectionately named Crackhead. In all honesty, the implementation of crackhead isn't perfect. For some reason, it will occasionally do the worst move possible. Other times, it plays and easily beats me. It implements alpha beta pruning and negamax to a depth of 7 (discovered via testing). These things were relatively easy to implement, but there are small errors that confuse me. I will continue ironing out bugs after the submission.