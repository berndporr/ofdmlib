# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#
# This Script Installs tools and dependencies for ofdmlib development
!/bin/sh


#BASEDIR=$(dirname $0)
#echo "Script location: ${BASEDIR}"

sudo apt-get update -y
sudo apt-get upgrade -y

# Dev Tools
sudo apt-get install git -y
sudo apt-get install cmake -y
sudo apt-get install cppcheck -y
sudo apt-get install doxygen -y
sudo apt-get install graphviz -y
sudo apt-get install build-essential -y
sudo apt-get install libboost-all-dev -y

# ofdmlib Dependencies
sudo apt-get install fftw3 -y
sudo apt-get install libfftw3-bin -y
sudo apt-get install libfftw3-dev -y

# Audio Demo Dependencies
sudo apt-get install libpulse-dev -y
sudo apt-get install libasound2-dev -y
sudo apt-get install librtaudio-dev -y
