#include "mustang.h"
#include <iostream>
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

Mustang::usb_id Mustang::ids[] = {
    { OLD_USB_PID,      0x03, false },
    { NEW_USB_PID,      0xc1, false },
    { V2_USB_PID,       0x03, false },
    { MINI_USB_PID,     0x03, false },
    { FLOOR_USB_PID,    0x03, false },
    { BRONCO40_USB_PID, 0x03, false },
    { V2_III_PID,       0xc1, true },
    { V2_IV_PID,        0xc1, true },
    { 0,                0x00, false }
};
    

Mustang::Mustang()
{
    amp_hand = NULL;
    curr_amp = NULL;
    tuner_active = false;
    isV2 = false;
    
    // "apply efect" command
    memset(execute, 0x00, LENGTH);
    execute[0] = 0x1c;
    execute[1] = 0x03;

    memset(prev_array, 0x00, LENGTH*4);
    for(int i = 0; i < 4; i++)
    {
        prev_array[i][0] = 0x1c;
        prev_array[i][1] = 0x03;
        prev_array[i][6] = prev_array[i][7] = prev_array[i][21] = 0x01;
        prev_array[i][20] = 0x08;
        prev_array[i][FXSLOT] = 0xff;
    }
}

Mustang::~Mustang()
{
    this->stop_amp();
}

int Mustang::start_amp(void)
{
    int ret, received;
    static int init_value = -1;
    unsigned char array[LENGTH];

    if(amp_hand == NULL)
    {
        // initialize libusb
        ret = libusb_init(NULL);
        if (ret)
            return ret;

        for ( int idx=0; ids[idx].pid != 0; idx++ ) {
            if ( (amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, ids[idx].pid)) != NULL ) {
                init_value = ids[idx].init_value;
                isV2 = ids[idx].isV2;
                break;
            }
        }
        
        if ( init_value < 0 ) {
            // No amp found
            libusb_exit( NULL );
            fprintf( stderr, "S - No Mustang USB device found\n" );
            return -100;
        }

        // detach kernel driver
        ret = libusb_kernel_driver_active(amp_hand, 0);
        if(ret)
        {
            ret = libusb_detach_kernel_driver(amp_hand, 0);
            if(ret)
            {
                stop_amp();
                return ret;
            }
        }

        // claim the device
        ret = libusb_claim_interface(amp_hand, 0);
        if(ret)
        {
            stop_amp();
            return ret;
        }
    }

    // initialization which is needed if you want
    // to get any replies from the amp in the future
    memset(array, 0x00, LENGTH);
    array[1] = 0xc3;
    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    memset(array, 0x00, LENGTH);
    array[0] = 0x1a;
    array[1] = init_value;

    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    memset(array, 0x00, LENGTH);
    array[0] = 0xff;
    array[1] = 0xc1;
    
    // Request parameter dump from amplifier
    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    
    handle_parm_dump();
    
    return 0;
}

void Mustang::handle_parm_dump()
{
    int ret, received;
    unsigned char array[LENGTH];
    unsigned char received_data[296][LENGTH], data[7][LENGTH];
    memset(received_data, 0x00, 296*LENGTH);

    int i = 0, j = 0;

    // Count probably varies by model. Brute-force flush appears to create problems
    // so we'll need to get this right case by case.
    for(i = 0; i < 210; i++)
    {
        int rc = libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);
        if (rc) fprintf( stderr, "DEBUG: Timeout. i = %d, rc = %d\n", i, rc );
        memcpy(received_data[i], array, LENGTH);
    }
    
    // fprintf( stderr, "DEBUG: Packet count = %d\n", i );
    
    int max_to_receive;
    i > 143 ? max_to_receive = 200 : max_to_receive = 48;

    // List of preset names
    for(i = 0, j = 0; i<max_to_receive; i+=2, j++);
    
    // Current preset name, amp settings, efx settings
    for(j = 0; j < 7; i++, j++) memcpy(curr_state[j], received_data[i], LENGTH);

    updateAmpObj();
    updateReverbObj();
    updateDelayObj();
    updateModObj();
    updateStompObj();
}


