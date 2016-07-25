#include "mustang.h"

#include <cstdio>

#include <cmath>

#include "magic.h"

#include "amp.h"
#include "reverb.h"
#include "delay.h"
#include "mod.h"
#include "stomp.h"

#include "amp_defaults.h"
#include "reverb_defaults.h"
#include "delay_defaults.h"
#include "mod_defaults.h"
#include "stomp_defaults.h"

#include "amp_models.h"
#include "stomp_models.h"
#include "mod_models.h"
#include "delay_models.h"

// Parameter report (preset names + DSP states)
const unsigned char Mustang::state_prefix[] = { 0x1c, 0x01 };

// End of full parameter dump
const unsigned char Mustang::parms_done[] = { 0xff, 0x01 };

// Acknowledge tuner toggle
const unsigned char Mustang::tuner_ack[] = { 0x0a, 0x01 };

// Tuner display update 
const unsigned char Mustang::tuner_prefix[] = { 0x0b, 0x01 };

// Acknowledge model-select 
const unsigned char Mustang::select_ack[] = { 0x00, 0x00, 0x1c };

// Acknowledge CC
const unsigned char Mustang::cc_ack[] = { 0x00, 0x00, 0x05 };


const Mustang::usb_id Mustang::amp_ids[] = {
    { MI_II_V1,         0x03, false },
    { MIII_IV_V_V1,     0xc1, false },
    { M_BRONCO_40,      0x03, false },
    { M_MINI,           0x03, false },
    { M_FLOOR,          0x03, false },
    { MI_II_V2,         0xc1, true },
    { MIII_IV_V_V2,     0xc1, true },
    { 0,                0x00, false }
};


Mustang::Mustang( void ) {
  // Make model select effective
  memset( execute, 0x00, 64 );
  execute[0] = 0x1c;
  execute[1] = 0x03;
}


void *
Mustang::threadStarter( void *self ) {
  ((Mustang *)self)->handleInput();
  return NULL;
}
    

void 
Mustang::handleInput( void ) {

  int rc;
  unsigned char read_buf[64];
  int total_count = 0;

  while ( 1 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_IN, read_buf, 64, &count, USB_TIMEOUT_MS );
    total_count += count;

    // Check for exit flag
    pthread_mutex_lock( &shutdown_lock );
    bool finished = want_shutdown;
    pthread_mutex_unlock( &shutdown_lock );
    if ( finished ) break;

    if ( rc!=0 ) {
      // Retry on timeout, otherwise exit
      if ( rc==LIBUSB_ERROR_TIMEOUT ) continue;
      else                            break;
    }

    // Retry on short read
    if ( total_count!=64 ) continue;
    total_count = 0;

#if 0
    for ( int i=0; i<64; i++ ) fprintf( stderr, "%02x ", read_buf[i] );
    fprintf( stderr, "\n" );
#endif

    if ( 0==memcmp(read_buf,state_prefix,2) ) {
      // Only care about amp state messages, and not even all of them...
      int dsp_category = read_buf[2];
      switch( dsp_category ) {
        case 0x04:
        {
          // Preset name
          int idx = read_buf[4];
          pthread_mutex_lock( &preset_names_sync.lock );

          strncpy( preset_names[idx], (const char *)read_buf+16, 32 );
          preset_names[idx][32] = '\0';
          // Always take the most recent one as the current preset. This
          // will properly account for its appearance at the end of a complete
          // parm dump or when manual patch change occurs.
          curr_preset_idx = idx;

          preset_names_sync.value = true;
          pthread_cond_signal( &preset_names_sync.cond );
          pthread_mutex_unlock( &preset_names_sync.lock );
          break;
        }
        case 0x05:
        {
          // AMP
          // DSP parms (make 0x05..0x0a normal to zero)
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );
          updateAmpObj( read_buf );

          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x06:
        {
          // STOMP
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );
          updateStompObj( read_buf );

          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x07:
        {
          // MOD
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );
          updateModObj( read_buf );
          
          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x08:
        {
          // DELAY
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );
          updateDelayObj( read_buf );
          
          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x09:
        {
          // REVERB
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );
          updateReverbObj( read_buf );
          
          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x0a:
        {
          // EXP PEDAL
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );

          dsp_sync[idx].value = true;
          pthread_cond_signal( &dsp_sync[idx].cond );
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        default:
          // Filler or unknown packet type
          break;
      }
    }
    else if ( 0==memcmp(read_buf,parms_done,2) ){
      // Parameter dump complete, notify main thread
      pthread_mutex_lock( &parm_read_sync.lock );

      parm_read_sync.value = true;
      pthread_cond_signal( &parm_read_sync.cond );
      pthread_mutex_unlock( &parm_read_sync.lock );
    }
  }

  long result = rc;
  pthread_exit( (void *)result );
}


