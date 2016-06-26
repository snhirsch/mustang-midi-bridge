#include <iostream>
#include <cstdlib>
#include <RtMidi.h>
#include <cerrno>

#include "mustang.h"

#include "amp.h"
#include "reverb.h"
#include "delay.h"


static Mustang mustang;

static int channel;

void message_action( double deltatime, std::vector< unsigned char > *message, void *userData ) {
#if 0
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

  int msg_type = (*message)[0] & 0xf0;

  switch ( msg_type ) {

  case 0xc0: {
    // Program change
    int bank = (int)(*message)[1];
    int rc = mustang.load_memory_bank( bank );
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
    
    // Effects on/off
    if ( cc >= 23 && cc <= 26 ) {
      rc = mustang.effect_toggle( cc, value );
    }
    // Set delay model
    else if ( cc == 48 ) {
      rc = mustang.setDelay( value );
    }
    // Delay CC handler
    else if ( cc >= 49 && cc <= 54 ) {
      DelayCC *delayObj = mustang.getDelay();
      rc = delayObj->dispatch( cc, value );
    }
    // Set reverb model
    else if ( cc == 58 ) {
      rc = mustang.setReverb( value );
    }
    // Reverb CC handler
    else if ( cc >= 59 && cc <= 63 ) {
      ReverbCC *reverbObj = mustang.getReverb();
      rc = reverbObj->dispatch( cc, value );
    }
    // Set amp model
    else if ( cc == 68 ) {
      rc = mustang.setAmp( value );
    }
    // Amp CC Handler
    else if ( cc >= 69 && cc <= 79 ) {
      AmpCC *ampObj = mustang.getAmp();
      rc = ampObj->dispatch( cc, value );
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

  int rc = mustang.start_amp();
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
