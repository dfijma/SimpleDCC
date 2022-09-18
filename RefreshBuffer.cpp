#include "RefreshBuffer.h"

//// Packet

static const byte mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; // precalculated masks to get a bit

boolean Packet::getBit(int bit) {
  Serial.print("getb: "); Serial.print((long)this); Serial.print(" "); Serial.print(buffer[0], BIN); Serial.print(" ");Serial.print(bit);
  boolean res = buffer[bit / 8] & mask[bit % 8];
  Serial.print("->"); Serial.println(res);
  return res;
}

Packet& Packet::withIdleCmd() {
  byte cmd[1];
  cmd[0] = 0;
  loadCmd(cmd, 1);
  return *this;
}

Packet& Packet::withThrottleCmd(int address, byte speed /* 0..126 */, boolean forward, boolean emergencyStop) {
  byte cmd[1];
  cmd[0] = speed;
  loadCmd(cmd, 1);
  return *this;
}

void Packet::loadCmd(byte in[], byte nBytes) {
  // assume nBytes == 1 for this mock
  // Serial.print("load: "); Serial.print((long)this); Serial.print(" "); Serial.print(in[0], BIN); Serial.println();
  buffer[0] = in[0];
  buffered = 1;
}

//// Slot
  
void Slot::flip() {
  if (updatePacket->length() == 0)  return; // no update
  Packet* tmp = activePacket;
  activePacket = updatePacket;
  updatePacket = tmp;
  updatePacket->reset();
}

//// RefreshBuffer

RefreshBuffer::RefreshBuffer() {
  // initially, each active packet is IDLE, each update packet is empty
  for (int i=0; i<SLOTS; ++i) {
    slots[i].update().withIdleCmd();
    slots[i].flip();
  }
  brk = 1; // at least one slot in use 
  // this is a valid bit, because we have all these idle cmds:
  currentSlot = 0;
  currentBit = 0;
}

Slot& RefreshBuffer::slot(byte s) {
  if (s >= brk) brk = s+1;
  return slots[s]; 
}

bool RefreshBuffer::nextBit() {
  boolean res = slots[currentSlot].getBit(currentBit);
  currentBit++;
  if (currentBit >= slots[currentSlot].length()) {
    // next slot
    currentSlot = (currentSlot + 1) % brk;
    slots[currentSlot].flip(); // if there is an update, flip into active
    currentBit = 0; // this assumes that the slot is not empty!
  }  
  return res;
}
