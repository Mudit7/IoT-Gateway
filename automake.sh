#! /bin/sh
export LD_LIBRARY_PATH=/home/km/KM_GIT/iot/gateway/lib
make clean
make
python gateway_config.py&
bin/main 
#make copy
