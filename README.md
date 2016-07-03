# mustang-midi-bridge

Allow Fender Mustang series guitar amplifiers to be controlled by MIDI
messages

# Introduction

The intent is to implement the published MIDI spec for the Fender
Mustang Floor pedal with whatever extensions are necessary to expose
features added to the 'v2' series. 

I am developing on a Ubuntu Precise desktop machine, but the code is
routinely tested on a Raspberry Pi 'B' and Beagelbone Green to ensure
these remain viable deployment targets.  At this point I'm still
experiencing issues with USB latency on the RPi and am currently
recommending the BBG for real-world use.

A special thanks to the original developer and contributors to 'PLUG',
from whence the Mustang USB interface code is stolen.

## For the non-techies

I have written a Wiki page here:

https://github.com/snhirsch/mustang-midi-bridge/wiki/Install

that attempts to walk less technical users through the installation
and build process on Raspberry Pi or Beaglebone.  It's hard to know
what level of detail to hit and suggestions or comments would be
appreciated if I've omitted or glossed over something critical.

# Status

Currently, the following elements of the Mustang Floor MIDI spec are
implemented:

  + Patch change
  + EFX bypass (on/off)
  + Amp CC messages (except for on/off)
  + Reverb CC messages
  + Delay CC messages

I'm using WinPCAP and tshark to snoop communication and plan to
implement all features accessible from the Fender FUSE application.
However, some targets listed in the MIDI spec (e.g. amplifier on/off,
tuner mode) are not controllable from FUSE and it may take some time
and luck to figure out the protocol.

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

