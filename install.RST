INSTALLATION STEPS TO BUILD SWWM5 on Ubuntu Linux

# Install packages

sudo apt-get update
sudo apt-get install -y libboost-test-dev
sudo apt-get install -y swig

mkdir build
cd build

cmake -DBUILD_TESTS=1 ..

cmake --build .

cd tests

ctest