int
Mustang::initialize( void ) {
  int rc;

  if ( usb_io!=NULL ) return -1;
  int init_value = -1;

  rc = libusb_init( NULL );
  if ( rc ) return rc;

  for ( int idx=0; amp_ids[idx].pid != 0; idx++ ) {
    if ( ( usb_io = libusb_open_device_with_vid_pid(NULL, FENDER_VID, amp_ids[idx].pid)) != NULL ) {
      init_value = amp_ids[idx].init_value;
      isV2 = amp_ids[idx].isV2;
      break;
    }
  }
        
  if ( init_value < 0 ) {
    // No amp found
    libusb_exit( NULL );
    fprintf( stderr, "S - No Mustang USB device found\n" );
    return -1;
  }

  // If kernel driver is active, detach it
  if ( libusb_kernel_driver_active( usb_io,0) ) {
    // If detach fails, we're hosed...
    if ( 0 != (rc = libusb_detach_kernel_driver( usb_io,0)) ) return rc;
  }

  // Make it ours
  if ( 0 != (rc = libusb_claim_interface( usb_io,0)) ) return rc;

  unsigned char buffer[64];
  int total_count;
  
  // Phase 1 of amp init
  memset( buffer, 0, 64 );
  buffer[1] = 0xc3;

  total_count = 0;
  while ( total_count < 64 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc && rc!=LIBUSB_ERROR_TIMEOUT ) return rc;
    total_count += count;
  }

  // Clear reply
  total_count = 0;
  while ( total_count < 64 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc && rc!=LIBUSB_ERROR_TIMEOUT ) return rc;
    total_count += count;
  }
  
  // Phase 2 of amp init
  memset( buffer, 0, 64 );
  buffer[0] = 0x1a;
  buffer[1] = init_value;

  total_count = 0;
  while ( total_count < 64 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc && rc!=LIBUSB_ERROR_TIMEOUT ) return rc;
    total_count += count;
  }

  // Clear reply
  total_count = 0;
  while ( total_count < 64 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc && rc!=LIBUSB_ERROR_TIMEOUT ) return rc;
    total_count += count;
  }

  return 0;
}


int
Mustang::commStart( void ) {
  int rc;
  unsigned char buffer[64];

  // Mark as running
  want_shutdown = false;

  // Lock the flag 
  pthread_mutex_lock( &parm_read_sync.lock );
  parm_read_sync.value = false;
  
  // Start thread
  pthread_create( &worker, NULL, threadStarter, this );

  // Request parm dump
  memset( buffer, 0, 64 );
  buffer[0] = 0xff;
  buffer[1] = 0xc1;
  int total_count = 0;
  
  while ( total_count < 64 ) {
    int count;
    rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc && rc!=LIBUSB_ERROR_TIMEOUT ) return rc;
    total_count += count;
  }

  // Block until background thread tells us it's done
  while ( !parm_read_sync.value ) pthread_cond_wait( &parm_read_sync.cond, &parm_read_sync.lock );
  pthread_mutex_unlock( &parm_read_sync.lock );

  return 0;
}


int
Mustang::commShutdown( void ) {
    pthread_mutex_lock( &shutdown_lock );
    want_shutdown = true;
    pthread_mutex_unlock( &shutdown_lock );

    void *status;
    pthread_join( worker, &status );

    int rc = (long)status;
    return rc;
}


int
Mustang::deinitialize( void ) {
  if ( usb_io==NULL) return 0;
  
  int rc = libusb_release_interface( usb_io, 0 );
  if ( rc && rc != LIBUSB_ERROR_NO_DEVICE ) return rc;

  if ( (rc = libusb_attach_kernel_driver( usb_io,0)) ) return rc;

  libusb_close( usb_io );
  usb_io = NULL;

  libusb_exit( NULL );
  return 0;
}


int 
Mustang::tunerMode( int value ) {
  int rc, count;
  unsigned char buffer[64];
  memset(buffer, 0x00, 64);

  buffer[0] = 0x0a;
  buffer[1] = 0x01;

  if ( value > 63 && value <= 127 ) {
    // Tuner on
    buffer[2] = buffer[3] = buffer[4] = 0x01;
    rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );

    tuner_active = true;
  }
  else if ( value >= 0 && value <= 63 ) {
    // Tuner off
    rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );

    libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );
    // sleep( 1 );
    tuner_active = false;
  }

  return rc;
}