int Mustang::stop_amp()
{
    int ret;

    if(amp_hand != NULL)
    {
        // release claimed interface
        ret = libusb_release_interface(amp_hand, 0);
        if(ret && (ret != LIBUSB_ERROR_NO_DEVICE))
            return ret;

        if(ret != LIBUSB_ERROR_NO_DEVICE)
        {
            // re-attach kernel driver
            ret = libusb_attach_kernel_driver(amp_hand, 0);
            if(ret)
                return ret;
        }

        // close opened interface
        libusb_close(amp_hand);
        amp_hand = NULL;

        // stop using libusb
        libusb_exit(NULL);
    }

    return 0;
}

int Mustang::tunerMode( int value )
{
    int ret, received;
    unsigned char array[LENGTH];
    memset(array, 0x00, LENGTH);

    array[0] = 0x0a;
    array[1] = 0x01;

    // This is a bit odd.  When turning ON the tuner, the amp responds
    // with a complete parameter dump reflecting all null devices
    // (basically useless).  When turning it off, we get only a single
    // response message.
    //
    if ( value > 63 && value <= 127 ) {
        // Tuner on
        array[2] = array[3] = array[4] = 0x01;
        ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);

        int i = 0;
        for(i = 0; i < 210; i++) {
            int rc = libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);
            if (rc) fprintf( stderr, "DEBUG: Timeout. i = %d, rc = %d\n", i, rc );
        }
        tuner_active = true;
    }
    else if ( value >= 0 && value <= 63 ) {
        // Tuner off
        ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
        libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);
        sleep( 1 );
        tuner_active = false;
    }

    return ret;
}


void Mustang::updateAmpObj(void) {

    int curr = curr_state[AMP_STATE][MODEL];
    AmpCC * new_amp = NULL;

    switch (curr) {
    case 0:
        // No amp
        break;

    case F57_DELUXE_ID:
    case F57_CHAMP_ID:
    case F65_DELUXE_ID:
    case F65_PRINCETON_ID:
    case F65_TWIN_ID:
    case S60S_THRIFT_ID:
        new_amp = new AmpCC(this);
        break;
    
    case F59_BASSMAN_ID:
    case BRIT_70S_ID:
        new_amp = new AmpCC1(this);
        break;
        
    case F_SUPERSONIC_ID:
        new_amp = new AmpCC2(this);
        break;
        
    case BRIT_60S_ID:
        new_amp = new AmpCC3(this);
        break;
        
    case BRIT_80S_ID:
    case US_90S_ID:
    case METAL_2K_ID:
    case BRIT_WATT_ID:
        new_amp = new AmpCC4(this);
        break;
        
    case STUDIO_PREAMP_ID:
        new_amp = new AmpCC5(this);
        break;

    case BRIT_COLOR_ID:
        new_amp = new AmpCC6(this);
        break;
        
    case F57_TWIN_ID:
        new_amp = new AmpCC7(this);
        break;
        
    default:
        fprintf( stderr, "W - Amp id %x not supported yet\n", curr );
        break;
    }

    if ( new_amp!=NULL ) {
        delete curr_amp;
        curr_amp = new_amp;
    }
}


void Mustang::updateReverbObj(void) {

    int curr = curr_state[REVERB_STATE][MODEL];
    
    switch (curr) {
    case 0:
        break;

    default:
        delete curr_reverb;
        curr_reverb = new ReverbCC( this );
        break;
    }
}


