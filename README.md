# Kevlar

This Kevlar material is designed to provide an interface to software *external* to LArSoft by providing a data format interface layer. Currently, the only format supported is HDF5, but this is planned to be extended to other data formats.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Contents

* Converters
  * HDF5Image
    * Converts RawDigits to HDF5 Image format
  * HDF5Label
    * Converts MCTruthParticles to a vector of labels
* Services
  * HDF5File
    * Provides file output service for HDF5 analyzers

## Requirements

This requires the `uboonecode` UPS environment exist with UPS setup. In addition, the [HDF5 UPS](https://github.com/kwierman/hdf5) package will need to be built and installed in the product path. This needs to be done _before_ the following steps as the HDF5 Cmake Environment doesn't interact well with CETBuildTools during the configuration phase.

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
mrb newDev -v v1_00_00 -qe10:debug
source local*/setup
~~~

Replace debug with whatever buildspec you feel suits your needs.

4. Pull down the product and update mrb

~~~ bash
cd srcs
git clone https://github.com/kwierman/kevlar/
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

### Usage

Typcial larsoft invocation:

~~~ bash
lar -c converter.fcl -s <your input root file>
~~~
