#!/usr/bin/python

# Use aconnectgui to wire Midi Through Port-0 out to
# mustang bridge virtual input port name

# OR

# hirsch@z87:~$ aconnect -o
# client 14: 'Midi Through' [type=kernel]
#    0 'Midi Through Port-0'
# client 128: 'RtMidi Input Client' [type=user]
#    0 'TestPort        '
#
# Given the above, open RtMidi as: 'RtMidi Input Client 128:0'
#
# outport = mido.open_output('Midi Through 14:0')

from time import sleep
import sys
import itertools as it
import mido
mido.set_backend('mido.backends.rtmidi')

pc = mido.Message('program_change')
cc = mido.Message('control_change')

def analog_send( outport, sleeptime=0.25 ):
    for value in [ 0, 32, 64, 96, 127, 96, 64, 32, 0 ]:
        cc.value = value
        outport.send( cc )
        sleep( sleeptime )

def discrete_send( outport, max ):
    for value in it.chain( range(0,max+1), range(max,-1,-1) ):
        cc.value = value
        outport.send( cc )
        sleep( 0.25 )

# Screen 1 is the same for all models
def amp_screen1( outport ):
    for i in range( 69, 74 ):
        cc.control = i
        analog_send( outport )

# Some variation in screen 2 across models
def amp_screen2( outport, bias_sag, extra ):
    if bias_sag:
        cc.control = 74
        discrete_send( outport, 2 )
        cc.control = 75
        analog_send( outport )

    cc.control = 76
    discrete_send( outport, 4 )

    cc.control = 77
    discrete_send( outport, 12 )

    if extra:
        cc.control = 78
        analog_send( outport )
        cc.control = 79
        analog_send( outport )

# Step through all amp models
def amp_select( outport ):
    cc.control = 68
    for i in range( 0, 13 ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_amp_test( struct, outport ):
    raw_input( "Hit ENTER to run amp model select test...\n" )
    amp_select( outport )

    for i in range( 0, len(struct) ):
        amp_rec = struct[i]

        raw_input( "Hit ENTER to run parm edit check for %s\n" % amp_rec[0] )
        cc.control = 68
        cc.value = amp_rec[1]
        outport.send( cc )

        raw_input( "Enter amp edit mode on Mustang and hit ENTER to proceed...\n" )
        amp_screen1( outport )

        raw_input( "Step to amp edit screen 2 and hit ENTER...\n" )
        amp_screen2( outport, amp_rec[2], amp_rec[3] )

# Step through all reverb models
def reverb_select( outport ):
    cc.control = 58
    for i in range( 0, 11 ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_reverb_test( outport ):
    raw_input( "Hit ENTER to run reverb model select test...\n" )
    reverb_select( outport )

    raw_input( "Enter Reverb edit mode and hit ENTER...\n" )
    for i in range ( 59, 64 ):
        cc.control = i
        analog_send( outport )
        sleep( 0.25 )

# Step through all delay models
def delay_select( outport ):
    cc.control = 48
    for i in range( 0, 10 ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_delay_test( struct, outport ):
    raw_input( "Hit ENTER to run delay model select test...\n" )
    delay_select( outport )

    for i in range( 0, len(struct) ):
        delay_rec = struct[i]

        raw_input( "Hit ENTER to run parm edit check for %s\n" % delay_rec[0] )
        cc.control = 48
        cc.value = delay_rec[1]
        outport.send( cc )

        raw_input( "Enter delay edit mode on Mustang and hit ENTER to proceed...\n" )
        for i in range( 49, 54 ):
            cc.control = i
            # Multitap Delay has a single discrete control
            if i==53 and delay_rec[2]:
                discrete_send( outport, 3 )
            else:
                analog_send( outport )

        if delay_rec[3]:
            analog_send( outport )

# Step through all mod models
def mod_select( outport ):
    cc.control = 38
    for i in range( 0, 12 ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_mod_test( struct, outport ):
    raw_input( "Hit ENTER to run mod model select test...\n" )
    mod_select( outport )

    for i in range( 0, len(struct) ):
        mod_rec = struct[i]

        raw_input( "Hit ENTER to run parm edit check for %s\n" % mod_rec[0] )
        cc.control = 38
        cc.value = mod_rec[1]
        outport.send( cc )

        raw_input( "Enter mod edit mode on Mustang and hit ENTER to proceed...\n" )
        for i in range( 39, 44 ):
            cc.control = i
            if (i==42 and mod_rec[2]) or (i==43 and mod_rec[3]):
                discrete_send( outport, 1 )
            else:
                analog_send( outport )

# Step through all stomp models
def stomp_select( outport ):
    cc.control = 28
    for i in range( 0, 8 ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_stomp_test( struct, outport ):
    raw_input( "Hit ENTER to run stomp model select test...\n" )
    stomp_select( outport )

    for i in range( 0, len(struct) ):
        stomp_rec = struct[i]

        raw_input( "Hit ENTER to run parm edit check for %s\n" % stomp_rec[0] )
        cc.control = 28
        cc.value = stomp_rec[1]
        outport.send( cc )

        raw_input( "Enter stomp edit mode on Mustang and hit ENTER to proceed...\n" )
        for i in range( 29, 34 ):
            cc.control = i
            if i==33 and stomp_rec[2]:
                discrete_send( outport, 1 )
            elif i==29 and stomp_rec[3]:
                discrete_send( outport, 3 )
                break
            else:
                analog_send( outport )

# Step through program changes
def program_change_test( outport ):
    raw_input( "Hit ENTER to run program change test...\n" )
    for i in ( 0, 25, 75, 99, 60, 40, 20, 0 ):
        pc.program = i
        outport.send( pc )
        sleep( 0.5 )

args = sys.argv

if not len(args) == 3:
    print "Usage: test.py <virtual_port_name> <midi_channel>\n"
    sys.exit( 1 )

try:
    pc.channel = cc.channel = int( args[2] ) - 1
except ValueError:
    print "Arg2 must be numeric!\n"
    sys.exit( 1 )

outport = mido.open_output( args[1] )

program_change_test( outport )

#               Model         #  33 Dig?
stomp_test = (
               ( "Overdrive",    1, False, False ),
               ( "Wah",          2, True,  False ),
               ( "Simple Comp",  6, False, True ),
               ( "Comp",         7, False, False )
             )

run_stomp_test( stomp_test, outport )

#               Model         #  42 Dig?  43 Dig?
mod_test = (
             ( "Sine Chorus",  1, False, False ),
             ( "Vibratone",    5, False, False ),
             ( "Vintage Trem", 6, False, False ),
             ( "Ring Mod",     8, True,  False ),
             ( "Phaser",      10, False, True  ),
             ( "Pitch Shift", 11, False, False )
           )

run_mod_test( mod_test, outport )

run_reverb_test( outport )

#                 Model        #  53 Dig?, Has 54?
delay_test = (
               ( "Mono Delay", 1, False, False ),
               ( "Multitap",   4, True,  False ),
               ( "Tape Delay", 8, False, False )
             )

run_delay_test( delay_test, outport )

#               Model,           #, Bias/Sag?, Extra? 
amp_test = ( 
             ( "Fender 65 Twin",     6, True, False ), 
             ( "Fender SuperSonic",  7, True, True ),
             ( "British 60s",        8, True, True ),
             ( "British 70s",        9, True, True ),
             ( "British 80s",       10, True, True )
           )

run_amp_test( amp_test, outport )

