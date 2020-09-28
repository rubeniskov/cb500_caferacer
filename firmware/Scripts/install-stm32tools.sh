#!/usr/bin/env bash

PROGRAM_SHELLNAME="$0"
PROGRAM_ARGS="$@"
PROGRAM_FILENAME=$(basename $0)

LOGIN_USERNAME="$1"
LOGIN_PASSWORD="$2"
LOGIN_URL="https://my.st.com/cas/login?service=https://my.st.com/content/my_st_com/en.html"

CUBE_MX_INFO_URL_PAGE="https://my.st.com/content/my_st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html"
CUBE_MX_DOWNLOAD_FILENAME=en.stm32cubemx_v6-0-1
CUBE_MX_DOWNLOAD_URL="https://my.st.com/content/ccc/resource/technical/software/sw_development_suite/group0/9e/ce/6e/18/d2/ff/46/57/stm32cubemx_v6-0-1/files/stm32cubemx_v6-0-1.zip/jcr:content/translations/$CUBE_MX_DOWNLOAD_FILENAME.zip"

STLINK_BU_INFO_URL_PAGE="https://www.st.com/en/development-tools/stsw-link007.html"
STLINK_BU_DOWNLOAD_FILENAME=en.stsw-link007_V2-37-26
STLINK_BU_DOWNLOAD_URL="https://my.st.com/content/ccc/resource/technical/software/firmware/group0/62/1d/93/a3/06/23/4f/a6/stsw-link007_V2-37-26/files/stsw-link007_V2-37-26.zip/jcr:content/translations/${STLINK_BU_DOWNLOAD_FILENAME}.zip"

LICENSE_AGREEMENT="https://my.st.com/resource/en/license/SLA0048_STM32CubeMX.pdf"
HOST_OS=$(uname -s)
ACCEPT_AGREEMENT_FLAG="--accept-agreement"

WORK_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
TOOLS_DIR="$WORK_DIR/../Tools"
TMP_DIR="$(dirname $(mktemp -u))/dwtmp-${PROGRAM_FILENAME%.*}"
CLEAN_FILES="cookies.txt ltoken"

urlencode() {
    # urlencode <string>

    old_lc_collate=$LC_COLLATE
    LC_COLLATE=C

    local length="${#1}"
    for (( i = 0; i < length; i++ )); do
        local c="${1:$i:1}"
        case $c in
            [a-zA-Z0-9.~_-]) printf '%s' "$c" ;;
            *) printf '%%%02X' "'$c" ;;
        esac
    done

    LC_COLLATE=$old_lc_collate
}

login() {
    if [ -z "${1}" ] || [ -z "${2}" ]
    then
        echo "Please specify username and password: $PROGRAM_SHELLNAME test@email.com password"
        exit 1
    fi
    
    if [ ! -f ltoken ]; then 
        echo "-> Login to $1"
        # Getting ssid cookies
        wget $1 -q \
            -O - \
            --keep-session-cookies \
            --save-cookies cookies.txt \
            | grep name=\"lt\"|sed "s/.* value=\"\(.*\)\".*/\1/" > ltoken

        # Getting ssid login
        wget $1 -q \
            --delete-after \
            --load-cookies cookies.txt \
            --save-cookies cookies.txt \
            --keep-session-cookies \
            --post-data "username=$(urlencode $2)&password=$(urlencode $3)&_eventId=Login&lt=$(cat ltoken)"
    fi
}

check_accept_agreement() {
    download_aggrement
    if [[ ! "$PROGRAM_ARGS" == *"$ACCEPT_AGREEMENT_FLAG"* ]]; then 
        echo "To download this software you must accept the license agreement, visit https://my.st.com/resource/en/license/SLA0048_STM32CubeMX.pdf"
        echo "$PROGRAM_SHELLNAME test@email.com password $ACCEPT_AGREEMENT_FLAG"
        exit 1
    fi
}

progressfilt () {
    local flag=false c count cr=$'\r' nl=$'\n'
    while IFS='' read -d '' -rn 1 c
    do
        if $flag
        then
            printf '%s' "$c"
        else
            if [[ $c != $cr && $c != $nl ]]
            then
                count=0
            else
                ((count++))
                if ((count > 1))
                then
                    flag=true
                fi
            fi
        fi
    done
}

moreinfo(){
    echo "Please refer to this page for more information ${@}"
}

download() {
    echo "-> Downloading $1"
    wget "$@" \
        --no-clobber \
        --header 'Connection: keep-alive' \
        --header 'Pragma: no-cache' \
        --header 'Cache-Control: no-cache' \
        --header 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9' \
        --progress=bar:force \
        --header 'Referer: https://my.st.com/content/my_st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html' \
        --load-cookies cookies.txt 2>&1 | progressfilt
}

download_aggrement(){
    download $LICENSE_AGREEMENT --directory-prefix=$TOOLS_DIR
}
install_stm32cubemx() {
    EXIT_CODE=0
    pushd $TMP_DIR > /dev/null
    if [ ! -f SetupSTM32CubeMX-6.0.1.exe ]; then 
        check_accept_agreement
        login $LOGIN_URL $LOGIN_USERNAME $LOGIN_PASSWORD
        download $CUBE_MX_DOWNLOAD_URL
        unzip -a $CUBE_MX_DOWNLOAD_FILENAME.zip
        moreinfo ${CUBE_MX_INFO_URL_PAGE}
    fi
    echo "-> Launching STM32CubeMX setup"
    echo "Sudo password"
    if [ $EXIT_CODE -eq 0 ] && [[ $HOST_OS == "Darwin" ]]; then  
        sudo java -jar SetupSTM32CubeMX-6.0.1.exe
        EXIT_CODE=$?
    else
        chmod 777 *.linux
        sudo java -jar SetupSTM32CubeMX-6.0.1.linux
        EXIT_CODE=$?
    fi

    if [ $EXIT_CODE -eq 0 ]; then
        echo "Installation success!"
    else
        echo "Installation failed!"
    fi 
    popd > /dev/null
    return $EXIT_CODE
}

install_stlink_board_upgrade() {
    pushd $TMP_DIR > /dev/null
    if [ ! -d "stsw-link007" ]; then 
        check_accept_agreement
        login $LOGIN_URL $LOGIN_USERNAME $LOGIN_PASSWORD
        download $STLINK_BU_DOWNLOAD_URL
        unzip -a $STLINK_BU_DOWNLOAD_FILENAME.zip
        moreinfo ${STLINK_BU_INFO_URL_PAGE}
    fi

    if [ ! -d $TOOLS_DIR/stlink_upgrade ]; then
        mv stsw-link007/AllPlatforms $TOOLS_DIR/stlink_upgrade
    fi
    popd > /dev/null
}

install_stlink_flash() {
    pushd $TMP_DIR > /dev/null
    if [ ! -d stm32flash ]; then 
        git clone https://github.com/ARMinARM/stm32flash.git
        pushd stm32flash > /dev/null
        make
        cp stm32flash $TOOLS_DIR
        popd > /dev/null
    fi
    popd > /dev/null
}

cleanup() {
    for FILE in $CLEAN_FILES; do
        if [ -f $TMP_DIR/$FILE ]; then
            rm $TMP_DIR/$FILE
        fi
    done
}

echo "-> Creating temp folder $TMP_DIR"
mkdir -p $TMP_DIR
install_stm32cubemx
install_stlink_board_upgrade
install_stlink_flash
cleanup

echo "Installed Tools in $TOOLS_DIR"
ls -lah $TOOLS_DIR