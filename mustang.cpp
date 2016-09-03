#include "mustang.h"

#include <cstdio>
#include <unistd.h>

#include "amp_models.h"
#include "amp_defaults.h"
#include "amp.h"

#include "stomp_models.h"
#include "stomp_defaults.h"
#include "stomp.h"

#include "mod_models.h"
#include "mod_defaults.h"
#include "mod.h"

#include "delay_models.h"
#include "delay_defaults.h"
#include "delay.h"

#include "reverb_models.h"
#include "reverb_defaults.h"
#include "reverb.h"

// Parameter report (preset names + DSP states)
const unsigned char Mustang::state_prefix[] = { 0x1c, 0x01 };

// End of full parameter dump
const unsigned char Mustang::parm_read_ack[] = { 0xff, 0x01 };

// Acknowledge tuner change
const unsigned char Mustang::tuner_ack[] = { 0x0a, 0x01 };

// Acknowledge model-select 
const unsigned char Mustang::model_change_ack[] = { 0x00, 0x00, 0x1c };

// Acknowledge EFX toggle
const unsigned char Mustang::efx_toggle_ack[] = { 0x00, 0x00, 0x19 };

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

  // So far, this is the only state flag we need to pre-condition
  tuner_ack_sync.value = false;
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
      // Always retry on timeout since we expect it.  Otherwise exit
      if ( rc!=LIBUSB_ERROR_TIMEOUT ) break;
    }

    // Retry on short read
    if ( total_count!=64 ) continue;
    total_count = 0;

#ifdef DEBUG
    for ( int i=0; i<64; i++ ) fprintf( stderr, "%02x ", read_buf[i] );
    fprintf( stderr, "\n" );
#endif

    if ( 0==memcmp(read_buf,state_prefix,2) ) {
      // Only care about amp state messages, and not even all of them...
      int dsp_category = read_buf[2];
      switch( dsp_category ) {
        // Response for DSP data and/or patch-change
        //
        case 0x00:
        {
          // Patch change acknowledge sequence done
          pthread_mutex_lock( &pc_ack_sync.lock );
          pc_ack_sync.value = true;
          pthread_cond_signal( &pc_ack_sync.cond );
          pthread_mutex_unlock( &pc_ack_sync.lock );
          break;
        }
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
          
          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        case 0x0a:
        {
          // EXP PEDAL
          int idx = dsp_category - 5;
          pthread_mutex_lock( &dsp_sync[idx].lock );

          memcpy( dsp_parms[idx], (const char *)read_buf, 64 );

          pthread_mutex_unlock( &dsp_sync[idx].lock );
          break;
        }
        default:
          // Filler or unknown packet type
          break;
      }
    }
    else if ( 0==memcmp(read_buf,model_change_ack,3) ) {
      // Model change acknowledge
      pthread_mutex_lock( &model_change_sync.lock );
      model_change_sync.value = true;
      pthread_cond_signal( &model_change_sync.cond );
      pthread_mutex_unlock( &model_change_sync.lock );
    }
    else if ( 0==memcmp(read_buf,cc_ack,3) ){
      // Direct command acknowledge
      pthread_mutex_lock( &cc_ack_sync.lock );
      cc_ack_sync.value = true;
      pthread_cond_signal( &cc_ack_sync.cond );
      pthread_mutex_unlock( &cc_ack_sync.lock );
    }
    else if ( 0==memcmp(read_buf,efx_toggle_ack,3) ){
      // EFX toggle acknowledge
      pthread_mutex_lock( &efx_toggle_sync.lock );
      efx_toggle_sync.value = true;
      pthread_cond_signal( &efx_toggle_sync.cond );
      pthread_mutex_unlock( &efx_toggle_sync.lock );
    }
    else if ( 0==memcmp(read_buf,parm_read_ack,2) ){
      // Parameter dump completion acknowledge
      pthread_mutex_lock( &parm_read_sync.lock );
      parm_read_sync.value = true;
      pthread_cond_signal( &parm_read_sync.cond );
      pthread_mutex_unlock( &parm_read_sync.lock );
    }
    else if ( 0==memcmp(read_buf,tuner_ack,2) ){
      // Tuner toggle notify
      pthread_mutex_lock( &tuner_ack_sync.lock );
      tuner_ack_sync.value = read_buf[2]==0x01 ? true : false;
      pthread_cond_signal( &tuner_ack_sync.cond );
      pthread_mutex_unlock( &tuner_ack_sync.lock );
    }
  }

  long result = rc;
  pthread_exit( (void *)result );
}


