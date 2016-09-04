// -*-c++-*-

#ifndef MUSTANG2_H
#define MUSTANG2_H

#include <stdint.h>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include "constants.h"

#define USB_IN  0x81
#define USB_OUT 0x01

#define USB_TIMEOUT_MS 500

class AmpCC;
class ReverbCC;
class DelayCC;
class ModCC;
class StompCC;


class Mustang {

  // Pre-built 'execute' command
  unsigned char execute[64];
  
  libusb_device_handle *usb_io;

  // USB listen thread
  pthread_t worker;

  // Shutdown request flag
  pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER;
  bool want_shutdown;

  template <class T>
  class Condition {
  public:
    pthread_mutex_t lock;
    pthread_cond_t cond;
    T value;
    Condition( void ) {
      pthread_mutex_init( &(lock), NULL);
      pthread_cond_init( &(cond), NULL);
    }
  };

  // Identify patch-change ack / DSP parm update
  static const unsigned char state_prefix[];

  // Received {0x1c, 0x01, 0x00, ...}
  // --> End of preset select acknowledge stream
  Condition<bool> pc_ack_sync;

  // Synchronize access to preset names
  Condition<bool> preset_names_sync;

  // 0-99 = amp preset, 100-111 = mod preset, 112-123 = rev/delay preset
  char preset_names[124][33];

  // Index to current amp preset 
  unsigned curr_preset_idx;

  // Manage access to each DSP data block and/or associated object.
  Condition<bool> dsp_sync[6];
  unsigned char dsp_parms[6][64];

  // Manage data and behavior for specific DSP models
  AmpCC * curr_amp;
  StompCC * curr_stomp;
  ModCC * curr_mod;
  DelayCC * curr_delay;
  ReverbCC * curr_reverb;

  // Synchronize on end of parm dump
  Condition<bool> parm_read_sync;
  static const unsigned char parm_read_ack[];

  // Synchronize on receipt of direct control acknowledge
  Condition<bool> cc_ack_sync;
  static const unsigned char cc_ack[];

  // Received {0x00, 0x00, 0x19, ... }
  // --> Acknowledge efx on/off toggle
  Condition<bool> efx_toggle_sync;
  static const unsigned char efx_toggle_ack[];

  // Synchronize on receipt of model change acknowledge
  Condition<bool> model_change_sync;
  static const unsigned char model_change_ack[];

  // Sync on tuner on/off ack
  Condition<bool> tuner_ack_sync;
  static const unsigned char tuner_ack[];
  
  // Attached device probe structure
  struct usb_id {
    // product id
    int pid;
    // magic value for init packet
    int init_value;
    // v2?
    bool isV2;
  };

  static const usb_id amp_ids[];
  bool isV2;

  static void *threadStarter( void * );
  void handleInput( void );

  int direct_control( unsigned char *cmd );
  
  int sendCmd( unsigned char *buffer );
  int requestDump( void );
  int executeModelChange( unsigned char *buffer );

  void updateAmpObj( const unsigned char *data );
  void updateStompObj( const unsigned char *data );
  void updateReverbObj( const unsigned char *data );
  void updateDelayObj( const unsigned char *data );
  void updateModObj( const unsigned char *data );

  int checkOrDisableTuner( void );

  // Check for equality of 2-byte values
  inline bool match16(const unsigned char *a, const unsigned char *b) {
    return ( 0==memcmp(a,b,2) );
  }

public:
  Mustang( void );

  int initialize( void );
  int deinitialize( void );
  
  int commStart( void );
  int commShutdown( void );

  int setAmp( int ord );
  int ampControl( int cc, int value );

  int setStomp( int ord );
  int stompControl( int cc, int value );

  int setMod( int ord );
  int modControl( int cc, int value );

  int setDelay( int ord );
  int delayControl( int cc, int value );

  int setReverb( int ord );
  int reverbControl( int cc, int value );

  int tunerMode( int value );
    
  int patchChange( int );

  int effectToggle( int cc, int value );
};


#endif
