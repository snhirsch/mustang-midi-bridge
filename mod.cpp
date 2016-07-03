
#include "mod.h"
#include "mustang.h"

int 
ModCC::continuous_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = MOD_STATE;
  cmd.parm2 = 0x04;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;
  
  return amp->continuous_control( cmd );
}

int 
ModCC::discrete_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = MOD_STATE;
  cmd.parm2 = 0x04;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;

  return amp->discrete_control( cmd );
}

int
ModCC::dispatch( int cc, int value ) {

  switch ( cc ) {
  case 39:
    // Level / Mix
    return cc39( value );
    break;
  case 40:
    // Rate / Rotor / Freq / Pitch
    return cc40( value );
    break;
  case 41:
    // Depth / Duty Cycle / Resonance / Detune / Heel Freq
    return cc41( value );
    break;
  case 42:
    // Delay / Fdbk / LFO / Min Freq / Feedback / Toe Freq
    return cc42( value );
    break;
  case 43:
    // LR Phase / Release / Tri Shape / PreDelay / High Q 
    return cc43( value );
    break;
  default:
    return 0;
    break;
  }
}