int
Mustang::sendCmd( unsigned char *buffer ) {
  int total_count = 0;
  int attempts = 5;
  
  while ( total_count < 64 ) {
    int count;
    int rc = libusb_interrupt_transfer( usb_io, USB_OUT, buffer, 64, &count, USB_TIMEOUT_MS );
    if ( rc ) {
      if ( rc==LIBUSB_ERROR_TIMEOUT ) {
        // Up to five retries on timeout
        if ( attempts==0 ) return rc;
        else               attempts--;
      }
      // All other errors immediately fatal
      else return rc;
    }
    total_count += count;
  }
  return 0;
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
    if ( 0 != (rc = libusb_detach_kernel_driver(usb_io,0)) ) return rc;
  }

  // Make it ours
  if ( 0 != (rc = libusb_claim_interface(usb_io,0)) ) return rc;

  unsigned char buffer[64];
  int total_count;
  
  // Phase 1 of amp init
  memset( buffer, 0, 64 );
  buffer[1] = 0xc3;

  rc = sendCmd( buffer );
  if ( rc!=0 ) return rc;
  
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

  rc = sendCmd( buffer );
  if ( rc!=0 ) return rc;

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

  ///// Critical Section
  //
  pthread_mutex_lock( &parm_read_sync.lock );
  parm_read_sync.value = false;

  // Start thread
  pthread_create( &worker, NULL, threadStarter, this );

  // Request parm dump
  memset( buffer, 0, 64 );
  buffer[0] = 0xff;
  buffer[1] = 0xc1;

  rc = sendCmd( buffer );

  // Block until background thread tells us it's done
  while ( rc==0 && !parm_read_sync.value ) pthread_cond_wait( &parm_read_sync.cond, &parm_read_sync.lock );
  pthread_mutex_unlock( &parm_read_sync.lock );
  //
  /////

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
Mustang::requestDump( void ) {
  int rc;
  unsigned char buffer[64];

  ///// Critical Section
  //
  pthread_mutex_lock( &parm_read_sync.lock );
  
  // Request parm dump
  memset( buffer, 0, 64 );
  buffer[0] = 0xff;
  buffer[1] = 0xc1;

  parm_read_sync.value = false;
  rc = sendCmd( buffer );

  // Block until background thread tells us it's done
  while ( rc==0 && !parm_read_sync.value ) pthread_cond_wait( &parm_read_sync.cond, &parm_read_sync.lock );
  pthread_mutex_unlock( &parm_read_sync.lock );
  //
  //////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Parm dump completion acknowledged\n" );
#endif

  return 0;
}


int
Mustang::executeModelChange( unsigned char *buffer ) { 
  // 5..9 --> 0..4
  int idx = buffer[2] - 5;

  /////// Critical Section 1
  //
  pthread_mutex_lock( &model_change_sync.lock );

  // Setup amp personality
  model_change_sync.value = false;
  int rc = sendCmd( buffer );
  while ( rc==0 && !model_change_sync.value ) pthread_cond_wait( &model_change_sync.cond, &model_change_sync.lock );

  // Execute command
  model_change_sync.value = false;
  rc = sendCmd( execute );
  while ( rc==0 && !model_change_sync.value ) pthread_cond_wait( &model_change_sync.cond, &model_change_sync.lock );

  pthread_mutex_unlock( &model_change_sync.lock );
  //
  //////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Model change acknowledged\n" );
#endif

  ///// Critical Section 2
  //
  // Lock the DSP device and update for what we just sent
  pthread_mutex_lock( &dsp_sync[idx].lock );

  // Update DSP state buffer
  memcpy( dsp_parms[idx], (const char *)buffer, 64 );
  unsigned char *curr = dsp_parms[idx];

  // Make it look like a status report:
  curr[1] = 0x03;
  curr[4] = 0x00;
  curr[7] = 0x01;

  switch ( idx ) {
    case 0:
      updateAmpObj( curr );
      break;
    case 1:
      updateStompObj( curr );
      break;
    case 2:
      updateModObj( curr );
      break;
    case 3:
      updateDelayObj( curr );
      break;
    case 4:
      updateReverbObj( curr );
      break;
  }
  pthread_mutex_unlock( &dsp_sync[idx].lock );
  //
  ///////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: State updated\n" );
#endif

  return rc;
}


