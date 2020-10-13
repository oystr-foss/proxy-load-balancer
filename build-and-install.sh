#!/usr/bin/env bash

rm -rf build && mkdir build && cd build || exit 1
cmake .. && make && make install && cd .. || exit 1

if [ ! -e /etc/oplb/ ];
then
    mkdir /etc/oplb/
fi

cp proxy.conf /etc/oplb/