void Mustang::updateDelayObj(void) {

    int curr = curr_state[DELAY_STATE][MODEL];
    DelayCC * new_delay = NULL;
    
    switch (curr) {
    case 0:
        break;
        
    case MONO_DLY_ID:
        new_delay = new MonoDelayCC(this);
        break;
    
    case MONO_FILTER_ID:
    case ST_FILTER_ID:
        new_delay = new EchoFilterCC(this);
        break;
        
    case MTAP_DLY_ID:
        new_delay = new MultitapDelayCC(this);
        break;
        
    case PONG_DLY_ID:
        new_delay = new PingPongDelayCC(this);
        break;
        
    case DUCK_DLY_ID:
        new_delay = new DuckingDelayCC(this);
        break;

    case REVERSE_DLY_ID:
        new_delay = new ReverseDelayCC(this);
        break;
        
    case TAPE_DLY_ID:
        new_delay = new TapeDelayCC(this);
        break;
        
    case ST_TAPE_DLY_ID:
        new_delay = new StereoTapeDelayCC(this);
        break;
        
    default:
        fprintf( stderr, "W - Delay id %x not supported yet\n", curr );
        break;
    }

    if ( new_delay!=NULL ) {
        delete curr_delay;
        curr_delay = new_delay;
    }
}


void Mustang::updateModObj(void) {

    int curr = curr_state[MOD_STATE][MODEL];
    ModCC * new_mod = NULL;
    
    switch (curr) {
    case 0:
        break;
        
    case SINE_CHORUS_ID:
    case TRI_CHORUS_ID:
        new_mod = new ChorusCC(this);
        break;
    
    case SINE_FLANGE_ID:
    case TRI_FLANGE_ID:
        new_mod = new FlangerCC(this);
        break;
        
    case VIBRATONE_ID:
        new_mod = new VibratoneCC(this);
        break;
        
    case VINT_TREM_ID:
    case SINE_TREM_ID:
        new_mod = new TremCC(this);
        break;
        
    case RING_MOD_ID:
        new_mod = new RingModCC(this);
        break;

    case STEP_FILT_ID:
        new_mod = new StepFilterCC(this);
        break;
        
    case PHASER_ID:
        new_mod = new PhaserCC(this);
        break;
        
    case PITCH_SHIFT_ID:
    {
        int xtra = curr_state[MOD_STATE][MODELX];
        if      ( xtra == 0 )    new_mod = new PitchShifterCC(this);
        else if ( xtra == 0x10 ) new_mod = new DiatonicShiftCC(this);
    }
    break;
        
    case M_WAH_ID:
    case M_TOUCH_WAH_ID:
        new_mod = new ModWahCC(this);
        break;
        
    default:
        fprintf( stderr, "W - Mod id %x not supported yet\n", curr );
        break;
    }

    if ( new_mod!=NULL ) {
        delete curr_mod;
        curr_mod = new_mod;
    }
}


void Mustang::updateStompObj(void) {

    int curr = curr_state[STOMP_STATE][MODEL];
    StompCC * new_stomp = NULL;
    
    switch (curr) {
    case 0:
        break;
        
    case OVERDRIVE_ID:
        new_stomp = new OverdriveCC(this);
        break;
    
    case WAH_ID:
    case TOUCH_WAH_ID:
        new_stomp = new WahCC(this);
        break;
        
    case FUZZ_ID:
        new_stomp = new FuzzCC(this);
        break;
        
    case FUZZ_TWAH_ID:
        new_stomp = new FuzzTouchWahCC(this);
        break;
        
    case SIMPLE_COMP_ID:
        new_stomp = new SimpleCompCC(this);
        break;

    case COMP_ID:
        new_stomp = new CompCC(this);
        break;

    case RANGE_BOOST_ID:
        new_stomp = new RangerCC(this);
        break;
        
    case GREEN_BOX_ID:
        new_stomp = new GreenBoxCC(this);
        break;
        
    case ORANGE_BOX_ID:
        new_stomp = new OrangeBoxCC(this);
        break;
        
    case BLACK_BOX_ID:
        new_stomp = new BlackBoxCC(this);
        break;
        
    case BIG_FUZZ_ID:
        new_stomp = new BigFuzzCC(this);
        break;

    default:
        fprintf( stderr, "W - Stomp id %x not supported yet\n", curr );
        break;
    }

    if ( new_stomp!=NULL ) {
        delete curr_stomp;
        curr_stomp = new_stomp;
    }
}