void 
Mustang::updateAmpObj( const unsigned char *data ) {

  AmpCC * new_amp = NULL;

  const unsigned char *model = data + MODEL;
    
  if ( match16(f57_deluxe_id,model) ||
       match16(f57_champ_id,model) ||
       match16(f65_deluxe_id,model) ||
       match16(f65_princeton_id,model) ||
       match16(s60s_thrift_id,model) ) {
    new_amp = new AmpCC( this, model, 0 );
  }
  else if ( match16(f59_bassman_id,model) ||
            match16(brit_70s_id,model) ) {
    new_amp = new AmpCC1( this, model, 0 );
  }
  else if ( match16(f_supersonic_id,model) ) {
    new_amp = new AmpCC2( this, model, 0 );
  }
  else if ( match16(brit_60s_id,model) ) {
    new_amp = new AmpCC3( this, model, 0);
  }
  else if ( match16(brit_80s_id,model) ||
            match16(us_90s_id,model) ||
            match16(metal_2k_id,model) ||
            match16(brit_watt_id,model) ) {
    new_amp = new AmpCC4( this, model, 0 );
  }
  else if ( match16(studio_preamp_id,model) ) {
    new_amp = new AmpCC5( this, model, 0 );
  }
  else if ( match16(brit_color_id,model) ) {
    new_amp = new AmpCC6( this, model, 0 );
  }
  else if ( match16(f57_twin_id,model) ) {
    new_amp = new AmpCC7( this, model, 0 );
  }
  else if ( match16(f65_twin_id,model) ) {
    new_amp = new AmpCC8( this, model, 0 );
  }
  else if ( match16(null_amp_id,model) ) {
    new_amp = new NullAmpCC( this, model, 0 );
  }
  else {
    fprintf( stderr, "W - Amp id {%x,%x} not expected\n", model[0], model[1] );
  }

  if ( new_amp!=NULL ) {
    delete curr_amp;
    curr_amp = new_amp;
  }
}


int 
Mustang::setAmp( int ord ) {
  if ( checkOrDisableTuner() < 0 ) return -1;
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
            return 0;
        }
      }
      else {
        return 0;
      }
  }

  return executeModelChange( buffer );
}


int
Mustang::ampControl( int cc, int value ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char cmd[64];
  memset( cmd, 0, 64 );
  
  pthread_mutex_lock( &dsp_sync[AMP_STATE].lock );
  int rc = curr_amp->dispatch( cc, value, cmd );
  pthread_mutex_unlock( &dsp_sync[AMP_STATE].lock );

  // If value out-of-range, just return gracefully
  if ( rc<0 ) return 0;
  rc = direct_control( cmd );
  return rc;
}


void 
Mustang::updateStompObj( const unsigned char *data ) {

  StompCC * new_stomp = NULL;

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  if ( match16(overdrive_id,model) ) {
    new_stomp = new OverdriveCC( this, model, slot );
  }
  else if ( match16(wah_id,model) ||
            match16(touch_wah_id,model) ) {
    new_stomp = new WahCC( this, model, slot );
  }
  else if ( match16(fuzz_id,model) ) {
    new_stomp = new FuzzCC( this, model, slot );
  }
  else if ( match16(fuzz_twah_id,model) ) {
    new_stomp = new FuzzTouchWahCC( this, model, slot );
  }
  else if ( match16(simple_comp_id,model) ) {
    new_stomp = new SimpleCompCC( this, model, slot );
  }
  else if ( match16(comp_id,model) ) {
    new_stomp = new CompCC( this, model, slot );
  }
  else if ( match16(range_boost_id,model) ) {
    new_stomp = new RangerCC( this, model, slot );
  }
  else if ( match16(green_box_id,model) ) {
    new_stomp = new GreenBoxCC( this, model, slot );
  }
  else if ( match16(orange_box_id,model) ) {
    new_stomp = new OrangeBoxCC( this, model, slot );
  }
  else if ( match16(black_box_id,model) ) {
    new_stomp = new BlackBoxCC( this, model, slot );
  }
  else if ( match16(big_fuzz_id,model) ) {
    new_stomp = new BigFuzzCC( this, model, slot );
  }
  else if ( match16(null_stomp_id,model) ) {
    new_stomp = new NullStompCC( this, model, 0 );
  }
  else {
    fprintf( stderr, "W - Stomp id {%x,%x} not expected\n", model[0], model[1] );
  }

  if ( new_stomp!=NULL ) {
    delete curr_stomp;
    curr_stomp = new_stomp;
  }
}


