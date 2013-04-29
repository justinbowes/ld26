#!/bin/bash
while true ; do
	if ps -ef | grep -q 'echoserver' ; then
		echo "Server crashed, restarting"
		nohup build/echoserver 3000
	fi
	sleep 10
done
