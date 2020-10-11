#!/usr/bin/env bash
# Links
# https://wiki.qt.io/RaspberryPi2EGLFS
# https://freckled.dev/how-to/2019/07/29/create-gcc-crosscompiler/
# https://www.nongnu.org/thug/cross.html

SCRIPT_SHELL_NAME="$0"
SCRIPT_ARGS="$@"
SCRIPT_FILENAME=$(basename $0)
SCRIPT_DIR=$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)

source $SCRIPT_DIR/../../scripts/common.sh
source $SCRIPT_DIR/../../scripts/log.sh
source $SCRIPT_DIR/../../scripts/pkg.sh
source $SCRIPT_DIR/rpi.sh

DIR_PWD=$(pwd)
DIR_TMP="$(mktempd $SCRIPT_FILENAME)"

LOGFILE="$DIR_PWD/$SCRIPT_FILENAME.log"

HOST_OS=$(uname -s)
HOST_NPROC=$([ type nproc 2> /dev/null ] && nproc || sysctl -n hw.physicalcpu)

SYSROOT_RELATIVELINK_SCRIPT_URL="https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py"

#'auto'
RPI_MODEL="BCM2835"
RPI_QT_FOLDER=/usr/local/qt5pi
RPI_SSH_PATH=pi@raspberrypi.local
RPI_RSYNC_EXCLUDE="*.ko *.fw *.bin *.img *.sh *.txt *.dm *.debug *git-core*"
RPI_HARDWARE_SOC=
RPI_HARDWARE_BUILD_FLAGS=
RPI_HARDWARE_BUILD_FLAGS_TABLE=$(cat <<EOF
BCM2835 --with-arch=armv6zk tune=arm1176jzf-s --with-fpu=vfp --with-float=hard
EOF
)

DOWNLOAD_FLAGS=--no-clobber
BUILD_FLAGS="\
--target=arm-linux-gnueabihf \ \
--with-sysroot="$DIR_SYSROOT" \
"

# gcc_version="gcc-4.9"
# export CC="$(which $gcc_version)"
# export CXX="$(which $gcc_version)" 
# export LD=$CC
# export CFLAGS="-Wno-error=deprecated-declarations"
# export PATH="$DIR_TOOLCHAIN/bin:$PATH"


clean() {
  if [[ "$@" == *"--clean-all"* ]]; then  
    rm -rf $DIR_TOOLCHAIN $DIR_SYSROOT
  fi
  rm -rf $DIR_TMP
}

set -- $@

while [[ $# -gt 0 ]]; do
  case $1 in
    --clean|--clean-all)
      clean
      exit
    ;;
    --force-sync)
      forcesync=true
      shift
    ;;
    *)
      break
    ;;
  esac
done 

install_requirements() {
log --header "Checking requirements" 
if [ $HOST_OS = Darwin ]; then 
  xcode-select --install 2> /dev/null

  if semver "$(rsync --version|head -n 1|awk '{ print $3}') < 3.2.3"; then
    rsyncPath="$(brew --prefix rsync 2>/dev/null)/bin/rsync"
    if [ ! -f $rsyncPath ]; then
      log "Seems you have an old rsync version... so it will install the newest" 
      log_wrap --header "Installing rsync"  -- brew install rsync
    fi
    log "Pointing rsync to $rsyncPath v$($rsyncPath --version|head -n 1|awk '{ print $3}')"
    calias rsync $rsyncPath
  fi
fi
#if [ $HOST_OS == Darwin ]; then
#   BUILD_FLAGS+=" CFLAGS="-lstdc++""
}

install_toolchain() {
  pkg_parse_config \
    $DIR_PWD/pkg-devtools.conf \
    --tempdir $DIR_TMP \
    --logfile $LOGFILE \
    "$@" \
    -- $BUILD_FLAGS -v
}

