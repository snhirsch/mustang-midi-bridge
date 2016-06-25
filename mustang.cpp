#include "mustang.h"
#include <iostream>
#include <cmath>

#include "magic.h"
#include "amp_defaults.h"
#include "reverb_defaults.h"

Mustang::Mustang()
{
    amp_hand = NULL;
    curr_amp = NULL;

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
    unsigned char array[LENGTH];
    unsigned char received_data[296][LENGTH], data[7][LENGTH];
    memset(received_data, 0x00, 296*LENGTH);

    if(amp_hand == NULL)
    {
        // initialize libusb
        ret = libusb_init(NULL);
        if (ret)
            return ret;

        // get handle for the device
        if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, OLD_USB_PID)) == NULL)
            if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, NEW_USB_PID)) == NULL)
                if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, V2_USB_PID)) == NULL)
                  if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, MINI_USB_PID)) == NULL)
                    if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, FLOOR_USB_PID)) == NULL)
                        if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, BRONCO40_USB_PID)) == NULL)
                          if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, V2_III_PID)) == NULL)
                            if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, V2_IV_PID)) == NULL)
                    {
                      libusb_exit(NULL);
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

    // This seems model specific
    // Perhaps for Mustang II?
    //  array[1] = 0x03;

    // Correct value for Mustang III
    array[1] = 0xc1;

    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    int i = 0, j = 0;

    memset(array, 0x00, LENGTH);
    array[0] = 0xff;
    array[1] = 0xc1;
    
    // Request parameter dump from amplifier
    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    
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

    updateAmp();
    curr_reverb = new ReverbCC( this );

    return 0;
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

void Mustang::updateAmp(void) {

    int curr = curr_state[AMP_STATE][AMPLIFIER];
    switch (curr) {
    case F57_DELUXE_ID:
    case F57_CHAMP_ID:
    case F65_DELUXE_ID:
    case F65_PRINCETON_ID:
    case F65_TWIN_ID:
        delete curr_amp;
        curr_amp = new AmpCC(this);
        break;
    
    case F59_BASSMAN_ID:
    case BRIT_70S_ID:
        delete curr_amp;
        curr_amp = new AmpCC1(this);
        break;
        
    case F_SUPERSONIC_ID:
        delete curr_amp;
        curr_amp = new AmpCC2(this);
        break;
        
    case BRIT_60S_ID:
        delete curr_amp;
        curr_amp = new AmpCC3(this);
        break;
        
    case BRIT_80S_ID:
    case US_90S_ID:
    case METAL_2K_ID:
        delete curr_amp;
        curr_amp = new AmpCC4(this);
        break;
        
    case STUDIO_PREAMP_ID:
        delete curr_amp;
        curr_amp = new AmpCC5(this);
        break;
        
    default:
        fprintf( stderr, "W - Amp id %x not supported yet\n", curr );
        break;
    }
}


void Mustang::updateReverb(void) {
  // No-op for now
}


int Mustang::effect_toggle(int cc, int value)
{
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

int Mustang::control_common1(int parm, int bucket, int value)
{
    static unsigned short previous = 0;

    int ret, received;
    unsigned char array[LENGTH];
    
    memset(array, 0x00, LENGTH);
    array[0] = 0x05;
    array[1] = 0xc3;
    array[2] = 0x02;
    array[3] = curr_state[AMP_STATE][AMPLIFIER];
    // target parameter
    array[5] = array[6] = parm;
    // bucket 
    array[7] = bucket;

    // Scale and clamp to valid index range
    int index = (int) ceil( (double)value * magic_scale_factor );
    if ( index > magic_max ) index = magic_max;
    
    unsigned short eff_value = magic_values[index];

    // Try to prevent extra traffic if successive calls resolve to the same
    // slot.
    if (eff_value == previous) return 0;
    previous = eff_value;

    array[9] = eff_value & 0xff;
    array[10] = (eff_value >> 8) & 0xff;

    // Send command and flush reply
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    return ret;
}


int Mustang::control_common2(int parm, int bucket, int value)
{
    int ret, received;
    unsigned char array[LENGTH];
    
    memset(array, 0x00, LENGTH);
    array[0] = 0x05;
    array[1] = 0xc3;
    array[2] = 0x02;
    array[3] = curr_state[AMP_STATE][AMPLIFIER];
    // target parameter
    array[5] = array[6] = parm;
    // bucket 
    array[7] = bucket;
    // value
    array[9] = value;
    
    // Send command and flush reply
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    return ret;
}

int Mustang::setAmp( int ord ) {
    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
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
        fprintf( stderr, "W - Amp select %d not yet supported\n", ord );
        return 0;
    }

    // Setup amp personality
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    // Copy to current setting store
    memcpy(curr_state[AMP_STATE], array, LENGTH);
    updateAmp();

    // Setup USB gain
    memset(scratch, 0x00, LENGTH);
    scratch[0] = 0x1c;
    scratch[1] = 0x03;
    scratch[2] = 0x0d;
    scratch[6] = 0x01;
    scratch[7] = 0x01;
    scratch[16] = 0x80;

    ret = libusb_interrupt_transfer(amp_hand, 0x01, scratch, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, scratch, LENGTH, &received, TMOUT);

    return ret;
}

int Mustang::setReverb( int ord ) {
    int ret, received;
    unsigned char scratch[LENGTH];
    
    unsigned char *array;

    switch (ord) {
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
    updateReverb();

    return ret;
}

int Mustang::save_on_amp(char *name, int slot)
{
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

int Mustang::efx_common1(int parm, int bucket, int type, int value)
{
    static unsigned short previous = 0;

    int ret, received;
    unsigned char array[LENGTH];
    
    memset(array, 0x00, LENGTH);
    array[0] = 0x05;
    array[1] = 0xc3;
    array[2] = 0x06;
    array[3] = curr_state[type][EFFECT];
    // target parameter
    array[5] = array[6] = parm;
    // bucket 
    array[7] = bucket;

    // Scale and clamp to valid index range
    int index = (int) ceil( (double)value * magic_scale_factor );
    if ( index > magic_max ) index = magic_max;
    
    unsigned short eff_value = magic_values[index];

    // Try to prevent extra traffic if successive calls resolve to the same
    // slot.
    if (eff_value == previous) return 0;
    previous = eff_value;

    array[9] = eff_value & 0xff;
    array[10] = (eff_value >> 8) & 0xff;

    // Send command and flush reply
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &received, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &received, TMOUT);

    return ret;
}


int Mustang::load_memory_bank( int slot )
{
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
        if(i < 7)
            memcpy(curr_state[i], array, LENGTH);
    }
    updateAmp();
    
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

