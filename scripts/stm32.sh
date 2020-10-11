#!/usr/bin/env bash

# Erase memory  
stm32_erase() {
    ./Tools/st-flash erase
}

stm32_unlock() {
    openocd -f Config/stlink.cfg \
          -f Config/stm32f1x.cfg \
          -c "init; reset halt; stm32f1x unlock 0; reset halt; exit"    
}


case "$1" in
    erase) stm32_erase "${@:1}";;
    unlock) stm32_unlock "${@:1}";;
esac