#! /bin/sh
export LD_LIBRARY_PATH=/home/km/KM_GIT/iot/gateway/lib  #for dynamic library
make clean
make
python gateway_config.py&				#python script in background periodically updates config file
bin/main 						#finally execute the binary file