int 
Mustang::setStomp( int ord ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

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
            return 0;
        }
      }
      else {
        return 0;
      }
  }

  pthread_mutex_lock( &dsp_sync[STOMP_STATE].lock );
  buffer[FXSLOT] = curr_stomp->getSlot();
  pthread_mutex_unlock( &dsp_sync[STOMP_STATE].lock );
  
  return executeModelChange( buffer );
}


int
Mustang::stompControl( int cc, int value ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char cmd[64];
  memset( cmd, 0, 64 );
  
  pthread_mutex_lock( &dsp_sync[STOMP_STATE].lock );
  int rc = curr_stomp->dispatch( cc, value, cmd );
  pthread_mutex_unlock( &dsp_sync[STOMP_STATE].lock );

  if ( rc<0 ) return 0;
  rc = direct_control( cmd );
  return rc;
}


void 
Mustang::updateModObj( const unsigned char *data ) {

  ModCC * new_mod = NULL;

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  if ( match16(sine_chorus_id,model) ||
       match16(tri_chorus_id,model) ) {
    new_mod = new ChorusCC( this, model, slot );
  }
  else if ( match16(sine_flange_id,model) ||
            match16(tri_flange_id,model) ) {
    new_mod = new FlangerCC( this, model, slot );
  }
  else if ( match16(vibratone_id,model) ) {
    new_mod = new VibratoneCC( this, model, slot );
  }
  else if ( match16(vint_trem_id,model) ||
            match16(sine_trem_id,model) ) {
    new_mod = new TremCC( this, model, slot );
  }
  else if ( match16(ring_mod_id,model) ) {
    new_mod = new RingModCC( this, model, slot );
  }
  else if ( match16(step_filt_id,model) ) {
    new_mod = new StepFilterCC( this, model, slot );
  }
  else if ( match16(phaser_id,model) ) {
    new_mod = new PhaserCC( this, model, slot );
  }
  else if ( match16(pitch_shift_id,model) ) {
    new_mod = new PitchShifterCC( this, model, slot );
  }
  else if ( match16(m_wah_id,model) ||
            match16(m_touch_wah_id,model) ) {
    new_mod = new ModWahCC( this, model, slot );
  }
  else if ( match16(dia_pitch_id,model) ) {
    new_mod = new DiatonicShiftCC( this, model, slot );
  }
  else if ( match16(null_mod_id,model) ) {
    new_mod = new NullModCC( this, model, 0 );
  }
  else {
    fprintf( stderr, "W - Mod id {%x,%x} not expected\n", model[0], model[1] );
  }

  if ( new_mod!=NULL ) {
    delete curr_mod;
    curr_mod = new_mod;
  }
}


int 
Mustang::setMod( int ord ) {
  if ( checkOrDisableTuner() < 0 ) return -1;
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
            return 0;
        }
      }
      else {
        return 0;
      }
  }

  pthread_mutex_lock( &dsp_sync[MOD_STATE].lock );
  buffer[FXSLOT] = curr_mod->getSlot();
  pthread_mutex_unlock( &dsp_sync[MOD_STATE].lock );
  
  return executeModelChange( buffer );
}


int
Mustang::modControl( int cc, int value ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char cmd[64];
  memset( cmd, 0, 64 );
  
  pthread_mutex_lock( &dsp_sync[MOD_STATE].lock );
  int rc = curr_mod->dispatch( cc, value, cmd );
  pthread_mutex_unlock( &dsp_sync[MOD_STATE].lock );

  if ( rc<0 ) return 0;
  rc = direct_control( cmd );
  return rc;
}


