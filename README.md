# HOLMES Library

This library provides efficient zero-knowledge circuits and their benchmarks for the HOLMES paper. We also show how to use these checks by performing them on three public datasets.

## Requirements

**CMake.**

Mac OS X:
```bash
brew install cmake
```

Debian:
```bash
sudo apt install cmake
```


**OpenMP.** Most general-use C/C++ compilers support OpenMP. To install OpenMP for Apple Clang++, use:

```bash
brew install libomp
```

**OpenSSL install instructions.** Most Linux distributions have OpenSSL installed by default. To install OpenSSL for Mac OS X, use:

BASH:
```bash
brew update
brew install openssl
echo 'export PATH="/usr/local/opt/openssl/bin:$PATH"' >> ~/.bash_profile
source ~/.bash_profile
```

ZSH:
```bash
brew update
brew install openssl
echo 'export PATH="/usr/local/opt/openssl/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

**FLINT.**  
1. Install GMP first (Linux): 
- Download the [GMP Tarball](https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz)
- Extract the Tarball using `tar -xzvf gmp-6.2.1.tar.lz` 
- Change directory into the GMP installation files using `cd gmp-6.2.1`
- Install GMP using `./configure && make && make check && sudo make install`
1. Install GMP first (Mac OS X)
- `brew install gmp`
- If the symlinks are not working, `brew link --overwrite gmp` to fix the symlinks
2. Install MPFR second (Linux):
- Download the [MPFR Tarball](https://www.mpfr.org/mpfr-current/mpfr-4.1.0.tar.gz)
- Extract the Tarball using `tar -xzvf mpfr-4.1.0.tar.gz` 
- Change directory into the GMP installation files using `cd mpfr-4.1.0`
- Install MPFR using `./configure && make && make check && sudo make install`
2. Install MPFR second (Mac OS X):
- `brew install mpfr`
-  If the symlinks are not working, `brew link --overwrite mpfr` to fix the symlinks
3. Download the latest [FLINT tarball](https://www.flintlib.org/downloads.html)
4. Extract the FLINT tarball using  `tar -xzvf flint-2.8.4.tar.gz` 
5. Change directory into the FLINT installation files using `cd flint-2.8.4`
6. Install FLINT using `./configure && make && make check && sudo make install`


**EMP-ZK-HOLMES.** 
1. Install emp-tool first.
```bash
git clone https://github.com/emp-toolkit/emp-tool
cd emp-tool
cmake . && make && sudo make install
```

2. Install emp-ot second.
```bash
git clone https://github.com/emp-toolkit/emp-ot
cd emp-ot
cmake . && make && sudo make install
```

3. Install emp-zk-holmes third. 
```bash
git clone https://github.com/holmes-anonymous-submission/emp-zk
cd emp-zk
cmake . && make && sudo make install
```

**HOLMES library.**
```bash
git clone https://github.com/holmes-anonymous-submission/holmes-library  
cd holmes-library
cmake . && make
```

## Supported tests
This library supports the following checks:
- range check
- histogram check
- mean check
- variance check
- trimmed mean check
- Johnson-Lindenstrauss lemma using Legendre PRF
- polynomial identity testing

### Range check
We can check that an input value is bounded between a low value and a high value.

**Our approach.** We check that `x - B_low >= 0` and `B_high - x >= 0`. This is done by proving that `x - B_low` and `B_high - x` uses at most `ceil(log_2(B_high - B_low))` bits. To do so, we
decompose `(x - B_low)` into bits, denoted by `bits_of_lower_gap[j]`, and `(B_high - x)` into bits, denoted by `bits_of_higher_gap[j]`. 

The proof system shows that:
- Bit test: For all values in `bits_of_lower_gap[j]` and `bits_of_higher_gap[j]`, `bit(bit - 1) = 0`
- Equality check: `(B_low - x) + \sum_{0..} (bits_of_lower_gap[j] * 2^j) = 0`
- Equality check: `(x - B_high) + \sum_{0..} (bits_of_higher_gap[j] * 2^j) = 0`

**Related files.**

- The range check protocol is given in [bench/range_check.hpp](bench/range_check.hpp)
- The benchmark is given in [bench/bench_range_check.cpp](bench/bench_range_check.cpp)

### Histogram check for nominal and numeric values

We can check that the provided inputs matches a histogram.

**Our approach.** We arrange the numbers into a pre-defined list of groups, where each group consists of a range of values. For each input value, we perform a range check to ensure that the value falls within the corresponding group.

Let `group_bits` denote a one-hot encoding vector consisting of only one "1" entry, as follows. If the input belongs to the j-th group, then `group_bits[j] = 1`, and the rest of `group_bits` will be 0.

The proof system shows that:
- Bit test: For all values in `group_bits`, `group_bits[j](group_bits[j] - 1) = 0`
- Equality test: Sum all values in `group_bits`, `\sum_{0..} group_bits[j] = 1`
- Range check: Given the range `[j_low, j_high]` of the group where `group_bits[j] = 1`, that `j_low <= input < j_high`

For nominal values, the range check becomes a simple equality check as `j_low = j_high`.

**Related files.**

- The protocol for histogram check for nominal values is given in [bench/histogram_nominal_check.hpp](bench/histogram_nominal_check.hpp)
- The protocol for histogram check for numerical values is given in [bench/histogram_numeric_check.hpp](bench/histogram_numeric_check.hpp)
- The benchmark for histogram check for nominal values is given in [bench/bench_histogram_nominal.cpp](bench/bench_histogram_nominal.cpp)
- The benchmark for histogram check for numeric values is given in [bench/bench_histogram_numeric.cpp](bench/bench_histogram_numeric.cpp)

### Mean check

We can check if the provided mean value matches the inputs.

**Our approach.** Note that the mean value can only be represented with a certain precision, so, instead of an equality check, the mean check consists of a range check.

For example, consider a mean value with two decimal places, represented as a fixed-point number (that is, `MeanTimes100 = 567` means a mean of 5.67). Let the sum of the inputs be `Input Sum` and let the number of samples be `Number of Samples`.

The proof system shows that:
- Range check: `Number of Samples * MeanTimes100 <= Input Sum * 100 < Number of Samples * (MeanTimes100 + 1)`

**Related files.**

- The mean check protocol is given in [bench/mean_check.hpp](bench/mean_check.hpp)
- The mean check benchmark is given in [bench/bench_mean_check.cpp](bench/bench_mean_check.cpp)

### Variance check

We can check if the provided variance value matches the inputs.

**Our approach.** Similarly, the variance value can only be represented with a certain precision, so instead of an equality check, we perform a range check.

For example, consider a variance value with two decimal places, represented as a fixed-point number (that is, `VarianceTimes100 = 567` means a variance of `5.67`). Let the sum of squares of the inputs be `Sum of Squares` and let the number of samples be `Number of Samples`. Let the sum of the inputs be `Input Sum`.

The proof system shows that:
- Range check: `Number of Samples * Number of Samples * VarianceTimes100 <= Sum of Squares * Number of Samples * 100 - Input Sum * Input Sum * 100 < Number of Samples * Number of Samples * (VarianceTimes100 + 1)`

**Related files.**

- The variance check protocol is given in [bench/variance_check.hpp](bench/variance_check.hpp)
- The variance check benchmark is given in [bench/bench_variance_check.cpp](bench/bench_variance_check.cpp)

### Trimmed mean check

It is a known issue that the mean value tends to be affected by extreme values. We consider an alternative, trimmed mean, which considers the mean of only those values within the range `[a,cutoff)` and excludes those values in `[cutoff,b]`, for input values bounded in `[a,b]`.

**Our approach.** We first compute the effective sum, which is the sum of the input values that fall within `[a,cutoff)`, as well as the effective count, which is the number of input values that fall within `[a,cutoff)`.

For each input value, we define a choice bit. When the choice bit is 1, it means that this input value is within `[a,cutoff)`; otherwise, when the choice bit is 0, the input value is within `[cutoff,b]`.

Then, we define the witness bits for each input value, as follows. For an input value that falls within `[a,cutoff)`, we decompose `(cutoff - input value)` into bits. For an input value that falls within `[cutoff, b]`, we decompose `(input value - cutoff)` into bits.

Let the trimmed mean be a fixed-point value with two decimal places, i.e., `TrimmedMeanTimes100`. Let the effective sum (denoted by `Effective Sum`) be the sum of an input value multiplied by its corresponding choice bit, and let the effective count (denoted by `Effective Count`) be the sum of choice bits.

The proof system shows that:
- For each input value,
    * Binary test: the choice bit is 0 or 1
    * Binary test: each of the witness bits `Witness Bits[0], Witness Bits[1], ...` is 0 or 1
    * Equality test: `Choice Bit * [(cutoff - Input Value) - (Input Value - cutoff)] + (Input Value - cutoff) = Witness Bits[0] * 1 + Witness Bits[1] * 2 + Witness Bits[2] * 4 + ...` 
- Range check: `Effective Count * TrimmedMeanTimes100 <= Effective Sum * 100 < Effective Count * (TrimmedMeanTimes100 + 1)`

**Related files.**

- The trimmed mean protocol is given in [bench/trimmed_mean.hpp](bench/trimmed_mean.hpp)
- The trimmed mean benchmark is given in [bench/bench_trimmed_mean.cpp](bench/bench_trimmed_mean.cpp)

### Johnson-Lindenstrauss dimensionality reduction

We can perform a Pearson goodness-of-fit test over the multidimensional input entries. To perform this test efficiently, it is useful to perform the dimensionality reduction using the [Johnson-Lindenstrauss lemma](https://en.wikipedia.org/wiki/Johnson%E2%80%93Lindenstrauss_lemma).

We implement this dimensionality reduction, with a new optimization: we use the Legendre pseudorandom function (PRF) to instantiate the random matrix used in the dimensionality reduction. This allows us to apply the matrix multiplication efficiently.

**Our approach.** For an input entry of `d` dimensions, we first embed the values in these `d` dimensions, denoted by `v[1], v[2], ..., v[d]`, into one value, denoted by `T`. Without loss of generality, let us assume that the value in `i`-th dimension is in the range `[0, n[i])`. This embedding computes:

```
T = v[1] * n[2] * n[3] * ... * n[d] 
  + v[2] * n[3] * ... * n[d] 
  + v[3] * n[4] * ... * n[d]
  + ...
  + v[d]
