

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

mktempd() {
  local tempdir=$(dirname $(mktemp -u))/${1%.*}
  echo $tempdir
}

# nasty way to make an alias inside non-interactive shell
calias() {
  local alias=$1
  eval "${alias}(){ "${@:2}" "\$@" ; }"
}

scriptd() {
  echo $(cd "$( dirname "$1" )" >/dev/null 2>&1 && pwd)
}

vercomp () {
    if [[ $1 == $2 ]]
    then
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
            return 1
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
            return 2
        fi
    done
    return 0
}

semver() {
    set -- $1
    vercomp ${1} ${3}
    case $? in
        0) op='=';;
        1) op='>';;
        2) op='<';;
    esac
    if [[ $op != $2 ]]; then
      return 1
    else
      return 0
    fi
}

osdistver() {
  case $(uname -s) in
    Darwin) sw_vers -productVersion;;
    Linux) lsb_release -a|grep Release|awk '{ print $2 }';;
  esac
}