rpi_config() {
  if [[ $RPI_MODEL = "auto" ]]; then

    rpi_check_conection ${RPI_SSH_PATH}
    if [ ! $? -eq 0 ]; then 
      exit 1
    fi

    RPI_HARDWARE_SOC=$(grep Hardware <(rpi_exec_cmd ${RPI_SSH_PATH} cat /proc/cpuinfo) | awk '{ print $3 }')
  else 
    RPI_HARDWARE_SOC=$RPI_MODEL
    RPI_MODEL="manual"
  fi 

  RPI_HARDWARE_BUILD_FLAGS=$(echo $RPI_HARDWARE_BUILD_FLAGS_TABLE | grep $RPI_HARDWARE_SOC 2>/dev/null | cut -d' ' -f 2-)
  BUILD_FLAGS+=" $RPI_HARDWARE_BUILD_FLAGS"
  cat <<-EOF

    Summary 
      > Selected RaspberryPi using "$RPI_MODEL" strategy -> $RPI_HARDWARE_SOC
        * Cross compile flags: $RPI_HARDWARE_BUILD_FLAGS
    
EOF
}

rpi_prepare_qt() {
local rpi_stat_response=r$(pi_exec_cmd ${RPI_SSH_PATH} stat $RPI_QT_FOLDER 2>&1)
  if [ ! $? -eq 0 ]; then 
    rpi_exec_batch ${RPI_SSH_PATH} <<-EOF
echo "-> Install development libraries"
sudo sed -i 's/#deb-src/deb-src/' /etc/apt/sources.list 
sudo apt-get update
sudo apt-get build-dep qt4-x11 -y
sudo apt-get build-dep libqt5gui5 -y
sudo apt-get install libudev-dev libinput-dev libts-dev libxcb-xinerama0-dev libxcb-xinerama0 -y
echo "-> Prepare target folder "
sudo mkdir -p ""$RPI_QT_FOLDER""
sudo chown pi:pi ""$RPI_QT_FOLDER""
EOF
  else
    echo $rpi_stat_response
  fi 
} 

rpi_copy_sysroot() {
  if [ ! -d $DIR_SYSROOT ] || [[ $forcesync = true ]]; then 
    log --header "Syncing rpi $RPI_SSH_PATH sysroot in $DIR_SYSROOT"
    rsync -vrzLR \
      $(sed -e 's/[^ ]*/--exclude="&"/g' <<< $RPI_RSYNC_EXCLUDE) \
      --safe-links \
      $RPI_SSH_PATH:/lib \
      $RPI_SSH_PATH:/usr/include \
      $RPI_SSH_PATH:/usr/lib \
      $RPI_SSH_PATH:/opt/vc \
      $DIR_SYSROOT/
  fi

  python \
    <(wget -q -O - -o /dev/null $SYSROOT_RELATIVELINK_SCRIPT_URL ) \
    sysroot
}

log "Using temporary directory $DIR_TMP"

install_requirements $@
rpi_config $@
rpi_copy_sysroot $@
install_toolchain $@
rpi_prepare_qt
exit 

### START pkg's setup
# pkg_build_install \
#   https://ftpmirror.gnu.org/binutils/binutils-$VERSION_BINUTILS.tar.bz2 \
#   $DIR_TOOLCHAIN/bin/arm-linux-gnueabihf-ld \
#   $RPI_HARDWARE_BUILD_FLAGS \
#   --without-cvs \
#   --enable-add-ons=crypt \
#   --enable-lto

# pkg_build_install \
#   http://ftp.gnu.org/gnu/libc/glibc-$VERSION_GLIBC.tar.xz \
#   $DIR_TOOLCHAIN/bin/arm-linux-gnueabihf-ld \
#   $RPI_HARDWARE_BUILD_FLAGS \
#   --without-cvs \
#   --enable-add-ons=crypt

# pkg_build_install \
#   https://ftp.wayne.edu/gnu/gcc/gcc-$VERSION_GCC/gcc-$VERSION_GCC.tar.xz \
#   $DIR_TOOLCHAIN/bin/arm-linux-gnueabihf-gcc \
#   $RPI_HARDWARE_BUILD_FLAGS \
#   --enable-lto \
#   --enable-languages=c,c++,go,d <<EOF
# ./contrib/download_prerequisites
# EOF
### END pkg's setup