void 
Mustang::updateAmpObj( const unsigned char *data ) {

  AmpCC * new_amp = NULL;

  const unsigned char *model = data + MODEL;
  
  if ( is_type(f57_deluxe_id,model) ||
       is_type(f57_champ_id,model) ||
       is_type(f65_deluxe_id,model) ||
       is_type(f65_princeton_id,model) ||
       is_type(f65_twin_id,model) ||
       is_type(s60s_thrift_id,model) ) {
    new_amp = new AmpCC( this, model, 0 );
  }
  else if ( is_type(f59_bassman_id,model) ||
            is_type(brit_70s_id,model) ) {
    new_amp = new AmpCC1( this, model, 0 );
  }
  else if ( is_type(f_supersonic_id,model) ) {
    new_amp = new AmpCC2( this, model, 0 );
  }
  else if ( is_type(brit_60s_id,model) ) {
    new_amp = new AmpCC3( this, model, 0);
  }
  else if ( is_type(brit_80s_id,model) ||
            is_type(us_90s_id,model) ||
            is_type(metal_2k_id,model) ||
            is_type(brit_watt_id,model) ) {
    new_amp = new AmpCC4( this, model, 0 );
  }
  else if ( is_type(studio_preamp_id,model) ) {
    new_amp = new AmpCC5( this, model, 0 );
  }
  else if ( is_type(brit_color_id,model) ) {
    new_amp = new AmpCC6( this, model, 0 );
  }
  else if ( is_type(f57_twin_id,model) ) {
    new_amp = new AmpCC7( this, model, 0 );
  }
  else {
    fprintf( stderr, "W - Amp id {%x,%x} not supported yet\n", model[0], model[1] );
  }

  if ( new_amp!=NULL ) {
    delete curr_amp;
    curr_amp = new_amp;
  }
}


void 
Mustang::updateStompObj( const unsigned char *data ) {

  StompCC * new_stomp = NULL;

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  if ( is_type(overdrive_id,model) ) {
    new_stomp = new OverdriveCC( this, model, slot );
  }
  else if ( is_type(wah_id,model) ||
            is_type(touch_wah_id,model) ) {
    new_stomp = new WahCC( this, model, slot );
  }
  else if ( is_type(fuzz_id,model) ) {
    new_stomp = new FuzzCC( this, model, slot );
  }
  else if ( is_type(fuzz_twah_id,model) ) {
    new_stomp = new FuzzTouchWahCC( this, model, slot );
  }
  else if ( is_type(simple_comp_id,model) ) {
    new_stomp = new SimpleCompCC( this, model, slot );
  }
  else if ( is_type(comp_id,model) ) {
    new_stomp = new CompCC( this, model, slot );
  }
  else if ( is_type(range_boost_id,model) ) {
    new_stomp = new RangerCC( this, model, slot );
  }
  else if ( is_type(green_box_id,model) ) {
    new_stomp = new GreenBoxCC( this, model, slot );
  }
  else if ( is_type(orange_box_id,model) ) {
    new_stomp = new OrangeBoxCC( this, model, slot );
  }
  else if ( is_type(black_box_id,model) ) {
    new_stomp = new BlackBoxCC( this, model, slot );
  }
  else if ( is_type(big_fuzz_id,model) ) {
    new_stomp = new BigFuzzCC( this, model, slot );
  }
  else {
    fprintf( stderr, "W - Stomp id {%x,%x} not supported\n", model[0], model[1] );
  }

  if ( new_stomp!=NULL ) {
    delete curr_stomp;
    curr_stomp = new_stomp;
  }
}


