#include "RefreshBuffer.h"

//// Packet

static const byte mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; // precalculated masks to get a bit

boolean Packet::getBit(int bit) {
  Serial.print("getb: "); Serial.print((long)this); Serial.print(" "); 
  for (int i = 0; i<buffered; ++i) {
    Serial.print(buffer[i], BIN); Serial.print("_");
  }
  Serial.print(" ");Serial.print(bit);
  boolean res = buffer[bit / 8] & mask[bit % 8];
  Serial.print("->"); Serial.println(res);
  return res;
}

Packet& Packet::withIdleCmd() {
  static byte idlePacket[3] = {0xFF, 0x00, 0x00}; // length 2 + room for checksum
  loadCmd(idlePacket, 2);
  return *this;
}

Packet& Packet::withThrottleCmd(int address, byte speed /* 0..126 */, boolean forward, boolean emergencyStop) {
  static byte buffer[5]; // max 5 bytes, including checksum byte
  byte nB = loadAddress(buffer, address);
  buffer[nB++] = 0x3F;                      // 128-step speed control byte
  if (emergencyStop) {
    buffer[nB++] = 1; // emergency stop
  } else {
    // max speed is 126, but speed codes range from 2-127 (0=stop, 1=emergency stop)
    // so map 0, 1, 2 .. 126 -> to 0, 2, 3, .. 127 and set high bit iff forward
    buffer[nB++] = (speed + (speed > 0 ? 1 : 0)) | (forward ? 0x80 : 0x00);
  }
  loadCmd(buffer, nB);
  return *this;
}

byte Packet::loadAddress(byte* buffer, int address) {
  byte res = 0;
  if (address > 127) {
    buffer[res++] = highByte(address) | 0xC0;    // convert train number into a two-byte address
  }
  buffer[res++] = lowByte(address);
  return res;
}

void Packet::loadCmd(byte in[], byte nBytes) {
  // this could be generalized, but we know a DCC cmd is always 3, 4, 5 or 6 bytes, so we unfold a bit
  byte *out = &buffer[buffered];
  in[nBytes] = in[0]; // copy first byte into what will become the checksum byte
  for (int i = 1; i < nBytes; i++) {      // XOR remaining bytes into checksum byte
    in[nBytes] ^= in[i];
  }
  nBytes++;                              // increment number of bytes in packet to include checksum byte

  // TODO: we could do with a somewhat smaller preamble (14 minimum)
  out[0] = 0xFF;                       // first 8 bits of 22-byte preamble
  out[1] = 0xFF;                       // second 8 bits of 22-byte preamble
  out[2] = 0xFC + bitRead(in[0], 7);   // last 6 bits of 22-byte preamble + data start bit + b[0], bit 7
  out[3] = in[0] << 1;                 // in[0], bits 6-0 + data start bit
  out[4] = in[1];                      // in[1], all bits
  out[5] = in[2] >> 1;                 // in[2], bits 7-1
  out[6] = in[2] << 7;                 // in[2], bit 0

  if (nBytes == 3) {
    out[6] = out[6] | 0x7F; // + 7 padding/stop bits
    buffered += 7;
  } else {
    out[6] += in[3] >> 2;              // in[3], bits 7-2
    out[7] = in[3] << 6;               // in[3], bit 1-0
    if (nBytes == 4) {
      out[7] = out[7] | 0x3F; // + 6 padding/stop bits
      buffered += 8;
    } else {
      out[7] += in[4] >> 3;            // in[4], bits 7-3
      out[8] = in[4] << 5;             // in[4], bits 2-0
      if (nBytes == 5) {
        out[8] = out[8] | 0x1F; // + 5 padding/stop bits
        buffered += 9;
      } else {
        out[8] += in[5] >> 4;          // in[5], bits 7-4
        out[9] = (in[5] << 4) | 0x0F ; // in[5], bits 3-0 + 4 padding/stop bits
        buffered += 10;
      } 
    } 
  } 
  Serial.print("cmd loaded: packet length="); Serial.println(buffered);
}

//// Slot

Packet& Slot::update() { 
  updatePacket->reset(); 
  return *updatePacket;
} // updatable packet for new cmds

void Slot::flip() {
  if (!updatePacket->isSaved())  return; // no update
  Packet* tmp = activePacket;
  activePacket = updatePacket;
  updatePacket = tmp;
  updatePacket->reset();
}

//// RefreshBuffer

RefreshBuffer::RefreshBuffer() {
  // initially, each active packet is IDLE, each update packet is empty
  for (int i=0; i<SLOTS; ++i) {
    slots[i].update().withIdleCmd().save();
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
