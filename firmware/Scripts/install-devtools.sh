#!/usr/bin/env bash

HOST_OS=$(uname -s)

if [[ $HOST_OS == "Darwin" ]] && type brew > /dev/null ; then  
    brew tap ArmMbed/homebrew-formulae
    brew install arm-none-eabi-gcc
    brew install open-ocd --HEAD
fi 