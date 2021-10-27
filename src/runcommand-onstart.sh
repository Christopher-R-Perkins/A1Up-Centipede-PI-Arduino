#!/bin/bash
NAME=$(basename $3 .zip)
if [ "$1" = "arcade" ]
then 
	echo -ne 't' > /dev/ttyACM0
elif [ "$1" = "scummvm" ]
then
	echo -ne 'm' > /dev/ttyACM0
fi
