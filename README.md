# SHiP implementation in ZSim (casim)
Computer Architecture Simulation Infrastructure for CSCE 614 Computer Architecture Term Project Artifact
Modified Compilation Steps and Data Generation by: David Armijo
Replacment Policy implementation by: Junyi Wu

#### Project Description
Project Goal:
<Ask gpt about this for geez sake lmao>
This project aims to integrate and evaluate the SHiP (Signature-based Hit Predictor) cache replacement policy within the ZSim processor simulator using the westmere processor. SHiP is designed to intelligently predict the likelihood of a cache block being reused by associating it with the program-specific signatures. By learning those patterns over time, SHiP improves cache insertion decisions, reducing cache miss rates and boosting overall system performance as a result.

This project will be implementing the SHiP-Mem replacement policy. SHiP-Mem uses memory address-based signatures to make cahce insertion decisions.

Paper Source:
SHiP: signature-based hit predictor for high performance caching:
https://dl.acm.org/doi/abs/10.1145/2155620.2155671

Project Results are stored at `/zsim/outputs/project-results.zip`.

#### Project Structure
`Group11/` : this will be the root repository folder
`Group11/zsim/` : Project simulation directory. This is also where you run the simulations.

`/zsim/configs/` : where the simulation configs are stored
`/zsim/outputs/` : where the outputs from the simulations will be stored
`/zsim/outputs/project/` : where the outputs for this project simulations are stored
`/zsim/outputs/project/<repl_policy>` : where the output of the simulations are stored give a replacement policy.
These replacement policies include:
- LFU
- LRU
- SHiP (this project's focus)
- SRRIP (implemented back in hw4)

`/zsim/outputs/project/<repl_policy>/<benchmarks>` : where the output of the simulations are stored given a replacement policy and benchmark. This is where the actual results and logs are stored.

`/zsim/py_scripts` : where the python script to extract_cache_data_points and generate_grouped_bar_charts are located (explained later in this README)
`/zsim/reference_scripts` : This contains the scripts that came with forking the original casim-csce469 repository. Not necessary to run scripts for simulations
`/zsim/sim_logs/` : Where the logs for simulations are stored when running either `/zsim/controller` or `/zsim/single-controller` binary.
`/zsim/src` : Where the source code is located

Files modified for this project:
- `init.cpp`

Files added for this project:
- `ship_repl.h`

#### Compilation and Run Steps:

Prerequesite: zsim must be compiled and run on Linux.
Make sure to have the following packages installed based from your respective package manager:
- zlib
- gcc
- python3

Note: These packages are what are tested to properly compile and run zsim

##### 1. Unzip benchmarks files

```
zip -F benchmarks.zip --out single-benchmark.zip && unzip single-benchmark.zip && mkdir benchmarks/parsec-2.1/inputs/streamcluster
```

### 2. Environemnt setup

To set up the Python environment for the first time, run the following commands.

```
$ python -m venv venv
$ source venv/bin/activate
$ pip install scons
```

Everytime you want to build or run zsim, you need to setup the environment variables first.

```
$ source venv/bin/activate
$ source setup_env
```

##### 3. Compile zsim

```
$ cd zsim
$ scons -j4
```

You need to compile the code each time you make a change.

### Notes about compilation:
In case it does not compile because of a warning (particularly the "no-null-compare warning"):
Edit the zsim/SConstruct file and comment out line 81 where it says:

```
81 | env["CPPFLAGS"] += " -Werror "
```

Then you should be able to compile the code hopefully.
This fix has been made to run simulations locally in Arch Linux as of 4/23/2025

Before running the simulations (particularly locally): You must run the following command in order to allow simulations to properly run:

```
# echo 0 > /proc/sys/kernel/yama/ptrace_scope
```

###### For more information, check `zsim/README.md`

#### Running zsim simulations:
There are multiple options that you choose to run zsim:

There is one thing that they all share in common:
The outputs for the simulations can be found in the `zsim/outputs/project/<repl_policy>/` directory once they are done executing.

## One simulation for one replacement policy:
```sh
$ ./projectrunscript <suite> <benchmark> <repl_policy>
```
Suite: SPEC
benchmarks: bzip2 gcc mcf hmmer sjeng libquantum xalan milc cactusADM leslie3d namd soplex calculix lbm

Suite: PARSEC
benchmarks: blackscholes bodytrack canneal dedup fluidanimate freqmine streamcluster swaptions x264


## One Replacement Policy Complete Simulation Suite (With Concurrent Simulations):
In order to produce faster results, there is a c++ program named controller.cpp that runs `./projectrunscript` multiple times concurrently with a limit of X concurrent simulations. This can be adjusted in controller.cpp line 16 where it says:

```c++
#define MAX_CONCURRENT 7
```

The line above is the default number of concurrent simulations that the controller will run and has already compiled to its binary form called `./controller`. 
If you desire to run more simulations concurrently, change the number of `MAX_CONCURRENT` simulations to the number you desire and use this line to compile the controller.cpp:
```sh

$ g++ -o controller controller.cpp
```

When you are ready to execute the program, make sure you are already in zsim/ directory and run the `./controller` binary.
The program will prompt you the benchmark you want to run: LRU LFU, SHiP SRRIP

Once the user typed their desired policy, the concurrent simulations will start executing and will not go above the MAX_CONCURRENT threshold.
The main controller log will be found in `zsim/controller.log` where each simulation's log can be found in `zsim/sim_logs`.


## One Replacement Policy Complete Simulation Suite (Serial Simulation):
In order to comply with CSCE resource regulations, we have provided the binary to run the simulations to run one at a time given a replacement policy to run. This binary can be executed in the `/zsim` directory by executing the following command:

```sh
$ ./single-controller
```


## All Replacement Policies, Complete Simulation Suite (Serial Simulation):
As mentioned above, to comply with CSCE resource regulations, we have modified the .sh script to run the simulations one at a time for all replacement policies and their respective simulations. This script can be executed by the following command in the `/zsim` directory:

```sh
$ sh ./projectautorunscript.sh
```


### Extracting and Generating Results
After running the benchmarks, there are two python files under the zsim/py_scripts directory:

`extract_cache_data_points.py`: Should be placed in `/zsim/outputs/project/<\repl_policy>` directory
`generate_grouped_bar_charts.py`: Should be placed in `/zsim/outputs/project` directory. This should be executed once you are done with all of the simulations.

In the zsim directory, the following commands can be run if it already doesn't have the extract_cache_data_points.py in the benchmark outputs folder (Assuming you have completed at least one `<\repl_policy>` simulation):

```sh
$ mv py_scripts/extract_cache_data_points.py /outputs/project/<repl_policy>
$ cd /outputs/project/<repl_polcy>
$ python3 ./extract_cache_data_points.py
```

After executing those commands, the script will create a l3_and_instrs_summary.csv file containing the extracted data points from all of the simulations that have been run so far.

Once all of the simulations from every replacement policy have been run, you can execute the following commands to generate_grouped_bar_charts from the extract_cache_data_points in the zsim/ directory:

```sh
$ mv py_scripts/generate_grouped_bar_charts.py outputs/project/
$ cd outputs/project/
$ python3 generate_grouped_bar_charts.py
```

To successfully generate the grouped bar charts however, you must have the following python libraries installed:
- pandas
- matplotlib