# pushd $DIR_TMP

# if [ ! -f binutils.tar.bz2 ]; then 
#   log "Downloading binutils $VERSION_BINUTILS"
#   download \
#     --no-clobber \
#     --output-document=binutils.tar.bz2 \
#     "https://ftpmirror.gnu.org/binutils/binutils-$VERSION_BINUTILS.tar.bz2"
# fi

# if [ ! -d binutils_source ]; then
#   log "Unpacking binutils $VERSION_BINUTILS"
#   mkdir -p binutils_source
#   pushd binutils_source
#   unpack --strip-components 1 -xf ../binutils.tar.bz2
#   popd 
# fi

# if [ ! -f $DIR_TOOLCHAIN/bin/arm-linux-gnueabihf-ld ]; then 
#   log "Building binutils $VERSION_GCC"
#   pushd binutils_source
#   patchakra $DIR_PATCHES/binutils

#   ./configure \
#     $RPI_HARDWARE_BUILD_FLAGS \
#     --target=arm-linux-gnueabihf \
#     --prefix="$DIR_TOOLCHAIN" \
#     --with-build-sysroot="$DIR_TOOLCHAIN" \
#     --with-sysroot="$DIR_SYSROOT" \
#     --disable-multilib \
#     --enable-gold=yes \
#     --enable-lto > /dev/null

#   log "Installing binutils $VERSION_GCC"
#   make -j$HOST_NPROC
#   make install
#   popd
# fi
# popd
# ### END binutils setup

# ### START glibc setup
# pushd $DIR_TMP
# if [ ! -f glibc.tar.xz ]; then 
#   log "Downloading glibc $VERSION_GCC"
#   download \
#       --no-clobber \
#       --output-document=gcc.tar.xz \
#       "http://ftp.gnu.org/gnu/libc/glibc-$VERSION_GLIBC.tar.xz"
# fi

# if [ ! -d glibc_source ]; then
#   log "Unpacking glibc $VERSION_GCC"
#   mkdir -p glibc_source
#   pushd glibc_source
#   unpack --strip-components 1 -xf ../glibc.tar.xz
#   popd 
# fi

# if [ ! -f $DIR_TOOLCHAIN/lib/libcc1.so ]; then 
#   log "Building glibc $VERSION_GCC"
#   pushd glibc_source
#   ./contrib/download_prerequisites
#   ./configure \
#     $RPI_HARDWARE_BUILD_FLAGS \
#     --target=arm-linux-gnueabihf \
#     --prefix="$DIR_TOOLCHAIN" \
#     --with-build-sysroot="$DIR_TOOLCHAIN" \
#     --with-sysroot="$DIR_SYSROOT" \
#     --disable-multilib \
#     --without-cvs \
#     --enable-add-ons=crypt

#   log "Installing gcc $VERSION_GCC"
#   make -j$HOST_NPROC
#   make install
#   popd
# fi
# popd
# ### END glibc setup

# ### START gcc setup
# pushd $DIR_TMP
# if [ ! -f gcc.tar.xz ]; then 
#   log "Downloading gcc $VERSION_GCC"
#   download \
#       --no-clobber \
#       --output-document=gcc.tar.xz \
#       "https://ftp.wayne.edu/gnu/gcc/gcc-$VERSION_GCC/gcc-$VERSION_GCC.tar.xz"
# fi

# if [ ! -d gcc_source ]; then
#   log "Unpacking gcc $VERSION_GCC"
#   mkdir -p gcc_source
#   pushd gcc_source
#   unpack --strip-components 1 -xf ../gcc.tar.xz
#   popd 
# fi

