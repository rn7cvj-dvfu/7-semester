#!/bin/bash

KIOSK_USER="kiosk"
KIOSK_HOME="/home/$KIOSK_USER"
APP_PATH="/home/vboxuser/Projects/7-semester/lab6/src/build/temperature_monitor"

sudo adduser --disabled-password --gecos "" $KIOSK_USER

sudo apt-get update
sudo apt-get install -y lightdm
sudo bash -c "cat > /etc/lightdm/lightdm.conf" <<EOF
[Seat:*]
autologin-user=$KIOSK_USER
autologin-user-timeout=0
user-session=ubuntu
EOF

sudo bash -c "cat > $KIOSK_HOME/.xsession" <<EOF
#!/bin/bash
xset -dpms      
xset s off     
xset s noblank  
$APP_PATH
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.xsession
sudo chmod +x $KIOSK_HOME/.xsession

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

sudo sed -i "2i xmodmap \$HOME/.Xmodmap" $KIOSK_HOME/.xsession


sudo sed -i 's/^NAutoVTs=.*/NAutoVTs=0/' /etc/systemd/logind.conf || echo "NAutoVTs=0" | sudo tee -a /etc/systemd/logind.conf
sudo sed -i 's/^ReserveVT=.*/ReserveVT=0/' /etc/systemd/logind.conf || echo "ReserveVT=0" | sudo tee -a /etc/systemd/logind.conf


sudo bash -c "cat > $KIOSK_HOME/.Xresources" <<EOF
xterm*VT100.translations: #override <Btn3Down>: ignore()
EOF
sudo chown $KIOSK_USER:$KIOSK_USER $KIOSK_HOME/.Xresources

