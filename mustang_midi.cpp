#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <RtMidi.h>
#include <cerrno>

#include "mustang.h"

static Mustang mustang;

static int channel;


void message_action( double deltatime, std::vector< unsigned char > *message, void *userData ) {
#if 0
  unsigned int nBytes = message->size();
  if      ( nBytes == 2 ) fprintf( stdout, "%02x %d\n", (int)message->at(0), (int)message->at(1) );
  else if ( nBytes == 3 ) fprintf( stdout, "%02x %d %d\n", (int)message->at(0), (int)message->at(1), (int)message->at(2) );
#endif

  // Is this for us?
  int msg_channel = (*message)[0] & 0x0f;
  if ( msg_channel != channel ) return;

  int msg_type = (*message)[0] & 0xf0;

  switch ( msg_type ) {

  case 0xc0: {
    // Program change
    int bank = (int)(*message)[1];
    int rc = mustang.patchChange( bank );
    if ( rc ) {
      fprintf( stderr, "Error: PC#%d failed. RC = %d\n", bank, rc );
    }
  }
  break;
    
  case 0xb0: {
    // Control change
    int rc = 0;
    int cc = (*message)[1];
    int value = (*message)[2];
    
    // Tuner toggle
    if ( cc == 20 ) {
      rc = mustang.tunerMode( value );
    }
    // All EFX toggle
    else if ( cc == 22 ) {
      rc = mustang.effectToggle( 23, value );
      if ( rc == 0 ) {
        rc = mustang.effectToggle( 24, value );
        if ( rc == 0 ) {
          rc = mustang.effectToggle( 25, value );
          if ( rc == 0 ) {
            rc = mustang.effectToggle( 26, value );
          }
        }
      }
    }
    // Effects on/off
    else if ( cc >= 23 && cc <= 26 ) {
      rc = mustang.effectToggle( cc, value );
    }
    // Set stomp model
    else if ( cc == 28 ) {
      rc = mustang.setStomp( value );
    }
    // Stomp CC handler
    else if ( cc >= 29 && cc <= 33 ) {
      rc = mustang.stompControl( cc, value );
    }
    // Set mod model
    else if ( cc == 38 ) {
      rc = mustang.setMod( value );
    }
    // Mod CC handler
    else if ( cc >= 39 && cc <= 43 ) {
      rc = mustang.modControl( cc, value );
    }
    // Set delay model
    else if ( cc == 48 ) {
      rc = mustang.setDelay( value );
    }
    // Delay CC handler
    else if ( cc >= 49 && cc <= 54 ) {
      rc = mustang.delayControl( cc, value );
    }
    // Set reverb model
    else if ( cc == 58 ) {
      rc = mustang.setReverb( value );
    }
    // Reverb CC handler
    else if ( cc >= 59 && cc <= 63 ) {
      rc = mustang.reverbControl( cc, value );
    }
    // Set amp model
    else if ( cc == 68 ) {
      rc = mustang.setAmp( value );
    }
    // Amp CC Handler
    else if ( cc >= 69 && cc <= 79 ) {
      rc = mustang.ampControl( cc, value );
    }
    if ( rc ) {
      fprintf( stderr, "Error: CC#%d failed. RC = %d\n", cc, rc );
    }
  }
  break;

  default:
    break;
  }

}

// void errorcallback( RtError::Type type, const std::string & detail, void *userData ) {
//   std::cout << "Error: Code = " << type << ", Msg: " << detail << "\n";
// }

void usage() {
    fprintf( stderr, "Usage: mustang_midi <controller_port#> <midi_channel#>\n" );
    fprintf( stderr, "       port = 0..n,  channel = 1..16\n" );

    exit( 1 );
}

int main( int argc, const char **argv ) {
  if ( argc != 3 ) usage();

  char *endptr;
  errno = 0;
  int port = (int) strtol( argv[1], &endptr, 10 );
  if ( endptr == argv[0] ) usage();
  if ( port < 0 ) usage();

  channel = (int) strtol( argv[2], &endptr, 10 ) - 1;
  if ( endptr == argv[0] ) usage();
  if ( channel < 0 || channel > 15 ) usage();

  if ( 0 != mustang.initialize() ) {
    fprintf( stderr, "Cannot setup USB communication\n" );
    exit( 1 );
  }
  if ( 0 != mustang.commStart() ) {
    fprintf( stderr, "Thread setup and init failed\n" );
    exit( 1 );
  }

  RtMidiIn *input_handler = new RtMidiIn();

  // See if we have any ports
  unsigned int num_ports = input_handler->getPortCount();
  if ( num_ports == 0 ) {
    std::cout << "Cannot find a MIDI port\n";
    delete input_handler;
    return 8;
  }
  input_handler->openPort( port );
  // input_handler->openVirtualPort( "TestPort" );
  input_handler->setCallback( &message_action );

  // Don't want sysex, timing, active sense
  input_handler->ignoreTypes( true, true, true );

  // Block and wait for signal 
  pause();

  if ( 0 != mustang.commShutdown() ) {
    fprintf( stderr, "Thread shutdown failed\n" );
    exit( 1 );
  }
  if ( 0 != mustang.deinitialize() ) {
    fprintf( stderr, "USB shutdown failed\n" );
    exit( 1 );
  }
  
  // delete input_handler;
  return 0;

}