# if [ ! -f $DIR_TOOLCHAIN/bin/arm-linux-gnueabihf-gcc ]; then 
#   log "Building gcc $VERSION_GCC"
#   pushd gcc_source
#   ./contrib/download_prerequisites
#   ./configure \
#     $RPI_HARDWARE_BUILD_FLAGS \
#     --target=arm-linux-gnueabihf \
#     --prefix="$DIR_TOOLCHAIN" \
#     --with-build-sysroot="$DIR_TOOLCHAIN" \
#     --with-sysroot="$DIR_SYSROOT" \
#     --disable-multilib \
#     --enable-lto \
#     --enable-languages=c,c++,go,d

#   log "Installing gcc $VERSION_GCC"
#   make -j$HOST_NPROC
#   make install
#   popd
# fi
# popd

# if [ 1 -eq 1 ]; then 
#     pushd binutils_source
#     ./configure \
#         --prefix="$DIR_TOOLCHAIN" \
#         --with-sysroot="$DIR_SYSROOT" \
#         --target=arm-linux-gnueabihf \
#         --disable-multilib \
#         --enable-gold=yes \
#         --enable-lto
#     make -j$HOST_NPROC
#     make install
#     popd
# fi







# if [ ! -d qt-everywhere-src-5.12.5 ]; then
#     wget --no-clobber http://download.qt.io/official_releases/qt/5.12/5.12.5/single/qt-everywhere-src-5.12.5.tar.xz
#     tar xvf  qt-everywhere-src-5.12.5.tar.xz
# fi

# pushd  qt-everywhere-src-5.12.5


# SYSROOT=/Volumes/xtools/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sysroot/

# pushd qtbase

#sed -i.bu 's/\-lEGL/\-lbrcmEGL/' ./qtbase/mkspecs/devices/linux-rasp-pi-g++/qmake.conf
#sed -i.bu 's/-LGLESv2/-lbrcmGLESv2/' ./qtbase/mkspecs/devices/linux-rasp-pi-g++/qmake.conf

# ./configure \
#     -release \
#     -opengl es2 \
#     -device linux-rasp-pi-g++ \
#     -device-option CROSS_COMPILE=/Volumes/xtools/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi- \
#     -sysroot $SYSROOT \
#     -opensource \
#     -confirm-license \
#     -skip qtwayland \
#     -skip qtlocation \
#     -skip qtscript \
#     -make libs \
#     -prefix /usr/local/qt5pi \
#     -extprefix $(pwd)/qt5pi \
#     -hostprefix $(pwd)/qt5 \
#     -no-use-gold-linker \
#     -L $SYSROOT/lib/arm-linux-gnueabihf \
#     -v \
#     -no-gbm

# popd
# ./configure \
#     -release \
#     -opengl es2 \
#     -device linux-rasp-pi-g++ \
#     -device-option CROSS_COMPILE=/Volumes/xtools/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi- \
#     -sysroot /Volumes/xtools/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sysroot/ \
#     -opensource \
#     -confirm-license \
#     -skip qtwayland \
#     -skip qtlocation \
#     -skip qtscript \
#     -make libs \
#     -prefix /usr/local/qt5pi \
#     -extprefix $(pwd)/qt5pi \
#     -hostprefix $(pwd)/qt5 \
#     -no-use-gold-linker \
#     -v \
#     -no-gbm



# ./configure \
#     -release \
#     -opengl es2 \
#     -device linux-rasp-pi-g++ \
#     -opensource \
#     -confirm-license \
#     -skip qtwayland \
#     -skip qtlocation \
#     -skip qtscript \
#     -make libs \
#     -no-use-gold-linker \
#     -v \
    # -no-gbm
#-sysroot /Volumes/xtools/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.8.2/../../../../arm-none-linux-gnueabi/sysroot \

# /usr/local/opt/llvm/bin/clang \
#     --target=arm-linux-gnueabihf \
#     --sysroot=$PWD/sysroot \
#     -isysroot=$PWD/sysroot \
#     -isystem=$PWD/sysroot/usr/include \
#     -L$PWD/sysroot/usr/lib/gcc/arm-linux-gnueabihf/8 \
#     -B$PWD/sysroot/usr/lib/gcc/arm-linux-gnueabihf/8 \
#     --gcc-toolchain=`brew --prefix arm-linux-gnueabihf-binutils` \
#     -o hello \
#     hello.c

