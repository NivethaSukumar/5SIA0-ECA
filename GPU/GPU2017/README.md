# EEG Analysis using GPU
This is the code for the EEG analysis application used in the course Embedded Computer Architecture of Eindhoven University of Technology.
For background information on the application itself and the assignment, please refer to the [assignment website](https://ecatue.gitlab.io/GPU2017).

## Compilation
To compile this source code for GPU, issue ```make eeg``` from the command line.
An executable named "eeg" will be created, which you can run using ```./eeg```.
To automatically check if the program is correct, simply issue ```make```, which will run the eeg binary and compare the output against a the reference output in reference.txt.

## Flags
In the eeg.h file there are a couple of flags you can use to influence the behviour of this example program.
* VERBOSE: By uncommenting this definition, the code will be more verbose
* DEBUG: Uncomment this to get some built in debug prints for the GPU code
* CPU_ONLY: Use this flag to disable all GPu code, and run only the original CPU code

## Modification
You are free to modify this code however you see fit, with the final goal to port bits and pieces to the GPU to speed up the execution of the eeg analysis.
For this both CUDA and OpenCl are supported.

If you add CUDA code to a source file, change the extension to ".cu", to ensure the makefile uses the correct compiler.
For OpenCL code, change the extension t ".cl"

In case you are using CUDA and an Nvidia GPU, be aware that different GPUs have different capabilities.
Nvidia refers to this as "Compute Capability".
**Make sure to update the Makefile such that it compute capability matches your device**
In case you are using the servers provided by the university, no modification should be required.
The GTX570 GPUs have compute capability 2.0, which is the default in the Makefile.
When you are targeting a different GPU, please update the ```GENCODE_FLAGS``` in the Makefile accordingly.

Best of luck with the assignment!
