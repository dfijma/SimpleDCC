#pragma once

#include "Arduino.h"



class RefreshBuffer {
  public:
    RefreshBuffer();

    void slot_update_withTrottleCmd(int address, byte speed /* 0..126 */, boolean forward, boolean emergencyStop) { this->speed = speed; }

    bool nextBit();

  private:
    byte speed;
  
};
