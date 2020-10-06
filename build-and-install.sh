#!/usr/bin/env bash

rm -rf build && mkdir build && cd build || exit 1
cmake .. && make && sudo make install && cd .. || exit 1

if [ ! -e /etc/oplb/ ];
then
    sudo mkdir /etc/oplb/
fi

sudo cp proxy.conf /etc/oplb/
