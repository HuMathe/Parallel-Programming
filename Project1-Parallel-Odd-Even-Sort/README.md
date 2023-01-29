# Parallel-Odd-Even-Sort

Parallel implementation of Odd-Even Sort


## Code Usage

### Compilation

1. `cd` into `src` directory, 
2. Compiling all the programs via `make`, 4 executables are generated:
    - `data`: test data generator
    - `checker`: simple test program that check the correctness of the program output
    - `p_sort`: MPI parallel implementation
    - `s_sort`: sequential implementation

### Run

`mpirun -np <# of cores> ./p_sort <data-file-path>`

*Note*: the data file should be `ASCII` encoded, and the first line  should  contains a positive integer $N$ (indicates the number of items to be sorted) and with $N$ additional integers (one in each line), which represent the numbers to be sorted.

--- 
For more information about the design and the detailed code usage, please refer to `doc/report.pdf`