#!/bin/bash
# Ajudante para falar com o console do ITS dentro do tmux.
# its.sh start | send "texto" | key ESC | cap [n] | stop
S=its
case "$1" in
  start) tmux kill-session -t $S 2>/dev/null; pkill -x pdp10-ka; sleep 1; cp /home/claude/work/its_tape/run.ini /tmp/its_run.ini
         tmux new-session -d -s $S -x 200 -y 60 "cd /home/claude/ref/its_build && tools/sims/BIN/pdp10-ka /tmp/its_run.ini 2>&1 | tee -a /home/claude/work/its_tape/console.log" ;;
  send)  s="$2"; for ((i=0; i<${#s}; i++)); do c="${s:$i:1}"; if [ "$c" = " " ]; then tmux send-keys -t $S Space; elif [ "$c" = ";" ]; then tmux send-keys -t $S -l -- '\;'; else tmux send-keys -t $S -l -- "$c"; fi; sleep ${DELAY:-0.04}; done ;;
  cr)    tmux send-keys -t $S C-m ;;
  esc)   tmux send-keys -t $S Escape ;;
  ctrl)  tmux send-keys -t $S C-$2 ;;
  cap)   tmux capture-pane -t $S -p -S -${2:-40} ;;
  stop)  tmux kill-session -t $S 2>/dev/null; pkill -x pdp10-ka ;;  # o emulador sobrevive ao tmux: matar sempre
esac