# --target=arm-linux-gnueabihf \
# --sysroot=/Users/rubeniskov/Workspace/cb500_caferacer/ui/sysroot \
# -isysroot=/Users/rubeniskov/Workspace/cb500_caferacer/ui/sysroot \
# -isystem=/Users/rubeniskov/Workspace/cb500_caferacer/ui/sysroot/usr/include \
# -L/Users/rubeniskov/Workspace/cb500_caferacer/ui/sysroot/usr/lib/gcc/arm-linux-gnueabihf/8 \
# -B/Users/rubeniskov/Workspace/cb500_caferacer/ui/sysroot/usr/lib/gcc/arm-linux-gnueabihf/8 \
# --gcc-toolchain=`brew --prefix arm-linux-gnueabihf-binutils`

# /Users/rubeniskov/Workspace/cb500_caferacer/ui


# > rm -f verifyspec.o
# > rm -f *~ core *.core
# > /Volumes/xtools/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-g++ -c -pipe -marm -mfpu=vfp -mtune=arm1176jzf-s -march=armv6zk -mabi=aapcs-linux -mfloat-abi=hard --sysroot=/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/sysroot -O2 -std=gnu++11 -w -fPIC  -I/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/qtbase/config.tests/verifyspec -I. -I/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/qtbase/mkspecs/devices/linux-rasp-pi-g++ -o verifyspec.o /Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/qtbase/config.tests/verifyspec/verifyspec.cpp
# > /Volumes/xtools/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-g++ -mfloat-abi=hard --sysroot=/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/sysroot -Wl,-O1 -Wl,-rpath-link,/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/sysroot/opt/vc/lib -Wl,-rpath-link,/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/sysroot/usr/lib/arm-none-linux-gnueabi -Wl,-rpath-link,/Users/rubeniskov/Workspace/cb500_caferacer/ui/qt-everywhere-src-5.12.5/sysroot/lib/arm-none-linux-gnueabi -o verifyspec verifyspec.o      
# > /Volumes/xtools/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.8.2/../../../../arm-none-linux-gnueabi/bin/ld: cannot find crt1.o: No such file or directory
# > /Volumes/xtools/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.8.2/../../../../arm-none-linux-gnueabi/bin/ld: cannot find crti.o: No such file or directory
# > /Volumes/xtools/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.8.2/../../../../arm-none-linux-gnueabi/bin/ld: cannot find -lm











# In raspberry pi 

# maybe install glibc
# sudo apt-get install libgl-dev -yy





# wget 'https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz?revision=fed31ee5-2ed7-40c8-9e0e-474299a3c4ac&la=en&hash=76DAF56606E7CB66CC5B5B33D8FB90D9F24C9D20' -O gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz
# tar -xvf gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz




# https://gcc.gnu.org/gcc-4.9/


# https://www.gnu.org/software/gdb/
# https://sourceware.org/newlib/
# https://crosstool-ng.github.io/docs/install/
# git clone git://gcc.gnu.org/git/gcc.git
# brew install crosstool-ng

# cat /proc/cpuinfo |grep 'model name'|tr '-' ' '|tr '[:upper:]' '[:lower:]'|awk '{ print $4}'



# if [[ $HOST_OS == "Darwin" ]] ; then  
#     wget --no-clobber https://s3.amazonaws.com/jaredwolff/rpi-xtools-201402102110.dmg.zip
# fi 


# cd ../binutils_build
# ../binutils_source/configure \
#   --prefix="$install_dir" \
#   --target=arm-linux-gnueabihf \
#   --with-arch=$arch \
#   --with-tune=$tune \
#   --with-fpu=vfp \
#   --with-float=hard \
#   --disable-multilib \
#   --with-sysroot="$sysroot_dir" \
#   --enable-gold=yes \
#   --enable-lto
# make -j$(nproc)
# make install



