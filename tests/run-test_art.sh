#!/bin/bash
set -eu

# N=$1

for N in $(seq 1 3); do 
    # Leave server window open during debug (close with C-b :kill-pane)
    # tmux set-window-option remain-on-exit on

    K=1
    
    tmux split-pane -v -c $(pwd)/../ ./servidor 51511
    ../client.py --script test$N-script$K.txt
    if ! diff test$N-output$K.txt output$N.txt > /dev/null ; then
        echo "test$N failed"
        exit 1
    fi
    echo "test$N passed"
    rm -rf ../client.log output*.txt ../*.swp
done


