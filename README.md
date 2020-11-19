# piapf
Pia port forwarding automation

[![Build Status](https://travis-ci.com/darknessnerd/piapf.svg?branch=main)](https://travis-ci.com/darknessnerd/piapf)

# Prerequisite

## Windows
* cmake
* [mingw](https://nuwen.net/mingw.html)
* python
* conan

conan profile:
```
[build_requires]
[settings]
    os=Windows
    os_build=Windows
    arch=x86_64
    arch_build=x86_64
    compiler=gcc
    compiler.version=9.2
    compiler.libcxx=libstdc++11
    build_type=Debug
[options]
[env]
CC=C:\mingw-17\MinGW\bin\gcc.exe
CXX=C:\mingw-17\MinGW\bin\g++.exe

```


# Build with docker

docker build . -t cpp
docker run --rm -it -v $(pwd):/project cpp:latest "mkdir -p build && cd build && conan install .. && cmake .. && make"


# Build locally

```

mkdir build
cd build

conan .. --build=missing
cmake ..
make
```


# Build the debian package 

```

mkdir build
cd build

conan install .. --build=missing
cmake ..
make
make package
```


# Install the debian package 

apt-get install ./piapf-0.1.1-Linux.deb


change the configuration file 

/usr/etc/piapf/config

systemctl daemon-reload

start: 

systemctl start piapf.service

The logs are located in  /var/log/piapf.log

## TODO 

[] tests
[] Logging 
[] check errors
[] release package 