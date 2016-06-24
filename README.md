# mustang-midi-bridge

Allow Fender Mustang series guitar amplifiers to be controlled by MIDI
messages

# Introduction

The intent is to implement the published MIDI spec for the Fender
Mustang Floor pedal with whatever extensions are necessary to expose
features added to the 'v2' series. Currently only patch change and
effects on/off are implemented.

I am developing on a Ubuntu Precise desktop machine, but the code is
routinely tested on a Raspberry Pi 'B' and Beagelbone Green to ensure
these remain viable deployment targets.  At this point I'm still
experiencing issues with USB latency on the RPi and am currently
recommending the BBG for real-world use.

A special thanks to the original developer and contributors to 'PLUG',
from whence the Mustang USB interface code is stolen.

# Prerequisites

  + For Ubuntu Precise or Debian Jessie:

    - libasound2
    - librtmidi-dev
    - libusb-1.0-0-dev
    - libjack0 (Precise) 
    - libjackQ (Jessie)

Would appreciate feedback on requirements for other distributions.

# OS Configuration

  + Add the id of the user who will be running the bridge to the
  'audio' and 'plugdev' groups.  That user should then log out and back
  in to make the groups effective.

  + As root, copy the file '50-mustang.rules' to /etc/udev/rules.d and
  refresh the system with 'udevadm control --reload'.

There may be slight differences in requirements for other distributions.

# Build
```
$ make opt
```
or 
```
$ make debug
```
as appropriate

# Run

Both the amplifier and MIDI source should be connected first, then:
```
$ mustang_midi  midi_port#  midi_listen_channel#
```
NOTE: RPi and BBG are a bit fussy about enumeration of new USB
devices. If you are not getting proper communication, quit the program
and try replugging both the Fender amp and MIDI controller **after**
those devices are powered up.

NOTE2: I've had success using a passive USB hub with the single USB on
the BBG, but YMMV since most USB<->5Pin MIDI converters draw some
degree of bus power.  A powered hub might be necessary in some
situations.

