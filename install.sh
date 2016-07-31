#!/bin/bash

BINDIR=/usr/local/bin
INITDIR=/etc/init.d
UDEVDIR=/etc/udev/rules.d

if [ ! -f "mustang_midi" ]; then
    echo "Must build mustang_midi first!"
    exit 1
fi

if ! `grep -q mustang-user /etc/passwd`; then
    echo "Create non-privileged user for MIDI bridge"
    useradd -M -s /bin/false -G plugdev,audio mustang-user
fi

echo "Copy program and support scripts to $BINDIR"

cp -f mustang_bridge_start $BINDIR
chmod 0755 $BINDIR/mustang_bridge_start
chown root:root $BINDIR/mustang_bridge_start

cp -f mustang_bridge_stop $BINDIR
chmod 0755 $BINDIR/mustang_bridge_stop
chown root:root $BINDIR/mustang_bridge_stop

cp -f mustang_midi $BINDIR
chmod 0755 $BINDIR/mustang_midi
chown root:root $BINDIR/mustang_midi

echo "Copy init script to $INITDIR and register"

cp -f mustang_bridge $INITDIR
chmod 0755 $INITDIR/mustang_bridge
chown root:root $INITDIR/mustang_bridge

update-rc.d mustang_bridge defaults

echo "Copy udev rules to $UDEVDIR and refresh system"

cp -f 50-mustang.rules $UDEVDIR
chmod 0644 $UDEVDIR/50-mustang.rules
chown root:root $UDEVDIR/50-mustang.rules

cp -f 60-midi.rules $UDEVDIR
chmod 0644 $UDEVDIR/60-midi.rules
chown root:root $UDEVDIR/60-midi.rules

udevadm control --reload

echo "Done!"
