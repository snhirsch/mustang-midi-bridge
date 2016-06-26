// -*-c++-*-

#ifndef _DELAY_H
#define _DELAY_H

class Mustang;

class DelayCC {

protected:
  Mustang * amp;

  int continuous_control( int parm5, int parm6, int parm7, int value );
  int discrete_control( int parm5, int parm6, int parm7, int value );

public:
  DelayCC( Mustang * theAmp ) : amp(theAmp) {}

  int dispatch( int cc, int value );

private:
  // Level
  virtual int cc49( int value );
  // Delay Time
  virtual int cc50( int value );
  
  virtual int cc51( int value ) { return 0;}
  virtual int cc52( int value ) { return 0;}
  virtual int cc53( int value ) { return 0;}
  virtual int cc54( int value ) { return 0;}
};


class MonoDelayCC : public DelayCC {
public:
  MonoDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Brightness
  virtual int cc52( int value );
  // Attenuation
  virtual int cc53( int value );
};


class EchoFilterCC : public DelayCC {
public:
  EchoFilterCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Frequency
  virtual int cc52( int value );
  // Resonance
  virtual int cc53( int value );
  // Input Level
  virtual int cc54( int value );
};


class MultitapDelayCC : public DelayCC {
public:
  MultitapDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Brightness
  virtual int cc52( int value );
  // Mode
  virtual int cc53( int value );
};


class PingPongDelayCC : public DelayCC {
public:
  PingPongDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Brightness
  virtual int cc52( int value );
  // Stereo
  virtual int cc53( int value );
};


class DuckingDelayCC : public DelayCC {
public:
  DuckingDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Release
  virtual int cc52( int value );
  // Threshold
  virtual int cc53( int value );
};


class ReverseDelayCC : public DelayCC {
public:
  ReverseDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // FFdbk
  virtual int cc51( int value );
  // RFdbk
  virtual int cc52( int value );
  // Tone
  virtual int cc53( int value );
};


class TapeDelayCC : public DelayCC {
public:
  TapeDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Flutter
  virtual int cc52( int value );
  // Brightness
  virtual int cc53( int value );
  // Stereo
  virtual int cc54( int value );
};


class StereoTapeDelayCC : public DelayCC {
public:
  StereoTapeDelayCC( Mustang * theAmp ) : DelayCC(theAmp) {}
private:
  // Feedback
  virtual int cc51( int value );
  // Flutter
  virtual int cc52( int value );
  // Separation
  virtual int cc53( int value );
  // Brightness
  virtual int cc54( int value );
};


#endif
