#!/bin/sh

# Install Python platformio 
# pip install -U platformio

# Update library
# git fetch
# git pull

# Build example(s)
platformio ci --lib=".." --project-conf=platformio.ini ../examples/DHT22/DHT22.ino
platformio ci --lib=".." --project-conf=platformio.ini ../examples/DHT22DurationTest/DHT22DurationTest.ino
