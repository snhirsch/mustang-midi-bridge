# mustang-midi-bridge

Allow Fender Mustang series guitar amplifiers to be controlled by MIDI
messages

# Introduction

The intent is to implement the published MIDI spec for the Fender
Mustang Floor pedal with whatever extensions are necessary to expose
features added to the 'v2' series. Currently only patch change and
effects on/off are implemented.

Intended to build on Linux systems with the idea of deploying on
RaspberryPi.

A special thanks to the original developer and contributors to 'PLUG',
from whence the USB interface code is stolen.

# Prerequisites

I'm working on Ubuntu Precise, YMMV for other distributions.

libjack-dev
librtmidi-dev (2.0.1 - May need changes for newer versions)
libusb-1.0

# Build

$ make opt

or 

$ make debug

as appropriate

# Run

Both the amplifier and MIDI source should be connected first, then:

$ mustang_midi _midi_port#_ _midi_listen_channel#_