void 
Mustang::updateDelayObj( const unsigned char *data ) {

  DelayCC * new_delay = NULL;
    
  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];

  if ( is_type(mono_dly_id,model) ) {
    new_delay = new MonoDelayCC( this, model, slot );
  }
  else if ( is_type(mono_filter_id,model) ||
            is_type(st_filter_id,model) ) {
    new_delay = new EchoFilterCC( this, model, slot );
  }
  else if ( is_type(mtap_dly_id,model) ) {
    new_delay = new MultitapDelayCC( this, model, slot );
  }
  else if ( is_type(pong_dly_id,model) ) {
    new_delay = new PingPongDelayCC( this, model, slot );
  }
  else if ( is_type(duck_dly_id,model) ) {
    new_delay = new DuckingDelayCC( this, model, slot );
  }
  else if ( is_type(reverse_dly_id,model) ) {
    new_delay = new ReverseDelayCC( this, model, slot );
  }
  else if ( is_type(tape_dly_id,model) ) {
    new_delay = new TapeDelayCC( this, model, slot );
  }
  else if ( is_type(st_tape_dly_id,model) ) {
    new_delay = new StereoTapeDelayCC( this, model, slot );
  }
  else {
    fprintf( stderr, "W - Delay id {%x,%x} not supported\n", model[0], model[1] );
  }

  if ( new_delay!=NULL ) {
    delete curr_delay;
    curr_delay = new_delay;
  }
}


void 
Mustang::updateModObj( const unsigned char *data ) {

  ModCC * new_mod = NULL;

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  if ( is_type(sine_chorus_id,model) ||
       is_type(tri_chorus_id,model) ) {
    new_mod = new ChorusCC( this, model, slot );
  }
  else if ( is_type(sine_flange_id,model) ||
            is_type(tri_flange_id,model) ) {
    new_mod = new FlangerCC( this, model, slot );
  }
  else if ( is_type(vibratone_id,model) ) {
    new_mod = new VibratoneCC( this, model, slot );
  }
  else if ( is_type(vint_trem_id,model) ||
            is_type(sine_trem_id,model) ) {
    new_mod = new TremCC( this, model, slot );
  }
  else if ( is_type(ring_mod_id,model) ) {
    new_mod = new RingModCC( this, model, slot );
  }
  else if ( is_type(step_filt_id,model) ) {
    new_mod = new StepFilterCC( this, model, slot );
  }
  else if ( is_type(phaser_id,model) ) {
    new_mod = new PhaserCC( this, model, slot );
  }
  else if ( is_type(pitch_shift_id,model) ) {
    new_mod = new PitchShifterCC( this, model, slot );
  }
  else if ( is_type(m_wah_id,model) ||
            is_type(m_touch_wah_id,model) ) {
    new_mod = new ModWahCC( this, model, slot );
  }
  else if ( is_type(dia_pitch_id,model) ) {
    new_mod = new DiatonicShiftCC( this, model, slot );
  }
  else {
    fprintf( stderr, "W - Mod id {%x,%x} not supported\n", model[0], model[1] );
  }

  if ( new_mod!=NULL ) {
    delete curr_mod;
    curr_mod = new_mod;
  }
}


void 
Mustang::updateReverbObj( const unsigned char *data ) {

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  delete curr_reverb;
  curr_reverb = new ReverbCC( this, model, slot );
}


int 
Mustang::effectToggle(int cc, int value) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char buffer[64];

  // Translate 23..26 --> 2..5 (current state index)
  int state_index = cc - 21;
  int state;
  if      ( value >= 0 && value <= 63 )  state = 1;
  else if ( value > 63 && value <= 127 ) state = 0;

  memset(buffer, 0x00, 64);
  buffer[0] = 0x19;
  buffer[1] = 0xc3;
  // Translate DSP to family
  buffer[FAMILY] = dsp_parms[state_index][DSP] - 3;
  // Invert logic
  buffer[ACTIVE_INVERT] = state;
  buffer[FXSLOT] = dsp_parms[state_index][FXSLOT];
#if 0    
  for ( int i=0; i<15; i++ ) fprintf( stderr, "%02x ", buffer[i] );
  fprintf( stderr, "\n" );
#endif
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );

  // Note: Toggle gets three response packets
  for (int i=0; i < 3; i++) {
    libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );
  }

  return rc;
}