int Mustang::effect_toggle(int cc, int value)
{
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char array[LENGTH];

    // Translate 23..26 --> 2..5 (current state index)
    int state_index = cc - 21;
    int state;
    if      ( value >= 0 && value <= 63 )  state = 1;
    else if ( value > 63 && value <= 127 ) state = 0;

    memset(array, 0x00, LENGTH);
    array[0] = 0x19;
    array[1] = 0xc3;
    // Translate DSP to family
    array[FAMILY] = curr_state[state_index][DSP] - 3;
    // Invert logic
    array[ACTIVE_INVERT] = state;
    array[FXSLOT] = curr_state[state_index][FXSLOT];
#if 0    
    for ( int i=0; i<15; i++ ) fprintf( stderr, "%02x ", array[i] );
    fprintf( stderr, "\n" );
#endif
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);

    // Note: Toggle gets three response packets
    for (int i=0; i < 3; i++) {
        libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);
    }

    return ret;
}

int Mustang::setAmp( int ord ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
    case 0:
        array = amp_none;
        break;
    case 1:
        array = f57_deluxe;
        break;
    case 2:
        array = f59_bassman;
        break;
    case 3:
        array = f57_champ;
        break;
    case 4:
        array = f65_deluxe;
        break;
    case 5:
        array = f65_princeton;
        break;
    case 6:
        array = f65_twin;
        break;
    case 7:
        array = f_supersonic;
        break;
    case 8:
        array = brit_60;
        break;
    case 9:
        array = brit_70;
        break;
    case 10:
        array = brit_80;
        break;
    case 11:
        array = us_90;
        break;
    case 12:
        array = metal_2k;
        break;
    default:
        if ( isV2 ) {
            switch (ord) {
            case 13:
                array = studio_preamp;
                break;
            case 14:
                array = f57_twin;
                break;
            case 15:
                array = sixties_thrift;
                break;
            case 16:
                array = brit_watts;
                break;
            case 17:
                array = brit_color;
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
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[AMP_STATE], array, LENGTH);
    updateAmpObj();

    // Setup USB gain
    // memset(scratch, 0x00, LENGTH);
    // scratch[0] = 0x1c;
    // scratch[1] = 0x03;
    // scratch[2] = 0x0d;
    // scratch[6] = 0x01;
    // scratch[7] = 0x01;
    // scratch[16] = 0x80;

    // ret = libusb_interrupt_transfer(amp_hand, 0x01, scratch, LENGTH, &received, TMOUT);
    // libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    // libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    return ret;
}

int Mustang::setReverb( int ord ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
    case 0:
        array = reverb_none;
        break;
    case 1:
        array = small_hall;
        break;
    case 2:
        array = large_hall;
        break;
    case 3:
        array = small_room;
        break;
    case 4:
        array = large_room;
        break;
    case 5:
        array = small_plate;
        break;
    case 6:
        array = large_plate;
        break;
    case 7:
        array = ambient;
        break;
    case 8:
        array = arena;
        break;
    case 9:
        array = spring_63;
        break;
    case 10:
        array = spring_65;
        break;
    default:
        fprintf( stderr, "W - Reverb select %d not supported\n", ord );
        return 0;
    }

    array[FXSLOT] = curr_state[REVERB_STATE][FXSLOT];

    // Setup amp personality
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[REVERB_STATE], array, LENGTH);
    updateReverbObj();

    return ret;
}

