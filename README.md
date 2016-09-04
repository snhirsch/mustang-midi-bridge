# mustang-midi-bridge

This application enables a small computer running Linux to translate
MIDI messages to the proprietary USB protocol used by Fender Mustang
amplifiers.

# Introduction

Mustang bridge implements about 99% of the published MIDI spec for the
Fender Mustang Floor pedal with extensions to support features added
to the 'v2' series.

Code is developed on a Ubuntu Precise desktop machine, but I routinely
test on a Raspberry Pi 'B' and Beagelbone Green to ensure these remain
viable deployment targets.

(Due to occasional issues with USB on the RPi I am recommending the
BBG for live performance use - YMMV)

Special thanks to:

  + The original developer and contributors to 'PLUG' who blazed the
  path with reverse-engineering of Fender's communication scheme.

  + Robert Fransson (aka Codesmart) of Primova Sound for feedback and
  encouragement and general programming wizardry). 

  + Robert Heitman of Triton Interactive, author of the Android
  'Remuda' application, who provided valuable insight into the darker
  corners of Mustang communication protocol.

## For the non-techies

I have written a Wiki page here:

https://github.com/snhirsch/mustang-midi-bridge/wiki/Install

that attempts a detailed walk-through of the installation and build
process on Raspberry Pi or Beaglebone.  It's hard to know what level
of detail to hit and suggestions or comments would be appreciated if
I've omitted or glossed over something critical.

# Status

The Mustang Floor MIDI spec is about 99% implemented, with only the
following exceptions:

  + Amp Bypass

    It's possible to turn the amp "off" with CC#68 0, but this mutes
    all sound rather than acting as a bypass.  It appears that the
    combo amps do not support the notion of straight-through routing
    without an amplifier modeling block.

  + Pedal Volume

    I'm not sure why this would even be needed, since all continuous
    controllers are directly accessible through CC messages.

  + FX Insert

    Not supported on hardware other than Mustang Floor

  + Tap Tempo

    I was not able to find the magic incantation to set tap tempo over
    USB (if, indeed, it's even possible). However, this function can
    be assigned to one of the Fender foot pedals if you need it at
    your feet.  Haven't totally given up on this and have an idea for
    "faking" it over MIDI.  Now I just need the time to code it...
  
# Prerequisites

  + For Ubuntu Precise or Debian Jessie the following packages must be
    present.  Install all but 'pyusb' with 'apt-get' (see below).

    - libasound2
    - librtmidi-dev
    - libusb-1.0-0-dev
    - libjack0 (Precise) 
    - libjackQ (Jessie)
    - at
    - python2.7
    - python2.7-dev
    - python-pip
    - pyusb
```
    NOTE: The python-pip install may segfault at the end, but it doesn't
          seem to affect anything.
```
    'pyusb' must be installed last using 'pip':

    $ pip install pyusb

  + If you want to run the regression tests, you'll also need:

    - 'Mido' Python MIDI extension
    - Python rtmidi extension

    'mido' and rtmidi are not available as a DEB package and must be
    installed using 'pip':

    $ pip install --pre python-rtmidi

    $ pip install mido

Would appreciate feedback on requirements for other distributions.

# Build
```
$ make
 or
$ make CPPFLAGS=-DRTMIDI_2_0 (for older librtmidi)
```

# Configure

  1. Update ```60-midi.rules``` with the USB VID (vendor id) and PID
(product id) of your controller.  Must be on both lines, although the
attribute names are different.

  2. Edit ```mustang_bridge_start``` to set values marked as user
edits.  In addition to setting the VID and PID, you need to specify
the index of the MIDI interface (see note below) and the MIDI channel
you want the bridge to listen on.

NOTE: I'm not sure about other platforms, but on Linux the MIDI
port number is equivalent to the ALSA card index.  I had originally
treated port as 1..n, but since ALSA (and JACK? Not sure..) starts at
0, this has now been changed.  You can find the card index for your
controller by connecting it to the computer and examining the
pseudo-file, e.g.:

```
$ cat /proc/asound/cards
 0 [PCH            ]: HDA-Intel - HDA Intel PCH
                      HDA Intel PCH at 0xf7530000 irq 30
 1 [Interface      ]: USB-Audio - USB MS1x1 MIDI Interface
                      M-Audio USB MS1x1 MIDI Interface at usb-0000:00:14.0-1, full speed
```

To accept MIDI messages from devices behind the M-Audio interface you
would now specify '1' as the MIDI port value.

# Install

Run the ```install.sh``` script as root

# Run

If you have configured everything correctly, the bridge should start
automatically when both the controller and the Mustang amp are
connected via USB.  If either or both are disconnected or shut off,
the bridge will be killed automatically.

# In case of difficulty

RPi and BBG are a bit fussy about enumeration of new USB devices. If
you are not getting proper communication, quit the program and try
replugging both the Fender amp and MIDI controller **after** those
devices are powered up.

I've had success using a passive USB hub with the single USB on the
BBG, but YMMV since most USB<->5Pin MIDI converters draw some degree
of bus power.  A powered hub might be necessary in some situations.

# Recent Changes

Complete rewrite and restructure of code based on ever-increasing
familiarity with the Mustang communication protocol and API.  The
bridge code is now fully mulithreaded.  A persistent background thread
is constantly listening to incoming traffic from the amp and updating
shared data as necessary.  This greatly increases stability and makes
the bridge tolerant of manual adjustments on the amplifier while
nominally under MIDI control.  In particular, it is now possible to
engage the tuner function on the amp without ill side-effects.

The command line parameter for MIDI controller port is now assumed to
start at 0 rather than 1 in order to match the way Linux ALSA
enumerates devices.

I have added a runtime framework that starts and stops the program
automatically based on attached MIDI devices. There is a small amount
of customization required to account for your specific amp model and
MIDI controller interface. 

Support added for amp and effects models specific to the Mustang v2
products.

Added a python script to drive regression testing. Tests require LCD
display for feedback and are not going to be of much use unless you
have a Mustang III, IV or V model.
