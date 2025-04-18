#!/usr/bin/env bash
if [[ "$(id -u)" != "0" ]]; then
    echo "This setup script must be executed as root!"
    exit 1
fi


# Make sure OS is up-to-date and install required packages
apt update && apt upgrade -y

TZ=Etc/UTC apt install -y \
    moreutils \
    fake-hwclock \
    ntp \
    ntpsec \
    gpsd \
    gpsd-clients \
    libgoogle-glog-dev \
    libsndfile1-dev \
    libgps-dev \
    python3-yaml \
    tcpdump \
    screen \
    netcat-openbsd

# apt install -y \
#     libgdal-dev

# Mask the ModemManager service
systemctl mask ModemManager.service

# Other systemctl tweaks
systemctl mask bluetooth.service
systemctl mask bluetooth.target

systemctl mask alsa-utils.service
systemctl mask cups-browsed.service
systemctl mask cups.service
systemctl mask bluetooth.service

systemctl disable NetworkManager-wait-online.service


# Configure ntpsec to *ignore DHCP*; always use the configured servers & pools
sed -i 's/IGNORE_DHCP=""/IGNORE_DHCP="yes"/g' /etc/default/ntpsec


# Configure ntpsec to use time from GPS (via gpsd) when available
cat << EOF | tee -a /etc/ntpsec/ntp.conf

# GPS Serial data reference (NTP0)
server 127.127.28.0 minpoll 4 maxpoll 4
fudge 127.127.28.0 refid GPS

# GPS PPS reference (NTP1)
server 127.127.28.1 minpoll 4 maxpoll 4 prefer
fudge 127.127.28.1 refid PPS

EOF

# Create a convenience command that queries gpsd
# mkdir -p ${SUDO_HOME:-$HOME}/.local/bin
# cat << EOF | tee ${SUDO_HOME:-$HOME}/.local/bin/gpscat
# #!/usr/bin/env bash
# #echo '?WATCH={"enable":true,"nmea":true}' | nc localhost 2947
# echo '?WATCH={"enable":true,"nmea":true}' | nc localhost 2947 | ts "[%Y-%m-%d %T]"
# EOF

# Ensure the bindings are available in PYTHONPATH
# cat << EOF | tee -a ${SUDO_HOME:-$HOME}/.bashrc
# export PATH+=:~/.local/bin
# export PYTHONPATH+=:~/acbotics_interface_cpp/python/
# EOF

# Configure ethernet to manual, set expected static IP
# nmcli connection modify Wired\ connection\ 1 ipv4.addresses 192.168.1.115/24

# Install the Acbotics Beamformer
./acbotics_beamformer_installer_arm64.run

# Update initramfs to use custom splash screen
update-initramfs -u

# rm -f $0
mv $0 $0.bak

reboot
