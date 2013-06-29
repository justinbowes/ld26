#!/bin/bash
while true ; do
	if [ $(ps -ef | grep -e 'bin/ld26_server' | grep -v grep | wc -l | tr -s "\n") -eq 0 ]; then
		echo "LD26 Server crashed, restarting"
		nohup bin/ld26_server 3000 >> ld26.out &
	fi
	if [ $(ps -ef | grep -e 'bin/up_server' | grep -v grep | wc -l | tr -s "\n") -eq 0 ]; then
		echo "UP server crashed, restarting"
		nohup bin/up_server 3001 >> up.out &
	fi
	sleep 10
done
