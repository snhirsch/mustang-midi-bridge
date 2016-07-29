// -*-c++-*-

#ifndef _DELAY_H
#define _DELAY_H

#include <cstring>

class Mustang;

class DelayCC {

protected:
  Mustang * amp;
  unsigned char model[2];
  unsigned char slot;

  int continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );
  int discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );

public:
  DelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : 
    amp(theAmp), 
    slot(theSlot) 
  {
    memcpy( this->model, model, 2 );
  }

  int dispatch( int cc, int value, unsigned char *cmd );
  const unsigned char *getModel( void ) { return model;}
  const unsigned char getSlot( void ) { return slot;}

private:
  // Level
  virtual int cc49( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x01, value, cmd );}
  // Delay Time
  virtual int cc50( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x06, value, cmd );}
  
  virtual int cc51( int value, unsigned char *cmd ) = 0;
  virtual int cc52( int value, unsigned char *cmd ) = 0;
  virtual int cc53( int value, unsigned char *cmd ) = 0;
  virtual int cc54( int value, unsigned char *cmd ) = 0;
};


class MonoDelayCC : public DelayCC {
public:
  MonoDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Brightness
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Attenuation
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // no-op
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


class EchoFilterCC : public DelayCC {
public:
  EchoFilterCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Frequency
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Resonance
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // Input Level
  virtual int cc54( int value, unsigned char *cmd ) { return continuous_control( 0x05, 0x05, 0x01, value, cmd );}
};


class MultitapDelayCC : public DelayCC {
public:
  MultitapDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Delay Time
  virtual int cc50( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x08, value, cmd );}
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Brightness
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Mode
  virtual int cc53( int value, unsigned char *cmd ) { 
    if ( value > 3 ) return -1;
    else             return discrete_control( 0x04, 0x04, 0x8b, value, cmd );
  }
  // no-op
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


class PingPongDelayCC : public DelayCC {
public:
  PingPongDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Brightness
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Stereo
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // no-op
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


class DuckingDelayCC : public DelayCC {
public:
  DuckingDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Release
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Threshold
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // no-op
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


class ReverseDelayCC : public DelayCC {
public:
  ReverseDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // FFdbk
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // RFdbk
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Tone
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // no-op
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


class TapeDelayCC : public DelayCC {
public:
  TapeDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Flutter
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Brightness
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x01, value, cmd );}
  // Stereo
  virtual int cc54( int value, unsigned char *cmd ) { return continuous_control( 0x05, 0x05, 0x01, value, cmd );}
};


class StereoTapeDelayCC : public DelayCC {
public:
  StereoTapeDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x01, value, cmd );}
  // Flutter
  virtual int cc52( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x01, value, cmd );}
  // Separation
  virtual int cc53( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x05, 0x01, value, cmd );}
  // Brightness
  virtual int cc54( int value, unsigned char *cmd ) { return continuous_control( 0x05, 0x04, 0x01, value, cmd );}
};


class NullDelayCC : public DelayCC {
public:
  NullDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  virtual int cc49( int value, unsigned char *cmd ) { return -1;}
  virtual int cc50( int value, unsigned char *cmd ) { return -1;}
  virtual int cc51( int value, unsigned char *cmd ) { return -1;}
  virtual int cc52( int value, unsigned char *cmd ) { return -1;}
  virtual int cc53( int value, unsigned char *cmd ) { return -1;}
  virtual int cc54( int value, unsigned char *cmd ) { return -1;}
};


#endif