void 
Mustang::updateDelayObj( const unsigned char *data ) {

  DelayCC * new_delay = NULL;
    
  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];

  if ( match16(mono_dly_id,model) ) {
    new_delay = new MonoDelayCC( this, model, slot );
  }
  else if ( match16(mono_filter_id,model) ||
            match16(st_filter_id,model) ) {
    new_delay = new EchoFilterCC( this, model, slot );
  }
  else if ( match16(mtap_dly_id,model) ) {
    new_delay = new MultitapDelayCC( this, model, slot );
  }
  else if ( match16(pong_dly_id,model) ) {
    new_delay = new PingPongDelayCC( this, model, slot );
  }
  else if ( match16(duck_dly_id,model) ) {
    new_delay = new DuckingDelayCC( this, model, slot );
  }
  else if ( match16(reverse_dly_id,model) ) {
    new_delay = new ReverseDelayCC( this, model, slot );
  }
  else if ( match16(tape_dly_id,model) ) {
    new_delay = new TapeDelayCC( this, model, slot );
  }
  else if ( match16(st_tape_dly_id,model) ) {
    new_delay = new StereoTapeDelayCC( this, model, slot );
  }
  else if ( match16(null_dly_id,model) ) {
    new_delay = new NullDelayCC( this, model, 0 );
  }
  else {
    fprintf( stderr, "W - Delay id {%x,%x} not expected\n", model[0], model[1] );
  }

  if ( new_delay!=NULL ) {
    delete curr_delay;
    curr_delay = new_delay;
  }
}


int 
Mustang::setDelay( int ord ) {
  if ( checkOrDisableTuner() < 0 ) return -1;
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
      return 0;
  }

  pthread_mutex_lock( &dsp_sync[DELAY_STATE].lock );
  buffer[FXSLOT] = curr_delay->getSlot();
  pthread_mutex_unlock( &dsp_sync[DELAY_STATE].lock );
  
  return executeModelChange( buffer );
}


int
Mustang::delayControl( int cc, int value ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char cmd[64];
  memset( cmd, 0, 64 );
  
  pthread_mutex_lock( &dsp_sync[DELAY_STATE].lock );
  int rc = curr_delay->dispatch( cc, value, cmd );
  pthread_mutex_unlock( &dsp_sync[DELAY_STATE].lock );

  if ( rc<0 ) return 0;
  rc = direct_control( cmd );
  return rc;
}


void 
Mustang::updateReverbObj( const unsigned char *data ) {

  const unsigned char *model = data + MODEL;
  const unsigned char slot = data[FXSLOT];
  
  delete curr_reverb;

  if ( match16(null_reverb_id,model) ) {
    curr_reverb = new NullReverbCC( this, model, 0 );
  }
  else {
    curr_reverb = new ReverbCC( this, model, slot );
  }
}


int 
Mustang::setReverb( int ord ) {
  if ( checkOrDisableTuner() < 0 ) return -1;
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
      return 0;
  }

  pthread_mutex_lock( &dsp_sync[REVERB_STATE].lock );
  buffer[FXSLOT] = curr_reverb->getSlot();
  pthread_mutex_unlock( &dsp_sync[REVERB_STATE].lock );
  
  return executeModelChange( buffer );
}


int
Mustang::reverbControl( int cc, int value ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char cmd[64];
  memset( cmd, 0, 64 );
  
  pthread_mutex_lock( &dsp_sync[REVERB_STATE].lock );
  int rc = curr_reverb->dispatch( cc, value, cmd );
  pthread_mutex_unlock( &dsp_sync[REVERB_STATE].lock );

  if ( rc<0 ) return 0;
  rc = direct_control( cmd );
  return rc;
}


int 
Mustang::effectToggle(int cc, int value) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char buffer[64];
  memset(buffer, 0x00, 64);

  // Logic is inverted ==> 0 is 'on'
  int toggle;
  if      ( value >= 0 && value <= 63 )  toggle = 1;
  else if ( value > 63 && value <= 127 ) toggle = 0;

  buffer[0] = 0x19;
  buffer[1] = 0xc3;

  // Translate 23..26 --> 3..6 (EFX family)
  int family = cc - 20;
  buffer[2] = family;
  buffer[3] = toggle;

  ///// Critical Section
  //
  pthread_mutex_lock( &efx_toggle_sync.lock );

  // Translate 23..26 --> 1..4 (index into dsp parms array)
  int state_index = cc - 22;
  unsigned char slot;
  switch ( state_index ) {
    case 1:
      slot = curr_stomp->getSlot();
      break;
    case 2:
      slot = curr_mod->getSlot();
      break;
    case 3:
      slot = curr_delay->getSlot();
      break;
    case 4:
      slot = curr_reverb->getSlot();
      break;
  }
  buffer[4] = slot;

  efx_toggle_sync.value = false;
  int rc = sendCmd( buffer );
  while ( rc==0 && ! efx_toggle_sync.value ) pthread_cond_wait( &efx_toggle_sync.cond, &efx_toggle_sync.lock );

  pthread_mutex_unlock( &efx_toggle_sync.lock );
  //
  /////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Efx toggle done\n" );
