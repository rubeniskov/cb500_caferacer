#!/usr/bin/env bash

HOST_OS=$(uname -s)

if [[ $HOST_OS == "Darwin" ]]; then  
    wget --no-clobber 'http://download.qt.io/official_releases/online_installers/qt-unified-mac-x64-online.dmg'
    VOLUME=$(hdiutil attach qt-unified-mac-x64-online.dmg|grep /dev/disk|grep Apple_HFS|awk '{ print $3 }')
    open $VOLUME/qt-unified-mac-x64-3.2.3-online.app
fi