int Mustang::setDelay( int ord ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
    case 0:
        array = delay_none;
        break;
    case 1:
        array = mono_delay;
        break;
    case 2:
        array = mono_echo_filter;
        break;
    case 3:
        array = stereo_echo_filter;
        break;
    case 4:
        array = multitap_delay;
        break;
    case 5:
        array = ping_pong_delay;
        break;
    case 6:
        array = ducking_delay;
        break;
    case 7:
        array = reverse_delay;
        break;
    case 8:
        array = tape_delay;
        break;
    case 9:
        array = stereo_tape_delay;
        break;
    default:
        fprintf( stderr, "W - Delay select %d not supported\n", ord );
        return 0;
    }

    array[FXSLOT] = curr_state[DELAY_STATE][FXSLOT];

    // Setup amp personality
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[DELAY_STATE], array, LENGTH);
    updateDelayObj();

    return ret;
}

int Mustang::setMod( int ord ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
    case 0:
        array = mod_none;
        break;
    case 1:
        array = sine_chorus;
        break;
    case 2:
        array = triangle_chorus;
        break;
    case 3:
        array = sine_flanger;
        break;
    case 4:
        array = triangle_flanger;
        break;
    case 5:
        array = vibratone;
        break;
    case 6:
        array = vintage_tremolo;
        break;
    case 7:
        array = sine_tremolo;
        break;
    case 8:
        array = ring_modulator;
        break;
    case 9:
        array = step_filter;
        break;
    case 10:
        array = phaser;
        break;
    case 11:
        array = pitch_shifter;
        break;
    default:
        if ( isV2 ) {
            switch (ord) {
            case 12:
                array = mod_wah;
                break;
            case 13:
                array = mod_touch_wah;
                break;
            case 14:
                array = diatonic_pitch_shift;
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

    array[FXSLOT] = curr_state[MOD_STATE][FXSLOT];

    // Setup amp personality
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[MOD_STATE], array, LENGTH);
    updateModObj();

    return ret;
}

int Mustang::setStomp( int ord ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
    case 0:
        array = stomp_none;
        break;
    case 1:
        array = overdrive;
        break;
    case 2:
        array = wah;
        break;
    case 3:
        array = touch_wah;
        break;
    case 4:
        array = fuzz;
        break;
    case 5:
        if ( isV2 ) {
            fprintf( stderr, "W - Stomp select %d not supported\n", ord );
            return 0;
        }
        else {
            array = fuzz_touch_wah;
        }
        break;
    case 6:
        array = simple_comp;
        break;
    case 7:
        array = compressor;
        break;
    default:
        if ( isV2 ) {
            switch (ord) {
            case 8:
                array = ranger_boost;
                break;
            case 9:
                array = green_box;
                break;
            case 10:
                array = orange_box;
                break;
            case 11:
                array = black_box;
                break;
            case 12:
                array = big_fuzz;
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

    array[FXSLOT] = curr_state[STOMP_STATE][FXSLOT];

    // Setup amp personality
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[STOMP_STATE], array, LENGTH);
    updateStompObj();

    return ret;
}

int Mustang::save_on_amp(char *name, int slot)
{
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char array[LENGTH];

    memset(array, 0x00, LENGTH);
    array[0] = 0x1c;
    array[1] = 0x01;
    array[2] = 0x03;
    array[SAVE_SLOT] = slot;
    array[6] = 0x01;
    array[7] = 0x01;

    if(strlen(name) > 31)
        name[31] = 0x00;

    for(unsigned int i = 16, j = 0; name[j] != 0x00; i++,j++)
        array[i] = name[j];

    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    load_memory_bank(slot);

    return ret;
}

int Mustang::continuous_control( const Mustang::Cmd & cmd ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char array[LENGTH];
    
    memset(array, 0x00, LENGTH);
    array[0] = 0x05;
    array[1] = 0xc3;
    array[2] = cmd.parm2;
    array[3] = curr_state[cmd.state_index][MODEL];

    // target parameter
    array[5] = cmd.parm5;
    array[6] = cmd.parm6;
    array[7] = cmd.parm7;
    
    // Scale and clamp to valid index range
    int index = (int) ceil( (double)cmd.value * magic_scale_factor );
    if ( index > magic_max ) index = magic_max;
    
    unsigned short eff_value = magic_values[index];

    array[9] = eff_value & 0xff;
    array[10] = (eff_value >> 8) & 0xff;

    // Send command and flush reply
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    return ret;
}


int Mustang::discrete_control( const Mustang::Cmd & cmd ) {
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char array[LENGTH];
    
    memset(array, 0x00, LENGTH);
    array[0] = 0x05;
    array[1] = 0xc3;
    array[2] = cmd.parm2;
    array[3] = curr_state[cmd.state_index][MODEL];

    array[5] = cmd.parm5;
    array[6] = cmd.parm6;
    array[7] = cmd.parm7;

    // Discrete value
    array[9] = cmd.value;

    // Send command and flush reply
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    return ret;
}


int Mustang::load_memory_bank( int slot )
{
    if ( tuner_active ) return 0;

    int ret, received;
    unsigned char array[LENGTH], data[7][LENGTH];

    memset(array, 0x00, LENGTH);
    array[0] = 0x1c;
    array[1] = 0x01;
    array[2] = 0x01;
    array[SAVE_SLOT] = slot;
    array[6] = 0x01;

    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);

    // Mustang III has nine responses
    for(int i=0; i < 9; i++) {
        libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);
        int dsp = array[2];
        if ( dsp >= 4 && dsp <= 9 )
            memcpy(curr_state[dsp - 4], array, LENGTH);
    }
    updateAmpObj();
    updateReverbObj();
    updateDelayObj();
    updateModObj();
    updateStompObj();
    
    return ret;
}