#endif

  return rc;
}


int 
Mustang::direct_control( unsigned char *buffer ) {
  buffer[0] = 0x05;
  buffer[1] = 0xc3;

  ///// Critical Section
  //
  pthread_mutex_lock( &cc_ack_sync.lock );

  cc_ack_sync.value = false;
  int rc = sendCmd( buffer );
  while ( rc==0 && ! cc_ack_sync.value ) pthread_cond_wait( &cc_ack_sync.cond, &cc_ack_sync.lock );

  pthread_mutex_unlock( &cc_ack_sync.lock );
  //
  //////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Direct control done\n" );
#endif

  return rc;
}


int 
Mustang::patchChange( int patch ) {
  if ( checkOrDisableTuner() < 0 ) return -1;

  unsigned char buffer[64];
  memset(buffer, 0x00, 64);

  buffer[0] = 0x1c;
  buffer[1] = 0x01;
  buffer[2] = 0x01;
  buffer[4] = patch;
  buffer[6] = 0x01;

  ////// Critical Section
  //
  pthread_mutex_lock( &pc_ack_sync.lock );

  pc_ack_sync.value = false;
  int rc = sendCmd( buffer );
  while ( rc==0 && ! pc_ack_sync.value ) pthread_cond_wait( &pc_ack_sync.cond, &pc_ack_sync.lock );

  pthread_mutex_unlock( &pc_ack_sync.lock );

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Leaving patch change\n" );
#endif

  pthread_mutex_lock( &preset_names_sync.lock );
  curr_preset_idx = patch;
  pthread_mutex_unlock( &preset_names_sync.lock );
  //
  ///////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Updated current preset index\n" );
#endif

  return rc;
}


int
Mustang::checkOrDisableTuner( void ) {
  int rc = 0;
  
  ////// Critical Section
  //
  pthread_mutex_lock( &tuner_ack_sync.lock );

  // If tuner on, disable it
  if ( true==tuner_ack_sync.value ) {
    unsigned char buffer[64];
    memset(buffer, 0x00, 64);
    buffer[0] = 0x0a;
    buffer[1] = 0x01;

    rc = sendCmd( buffer );
    if ( rc==0 ) {
      while ( true==tuner_ack_sync.value ) pthread_cond_wait( &tuner_ack_sync.cond, &tuner_ack_sync.lock );
    }
  }
  pthread_mutex_unlock( &tuner_ack_sync.lock );
  //
  //////

  return rc;
}


int 
Mustang::tunerMode( int value ) {
  int rc;
  
  unsigned char buffer[64];
  memset(buffer, 0x00, 64);

  buffer[0] = 0x0a;
  buffer[1] = 0x01;

  ////// Critical Section
  //
  pthread_mutex_lock( &tuner_ack_sync.lock );

  if ( value>63 && value<=127 && false==tuner_ack_sync.value ) {
    // Tuner on
    buffer[2] = buffer[3] = buffer[4] = 0x01;
    rc = sendCmd( buffer );
    if ( rc==0 ) {
      while ( false==tuner_ack_sync.value ) pthread_cond_wait( &tuner_ack_sync.cond, &tuner_ack_sync.lock );
    }
  }
  else if ( value>=0 && value<=63 && true==tuner_ack_sync.value ) {
    // Tuner off
    rc = sendCmd( buffer );
    if ( rc==0 ) {
      while ( true==tuner_ack_sync.value ) pthread_cond_wait( &tuner_ack_sync.cond, &tuner_ack_sync.lock );
    }
  }
  pthread_mutex_unlock( &tuner_ack_sync.lock );
  //
  //////

#ifdef DEBUG
  fprintf( stderr, "DEBUG: Done tuner toggle\n" );
#endif

  return 0;
}
