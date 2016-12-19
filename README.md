# kevlar

You guessed it. This is Kevin's library for LArSoft related items

## Contents

* Analyzers
  * NeutronScanner
    * This is designed to fetch information out on neutrons in MCParticles


## Installation

1. Setup your local environment via ups

2. Setup MRB

~~~ bash
setup MRB
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
cd ../build
mrbsetenv
mrb b mrb i
~~~

6. Setup local product

~~~ bash
mrbslp
~~~

### Usage

Typcial larsoft invocation

~~~ bash
lar -c neutron_scanner.fcl -s <your input root file>
~~~
