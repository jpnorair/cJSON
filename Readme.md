# About cJSON for HB Distribution

You can read all about [Dave Gamble's cJSON library](https://github.com/DaveGamble/cJSON), which is the basis for this project.

**cJSON for HB** is a simplified, stripped-down version of the mainline cJSON project.  Here are the specific differences between this version and the mainline version:

* All tests and other fluff are removed.  You can test with the mainline repo.  It is a flat directory of .c and .h files.
* CMake is gone.  Now it builds on platforms that support very basic GNU make tools, with a good-old makefile.
* It fits into the [HB Distribution's](https://github.com/jpnorair/hbdist) packaging model.
* Supports external allocation/free functions (although must be on a process-level).  Namely, a lot of HB software uses talloc, and HB cJSON can be made to work with talloc.
* Has a new block duplication feature, which is a performance-oriented feature for working with memory pools in a database environment.

As with other HB software, the motivation is to optimize performance and utility on lightweight Linux SBCs, often running OpenWRT.
