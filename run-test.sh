#!/bin/bash
set -eu

# N=$1

for N in $(seq 1 1); do 
    # Leave server window open during debug (close with C-b :kill-pane)
    # tmux set-window-option remain-on-exit on
    for K in $(seq 1 2); do 
        tmux split-pane -v -c $(pwd)/ ./servidor 51511
        ./client.py --script test$N-script$K.txt
        while [[ -e test$N-output$K.txt ]] ; do
            if ! diff test$N-output$K.txt output$K.txt > /dev/null ; then
                echo "test$N failed"
                exit 1
            fi
            echo "test$N.$K passed"
            K=$(( K+1 ))
        done
        echo "test$N.$K passed"
    done
    echo "test$N passed"
    rm -rf client.log output*.txt
done


