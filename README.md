# Kevlar

HDF5 export utilities for LArSoft.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Requirements

This requires the `uboonecode` UPS environment exist with UPS setup. In addition, the [HEP-DL HDF5 UPS package](https://github.com/kwierman/hdf5) will need to be built and installed in the product path. This needs to be done _before_ the following steps as the HDF5 Cmake Environment doesn't interact well with CETBuildTools during the configuration phase. Bear in mind that unless the HDF5 package is built with zlib support, the build will fail.

>> Note: This will be relaxed in future versions. Currently, this is implemented as an R&D project on MicroBooNE, but the intended scope is LArSoft oriented.

## Installation

1. Setup your local environment via ups

2. Setup MRB

~~~ bash
setup mrb
export MRB_PROJECT=kevlar
~~~

3. Make your development directory

~~~ bash
mkdir kevlar
cd kevlar
mrb newDev -v v4_02_00 -qe14:debug
source local*/setup
~~~

Replace debug with whatever buildspec you feel suits your needs.

4. Pull down the product and update mrb

~~~ bash
cd srcs
git clone https://github.com/HEP-DL/kevlar/
cd kevlar
git checkout develop
cd ..
mrb updateDepsCM
~~~

5. Build

~~~ bash
cd ../build*
mrbsetenv
mrb b 
mrb i
~~~

6. Setup local product

~~~ bash
mrbslp
~~~
