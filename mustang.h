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
  friend class AmpCC;
  friend class ReverbCC;
  friend class DelayCC;
  friend class ModCC;
  friend class StompCC;
    
  char preset_names[100][33];
  unsigned curr_preset_idx;
  
  unsigned char dsp_parms[6][64];

  unsigned char curr_model[5][2];

  unsigned char execute[64];

  static const unsigned char state_prefix[];
  static const unsigned char parms_done[];

  static const unsigned char tuner_ack[];
  static const unsigned char tuner_prefix[];

  static const unsigned char select_ack[];
  static const unsigned char cc_ack[];
  
  libusb_device_handle *usb_io;

  pthread_t worker;

  pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;

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

  // Manage access to each DSP block 
  Condition<bool> dsp_sync[5];

  Condition<bool> preset_names_sync;

  // Synchronize init on end of initial parm dump
  Condition<bool> parm_read_sync;

  struct usb_id {
    // product id
    int pid;
    // magic value for init packet
    int init_value;
    // v2?
    bool isV2;
  };

  // For device probe
  static const usb_id amp_ids[];
  bool isV2;

  static void *threadStarter( void * );
  void handleInput( void );

  bool tuner_active;

  AmpCC * curr_amp;
  ReverbCC * curr_reverb;
  DelayCC * curr_delay;
  ModCC * curr_mod;
  StompCC * curr_stomp;

public:  
  struct Cmd {
    int state_index;
    int parm2;
    int parm5;
    int parm6;
    int parm7;
    int value;
  };

private:
  int continuous_control( const Mustang::Cmd & cmd );
  int discrete_control( const Mustang::Cmd & cmd );
  
  void updateAmpObj( const unsigned char *data );
  void updateStompObj( const unsigned char *data );
  void updateReverbObj( const unsigned char *data );
  void updateDelayObj( const unsigned char *data );
  void updateModObj( const unsigned char *data );

  inline bool is_type(const unsigned char *a, const unsigned char *b) {
    return ( 0==memcmp(a,b,2) );
  }

public:
  Mustang( void );

  int initialize( void );
  int deinitialize( void );
  
  int commStart( void );
  int commShutdown( void );

  int setAmp( int ord );
  int setReverb( int ord );
  int setDelay( int ord );
  int setMod( int ord );
  int setStomp( int ord );

  int tunerMode( int value );
    
  int patchChange( int );

  int effectToggle( int cc, int value );

  AmpCC * getAmp( void ) { return curr_amp;}
  ReverbCC * getReverb( void ) { return curr_reverb;}
  DelayCC * getDelay( void ) { return curr_delay;}
  ModCC * getMod( void ) { return curr_mod;}
  StompCC * getStomp( void ) { return curr_stomp;}
};


#endif
