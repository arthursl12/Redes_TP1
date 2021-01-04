#!/bin/bash
set -eu

# N=$1
rm -rf client.log
for N in $(seq 1 1); do 
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

# N=2
# for N in $(seq 2 2); do
#     # Leave server window open during debug (close with C-b :kill-pane)
#     # tmux set-window-option remain-on-exit on

#     K=1
#     tmux split-pane -v -c $(pwd)/../ ./servidor 51511
#     ../client.py --script test$N-script$K.txt
#     while [[ -e test$N-output$K.txt ]] ; do
#         if ! diff test$N-output$K.txt output$K.txt > /dev/null ; then
#             echo "test$N failed"
#             exit 1
#         fi
#         echo "output$K"
#         echo "test$N-output$K"
#     done
#     echo "test$N passed"
#     rm -rf ../client.log output*.txt ../*.swp 
# done



