# pkg_patchakra dir_patches 
pkg_patchakra() {
  local os=$(uname -s)
  local dir=$1
  
  if [ ! -d $1 ]; then 
    return 0
  fi

  log "Looking for patches to apply... in $1"

  for file in $(ls $1|grep -i $os); do
    local apply=false
    local chunks=$(echo $file | tr '-' ' ')
    set -- $chunks
    local criterias=$(echo $1 | tr '_' ' ')
    local desc=$2
    set -- $criterias

    # Version criteria
    if [[ "$2" =~ ^([\<\>\=])([0-9]+)(\.[0-9]+)*$ ]]; then
      local version="${2:1}"
      local matcher="${2:0:1}"
      if semver "$(osdistver) $matcher $version"; then
        apply=true  
      fi
    fi; 

    if [ $apply = true ];then
        patch -p0 -Nb < $dir/$file
        if [ $? -eq 0 ]; then
          log --ok "Applied patch $file"
        else
          log --err "Error when apply patch $file"
        fi
    fi 
  done 
}



pkg_parse_config() {
  local filecontent
  local pkgconfig
  local pkgname
  local args

  if [ ! -t 0 ]; then
    filecontent=`cat`
    args="$@"
  else
    filecontent=$(cat $1)
    args="${@:2}"
  fi
  
  while IFS= read -r line; do
    if [[ $line =~ ^(\[([a-z_-]+)\])$ ]]; then
      if [ ! -z $pkgname ]; then 
        pkg_build_install $pkgname $args <<< "$pkgconfig"
      fi
      pkgname=${BASH_REMATCH[2]}
      pkgconfig=
    else
      pkgconfig="$(printf "%s\n%s" "$pkgconfig" "$line")"
    fi
  done <<< "$filecontent"

  if [ ! -z $pkgname ]; then 
    pkg_build_install $pkgname $args <<< "$pkgconfig"
  fi
}

pkg_build_install() {
  local pkgname
  local pkgconfig
  local sources
  local filename
  local cflags=$(echo $@|awk -F' -- ' '{ print $2 }')
  local dflags
  local lflags
  local tmpdir
  local patchesdir
  local verbose
  local args

  # tokens
  #declare -A tokens 
  local ctoken
  local tcontent
  # local url 
  # local filecheck
  # local homepage
  # local sha256
  # local preconfig
  # local postconfig
  # local version
  
  if [ ! -t 0 ]; then
    pkgname=$1
    pkgconfig=`cat`
    shift 1
  else
    pkgname=$2
    pkgconfig=$(cat $1)
    shift 2
  fi

  set -- $@
   
  while [[ $# -gt 0 ]]; do
    case $1 in
      -t|--tempdir)
        tmpdir="$2"
        shift 2
      ;;
      -p|--patchesdir)
        patchesdir="$2"
        shift 2
      ;;
      -D|--download-flags)
        dflags="$2"
        shift 2
      ;;
      -l|--logfile)
        lflags+=" --logfile $2"
        shift 2
      ;;
      -V|--verbose)
        verbose=1
        shift 1
      ;;
      --)
        break
      ;;
      *)
        echo "Usage"
        exit 1
      ;;
    esac
  done 
  
  sources=${pkgname}_src
  
  if [ -z "$lflags" ]; then
    lflags+=" --logfile $(pwd)/pkg_build_install-$pkgname.log"
  fi

  if [ ! -z $verbose]; then
    lflags+=" --verbose"
  fi

  if [ -z "$tmpdir" ]; then
    log $lflags --err "Temporary directory must be defined\n" 
  fi
  
  while IFS=  read -r line; do
    set -- $line
    case $1 in 
      url|version|filecheck|homepage|\
      sha256|configflags|preconfig|postconfig)
        if [ ! -z "$ctoken" ]; then
          tcontent="$(eval "export version="${version}"; echo "$tcontent"")"
          # nasty way to create dynamic variable due 
          # there's no way to make an associative array in bash 3
          eval "local $ctoken="\"$tcontent\"""
        fi
        ctoken="$1"
        tcontent="${@:2}"
      ;;
      \#*)
        #ignore comments
      ;;
      *)
        tcontent+="$line"
      ;;
    esac 
  done <<< "$pkgconfig"
  
  
  if [ ! -z "$ctoken" ]; then
    eval "local $ctoken="\"$tcontent\"""
  fi

  filename="${url##*/}"
  signaturefile="${sha256##*/}"
  
  pushd $tmpdir > /dev/null
  
  # if [ ! -f $signaturefile ]; then 
  #   log --header "Downloading $signaturefile"
  #   log_wrap $lflags -n "Download" \
  #     pkg_download \
  #       $dflags \
  #       --output-document=$signaturefile \
  #       $sha256
  # fi

  if [ ! -f $filename ]; then 
    log $lflags --header "Downloading $filename"
    pkg_download \
        $dflags \
        --output-document=$filename \
        $url
  fi

  # if [ -f $signaturefile ]; then
  #   log --header "Checking signature $filename"
  #   gpg --keyserver $(echo $signaturefile | awk -F[/:] '{print $4}') ${KEY_ID}
  #   exit
  #   gpg --no-default-keyring --verify $signaturefile $filename

  #   if [ $? -eq 0 ]
  #   then
  #       log $lflags --ok "All is well"
  #   else
  #       log_errexit $lflags "Problem with signature"
  #   fi
  # fi

  if [ ! -d $sources ]; then
    log $lflags --header "Unpacking $filename"
    mkdir -p $sources
    pushd $sources > /dev/null
    pkg_unpack --strip-components 1 -xf ../$filename
    if [ ! $? -eq 0 ]; then
      rm $sources
    fi
    popd > /dev/null
  fi
  
  if [ -z "$filecheck" ] || [ ! -f "$filecheck" ]; then 
    pushd $sources > /dev/null
    #pkg_patchakra $DIR_PATCHES/$basename
    
    log $lflags --header "Setting up $filename"

    if [ ! -z "$preconfig" ]; then      
      log_wrap $lflags -n "Pre configure" -- \
        "$preconfig"
    fi
    mkdir -p build
    pushd build
    log_wrap $lflags -n "Configure" -- \
      ../configure $cflags $extraflags $configflags

    if [ ! -z $postconfig ]; then
      log_wrap $lflags -n "Post configure" -- \
        "$postconfig"
    fi

    log $lflags --header "Building $filename"
    log_wrap $lflags -n "Build" -- make -j$HOST_NPROC

    log $lflags --header "Installing $filename"
    log_wrap $lflags -n "Install" -- make install
    popd > /dev/null
    popd > /dev/null
  fi
  popd > /dev/null
}

pkg_download() {
    wget "$@" --progress=bar:force 2>&1 | log_wrap_wget_progress $lflags
}

pkg_unpack() {
  local total=0
  local lflags
  local stderr
  set -- $@

  while [[ $# -gt 0 ]]; do
    case $1 in
      -l|--logfile)
        lflags+="--logfile $2"
        shift 2
      ;;
      *)
        break
      ;;
    esac
  done

  tar v "$@" 2>&1 |\
  while read line; do
      stderr+=$line
      total=$((total+1))
      log $lflags "--prg:$total" "Unpacking ${line##*/}"
  done
  
  if [ ! ${PIPESTATUS[0]} -eq 0 ]; then
    echo >&2 $stderr
  fi

  return ${PIPESTATUS[0]}
}