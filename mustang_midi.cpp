#include <iostream>
#include <cstdlib>
#include <RtMidi.h>
#include <cerrno>

#include "mustang.h"

static Mustang amp;

static amp_settings amp_parms;
static fx_pedal_settings pedal_parms;
static char name[32];
static char names[100][32];

static int channel;

void message_action( double deltatime, std::vector< unsigned char > *message, void *userData ) {
#if 1
  unsigned int nBytes = message->size();
  if ( nBytes > 0 ) {
    for ( unsigned int i=0; i<nBytes; i++ )
      std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    std::cout << "stamp = " << deltatime << std::endl << std::flush;
  }
#endif

  // Is this for us?
  int msg_channel = (*message)[0] & 0x0f;
  if ( msg_channel != channel ) return;

  int rc;
  int msg_type = (*message)[0] & 0xf0;
  switch ( msg_type ) {
  case 0xc0: {
    // Program change
    int bank = (int)(*message)[1];
    rc = amp.load_memory_bank( bank, name, &amp_parms, &pedal_parms );
    if ( rc ) {
      std::cout << "Error: load_memory_bank " << bank << " failed. rc = " << rc << "\n";
    }
  }
  break;
    
  case 0xb0: {
    // Control change
    if ( (*message)[1] >= 23 && (*message)[1] <= 26 ) {
      int category = (*message)[1] - 23;
      int value = (*message)[2];
      int state;
      if      ( value >= 0 && value <= 63 )  state = 1;
      else if ( value > 63 && value <= 127 ) state = 0;
      rc = amp.effect_toggle( category, state );
      if ( rc ) {
        std::cout << "Error: effect_toggle " << category << " failed. rc = " << rc << "\n";
      }
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
    std::cerr << "Usage: prog usb_port midi_channel\n";
    exit( 1 );
}

int main( int argc, const char **argv ) {
  if ( argc != 3 ) usage();

  char *endptr;
  errno = 0;
  int port = (int) strtol( argv[1], &endptr, 10 ) - 1;
  if ( endptr == argv[0] ) usage();
  if ( port < 0 ) usage();

  channel = (int) strtol( argv[2], &endptr, 10 ) - 1;
  if ( endptr == argv[0] ) usage();
  if ( channel < 0 || channel > 15 ) usage();

  int rc = amp.start_amp( names, name, &amp_parms, &pedal_parms );
  if (rc) {
    std::cout << "Fender USB initialization failed: " << rc << "\n";
    return 8;
  }

  RtMidiIn *input_handler = new RtMidiIn();

  // See if we have any ports
  unsigned int num_ports = input_handler->getPortCount();
  if ( num_ports == 0 ) {
    std::cout << "Cannot find a MIDI port\n";
    delete input_handler;
    return 8;
  }

  // n.b. Midi port rank on the host system is 1..n, but this method
  // is normal to 0.
  input_handler->openPort( port );
  input_handler->setCallback( &message_action );

  // Don't want sysex, timing, active sense
  input_handler->ignoreTypes( true, true, true );

  std::cout << "\nTranslating MIDI input - press <enter> to quit.\n";
  char input;
  std::cin.get(input);

  delete input_handler;
  return 0;
}
