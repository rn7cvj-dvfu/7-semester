#!/bin/bash

get_processes() {
    ps -eo pid,comm --no-headers | sort -n
}



while true; do
    sleep 0.1
    

    new_processes=$(get_processes) | grep "counter"
    
    if [ -n "$new_processes" ]; then
        echo "$new_processes" | while read -r line; do
            pid=$(echo "$line" | awk '{print $1}')
            name=$(echo "$line" | awk '{print $2}')
            echo -e "\033[32mNEW: PID=$pid Name=$name\033[0m"
        done
    fi
    

done
