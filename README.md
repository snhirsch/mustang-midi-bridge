# mustang-midi-bridge

Allow Fender Mustang series guitar amplifiers to be controlled by MIDI
messages

# What's New

The command line parameter for MIDI controller port is now assumed to
start at 0 rather than 1 in order to match the way Linux ALSA
enumerates devices (see 'Run' below).

The program has been updated to run as a non-privileged daemon
process. You can still invoke it on the command line, but there will
be no output to the console and it no longer responds to keypress
input. Enter Ctrl-C (SIGINT) to exit.

I have added the first version of a runtime framework that starts and
stops the program automatically based on attached MIDI devices. This
seems to be working on a Beaglebone SBC, but has not been extensively
tested or documented yet. There is a small amount of customization
required to account for your specific amp model and MIDI controller
interface. Technically oriented users can probably work this out,
otherwise wait a bit until I can refine the packaging and arrange to
have the various pieces configured from a common setup file.

I'm currently working on implementation of amp and effects models
added in the Mustang v2 products and hope to have that checked in
soon. 

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

The Mustang Floor MIDI spec is about 99% implemented, with only the
following exceptions:

  + Tuner On/Off

    Will require substantial rework of the program to make this work
    reliably.  

  + Amp Bypass

    It's possible to turn the amp "off" with CC#68 0, but this mutes
    all sound rather than acting as a bypass.  I don't think the combo
    amps support signal bypass, but haven't given up yet.

  + Pedal Volume

    I'm not sure why this is needed, since all continuous controllers
    are directly accessible through CC messages.  Since the FUSE
    application cannot issue these commands it will be guesswork to
    figure out the protocol - if it's even supported on the combo
    amps. 

  + FX Insert

    Not supported on the combo amps

  + Tap Tempo

    If I can figure out how to do this from FUSE I can code it.
  
I'm using USBPcap and tshark to snoop communication and plan to
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
NOTE1: I'm not sure about other platforms, but on Linux the MIDI
port number is equivalent to the ALSA card index.  I had originally
treated port as 1..n, but since ALSA (and JACK? Not sure..) starts at
0, this has now been changed.  You can find the card index for your
controller by connecting it to the computer and examining the
pseudo-file, e.g.:

$ cat /proc/asound/cards
 0 [PCH            ]: HDA-Intel - HDA Intel PCH
                      HDA Intel PCH at 0xf7530000 irq 30
 1 [Interface      ]: USB-Audio - USB MS1x1 MIDI Interface
                      M-Audio USB MS1x1 MIDI Interface at usb-0000:00:14.0-1, full speed

To accept MIDI messages from devices behind the M-Audio interface you
would now specify '1' as the MIDI port value.

NOTE2: RPi and BBG are a bit fussy about enumeration of new USB
devices. If you are not getting proper communication, quit the program
and try replugging both the Fender amp and MIDI controller **after**
those devices are powered up.

NOTE3: I've had success using a passive USB hub with the single USB on
the BBG, but YMMV since most USB<->5Pin MIDI converters draw some
degree of bus power.  A powered hub might be necessary in some
situations.

