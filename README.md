# Preservation of AN-2025-132

![C++](https://img.shields.io/badge/C%2B%2B-17+-blue?style=flat-square) ![ROOT](https://img.shields.io/badge/ROOT-6.26%2B-2c8?style=flat-square) ![ONNX Runtime](https://img.shields.io/badge/ONNX%20Runtime-1.24.4-orange?style=flat-square) ![Python](https://img.shields.io/badge/Python-3.9%2B-3776AB?style=flat-square) ![Snakemake](https://img.shields.io/badge/Snakemake-Workflow-brightgreen?style=flat-square) 

This is a `TSelector`-based analysis framework that uses **dynamic typecasting** to adapt to changes in NanoAOD branches across different versions without rewriting the core analysis logic. Here is what it brings to the table:

- **Auto-adapts to versions:** Automatically detects the branch types in the input file and supports fallback names when branches change between productions. The core analysis logic does not need to be rewritten for each NanoAOD version.

- **Live DNN evaluation:** Integrates ONNX Runtime for event-by-event evaluation of pre-trained neural networks, enabling on-the-fly event selection and categorization. A working example DNN is included.

- **Plug-and-play ONNX:** Includes a pre-compiled ONNX Runtime library with the required paths already configured, avoiding the need to build ONNX Runtime from source or deal with additional environment setup.

-  **Batch processing & automated pipelines:** Includes scripts to submit and manage large-scale jobs on the CMS grid via [**`CRAB/`**](./CRAB), as well as a Snakemake pipeline setup for running automated, reproducible workflows on [**`reana/`**](./reana).

> For more details on the C++ API, check out the [official ONNX Runtime documentation](https://onnxruntime.ai/docs/).<br>
> **For users outside the CMS Collaboration:** NanoAOD samples are available through the [CERN Open Data Portal](https://opendata.cern.ch/docs/cms-getting-started-nanoaod).

## Structure
Here is how the `TSelector` based analysis framework is organized.
```bash
AN-2025-132
├── nanoAna.C            # Source code containing the analysis logic
├── nanoAna.h            # Primary header containing the analyzer class
├── headers
│   ├── DynamicReaders.h         # Core logic that handles nanoAOD branch types
│   ├── Init.h                   # Initializes the branch variables from input file
│   ├── AnalysisLogic.h          # Core analysis logic, including event selections
│   ├── BookHistograms.h         # Defines and initializes all the histograms.
│   ├── Corrections_MC           # Custom corrections applied to targeted MC samples
│   ├── Corrections_POG          # Standard corrections from POG, uses correctionlib
│   ├── GoldenJSONs              # Contains goldenJSONs locally, 2016 to 2023
│   ├── LoadGoldenJSONs.h        # Uses the goldenJSONs to filter events in data
│   ├── CustomFunctions.h        # Utility functions
│   ├── ProduceGenCollection.h   # Creates arrays of gen-objects
│   ├── ProduceRecoCollection.h  # Creates arrays of reco-objects
│   └── VetoEvents.h             # Additional event filters; removes overlap with 3L4L
├── onnxruntime-linux-x64-1.24.4 # Library for DNN compatibility
├── trainedDNNs          # Trained DNNs and input scaling parameters
├── runOnce.C            # Driver that runs the compiled setup.
├── compile_and_run.C    # Wrapper around the driver; handles library loading and compilation
├── runAll.py            # For local run
├── CRAB                 # Contains the CRAB setup 
├── reana                # Contains the REANA setup
└── reana.yaml           # REANA configuration
```

Here is how the pieces fit together during execution:

1.  **Compilation:** The pre-compiled ONNX shared libraries and the environment's `correctionlib` libraries are first loaded into ROOT. This is followed by the compilation of the main analyzer (`nanoAna.C` and its modular headers) into a shared object via ACLiC.
    
2.  **Execution:** Once compiled, the `runOnce.C` driver script gathers the inputs (including the DNN models), feeds them to the analyzer, and initiates the event loop. The code dynamically reads the branches, evaluates the DNN on the fly, and saves the final histograms into the `outputs/` directory.

## Running locally

Edit the dictionary of samples in the local run script, ensuring that the target files exist locally.
```
SAMPLES = {
    "dyv9": {
        "input": "inputs/DY_NanoAODv9.root",
        "output": "outputs/hst_DY_NanoAODv9.root",
        "era": "2018",
        "sample": "DYtoLL",
    },
...
}
```
Run the Python driver:
```bash
python3 runAll.py                 # Process all files in the dictionary
python3 runAll.py --sample dyv9   # Process a single specific sample
```
The `runAll.py` script iterates over the specified samples and dynamically constructs ROOT command strings. It passes these arguments to `compile_and_run.C`, which explicitly links `libcorrectionlib.so` using its absolute path to bypass missing `LD_LIBRARY_PATH` issues common in local Conda environments. After successfully compiling `nanoAna.C`, it executes the event loop on your local machine and drops the finalized ROOT histograms into the `outputs/` folder.


### A note on the arguments

- **Era names (must match exactly):** Internally, the era argument sets the variables `_era`, `_campaign`, and `_year`. Supported inputs include:
    ```
    2016-preVFP      2022-preEE
    2016-postVFP     2022-postEE
    2017             2023-preBPix
    2018             2023-postBPix
    
    ```
- **Sample:** This string must match the sample-specific corrections. It internally sets the variables `_sample` and `_data`. If the string contains `Muon`, `EGamma`, or `SingleElectron`, the `_data` flag is set to `1`; otherwise, it defaults to `0`. (See `AnalysisLogic.h` for details on how `_sample` is used to pick the right MC corrections).

## Running on CRAB
```
CRAB/
├── PSet.py
├── crab_config.py  # contains the CRAB configuration
├── crab_script.sh  # runs on the remote worker node and executes compile_and_run.C
├── samples.json    # contains the datasets and analysis settings
└── submitJobs.py   # reads the DAS samples and submits the CRAB jobs
```
The CRAB commands must be executed inside a CMSSW environment. While the C++ analysis code itself does not have a strict CMSSW dependency, the environment is required because CRAB and its supporting Python modules are provided through the CMS software stack.

A valid CMS grid proxy is also required so that the worker nodes can access the remote CMS data:
```bash
voms-proxy-init --voms cms
```
The `samples.json` file is the central hub for dataset configuration. Because the rest of the CRAB setup is modular, adding a new dataset only requires adding a new entry here:
```python
{
  "dyv12": { # becomes the job identifier
    "dataset": "/DYto2L-4Jets_MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v3/NANOAODSIM",
    "era": "2022-postEE",   # passed to era in the analysis _setup, which also sets _year
    "sample": "DYJetsToLL", # passed to sample in the analysis _setup, which also sets _data
    "outdir": "DYto2L"      # sub-directory in EOS
  }
}
```
These parameters are passed to the remote shell executable `crab_script.sh` via environment variables (`CRAB_ERA`, `CRAB_SAMPLE`, `CRAB_DATASET`, `CRAB_OUTDIR`). `submitJobs.py` acts as the main driver: it reads the JSON, creates a sandbox containing the analysis code (preserving the directory structure for headers, ONNX, and DNN models), verifies the datasets in DAS, and dispatches the CRAB requests.

The submission script defaults to a dry-run mode to validate the configuration before sending jobs to the grid:
```bash
python3 submitJobs.py
```
To execute the actual submission:
```bash
python3 submitJobs.py --dryrun False
```
Additional utilities:
-  Check status: `python3 displayStatus.py`
-  Kill submitted tasks: `python3 killAllJobs.py`

## Running on REANA

This framework utilizes **Snakemake** as the workflow management engine for REANA. Snakemake reads the workflow definition to build a dependency graph, determining which Condor jobs can run in parallel, when to merge output chunks, and when to publish to EOS. The actual ROOT macros execute on the CERN **HTCondor backend**, and the final merged files are saved natively to EOS storage via a **Kerberos** keytab.
```
AN-2025-132
├── reana
│   ├── prepare.py    # prepares the input file lists
│   ├── samples.json  # contains the datasets and analysis settings
│   ├── Snakefile     # defines the REANA workflow
│   ├── runJob.py     # runs the analysis for each input file in the worker nodes
│   └── submit.sh     # submits the workflow to REANA
└── reana.yaml        # REANA workflow configuration
```
### The physics core

The actual physics processing happens entirely in C++. `compile_and_run.C` kicks off the process by taking the XRootD file path, the sample era, and the output name, and then fires up the `nanoAna` event processor. To execute machine learning models during the event loop, the ONNX C++ API is loaded straight into ROOT, allowing the code to evaluate the models from the `trainedDNNs` directory on the fly.

### The REANA infrastructure

The REANA infrastructure handles distributing the jobs to the grid. It starts with `reana/samples.json`, a master list of dataset names, eras, and parameters. Running `reana/prepare.py` locally queries DAS for the actual XRootD URLs and spits out individual JSON files into the `reana/samples/` directory. Keeping these separate ensures Snakemake processes each sample independently—meaning a single failure does not require restarting the entire workflow.

The HTCondor worker nodes execute `reana/runJob.py`. This Python wrapper restricts ONNX and OpenMP to a single thread to prevent Condor from freezing up. It relies strictly on the Python `os` module to verify files and clean up the grid proxy afterward.

The workflow steps are mapped out in `reana/Snakefile`. To resolve operating system and glibc mismatches, the workflow uses a highly robust dual-container strategy:

1. **Processing chunks:** The heavy C++ analysis runs inside an official `cmssw/el8` container, pulling in the exact same CMSSW environment used by CRAB to guarantee `correctionlib` compiles natively.
    
2. **Merging data:** The final `hadd` step spins up in a lightweight REANA ROOT6 container, bypassing the heavy CMSSW setup overhead to instantly merge the outputs.
    
3. **Publishing:** The merged files are copied to EOS using Kerberos authentication.

### Set up the environment

Before running REANA commands, a `.env` file must be created in the local directory to authenticate the terminal session with the central REANA server. This file stores private credentials and **must never be committed to version control**.  The `.env` file should contain the following two lines:
```bash
export REANA_SERVER_URL=https://reana.cern.ch
export REANA_ACCESS_TOKEN=XXXXXXXXXXXXXXXXXX
```
> Note: The access token can be generated by logging into [reana.cern.ch](https://reana.cern.ch/) and checking the user profile page.

Once the file is created, activate the REANA virtual environment and source the `.env` file to load the access token. Finally, ping the server to verify the connection is active:
```bash
source /afs/cern.ch/user/r/reana/public/reana/bin/activate
source .env
reana-client ping
```

### Tuning the workflow

Before kicking off the submission, revisit the workflow parameters in `reana/Snakefile`:

- **`CHUNK_SIZE`**: Sets the number of ROOT files per HTCondor job. Use `1` for testing and `20-50` for production. Grouping files reduces Condor startup overhead, maximizes actual analysis time, and prevents a flood of tiny jobs from overwhelming the CERN schedulers.
    
- **`kubernetes_memory_limit`**: Set to `4G` in the chunk processing rule. This provides enough overhead for ONNX model loading and ROOT DataFrame operations, preventing the Condor batch system from killing jobs due to memory limit violations.
    
### Submission

First, edit the `reana/samples.json` file to configure your target datasets:
```bash
{
  "dyv12": {
    "source": "das",
    "dataset": "/DYto2L-4Jets_MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v3/NANOAODSIM",
    "era": "2022-postEE",
    "sample": "DYJetsToLL",
    "output": "outputs/hst_DY_NanoAODv12.root"
  },
...
```
Generate the complete list of files by running the local Python script. This queries DAS and builds the dataset JSON configurations inside the `reana/samples/` directory. _(For quick testing, leave just one JSON file in this directory with a couple of example files)._
```bash
python3 prepare.py
```
Finally, submit the jobs to the cluster. A helper script manages the grid proxy and the REANA client commands in the background:
```bash
bash submit.sh testjob
```

### Monitoring and managing jobs

Check the overall workflow status and fetch logs directly via the REANA client.
```bash
reana-client status -w testjob
reana-client logs -w testjob
```
To inspect the live output or trace errors on a specific HTCondor job while it runs, run the following.
```bash
condor_tail 12657109.0  # --> Example job ID
condor_tail -maxbytes 1000000 12657109.0
```
REANA automatically appends a run index to workflows submitted with the same name. To wipe the slate clean and remove all previous runs from the server, run the following.
```bash
reana-client delete -w testjob --include-all-runs
```
Completed or stalled Condor jobs can be manually cleared from the active queue as follows.
```bash
condor_rm $USER
```
## Additional information: how to customize the branches

Before starting, it is helpful to inspect the  `Events`  tree in the input NanoAOD file to verify the exact branch names. The details are also available at  [NanoAOD content self-documentation](https://cms-xpog.docs.cern.ch/autoDoc/)  [restricted to CERN members].

### Declare the variable name

[](https://github.com/phazarik/nanoAOD-dynamic#declare-the-variable-name)

In the main header file (`nanoAna.h`), the variable names and their intended types must be declared. At this stage, they are not yet connected to the input file. The type defined here dictates the internal typecasting, so compatible types must be chosen (e.g., avoiding incompatible conversions like string to int). These declared names are what will be used throughout the core analysis code.
```cpp
  DynamicValueReader<int> nElectron;
  DynamicArrayReader<bool> Electron_convVeto;
  DynamicArrayReader<int> Electron_cutBased;
  DynamicArrayReader<bool> Electron_cutBased_HEEP;
  DynamicArrayReader<bool> Electron_mvaIso_WP80;
```
### Initialize the variable to connect it to the NanoAOD branch

Next, over in the  `headers/Init.h`  file, these variables get initialized and mapped to the actual NanoAOD branch names. This step is what actually connects the local variables to the input data.
```cpp
  nElectron.Init(tree, fReader, "nElectron", 0);
  Electron_convVeto.Init(tree, fReader, "Electron_convVeto");
  Electron_cutBased.Init(tree, fReader, "Electron_cutBased");
  Electron_cutBased_HEEP.Init(tree, fReader, "Electron_cutBased_HEEP");
  Electron_mvaIso_WP80.Init(tree, fReader, "Electron_mvaIso_WP80");
```
The  `Init`  method binds the variable to the input tree. It takes the target tree pointer, the  `fReader`  object (which is the standard  `TTreeReader`  handling the event loop iteration), the expected branch name or list of alternatives, and an optional fallback value.
> Behind the scenes, all the heavy lifting is handled by  `DynamicValueReader`  and  `DynamicArrayReader`  defined in  `headers/DynamicReaders.h`. These wrappers inspect the file's leaf types on the fly and cast them safely, preventing the hard crashes that usually happen when a NanoAOD version changes an underlying type (like going from a  `Int_t`  to a  `UInt_t`).

### Tricks that make this setup useful

**Multiple branch names:**  The setup allows passing a list of potential branch names to deal with naming shifts between production campaigns (like Run 2 vs. Run 3). In the example below, if the first name is missing, the code just falls back to the second. If both are missing, it spits out the fallback value (`0.0`). No matter which name actually exists in the ROOT file, the variable is always called  `fixedGridRhoFastjetAll`  inside the main analysis loop.

fixedGridRhoFastjetAll.Init(tree,fReader,{"Rho_fixedGridRhoFastjetAll","fixedGridRhoFastjetAll"},0.0);

**Smart fallback:**  The best (and worst) part of this setup is that the compiled code will not automatically break during runtime if a branch happens to be missing in the input file, which can easily result in a silent bug in the logic. That is exactly why using the right fallback option is crucial. Deciding whether to pass a default fallback value changes how the reader behaves when a branch goes missing:

-   **Strict mode (no fallback provided):**  If a required branch is missing and no fallback value was given during initialization, accessing it later in the analysis throws a bright red  `std::runtime_error`. This avoids the silent bugs. This strictness is often completely intentional for safety. For example, a missing MVA score should absolutely trigger a crash rather than silently evaluating to a default value.
    
-   **Safe skipping (fallback set to zero):**  For MC-only variables like  `nGenPart`, passing a default of  `0`  means running over data won't break anything. The branch is missing, and  `nGenPart`  safely evaluates to  `0`, and any gen-level loops skip themselves. This trick allows the exact same  `fReader`  to handle both data and MC seamlessly. While throwing in an  `if(_data == 0)`  check is still recommended for good measure, maintaining separate readers or editing the source code for data runs is no longer required!
    
-   **Neutral fallbacks (weight variables):**  Within the same NanoAOD version, samples like QCD often drop LHE-weight branches. Providing a neutral fallback like  `1.0`  avoids recompiling or hacking up the core code while keeping math operations mathematically safe.
    ```cpp
     LHEWeight_originalXWGTUP.Init(tree, fReader, "LHEWeight_originalXWGTUP", 1.0);
    ```


---
Contact: Prachurjya Hazarika (prachurjya.hazarika@cern.ch)
