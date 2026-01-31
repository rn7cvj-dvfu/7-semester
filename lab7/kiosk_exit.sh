#!/bin/bash

# Скрипт для завершения киоска по Ctrl+Alt+Q

# Используем xbindkeys для перехвата сочетания клавиш
# Убедитесь, что xbindkeys установлен: sudo apt-get install -y xbindkeys

# Создать конфиг xbindkeys
cat > $HOME/.xbindkeysrc <<EOF
# Завершить X-сессию по Ctrl+Alt+Q
"pkill -KILL -u $USER"
  control+alt+q
EOF

# Запустить xbindkeys
xbindkeys
