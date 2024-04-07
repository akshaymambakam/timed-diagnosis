# The diagnoseTRE package
This repository is for sharing the source code of the diagnoseTRE tool. The tool is for doing timed pattern matching with diagnostics using timed regular expressions. We make use of **The GNU Multiple Precision Arithmetic Library** for accuracy.

## Installation
We give installation instructions for Ubuntu.
1. Install make.
```
sudo apt install make
```
2. Install C++ compiler.
```
sudo apt install g++
```
4. Install the GNU Multiple Precision (GMP) Arithmetic Library.
```
sudo apt install libgmp-dev
```
5. Build
```
make
```
6. Example run
```
./dtre intersection.tre testInter.csv
```
## Brief description
We parse the TRE and create a diagnosis tree. And treat intervals in the match set as the occurrences of faulty or malicious cases. Using the tree, we can find out which zones or intervals have contributed to the fault.
## Authors and acknowledgment
This package is built on top of the work done at the VERIMAG laboratory located in the Grenoble city of France. This work is based on the theory of timed pattern matching developed by [Dogan Ulus](https://www.cmpe.boun.edu.tr/tr/people/dogan.ulus). Check out his [github page](https://github.com/doganulus). Timed pattern matching has been implemented in [montre](https://github.com/doganulus/montre) and [timedrel](https://github.com/doganulus/timedrel).

## License
For open source projects, say how it is licensed.