int 
Mustang::setAmp( int ord ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char scratch[64];
    
  unsigned char *buffer;

  switch (ord) {
    case 0:
      buffer = amp_none;
      break;
    case 1:
      buffer = f57_deluxe;
      break;
    case 2:
      buffer = f59_bassman;
      break;
    case 3:
      buffer = f57_champ;
      break;
    case 4:
      buffer = f65_deluxe;
      break;
    case 5:
      buffer = f65_princeton;
      break;
    case 6:
      buffer = f65_twin;
      break;
    case 7:
      buffer = f_supersonic;
      break;
    case 8:
      buffer = brit_60;
      break;
    case 9:
      buffer = brit_70;
      break;
    case 10:
      buffer = brit_80;
      break;
    case 11:
      buffer = us_90;
      break;
    case 12:
      buffer = metal_2k;
      break;
    default:
      if ( isV2 ) {
        switch (ord) {
          case 13:
            buffer = studio_preamp;
            break;
          case 14:
            buffer = f57_twin;
            break;
          case 15:
            buffer = sixties_thrift;
            break;
          case 16:
            buffer = brit_watts;
            break;
          case 17:
            buffer = brit_color;
            break;
          default:
            fprintf( stderr, "W - Amp select %d not supported\n", ord );
            return 0;
        }
      }
      else {
        fprintf( stderr, "W - Amp select %d not supported\n", ord );
        return 0;
      }
  }

  // Setup amp personality
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // Copy to current setting store
  memcpy(dsp_parms[AMP_STATE], buffer, 64);
//  updateAmpObj("a");

  // Setup USB gain
  // memset(scratch, 0x00, 64);
  // scratch[0] = 0x1c;
  // scratch[1] = 0x03;
  // scratch[2] = 0x0d;
  // scratch[6] = 0x01;
  // scratch[7] = 0x01;
  // scratch[16] = 0x80;

  // rc = libusb_interrupt_transfer( usb_io, USB_OUT, scratch, 64, &count, USB_TIMEOUT_MS );
  // libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  // libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  return rc;
}

int 
Mustang::setReverb( int ord ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char scratch[64];
    
  unsigned char *buffer;

  switch (ord) {
    case 0:
      buffer = reverb_none;
      break;
    case 1:
      buffer = small_hall;
      break;
    case 2:
      buffer = large_hall;
      break;
    case 3:
      buffer = small_room;
      break;
    case 4:
      buffer = large_room;
      break;
    case 5:
      buffer = small_plate;
      break;
    case 6:
      buffer = large_plate;
      break;
    case 7:
      buffer = ambient;
      break;
    case 8:
      buffer = arena;
      break;
    case 9:
      buffer = spring_63;
      break;
    case 10:
      buffer = spring_65;
      break;
    default:
      fprintf( stderr, "W - Reverb select %d not supported\n", ord );
      return 0;
  }

  buffer[FXSLOT] = dsp_parms[REVERB_STATE][FXSLOT];

  // Setup amp personality
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // Copy to current setting store
  memcpy(dsp_parms[REVERB_STATE], buffer, 64);
  // updateReverbObj();

  return rc;
}

int 
Mustang::setDelay( int ord ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char scratch[64];
    
  unsigned char *buffer;

  switch (ord) {
    case 0:
      buffer = delay_none;
      break;
    case 1:
      buffer = mono_delay;
      break;
    case 2:
      buffer = mono_echo_filter;
      break;
    case 3:
      buffer = stereo_echo_filter;
      break;
    case 4:
      buffer = multitap_delay;
      break;
    case 5:
      buffer = ping_pong_delay;
      break;
    case 6:
      buffer = ducking_delay;
      break;
    case 7:
      buffer = reverse_delay;
      break;
    case 8:
      buffer = tape_delay;
      break;
    case 9:
      buffer = stereo_tape_delay;
      break;
    default:
      fprintf( stderr, "W - Delay select %d not supported\n", ord );
      return 0;
  }

  buffer[FXSLOT] = dsp_parms[DELAY_STATE][FXSLOT];

  // Setup amp personality
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // Copy to current setting store
  memcpy(dsp_parms[DELAY_STATE], buffer, 64);
//  updateDelayObj();

  return rc;
}

int 
Mustang::setMod( int ord ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char scratch[64];
    
  unsigned char *buffer;

  switch (ord) {
    case 0:
      buffer = mod_none;
      break;
    case 1:
      buffer = sine_chorus;
      break;
    case 2:
      buffer = triangle_chorus;
      break;
    case 3:
      buffer = sine_flanger;
      break;
    case 4:
      buffer = triangle_flanger;
      break;
    case 5:
      buffer = vibratone;
      break;
    case 6:
      buffer = vintage_tremolo;
      break;
    case 7:
      buffer = sine_tremolo;
      break;
    case 8:
      buffer = ring_modulator;
      break;
    case 9:
      buffer = step_filter;
      break;
    case 10:
      buffer = phaser;
      break;
    case 11:
      buffer = pitch_shifter;
      break;
    default:
      if ( isV2 ) {
        switch (ord) {
          case 12:
            buffer = mod_wah;
            break;
          case 13:
            buffer = mod_touch_wah;
            break;
          case 14:
            buffer = diatonic_pitch_shift;
            break;
          default:
            fprintf( stderr, "W - Mod select %d not supported\n", ord );
            return 0;
        }
      }
      else {
        fprintf( stderr, "W - Mod select %d not supported\n", ord );
        return 0;
      }
  }

  buffer[FXSLOT] = dsp_parms[MOD_STATE][FXSLOT];

  // Setup amp personality
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // Copy to current setting store
  memcpy(dsp_parms[MOD_STATE], buffer, 64);
  // updateModObj();

  return rc;
}

