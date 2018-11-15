#!/bin/sh

################################################################################
# Title         : .auto-build.sh
# Date created  : 2 August 2018
__AUTHOR__="Erriez"
#
# This script will start PlatformIO build.
#
################################################################################

################################################################################
##### Setup this script and get the current gh-pages branch.               #####
echo 'Setting up the script...'

# Exit with nonzero exit code if anything fails
set -e

# Install library dependency LowPower.h for example DHT22LowPower.ino
platformio lib install id=38

# Build sources
platformio ci --lib="." --board uno --board micro --board d1_mini --board nanoatmega328 --board pro16MHzatmega328 --board pro8MHzatmega328 --board leonardo --board megaatmega2560 --board due --board d1_mini --board nodemcuv2 --board lolin_d32 examples/DHT22/DHT22.ino
platformio ci --lib="." --board uno --board micro --board d1_mini --board nanoatmega328 --board pro16MHzatmega328 --board pro8MHzatmega328 --board leonardo --board megaatmega2560 --board due --board d1_mini --board nodemcuv2 --board lolin_d32 examples/DHT22Average/DHT22Average.ino
platformio ci --lib="." --board uno --board micro --board d1_mini --board nanoatmega328 --board pro16MHzatmega328 --board pro8MHzatmega328 --board leonardo --board megaatmega2560 --board d1_mini --board nodemcuv2 --board lolin_d32 examples/DHT22DurationTest/DHT22DurationTest.ino
platformio ci --lib="." --board uno --board micro --board d1_mini --board nanoatmega328 --board pro16MHzatmega328 --board pro8MHzatmega328 --board leonardo examples/DHT22LowPower/DHT22LowPower.ino
