INSTALLATION STEPS TO BUILD SWWM5 on Ubuntu Linux

## Install packages

sudo apt-get update
sudo apt-get install  build_essetials libboost-test-dev swig

mkdir build
cd build

cmake -DBUILD_TESTS=1 ..
cmake --build .

## Run tests

cd tests
ctest

## Regression Tests
cd ../..
pip install --user --src build/packages -r tools/requirements.txt

tools/before-test.sh nrtestsuite build/bin 5112
tools/run-nrtest.sh nrtestsuite 5112



