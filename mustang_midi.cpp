#include <iostream>
#include <cstdlib>
#include <RtMidi.h>
#include <cerrno>

#include "mustang.h"

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
    AmpCC *ampModel = mustang.getAmp();
    ReverbCC *reverbModel = mustang.getReverb();
    
    // Effects on/off
    if ( cc >= 23 && cc <= 26 ) {
      // Translate 23..26 --> 2..5 (current state index)
      int index = cc - 21;
      int state;
      if      ( value >= 0 && value <= 63 )  state = 0;
      else if ( value > 63 && value <= 127 ) state = 1;
      rc = mustang.effect_toggle( index, state );
    }
    // Set reverb model
    else if ( cc == 58 ) {
      rc = mustang.setReverb( value );
    }
    // Level
    else if ( cc == 59 ) {
      rc = reverbModel->cc59( value );
    }
    // Decay
    else if ( cc == 60 ) {
      rc = reverbModel->cc60( value );
    }
    // Dwell
    else if ( cc == 61 ) { 
      rc = reverbModel->cc61( value );
    }
    // Diffusion
    else if ( cc == 62 ) {
      rc = reverbModel->cc62( value );
    }
    // Tone
    else if ( cc == 63 ) {
      rc = reverbModel->cc63( value );
    }
    // Set amp model
    else if ( cc == 68 ) {
      // fprintf( stderr, "DEBUG: %d\n", value );
      rc = mustang.setAmp( value );
    }
    // Gain
    else if ( cc == 69 ) {
      rc = ampModel->cc69( value );
    }
    // Channel volume
    else if ( cc == 70 ) {
      rc = ampModel->cc70( value );
    }
    // Treble
    else if ( cc == 71 ) { 
      rc = ampModel->cc71( value );
    }
    // Mid
    else if ( cc == 72 ) {
      rc = ampModel->cc72( value );
    }
    // Bass
    else if ( cc == 73 ) {
      rc = ampModel->cc73( value );
    }
    // Sag
    else if ( cc == 74 ) {
      rc = ampModel->cc74( value );
    }
    // Bias
    else if ( cc == 75 ) {
      rc = ampModel->cc75( value );
    }
    // Noise Gate
    else if ( cc == 76 ) {
      rc = ampModel->cc76( value );
    }
    // Cabinet
    else if ( cc == 77 ) {
      rc = ampModel->cc77( value );
    }
    // Presence / Gain2 / Cut
    else if ( cc == 78 ) {
      rc = ampModel->cc78( value );
    }
    // Blend / Master Volume
    else if ( cc == 79 ) {
      rc = ampModel->cc79( value );
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
