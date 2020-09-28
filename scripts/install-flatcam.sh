#!/usr/bin/env bash

SCRIPT_SHELL_NAME="$0"
SCRIPT_ARGS="$@"
SCRIPT_FILENAME=$(basename $0)
SCRIPT_DIR=$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)

source "$SCRIPT_DIR/common.sh"
source "$SCRIPT_DIR/log.sh"
source "$SCRIPT_DIR/pkg.sh"

DIR_TMP="$(mktempd $SCRIPT_FILENAME)"

FLATCAM_VERSION=8.993
FLATCAM_FILENAME=FlatCAM_beta_${FLATCAM_VERSION}

log "Using temporary directory $DIR_TMP"
mkdir -p $DIR_TMP

pushd $DIR_TMP
case $(uname -s) in
  Darwin)
    log --header "Installing dependencies"
    brew install python@3 pyqt geos spatialindex gdal
    mkdir -p flatcam

    if [ ! -f ${FLATCAM_FILENAME}_sources.zip ]; then
      pkg_download https://bitbucket.org/jpcgt/flatcam/downloads/${FLATCAM_FILENAME}_sources.zip
    fi

    if [ ! -f flatcam/FlatCAM.py ]; then
      pushd flatcam
      pkg_unpack --strip-components 1 -xf ../${FLATCAM_FILENAME}_sources.zip
      popd
    fi

    pushd flatcam
    virtualenv env
    source env/bin/activate

    python3 -m ensurepip
    python3 -m pip install PyQt5
    python3 -m pip install -r requirements.txt
    deactivate

    log --header "Create a script to execute FlatCAM"
    cat <<'EOF' > FlatCAM
#!/bin/bash
export PATH='/usr/local/bin:/usr/local/sbin:'"$PATH"
script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if real_script_path="$( readlink "$script_directory/$( basename "$0" )" )"; then
    real_script_directory="$( dirname "$real_script_path" )"
else
    real_script_directory="$script_directory"
fi
{ source "$script_directory"'/env/bin/activate'; python3 "$script_directory"'/FlatCAM.py' &> ~/Library/Logs/FlatCAM.log; deactivate; } &
disown -h $!; exit $?
EOF
    chmod +x FlatCAM
    cat <<'EOF' > FlatCAM.scpt
set script_path to POSIX path of ((path to me as text) & "::") & "FlatCAM"
do shell script "/usr/bin/open --background --hide " & script_path
close window
EOF
    osacompile -o FlatCAM.app FlatCAM.scpt
    popd

    log --header "Installing in /Applications/FlatCAM/${FLATCAM_FILENAME}/FlatCAM.app"
    
    mkdir -p /Applications/FlatCAM/${FLATCAM_FILENAME}
    if [ -d /Applications/FlatCAM/${FLATCAM_FILENAME} ]; then
      rm -rf /Applications/FlatCAM/${FLATCAM_FILENAME}
    fi
    
    mv flatcam /Applications/FlatCAM/${FLATCAM_FILENAME}
    log --header "Create a script to execute FlatCAM"
    open /Applications/FlatCAM/${FLATCAM_FILENAME}/FlatCAM.app
    
    if [ $? -eq 0 ]; then
      log --ok "Yeah!!! you did it!"
    else
      log --err "Oh crap... not working!"
    fi
  ;;
esac