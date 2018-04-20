#! /bin/sh
export LD_LIBRARY_PATH=/home/km/KM_GIT/iot/gateway/lib
make clean
make
bin/main 
#make copy