int 
Mustang::setStomp( int ord ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char scratch[64];
    
  unsigned char *buffer;

  switch (ord) {
    case 0:
      buffer = stomp_none;
      break;
    case 1:
      buffer = overdrive;
      break;
    case 2:
      buffer = wah;
      break;
    case 3:
      buffer = touch_wah;
      break;
    case 4:
      buffer = fuzz;
      break;
    case 5:
      if ( isV2 ) {
        fprintf( stderr, "W - Stomp select %d not supported\n", ord );
        return 0;
      }
      else {
        buffer = fuzz_touch_wah;
      }
      break;
    case 6:
      buffer = simple_comp;
      break;
    case 7:
      buffer = compressor;
      break;
    default:
      if ( isV2 ) {
        switch (ord) {
          case 8:
            buffer = ranger_boost;
            break;
          case 9:
            buffer = green_box;
            break;
          case 10:
            buffer = orange_box;
            break;
          case 11:
            buffer = black_box;
            break;
          case 12:
            buffer = big_fuzz;
            break;
          default:
            fprintf( stderr, "W - Stomp select %d not supported\n", ord );
            return 0;
        }
      }
      else {
        fprintf( stderr, "W - Stomp select %d not supported\n", ord );
        return 0;
      }
  }

  buffer[FXSLOT] = dsp_parms[STOMP_STATE][FXSLOT];

  // Setup amp personality
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, execute, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, scratch, 64, &count, USB_TIMEOUT_MS );

  // Copy to current setting store
  memcpy(dsp_parms[STOMP_STATE], buffer, 64);
//  updateStompObj();

  return rc;
}


int 
Mustang::continuous_control( const Mustang::Cmd & cmd ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char buffer[64];
    
  memset(buffer, 0x00, 64);
  buffer[0] = 0x05;
  buffer[1] = 0xc3;
  buffer[2] = cmd.parm2;
  buffer[3] = dsp_parms[cmd.state_index][MODEL];

  // target parameter
  buffer[5] = cmd.parm5;
  buffer[6] = cmd.parm6;
  buffer[7] = cmd.parm7;
    
  // Scale and clamp to valid index range
  int index = (int) ceil( (double)cmd.value * magic_scale_factor );
  if ( index > magic_max ) index = magic_max;
    
  unsigned short eff_value = magic_values[index];

  buffer[9] = eff_value & 0xff;
  buffer[10] = (eff_value >> 8) & 0xff;

  // Send command and flush reply
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );

  return rc;
}


int 
Mustang::discrete_control( const Mustang::Cmd & cmd ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char buffer[64];
    
  memset(buffer, 0x00, 64);
  buffer[0] = 0x05;
  buffer[1] = 0xc3;
  buffer[2] = cmd.parm2;
  buffer[3] = dsp_parms[cmd.state_index][MODEL];

  buffer[5] = cmd.parm5;
  buffer[6] = cmd.parm6;
  buffer[7] = cmd.parm7;

  // Discrete value
  buffer[9] = cmd.value;

  // Send command and flush reply
  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
  libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );

  return rc;
}


int 
Mustang::patchChange( int patch ) {
  if ( tuner_active ) return 0;

  int rc, count;
  unsigned char buffer[64], data[7][64];

  memset(buffer, 0x00, 64);
  buffer[0] = 0x1c;
  buffer[1] = 0x01;
  buffer[2] = 0x01;
  buffer[PATCH_SLOT] = patch;
  buffer[6] = 0x01;

  rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );

  // Mustang III has nine responses
  for(int i=0; i < 9; i++) {
    libusb_interrupt_transfer( usb_io, USB_IN, buffer, 64, &count, USB_TIMEOUT_MS );
    int dsp = buffer[2];
    if ( dsp >= 4 && dsp <= 9 )
      memcpy(dsp_parms[dsp - 4], buffer, 64);
  }
//  updateAmpObj("a");
//  updateReverbObj();
//  updateDelayObj();
//  updateModObj();
//  updateStompObj();
    
  return rc;
}

