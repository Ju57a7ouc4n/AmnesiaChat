#!/bin/bash

if [ "$EUID" -ne 0 ]; then
  exit 1
fi

groupadd -f amnesia

id -u amnesiauser &>/dev/null || useradd -r -g amnesia -s /bin/false amnesiauser

cat <<EOF > /etc/security/limits.d/99-amnesia.conf
@amnesia    hard    memlock    65536
@amnesia    soft    memlock    65536
$SUDO_USER    hard    memlock    65536
$SUDO_USER    soft    memlock    65536
EOF

mkdir -p /etc/amnesia /var/log/amnesia

chown -R root:amnesia /etc/amnesia /var/log/amnesia

chmod -R 770 /var/log/amnesia

usermod -aG amnesia $SUDO_USER

if [ -f "../amnesia_engine/amnesia_engine" ]; then
    setcap 'cap_net_bind_service,cap_ipc_lock+ep' ../amnesia_engine/amnesia_engine
fi
