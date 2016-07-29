// -*-c++-*-

#ifndef _MOD_H
#define _MOD_H

#include <cstring>

class Mustang;

class ModCC {

protected:
  Mustang * amp;
  unsigned char model[2];
  unsigned char slot;

  int continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );
  int discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );

public:
  ModCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : 
    amp(theAmp), 
    slot(theSlot) 
  {
    memcpy( this->model, model, 2 );
  }

  int dispatch( int cc, int value, unsigned char *cmd );
  const unsigned char *getModel( void ) { return model;}
  const unsigned char getSlot( void ) { return slot;}

private:
  virtual int cc39( int value, unsigned char *cmd ) = 0;
  virtual int cc40( int value, unsigned char *cmd ) = 0;
  virtual int cc41( int value, unsigned char *cmd ) = 0;
  virtual int cc42( int value, unsigned char *cmd ) = 0;
  virtual int cc43( int value, unsigned char *cmd ) = 0;
 };


class ChorusCC : public ModCC {
public:
  ChorusCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rate
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x10, value, cmd );}
  // Depth
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Avg Delay
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // LR Phase
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class FlangerCC : public ModCC {
public:
  FlangerCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rate
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x10, value, cmd );}
  // Depth
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Feedback
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // LR Phase
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class VibratoneCC : public ModCC {
public:
  VibratoneCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rotor Speed
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x0f, value, cmd );}
  // Depth
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Feedback
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // LR Phase
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class TremCC : public ModCC {
public:
  TremCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rotor Speed
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x0e, value, cmd );}
  // Duty Cycle
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Attack / LFO Clip
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Release / Tri Shape
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class RingModCC : public ModCC {
public:
  RingModCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Freq
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x01, value, cmd );}
  // Depth
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // LFO Depth
  virtual int cc42( int value, unsigned char *cmd ) {
    if ( value > 1 ) return -1;
    else                          return discrete_control( 0x03, 0x03, 0x8c, value, cmd );
  }
  // LFO Phase
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class StepFilterCC : public ModCC {
public:
  StepFilterCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rate
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x10, value, cmd );}
  // Resonance
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Min Freq
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Max Freq
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};


class PhaserCC : public ModCC {
public:
  PhaserCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Rate
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x10, value, cmd );}
  // Depth
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Feedback
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // LFO Shape
  virtual int cc43( int value, unsigned char *cmd ) {
    if ( value > 1 ) return -1;
    else             return discrete_control( 0x04, 0x04, 0x8c, value, cmd );
  }
};


class PitchShifterCC : public ModCC {
public:
  PitchShifterCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Level
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Pitch
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x01, value, cmd );}
  // Detune
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Feedback
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Predelay
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
};

// Wah + Touch Wah
class ModWahCC : public ModCC {
public:
  ModWahCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Mix
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Freq
  virtual int cc40( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x01, value, cmd );}
  // Heel Freq
  virtual int cc41( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Toe Freq
  virtual int cc42( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Hi-Q
  virtual int cc43( int value, unsigned char *cmd ) { 
    if ( value > 1 ) return -1;
    else             return discrete_control( 0x04, 0x04, 0x81, value, cmd );
  }
};


class DiatonicShiftCC : public ModCC {
public:
  DiatonicShiftCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  // Mix
  virtual int cc39( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Pitch
  virtual int cc40( int value, unsigned char *cmd ) { 
    if ( value > 0x15 ) return -1;
    else                return discrete_control( 0x01, 0x0b, 0x98, value, cmd );
  }
  // Key
  virtual int cc41( int value, unsigned char *cmd ) { 
    if ( value > 0x0b ) return -1;
    else                return discrete_control( 0x02, 0x02, 0x99, value, cmd );
  }
  // Scale
  virtual int cc42( int value, unsigned char *cmd ) { 
    if ( value > 8 ) return -1;
    else             return discrete_control( 0x03, 0x03, 0x9a, value, cmd );
  }
  // Tone
  virtual int cc43( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x07, 0x01, value, cmd );}
};


class NullModCC : public ModCC {
public:
  NullModCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ModCC(theAmp,model,theSlot) {}
private:
  virtual int cc39( int value, unsigned char *cmd ) { return -1;}
  virtual int cc40( int value, unsigned char *cmd ) { return -1;}
  virtual int cc41( int value, unsigned char *cmd ) { return -1;}
  virtual int cc42( int value, unsigned char *cmd ) { return -1;}
  virtual int cc43( int value, unsigned char *cmd ) { return -1;}
};


#endif
