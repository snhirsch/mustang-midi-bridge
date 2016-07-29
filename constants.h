#ifndef CONSTANTS_H
#define CONSTANTS_H

// USB vendor id
#define FENDER_VID   0x1ed8

// USB product ids
#define MI_II_V1     0x0004
#define MIII_IV_V_V1 0x0005
#define M_BRONCO_40  0x000a
#define M_MINI       0x0010
#define M_FLOOR      0x0012
#define MI_II_V2     0x0014
#define MIII_IV_V_V2 0x0016

#define FXSLOT 18

// Offset to current device model for any state structure
#define MODEL        16

// Index into current state structure
#define AMP_STATE    0
#define STOMP_STATE  1
#define MOD_STATE    2
#define DELAY_STATE  3
#define REVERB_STATE 4
#define PEDAL_STATE  5

// Reverb model id values
#define SM_HALL_ID       0x0024
#define LG_HALL_ID       0x003a
#define SM_ROOM_ID       0x0026
#define LG_ROOM_ID       0x003b
#define SM_PLATE_ID      0x004e
#define LG_PLATE_ID      0x004b
#define AMBIENT_ID       0x004c
#define ARENA_ID         0x004d
#define SPRING_63_ID     0x0021
#define SPRING_65_ID     0x000b

#endif
