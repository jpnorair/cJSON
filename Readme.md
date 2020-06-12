About cJSON for HBuilder
========================
You can read all about [Dave Gamble's cJSON library](https://github.com/DaveGamble/cJSON), which is the basis for this project.

**cJSON for HBuilder** is a simplified stripped-down version of the mainline cJSON project.  Here are the specific differences between this version and the mainline version:

* All tests and other fluff are removed.  It is a single .c and single .h file pair.
* CMake is gone.  Now it builds on platforms that support GNU make tools, with a good-old makefile.
* It fits into the HBuilder package management framework, which is specific to other [jpnorair](https://github.com/jpnorair) (and [Haystack Technologies](http://www.haystacktechnologies.com)) projects.

## Projects That Use HBuilder Package Management
* [argtable](https://github.com/jpnorair/argtable)
* [bintex](https://github.com/jpnorair/bintex)
* [cJSON](https://github.com/jpnorair/cJSON)
* [cmdtab](https://github.com/jpnorair/cmdtab)
* [OTEAX](https://github.com/jpnorair/OTEAX)
* [otter](https://github.com/jpnorair/otter)
* Multiple proprietary software from Haystack Technologies

