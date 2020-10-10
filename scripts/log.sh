LOGFILE=

ESC_SEQ="\x1b["
COL_RESET=$ESC_SEQ"39;49;00m"
COL_RED=$ESC_SEQ"31;01m"
COL_GREEN=$ESC_SEQ"32;01m"
COL_YELLOW=$ESC_SEQ"33;01m"
COL_BLUE=$ESC_SEQ"34;01m"
COL_MAGENTA=$ESC_SEQ"35;01m"
COL_CYAN=$ESC_SEQ"36;01m"

log() {
  local ret
  local label
  local msg
  local color
  local cols=$(tput cols)
  local logfile=$(log_filename)
  local verbose

  set -- $@
  while [[ $# -gt 0 ]]; do
    case $1 in
      -l|--logfile)
        logfile="$(log_filename $2)"
        shift 2
      ;;
      --err)
        label="error"
        color=$COL_RED
        shift 1
        ;;
      --ok)
        label="ok"
        color=$COL_GREEN
        shift 1
        ;;
      --prg:*)
        ret=true
        label=${1//--prg:/}
        color=$COL_GREEN
        shift 1
        ;;
      --wrn)
        label="warn"
        color=$COL_YELLOW
        shift 1
        ;;
      --header)
        color=$COL_BLUE
        shift 1
        ;;
      --verbose)
        verbose=true
        shift 1
        ;;
      *)
        break
      ;;
    esac
  done 

  msg=$@

  if [ -z $color ]; then
    echo >&2 $@
    return
  fi

  if [ ! -z $label ]; then
    msg="[ $color$(printf "%5s" $label)$COL_RESET ] $msg"
    else
    msg="$color$msg$COL_RESET"
  fi

  if [[ -n "$detail" ]]; then
    msg="$msg ($COL_BLUE$detail$COL_RESET )"
  fi
  
  echo -en >&2 "$(printf '%*s\n' "${COLUMNS:-$(($cols))}" '')\r"
  if [[ $ret = true ]] && [[ -z $verbose ]]; then
    echo -en >&2 "$(echo $msg | sed ':a;N;$!ba;s/\n\r/ /g')\r"
  else
    echo -e >&2 "$msg"
  fi
}

log_wrap() {
  local exit_code=0
  local logfile=$(log_filename)
  local tracedest
  local pname
  local cmd
  
  set -- $@
  while [[ $# -gt 0 ]]; do
    case $1 in
      -l|--logfile)
        logfile="$(log_filename $2)"
        shift 2
      ;;
      -V|--verbose)
        verbose=1
        shift 1
      ;;
      -n|--process-name)
        pname="$(echo ${@:1} | cut -d - -f 1)"
        shift 1
      ;;
      --)
        cmd="${@:2}"
        break
      ;;
      *)
        shift 1
      ;;
    esac
  done 
  
  if [ ! -z $verbose ]; then 
    echo -ne >&2 "\nExecuting: $@\n\n" 
    $cmd 2>&1 | tee -a $logfile
    exit_code=${PIPESTATUS[0]}
  else
    ($cmd 2>&1 | tee -a $logfile; exit ${PIPESTATUS[0]}) &> /dev/null
    exit_code=${PIPESTATUS[0]}
  fi

  if [ $exit_code -eq 0 ]; then 
    log $lflags --ok "$pname done!"
  else
    log $lflags --err "$pname failed!"
    log_errexit $lflags <(tail -n30 $logfile)
  fi
  return $exit_code
}

log_filename() {
  if [ -z $1 ]; then
    echo "/dev/null"
  fi
  if [ ! -f $1 ]; then 
    echo > $1
  fi
  echo "$1"
}

log_wrap_wget_progress () {
  local flag=false c count cr=$'\r' nl=$'\n'
  while IFS='' read -d '' -rn 1 c; do
    if $flag; then
      printf '%s' "$c"
    else
      if [[ $c != $cr && $c != $nl ]]; then
        count=0
      else
        ((count++))
        if ((count > 1)); then
          flag=true
        fi
      fi
    fi
  done
}

log_errexit() {
  local logfile
  local info
  local infoheader
  local cols=$(tput cols) 

  set -- $@

  while [[ $# -gt 0 ]]; do
    case $1 in
      -l|--logfile)
        logfile="$2"
        shift 2
      ;;
      --verbose)
        shift 1
      ;;
      *)
        break
      ;;
    esac
  done

  info="$(cat $1)"

  if [[ ! -z $info ]]; then
    infoheader="This may help you"
    info=$(echo -ne "\n$infoheader\n$(printf "%${cols}s"|tr ' ' '-')\n$info\n$(printf "%${cols}s"|tr ' ' '-')")
  fi
  cat <<-EOF

Something wrong happend!!
$info

  Check the logfile to see more information:

  cat $logfile
EOF
  exit 1
}

# log_wrap_progress() {
#   local RET
#   local total=1
#   local msg="$1"
#   while IFS= read -d $'\n' -r line || { RET=$line && break; }; do
#     total=$((total+1))
#     echo $line
#     log "prg:$total" "$msg - ${line##* }"
#   done < <(${@:2} 2>&1; printf $?)
  
#   if [ "$RET" == 0 ]; then 
#     log --info"$msg"
#   else
#     log --err "$msg"
#   fi
#   return $RET
# }

