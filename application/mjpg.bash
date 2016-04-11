#!/bin/bash

#cd /home/pi/Documents/mjpg-streamer3/mjpg-streamer/mjpg-streamer-experimental/
cd /var/www/html/mjpg/mjpg-streamer3/mjpg-streamer/mjpg-streamer-experimental/

while true; do

	./mjpg_streamer -i "./input_uvc.so -n -r 1280x720" -o "./output_http.so -w ./www -p 8090"
	sleep 3

done

