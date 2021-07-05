# CS356-OS-project-2

A WRR scheduler for Android linux kernel.

## Build and run

First edit the android.source.sh with its internal instructions, and 
`source ./android.source.sh` to have the correct environment variables
and PATH.

With the Makefile in the root folder, you may use these commands to build:

First you need to config the kernel.

`make config`

The configuation settings with debug info.
`make menuconfig`

The CLI interface for further configuations.

Then you can build the kernel and the utility programs.

`make` `make all`: Build all kernels and the utility programs.

`make build`: Build the kernel with debug info and the utility programs.

`make build_nodebug`: Build the kernel without debug info and the utility programs. (for faster testing)

`make utils`: Build the utility programs

If you want to check the code with AVD please 

- create an AVD named OSPrj519021911045 and run `./run-sim.sh` separately.
- or create an avd.lock file in the directory to indicate AVD is created.

Then you may use this command:

`make push`: Push the utility programs to the AVD.

For more information, please refer to the `Makefile` file.

To utilize our utility programs:

`lshed`: Scheduler information viewer

`chshed`: Scheduler changer

These two programs have no arguments. Just run and fill in according to the prompt.

`proctest`: Our testing program

`./proctest <SUBPROCESS_NUMBER> <CHILD_PROCESS_SCHEDULER> <CHILD_STEP>`

- SUBPROCESS_NUMBER: this will set how many subprocesses will be created.
- CHILD_PROCESS_SCHEDULER: set the subprocesses' scheduling policy.
- CHILD_STEP: this is used to generate different priority/weight.

# Report

Build report by running `make report` and find `report.pdf` in `report` folder.

