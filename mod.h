// -*-c++-*-

#ifndef _MOD_H
#define _MOD_H

class Mustang;

class ModCC {

protected:
  Mustang * amp;

  int continuous_control( int parm5, int parm6, int parm7, int value );
  int discrete_control( int parm5, int parm6, int parm7, int value );

public:
  ModCC( Mustang * theAmp ) : amp(theAmp) {}

  int dispatch( int cc, int value );

private:
  virtual int cc39( int value ) = 0;
  virtual int cc40( int value ) = 0;
  virtual int cc41( int value ) = 0;
  virtual int cc42( int value ) = 0;
  virtual int cc43( int value ) = 0;
 };


class ChorusCC : public ModCC {
public:
  ChorusCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rate
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x10, value );}
  // Depth
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Avg Delay
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // LR Phase
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class FlangerCC : public ModCC {
public:
  FlangerCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rate
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x10, value );}
  // Depth
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Feedback
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // LR Phase
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class VibratoneCC : public ModCC {
public:
  VibratoneCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rotor Speed
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x0f, value );}
  // Depth
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Feedback
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // LR Phase
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class TremCC : public ModCC {
public:
  TremCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rotor Speed
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x0e, value );}
  // Duty Cycle
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Attack / LFO Clip
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Release / Tri Shape
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class RingModCC : public ModCC {
public:
  RingModCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Freq
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Depth
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // LFO Depth
  virtual int cc42( int value ) {
    if ( value > 1 ) return 0;
    else                          return discrete_control( 0x03, 0x03, 0x8c, value );
  }
  // LFO Phase
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class StepFilterCC : public ModCC {
public:
  StepFilterCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rate
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x10, value );}
  // Resonance
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Min Freq
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Max Freq
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class PhaserCC : public ModCC {
public:
  PhaserCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Rate
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x10, value );}
  // Depth
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Feedback
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // LFO Shape
  virtual int cc43( int value ) {
    if ( value > 1 ) return 0;
    else             return discrete_control( 0x04, 0x04, 0x8c, value );
  }
};


class PitchShifterCC : public ModCC {
public:
  PitchShifterCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Level
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Pitch
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Detune
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Feedback
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Predelay
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};

// Wah + Touch Wah
class ModWahCC : public ModCC {
public:
  ModWahCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Mix
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Freq
  virtual int cc40( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Heel Freq
  virtual int cc41( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Toe Freq
  virtual int cc42( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Hi-Q
  virtual int cc43( int value ) { 
    if ( value > 1 ) return 0;
    else             return discrete_control( 0x04, 0x04, 0x81, value );
  }
};


class DiatonicShiftCC : public ModCC {
public:
  DiatonicShiftCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  // Mix
  virtual int cc39( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Pitch
  virtual int cc40( int value ) { 
    if ( value > 0x15 ) return 0;
    else                return discrete_control( 0x01, 0x0b, 0x98, value );
  }
  // Key
  virtual int cc41( int value ) { 
    if ( value > 0x0b ) return 0;
    else                return discrete_control( 0x02, 0x02, 0x99, value );
  }
  // Scale
  virtual int cc42( int value ) { 
    if ( value > 8 ) return 0;
    else             return discrete_control( 0x03, 0x03, 0x9a, value );
  }
  // Tone
  virtual int cc43( int value ) { return continuous_control( 0x04, 0x07, 0x01, value );}
};


class NullModCC : public ModCC {
public:
  NullModCC( Mustang * theAmp ) : ModCC(theAmp) {}
private:
  virtual int cc39( int value ) { return 0;}
  virtual int cc40( int value ) { return 0;}
  virtual int cc41( int value ) { return 0;}
  virtual int cc42( int value ) { return 0;}
  virtual int cc43( int value ) { return 0;}
};


#endif