```

Then, after all the inputs have been committed in the ZKP system, we choose a random (degree-3) Legendre PRF, denoted by `f(T)`, which satisfies certain security and irreducibility requirements. The output of `f(T)` will be `1` or `-1` based on the outputs of the Legendre Symbol.

We instantiate the random matrix used in Johnson-Lindenstrauss lemma by this PRF (with some normalization). Then, the matrix multiplication in the Johnson-Lindenstratuss lemma can be simplified as a few queries to the PRF.

For each PRF call, the proof system shows that `f(T)` has been computed correctly. Let `QNR` be any quadratic nonresidue. Let the result of `f(T)` be `c`.

The proof systems shows that:
- Binary test: `(c - 1)(c + 1) = 0`
- Equality test: `f(T) * (1 + c)/2 + f(T) * QNR * (1 - c)/2 = r ^ 2 (mod p)`, where `r` is the square root of `f(T)` if `c = 1`, or of `QNR * f(T)` if `c = -1`

**Related files.**

- The JL dimensionality reduction protocol is given in [bench/jl_projector.hpp](bench/jl_projector.hpp)
- The JL dimensionality reduction benchmark is given in [bench/bench_jl.cpp](bench/bench_jl.cpp)
- The strawman JL benchmark is given in [bench/bench_strawman_jl.cpp](bench/bench_strawman_jl.cpp)

## Polynomial identity testing

When we want the EMP-ZK proof to interact with another cryptographic system, we may desire a way to efficiently check that the two cryptographic systems are "on the same page"; that is, the two systems are seeing the same input data.

Our library implements the polynomial identity testing based on the [DeMillo-Lipton-Schwartz–Zippel lemma](https://en.wikipedia.org/wiki/Schwartz%E2%80%93Zippel_lemma) (commonly referred to as "Schwartz-Zippel lemma"). The idea is to treat the input data as coefficients of a polynomial and evaluate this polynomial on a random point. If the evaluations are the same, then with a high probability (depending on the parameters), the two polynomials are the same, and so the two cryptographic systems are looking at the same data.

**Our approach.** Let the input data be `inp[1], inp[2], ..., inp[n]`. Let `r` be a random number used for masking to hide the input data. After the input data has been committed in the ZKP system, the proof system shows that the claimed evaluation result of the polynomial, `s`, at a randomly sampled point `x`, matches the input, by showing that:

```
r + inp[1] * x + inp[2] * x^2 + ... + inp[n] * x^n - s = 0
```

**Related files.**

- The random linear combination protocol is given in [bench/random_linear_combination.hpp](bench/random_linear_combination.hpp)

## Datasets used in the benchmark

**Bank marketing dataset [\[link\]](https://archive.ics.uci.edu/ml/datasets/Bank+Marketing)** from S. Moro, P. Cortez, and P. Ritaa, "A data-driven approach
to predict the success of bank telemarketing," in Decision Support Systems '14.

Our benchmark performs the following checks:
- histogram check for numeric values, for the age
- multidimensional Pearson goodness-of-fit test for age, job, educational level, and marital status
- mean and variance check for the call duration
- range check for all the features
- polynomial identity testing

The benchmark is given in [bench/bench_dataset_1.cpp](bench/bench_dataset_1.cpp).

**Diabetes dataset [\[link\]](https://archive.ics.uci.edu/ml/datasets/Diabetes+130-US+hospitals+for+years+1999-2008)** from B. Strack, J. P. DeShazo, C. Gennings, J. L. Olmo, S. Ventura, K. J. Cios, and J. N. Clore, "Impact of HbA1c
measurement on hospital readmission rates: Analysis of 70,000 clinical database patient records," in BioMed Research International '14.

Our benchmark performs the following checks:
- histogram check for nominal values, for the medical specialty of initial admissions
- mean and variance check for the number of lab procedures across hospitals
- range check for all the features
- polynomial identity testing

The benchmark is given in [bench/bench_dataset_2.cpp](bench/bench_dataset_2.cpp).

**Auctioning dataset [\[link\]](https://www.kaggle.com/saurav9786/real-time-advertisers-auction)** from Saurav Anand in Kaggle.

Our benchmark performs the following checks:
- trimmed mean check for the total impressions that are less than the half of the MAX of total impressions.
- range check for all the features
- polynomial identity testing

The benchmark is given in [bench/bench_dataset_3.cpp](bench/bench_dataset_3.cpp).

## Run the integration tests
Our three datasets are listed as bench_dataset1.py (Bank Marketing),  bench_dataset2.py (Diabetes), and  bench_dataset3.py (Auctioning).
1. Create two tabs in Terminal (Ctrl + Shift + T / Command + T)
2. In both tabs, `cd holmes-library`
3. Go to first tab in Terminal, and set the `EMP_MY_PARTY_ID` environment variable to 1 and run your choice of the bench dataset script using `EMP_MY_PARTY_ID=1 python3 bench-scripts/bench_dataset[1-3].py`
4. Switch to the second tab in Terminal, and set the `EMP_MY_PARTY_ID` environment variable to be 2 and run your choice of the bench dataset script using `EMP_MY_PARTY_ID=2 python3 bench-scripts/bench_dataset[1-3].py`
The end-to-end tests for any of the three dataset benchmarks should now run.

## Edit the unit tests
1. `cd holmes-library`
2. Edit the test file in `bench/bench_test.cpp` and/or `bench/test_check.hpp`
3. Compile the new test file using `make`

## Run the unit tests
1. Create two tabs in Terminal (Ctrl + Shift + T / Command + T)
2. In both tabs, `cd holmes-library`
3. Go to first tab in Terminal, and set the `EMP_MY_PARTY_ID` environment variable to 1 and run the unit test script using `EMP_MY_PARTY_ID=1 python3 bench-scripts/bench_test.py`
4. Switch to the second tab in Terminal, and set the `EMP_MY_PARTY_ID` environment variable to be 2 and run the unit test script using `EMP_MY_PARTY_ID=2 python3 bench-scripts/bench_test.py`

## Debug HOLMES with GDB
1. `vim CMakeLists.txt`, and ensure that:
- `set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb")` is added
- `-O3` and `-funroll loops` are removed from `CXX FLAGS`
2. Run `cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug`
3. Run `cmake --build build-debug`, or alternatively, `cd build-debug && make`
**In general, you need to do steps 1-3 to our CMakeFile to view the source code while debugging a C/C++ program**
4. Open a tmux session using tmux and create two panes using Ctrl + b + %, or just open two terminal tabs
5. On both panes/tabs, run `gdb build-debug/bin/arith/input_check` or `cgdb build-debug/bin/arith/input_check`
6. On both panes/tabs, type in `break test_input_check`
7. On the first pane/tab: start the GDB program using `r 1 12345` as the command arguments
8. On the second pane/tab: start the GDB debugging session using `r 2 12345` as the command arguments

Notes:
- If using GDB and not CGDB, you’ll need to use `layout src` to view source code. See the code of the binary file run in the src folder for more info on the arguments
- `focus src` does not seem to work in CGDB. However, `focus src` works in GDB.


## Regulatory issue

This repository is not subject to the U.S. Export Administration Regulation (EAR) because it is publicly available.

For more information about this regulatory issue, see [this post](https://www.eff.org/deeplinks/2019/08/us-export-controls-and-published-encryption-source-code-explained) by Electronic Frontier Foundation (EFF).
