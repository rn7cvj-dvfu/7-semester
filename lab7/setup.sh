#!/bin/bash

# === Настройки ===
KIOSK_USER="kiosk"
KIOSK_HOME="/home/$KIOSK_USER"
APP_PATH="/home/vboxuser/Projects/7-semester/lab6/src/build/temperature_monitor"

# === 1. Создание пользователя ===
sudo adduser --disabled-password --gecos "" $KIOSK_USER

# === 2. Настройка автологина (LightDM) ===
sudo apt-get update
sudo apt-get install -y lightdm xbindkeys
# Отключить GDM (если установлен) и выбрать LightDM
sudo systemctl disable gdm3 || true
sudo systemctl stop gdm3 || true
sudo dpkg-reconfigure -f noninteractive lightdm
sudo systemctl enable lightdm
sudo systemctl start lightdm
sudo bash -c "cat > /etc/lightdm/lightdm.conf" <<EOF
[Seat:*]
autologin-user=$KIOSK_USER
autologin-user-timeout=0
user-session=xsession
EOF

# === 3. Автозапуск приложения в X-сессии ===
sudo bash -c "cat > $KIOSK_HOME/.xsession" <<EOF
#!/bin/bash
xset -dpms      # Отключить энергосбережение
xset s off      # Отключить скринсейвер
xset s noblank  # Отключить затемнение экрана
# Запуск xbindkeys для обработки Ctrl+Alt+Q
if [ -f "$HOME/.xbindkeysrc" ]; then
	xbindkeys &
fi
$APP_PATH
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.xsession
sudo chmod +x $KIOSK_HOME/.xsession

# === 3.5. Настройка выхода по Ctrl+Alt+Q ===
sudo bash -c "cat > $KIOSK_HOME/.xbindkeysrc" <<EOF
# Завершить X-сессию по Ctrl+Alt+Q
\"pkill -KILL -u \$USER\"
  control+alt+q
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.xbindkeysrc

# === 4. Минимальная блокировка клавиш (Alt+Tab, Ctrl+Alt+Fx) ===
sudo bash -c "cat > $KIOSK_HOME/.Xmodmap" <<EOF
remove mod1 = Alt_L Alt_R
remove control = Control_L Control_R
keycode 67 = NoSymbol
keycode 68 = NoSymbol
keycode 69 = NoSymbol
keycode 70 = NoSymbol
keycode 71 = NoSymbol
keycode 72 = NoSymbol
keycode 73 = NoSymbol
keycode 74 = NoSymbol
keycode 75 = NoSymbol
keycode 76 = NoSymbol
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.Xmodmap

# Добавить загрузку Xmodmap в .xsession
sudo sed -i "2i xmodmap \$HOME/.Xmodmap" $KIOSK_HOME/.xsession

# === 5. Отключить доступ к терминалу (tty) ===
sudo sed -i 's/^NAutoVTs=.*/NAutoVTs=0/' /etc/systemd/logind.conf || echo "NAutoVTs=0" | sudo tee -a /etc/systemd/logind.conf
sudo sed -i 's/^ReserveVT=.*/ReserveVT=0/' /etc/systemd/logind.conf || echo "ReserveVT=0" | sudo tee -a /etc/systemd/logind.conf

# === 6. Отключить правую кнопку мыши (опционально) ===
sudo bash -c "cat > $KIOSK_HOME/.Xresources" <<EOF
xterm*VT100.translations: #override <Btn3Down>: ignore()
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.Xresources

# === 7. Готово ===
echo "Киоск-режим настроен. Перезагрузите систему для проверки."