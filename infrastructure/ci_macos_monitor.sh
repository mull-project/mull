#!/usr/bin/env bash

# Logs system state and samples long-running sandboxed processes, runs outside of Bazel

out_dir="${1:-ci-monitor}"
interval=10
threshold=40

mkdir -p "$out_dir"
log="$out_dir/monitor.log"
sampled=" "

seconds() {
  # ps etime: [[dd-]hh:]mm:ss
  echo "$1" | awk -F'[-:]' '{ s = 0; for (i = 1; i <= NF; i++) s = s * (NF == 4 && i == 2 ? 24 : 60) + $i; print s }'
}

while true; do
  {
    echo "=== $(date -u +%H:%M:%S) $(uptime | sed 's/.*load/load/')"
    sysctl -n vm.swapusage
    memory_pressure -Q 2>/dev/null | tail -1
  } >>"$log"

  ps -axo pid=,ppid=,stat=,etime=,wchan=,rss=,command= | grep "/sandbox/darwin-sandbox/" | grep -v grep |
    while read -r pid ppid stat etime wchan rss command; do
      if [ "$(seconds "$etime")" -ge "$threshold" ]; then
        echo "long-running: pid=$pid ppid=$ppid stat=$stat etime=$etime wchan=$wchan rss=$rss ${command:0:300}" >>"$log"
        echo "$pid"
      fi
    done >"$out_dir/.pids"

  for pid in $(cat "$out_dir/.pids"); do
    case "$sampled" in *" $pid "*) continue ;; esac
    sampled="$sampled$pid "
    sample "$pid" 1 -file "$out_dir/sample-$pid.txt" >/dev/null 2>&1 &
  done

  sleep "$interval"
done
