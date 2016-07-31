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

# 1 == v1, 2 == v2
type = 0

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
def amp_screen2( outport, template ):
    limit = len(template)

    i = 74
    j = 5
    while j < limit:
        cc.control = i
        i += 1
        if template[j] == "A":
            analog_send( outport )
        elif template[j] == "D":
            j += 1
            count = int( template[j:j+2] )
            j += 1
            discrete_send( outport, count )
        elif template[j] == "-":
            pass
        j += 1

# Step through all amp models
def amp_select( outport ):
    if type == 2:
        max = 18
    else:
        max = 13

    cc.control = 68
    for i in range( 0, max ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_amp_test( struct, outport ):
    raw_input( "Hit ENTER to run amp model select test...\n" )
    amp_select( outport )

    for i in range( 0, len(struct) ):
        control_rec = struct[i]
        if control_rec[3] and type != 2:
             continue

        raw_input( "Hit ENTER to run parm edit check for %s\n" % control_rec[0] )
        cc.control = 68
        cc.value = control_rec[1]
        outport.send( cc )

        raw_input( "Enter amp edit mode on Mustang and hit ENTER to proceed...\n" )
        amp_screen1( outport )

        raw_input( "Step to amp edit screen 2 and hit ENTER...\n" )
        amp_screen2( outport, control_rec[2] )

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
        control_rec = struct[i]
        if control_rec[3] and type != 2:
             continue

        raw_input( "Hit ENTER to run parm edit check for %s\n" % control_rec[0] )
        cc.control = 48
        cc.value = control_rec[1]
        outport.send( cc )

        raw_input( "Enter delay edit mode on Mustang and hit ENTER to proceed...\n" )
        limit = len(control_rec[2])
        template = control_rec[2]

        i = 49
        j = 0
        while j < limit:
            cc.control = i
            i += 1
            if template[j] == "A":
                analog_send( outport )
            elif template[j] == "D":
                j += 1
                count = int( template[j:j+2] )
                j += 1
                discrete_send( outport, count )
            j += 1


# Step through all mod models
def mod_select( outport ):
    if type == 2:
        max = 15
    else:
        max = 12

    cc.control = 38
    for i in range( 0, max ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_mod_test( struct, outport ):
    raw_input( "Hit ENTER to run mod model select test...\n" )
    mod_select( outport )

    for i in range( 0, len(struct) ):
        control_rec = struct[i]
        if control_rec[3] and type != 2:
             continue

        raw_input( "Hit ENTER to run parm edit check for %s\n" % control_rec[0] )
        cc.control = 38
        cc.value = control_rec[1]
        outport.send( cc )

        raw_input( "Enter mod edit mode on Mustang and hit ENTER to proceed...\n" )
        limit = len(control_rec[2])
        template = control_rec[2]

        i = 39
        j = 0
        while j < limit:
            cc.control = i
            i += 1
            if template[j] == "A":
                analog_send( outport )
            elif template[j] == "D":
                j += 1
                count = int( template[j:j+2] )
                j += 1
                discrete_send( outport, count )
            j += 1

# Step through all stomp models
def stomp_select( outport ):
    if type == 2:
        max = 13
    else:
        max = 8

    cc.control = 28
    for i in range( 0, max ):
        cc.value = i
        outport.send( cc )
        sleep( 0.5 )

def run_stomp_test( struct, outport ):
    raw_input( "Hit ENTER to run stomp model select test...\n" )
    stomp_select( outport )

    for i in range( 0, len(struct) ):
        control_rec = struct[i]
        if control_rec[3] and type != 2:
             continue

        raw_input( "Hit ENTER to run parm edit check for %s\n" % control_rec[0] )
        cc.control = 28
        cc.value = control_rec[1]
        outport.send( cc )

        raw_input( "Enter stomp edit mode on Mustang and hit ENTER to proceed...\n" )
        limit = len(control_rec[2])
        template = control_rec[2]

        i = 29
        j = 0
        while j < limit:
            cc.control = i
            i += 1
            if template[j] == "A":
                analog_send( outport )
            elif template[j] == "D":
                j += 1
                count = int( template[j:j+2] )
                j += 1
                discrete_send( outport, count )
            j += 1

# Step through program changes
def program_change_test( outport ):
    raw_input( "Hit ENTER to run program change test...\n" )
    for i in ( 0, 25, 75, 99, 60, 40, 20, 0 ):
        pc.program = i
        outport.send( pc )
        sleep( 0.5 )

args = sys.argv

if len(args) < 4:
    print "Usage: test.py <virtual_port_name> <midi_channel> <v1|v2> [test_name]\n"
    print "       Pass test name in {pc, stomp, mod, reverb, delay, amp} for single test\n"
    print "       Default is to run all of them if no arg 4 passed\n"
    sys.exit( 1 )

try:
    pc.channel = cc.channel = int( args[2] ) - 1
except ValueError:
    print "Arg2 must be numeric!\n"
    sys.exit( 1 )

if args[3] == "v1":
    type = 1
elif args[3] == "v2":
    type = 2
else:
    print "Arg 3 must be 'v1' or 'v2'"
    sys.exit( 1 )

single = "all"
if len(args) == 5:
    single = args[4]

outport = mido.open_output( args[1] )

if single == "all" or single == "pc":
    program_change_test( outport )

if single == "all" or single == "stomp":
    #       Model         #  Ctrl      v2only
    stomp_test = (
        ( "Ranger Boost", 8, "AAAA",   True ),
        ( "Green Box",    9, "AAAA",   True ),
        ( "Orange Box",  10, "AAA",    True ),
        ( "Black Box",   11, "AAA",    True ),
        ( "Big Fuzz",    12, "AAA",    True ),

        ( "Overdrive",    1, "AAAAA",  False ),
        ( "Wah",          2, "AAAAD01", False ),
        ( "Simple Comp",  6, "D03",     False ),
        ( "Comp",         7, "AAAAA",  False )
    )
    run_stomp_test( stomp_test, outport )

if single == "all" or single == "mod":
    #       Model         #  Ctrl      v2only
    mod_test = (
        ( "Wah",         12, "AAAAD02", True ),
        ( "Touch Wah",   13, "AAAAD02", True ),
        ( "Dia Shift",   14, "AD22D12D09A", True ),

        ( "Sine Chorus",  1, "AAAAA",  False ),
        ( "Vibratone",    5, "AAAAA",  False ),
        ( "Vintage Trem", 6, "AAAAA",  False ),
        ( "Ring Mod",     8, "AAAD01A", False ),
        ( "Phaser",      10, "AAAAD01", False ),
        ( "Pitch Shift", 11, "AAAAA",  False )
    )
    run_mod_test( mod_test, outport )

if single == "all" or single == "reverb":
    run_reverb_test( outport )

if single == "all" or single == "delay":
    #      Model        #  Ctrl     v2only
    delay_test = (
        ( "Mono Delay", 1, "AAAAA",  False ),
        ( "Multitap",   4, "AAAAD03", False ),
        ( "Tape Delay", 8, "AAAAA",  False )
    )
    run_delay_test( delay_test, outport )

if single == "all" or single == "amp":
    #      Model,               #, Bias/Sag? Has 78+79?
    amp_test = ( 
        ( "Studio Preamp",     13, "AAAAA--D04D12",     True  ),

        ( "Fender 65 Twin",     6, "AAAAAD02AD04D12",   False ), 
        ( "Fender SuperSonic",  7, "AAAAAD02AD04D12AA", False ),
        ( "British 60s",        8, "AAAAAD02AD04D12AA", False ),
        ( "British 70s",        9, "AAAAAD02AD04D12AA", False ),
        ( "British 80s",       10, "AAAAAD02AD04D12AA", False )
    )
    run_amp_test( amp_test, outport )

