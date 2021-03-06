#------------------------------------------------
# Additions
sudo apt-get install build-essential
sudo apt-get install software-properties-common
sudo apt-get install libpthread-stubs0-dev
#------------------------------------------------
sudo apt-get remove --auto-remove cmake
sudo apt-get purge --auto-remove cmake
#------------------------------------------------
# Install from repository - ubuntu
sudo add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update
sudo apt-get install cmake
sudo apt-get upgrade
#------------------------------------------------
# Install from src - debian
#wget https://cmake.org/files/v3.2/cmake-3.2.1-1-src.tar.bz2
#tar xvjf cmake-3.2.1-1-src.tar.bz2
#sh ./cmake-3.2.1-1.sh
#------------------------------------------------
#export PATH=/usr/local/bin:$PATH
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#------------------------------------------------
cmake --version
#------------------------------------------------