int Mustang::set_effect(struct fx_pedal_settings value)
{
    int ret, received;    // variables used when sending
    unsigned char slot;    // where to put the effect
    unsigned char temp[LENGTH], array[LENGTH] = {
        0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // clear effect on previous DSP before setting a new one
    int k=0;
    for (int i = 0; i < 4; i++)
    {
        if (prev_array[i][FXSLOT] == value.fx_slot || prev_array[i][FXSLOT] == (value.fx_slot+4))
        {
            memcpy(array, prev_array[i], LENGTH);
            prev_array[i][FXSLOT] = 0xff;
            k++;
            break;
        }
    }
    array[EFFECT] = 0x00;
    array[KNOB1] = 0x00;
    array[KNOB2] = 0x00;
    array[KNOB3] = 0x00;
    array[KNOB4] = 0x00;
    array[KNOB5] = 0x00;
    array[KNOB6] = 0x00;
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);

    if(value.effect_num == EMPTY)
        return ret;

    if(value.put_post_amp)    // put effect in a slot after amplifier
        slot = value.fx_slot + 4;
    else
        slot = value.fx_slot;

    // fill the form with data
    array[FXSLOT] = slot;
    array[KNOB1] = value.knob1;
    array[KNOB2] = value.knob2;
    array[KNOB3] = value.knob3;
    array[KNOB4] = value.knob4;
    array[KNOB5] = value.knob5;
    // some effects have more knobs
    if (value.effect_num == MONO_ECHO_FILTER ||
            value.effect_num == STEREO_ECHO_FILTER ||
            value.effect_num == TAPE_DELAY ||
            value.effect_num == STEREO_TAPE_DELAY)
    {
        array[KNOB6] = value.knob6;
    }

    // fill the form with missing data
    switch (value.effect_num) {
    case OVERDRIVE:
        array[DSP] = 0x06;
        array[EFFECT] = 0x3c;
        break;

    case WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x49;
        array[19] = 0x01;
        break;

    case TOUCH_WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x4a;
        array[19] = 0x01;
        break;

    case FUZZ:
        array[DSP] = 0x06;
        array[EFFECT] = 0x1a;
        break;

    case FUZZ_TOUCH_WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x1c;
        break;

    case SIMPLE_COMP:
        array[DSP] = 0x06;
        array[EFFECT] = 0x88;
        array[19] = 0x08;
        if(array[KNOB1] > 0x03)
        {
            array[KNOB1] = 0x03;
        }
        array[KNOB2] = 0x00;
        array[KNOB3] = 0x00;
        array[KNOB4] = 0x00;
        array[KNOB5] = 0x00;
        break;

    case COMPRESSOR:
        array[DSP] = 0x06;
        array[EFFECT] = 0x07;
        break;

    case SINE_CHORUS:
        array[DSP] = 0x07;
        array[EFFECT] = 0x12;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case TRIANGLE_CHORUS:
        array[DSP] = 0x07;
        array[EFFECT] = 0x13;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case SINE_FLANGER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x18;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case TRIANGLE_FLANGER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x19;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case VIBRATONE:
        array[DSP] = 0x07;
        array[EFFECT] = 0x2d;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case VINTAGE_TREMOLO:
        array[DSP] = 0x07;
        array[EFFECT] = 0x40;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case SINE_TREMOLO:
        array[DSP] = 0x07;
        array[EFFECT] = 0x41;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case RING_MODULATOR:
        array[DSP] = 0x07;
        array[EFFECT] = 0x22;
        array[19] = 0x01;
        if(array[KNOB4] > 0x01)
        {
            array[KNOB4] = 0x01;
        }
        break;

    case STEP_FILTER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x29;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case PHASER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x4f;
        array[19] = 0x01;
        array[20] = 0x01;
        if(array[KNOB5] > 0x01)
        {
            array[KNOB5] = 0x01;
        }
        break;

    case PITCH_SHIFTER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x1f;
        array[19] = 0x01;
        break;

    case MONO_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x16;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case MONO_ECHO_FILTER:
        array[DSP] = 0x08;
        array[EFFECT] = 0x43;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case STEREO_ECHO_FILTER:
        array[DSP] = 0x08;
        array[EFFECT] = 0x48;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case MULTITAP_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x44;
        array[19] = 0x02;
        array[20] = 0x01;
        if(array[KNOB5] > 0x03)
        {
            array[KNOB5] = 0x03;
        }
        break;

    case PING_PONG_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x45;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case DUCKING_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x15;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case REVERSE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x46;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case TAPE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x2b;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case STEREO_TAPE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x2a;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case SMALL_HALL_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x24;
        break;

    case LARGE_HALL_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x3a;
        break;

    case SMALL_ROOM_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x26;
        break;

    case LARGE_ROOM_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x3b;
        break;

    case SMALL_PLATE_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4e;
        break;

    case LARGE_PLATE_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4b;
        break;

    case AMBIENT_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4c;
        break;

    case ARENA_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4d;
        break;

    case FENDER_63_SPRING_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x21;
        break;

    case FENDER_65_SPRING_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x0b;
        break;
    }

    // send packet to the amp
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);

    // save current settings
    memcpy(prev_array[array[DSP]-6], array, LENGTH);

    return ret;
}

