// -*-c++-*-

#ifndef _STOMP_H
#define _STOMP_H

class Mustang;

class StompCC {

protected:
  Mustang * amp;

  int continuous_control( int parm5, int parm6, int parm7, int value );
  int discrete_control( int parm5, int parm6, int parm7, int value );

public:
  StompCC( Mustang * theAmp ) : amp(theAmp) {}

  int dispatch( int cc, int value );

private:
  virtual int cc29( int value ) = 0;
  virtual int cc30( int value ) = 0;
  virtual int cc31( int value ) = 0;
  virtual int cc32( int value ) = 0;
  virtual int cc33( int value ) = 0;
 };


class OverdriveCC : public StompCC {
public:
  OverdriveCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Level
  virtual int cc29( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Gain
  virtual int cc30( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Low
  virtual int cc31( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Mid
  virtual int cc32( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // High
  virtual int cc33( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class WahCC : public StompCC {
public:
  WahCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Mix
  virtual int cc29( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Freq
  virtual int cc30( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Heel Freq
  virtual int cc31( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Toe Freq
  virtual int cc32( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // High Q
  virtual int cc33( int value ) { 
    if ( value > 1 ) return 0;
    else             return discrete_control( 0x04, 0x04, 0x81, value );
  }
};


class FuzzCC : public StompCC {
public:
  FuzzCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Level
  virtual int cc29( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Gain
  virtual int cc30( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Octave
  virtual int cc31( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Low
  virtual int cc32( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // High
  virtual int cc33( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class FuzzTouchWahCC : public StompCC {
public:
  FuzzTouchWahCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Level
  virtual int cc29( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Gain
  virtual int cc30( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Sens
  virtual int cc31( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Octave
  virtual int cc32( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Peak
  virtual int cc33( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class SimpleCompCC : public StompCC {
public:
  SimpleCompCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Type
  virtual int cc29( int value ) { 
    if ( value > 3 ) return 0;
    else             return discrete_control( 0x00, 0x05, 0x92, value );
  }
  virtual int cc30( int value ) { return 0;}
  virtual int cc31( int value ) { return 0;}
  virtual int cc32( int value ) { return 0;}
  virtual int cc33( int value ) { return 0;}
};


class CompCC : public StompCC {
public:
  CompCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  // Level
  virtual int cc29( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Thresh
  virtual int cc30( int value ) { return continuous_control( 0x01, 0x01, 0x01, value );}
  // Ratio
  virtual int cc31( int value ) { return continuous_control( 0x02, 0x02, 0x04, value );}
  // Attack
  virtual int cc32( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Release
  virtual int cc33( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
};


class NullStompCC : public StompCC {
public:
  NullStompCC( Mustang * theAmp ) : StompCC(theAmp) {}
private:
  virtual int cc29( int value ) { return 0;}
  virtual int cc30( int value ) { return 0;}
  virtual int cc31( int value ) { return 0;}
  virtual int cc32( int value ) { return 0;}
  virtual int cc33( int value ) { return 0;}
};


#endif
