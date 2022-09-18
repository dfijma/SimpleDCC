#pragma once

#include "Arduino.h"

#define SLOTS 10 // slots in a buffer
#define MAX_CMDS_PACKET 3 // maximum number of cmds in a encoded packet containing multiple encoded cmds
#define MAX_CMD_ENCODED_SIZE 10 // the max size of a single encodeded cmd

//// a Packet contains one or more encoded DCC cmds

class Packet {
  public:
    Packet() { reset(); } // initially empty
    boolean getBit(int bit);
    int length() { return buffered * 8; } // length in bits
    void reset() { buffered = 0; saved = false; }
    Packet& withIdleCmd();
    Packet& withThrottleCmd(int address, byte speed /* 0..126 */, boolean forward, boolean emergencyStop);
    void save() { saved = true; }
    boolean isSaved() { return saved; }

  private:
    byte buffer[MAX_CMDS_PACKET*MAX_CMD_ENCODED_SIZE]; // room for max 3 packets of max length 10 bytes encoded
    byte buffered; // number of bytes in the packet
    boolean saved;

    void loadCmd(byte in[], byte nBytes);
    byte loadAddress(byte* buffer, int address);
    Packet& withCmd(int address, byte mask, byte cmdBits);

};

//// A Slot is a combination of an active packet (to be modulated) and an updatable packet (to latch new cmds)

class Slot {
  public:
    Slot() {
      activePacket = &packets[0];
      updatePacket = &packets[1];
    } 
    Packet& update(); // updatable packet for new cmds
    void flip(); // flip update packet into active
    boolean getBit(int bit) { return activePacket->getBit(bit); }
    int length() { return activePacket->length(); }
  private:
     Packet packets[2]; // two packets: the active packet and the packet to be updated, both initially empty
     Packet *activePacket;
     Packet *updatePacket; 
};


//// A RefreshBuffer is set of slots to be modulated 

// Unitially, the active packet in each slot is an idle cmd and the update packet is empty.
// After initializing a buffer, we can thus immediately start cycling through all bits using
// nextBit(), and modulate an endless loop of idle cmds, so that we at least have power.

class RefreshBuffer {
  public:
    RefreshBuffer();
    Slot& slot(byte s);
    bool nextBit();
  
  private:
    byte brk; 
    Slot slots[SLOTS]; // some SLOTS, initially active packet is an idle cmd and update packet is empty
    byte currentSlot;
    byte currentBit;
};