int Mustang::save_effects(int slot, char name[24], int number_of_effects, struct fx_pedal_settings effects[2])
{
    int ret, received;
    unsigned char fxknob, repeat;
    unsigned char temp[LENGTH], array[LENGTH] = {
        0x1c, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    execute[2]=0x00;  // why this must be here?

    if(number_of_effects > 2)
        repeat = 1;
    else
        repeat = number_of_effects;

    for(int i = 0; i < repeat; i++)
        if(effects[i].effect_num < SINE_CHORUS)
            return -1;

    if(effects[0].effect_num>=SINE_CHORUS && effects[0].effect_num<=PITCH_SHIFTER)
    {
        fxknob = 0x01;
        repeat = 1;  //just to be sure
    }
    else
        fxknob = 0x02;
    array[FXKNOB] = fxknob;

    array[SAVE_SLOT] = slot;

    // set and send the name
    if(name[24] != 0x00)
        name[24] = 0x00;
    for(int i = 0, j = 16; name[i] != 0x00; i++, j++)
        array[j] = name[i];
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);

    array[1] = 0x03;
    array[6] = 0x00;
    memset(array+16, 0x00, LENGTH-16);
    for(int i = 0; i < repeat; i++)
    {
        array[19] = 0x00;
        array[20] = 0x08;
        array[21] = 0x01;
        array[KNOB6] = 0x00;

        if(effects[i].put_post_amp)
            array[FXSLOT] = effects[i].fx_slot+4;
        else
            array[FXSLOT] = effects[i].fx_slot;
        array[KNOB1] = effects[i].knob1;
        array[KNOB2] = effects[i].knob2;
        array[KNOB3] = effects[i].knob3;
        array[KNOB4] = effects[i].knob4;
        array[KNOB5] = effects[i].knob5;
        // some effects have more knobs
        if (effects[i].effect_num == MONO_ECHO_FILTER ||
                effects[i].effect_num == STEREO_ECHO_FILTER ||
                effects[i].effect_num == TAPE_DELAY ||
                effects[i].effect_num == STEREO_TAPE_DELAY)
        {
            array[KNOB6] = effects[i].knob6;
        }

        // fill the form with missing data
        switch (effects[i].effect_num) {
        case SINE_CHORUS:
            array[DSP] = 0x07;
            array[EFFECT] = 0x12;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case TRIANGLE_CHORUS:
            array[DSP] = 0x07;
            array[EFFECT] = 0x13;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case SINE_FLANGER:
            array[DSP] = 0x07;
            array[EFFECT] = 0x18;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case TRIANGLE_FLANGER:
            array[DSP] = 0x07;
            array[EFFECT] = 0x19;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case VIBRATONE:
            array[DSP] = 0x07;
            array[EFFECT] = 0x2d;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case VINTAGE_TREMOLO:
            array[DSP] = 0x07;
            array[EFFECT] = 0x40;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case SINE_TREMOLO:
            array[DSP] = 0x07;
            array[EFFECT] = 0x41;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case RING_MODULATOR:
            array[DSP] = 0x07;
            array[EFFECT] = 0x22;
            array[19] = 0x01;
            if(array[KNOB4] > 0x01)
                array[KNOB4] = 0x01;
            break;

        case STEP_FILTER:
            array[DSP] = 0x07;
            array[EFFECT] = 0x29;
            array[19] = 0x01;
            array[20] = 0x01;
            break;

        case PHASER:
            array[DSP] = 0x07;
            array[EFFECT] = 0x4f;
            array[19] = 0x01;
            array[20] = 0x01;
            if(array[KNOB5] > 0x01)
                array[KNOB5] = 0x01;
            break;

        case PITCH_SHIFTER:
            array[DSP] = 0x07;
            array[EFFECT] = 0x1f;
            array[19] = 0x01;
            break;

        case MONO_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x16;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case MONO_ECHO_FILTER:
            array[DSP] = 0x08;
            array[EFFECT] = 0x43;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case STEREO_ECHO_FILTER:
            array[DSP] = 0x08;
            array[EFFECT] = 0x48;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case MULTITAP_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x44;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case PING_PONG_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x45;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case DUCKING_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x15;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case REVERSE_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x46;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case TAPE_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x2b;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case STEREO_TAPE_DELAY:
            array[DSP] = 0x08;
            array[EFFECT] = 0x2a;
            array[19] = 0x02;
            array[20] = 0x01;
            break;

        case SMALL_HALL_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x24;
            break;

        case LARGE_HALL_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x3a;
            break;

        case SMALL_ROOM_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x26;
            break;

        case LARGE_ROOM_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x3b;
            break;

        case SMALL_PLATE_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x4e;
            break;

        case LARGE_PLATE_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x4b;
            break;

        case AMBIENT_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x4c;
            break;

        case ARENA_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x4d;
            break;

        case FENDER_63_SPRING_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x21;
            break;

        case FENDER_65_SPRING_REVERB:
            array[DSP] = 0x09;
            array[EFFECT] = 0x0b;
            break;
        }
        // send packet
        ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
        libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);
    }

    execute[FXKNOB] = fxknob;
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &received, TMOUT);
    execute[FXKNOB] = 0x00;

    return 0;
}

