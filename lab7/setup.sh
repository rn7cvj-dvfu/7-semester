#!/bin/bash

KIOSK_USER="kiosk"
KIOSK_HOME="/home/$KIOSK_USER"
APP_PATH="/home/vboxuser/Projects/7-semester/lab7/start_apps.sh"

sudo chmod +x $APP_PATH

sudo adduser --disabled-password --gecos "" $KIOSK_USER

sudo apt-get update
sudo apt-get install -y lightdm xbindkeys

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

sudo bash -c "cat > /usr/share/xsessions/xsession.desktop" <<EOF
[Desktop Entry]
Name=XSession
Comment=Custom X Session
Exec=/bin/bash ~/.xsession
Type=Application
EOF

# === 3. Автозапуск приложения в X-сессии ===
sudo bash -c "cat > $KIOSK_HOME/.xsession" <<EOF
#!/bin/bash
echo "Starting kiosk session at \$(date)" >> /tmp/kiosk.log
xset -dpms      # Отключить энергосбережение
xset s off      # Отключить скринсейвер
xset s noblank  # Отключить затемнение экрана
xmodmap \$HOME/.Xmodmap
echo "Running start_apps.sh" >> /tmp/kiosk.log
$APP_PATH >> /tmp/kiosk.log 2>&1
echo "start_apps.sh finished" >> /tmp/kiosk.log
exec bash  # Держать сессию открытой
EOF

# === 5. Отключить доступ к терминалу (tty) ===
sudo sed -i 's/^NAutoVTs=.*/NAutoVTs=0/' /etc/systemd/logind.conf || echo "NAutoVTs=0" | sudo tee -a /etc/systemd/logind.conf
sudo sed -i 's/^ReserveVT=.*/ReserveVT=0/' /etc/systemd/logind.conf || echo "ReserveVT=0" | sudo tee -a /etc/systemd/logind.conf

# === 6. Отключить правую кнопку мыши ===
sudo bash -c "cat > $KIOSK_HOME/.Xresources" <<EOF
xterm*VT100.translations: #override <Btn3Down>: ignore()
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.Xresources

echo "Киоск-режим настроен. Перезагрузите систему для проверки."