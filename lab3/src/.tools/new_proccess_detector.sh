#!/bin/bash

# Функция для получения списка процессов
get_processes() {
    ps -eo pid,comm --no-headers | sort -n
}

# Сохраняем текущий список процессов
previous=$(get_processes)

while true; do
    sleep 0.1
    
    # Получаем актуальный список процессов
    current=$(get_processes)
    
    # Находим новые процессы
    new_processes=$(comm -13 <(echo "$previous") <(echo "$current"))
    
    if [ -n "$new_processes" ]; then
        echo "$new_processes" | while read -r line; do
            pid=$(echo "$line" | awk '{print $1}')
            name=$(echo "$line" | awk '{print $2}')
            echo -e "\033[32mNEW: PID=$pid Name=$name\033[0m"
        done
    fi
    
    # Обновляем список
    previous="$current"
done
