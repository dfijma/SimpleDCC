#include "RefreshBuffer.h"



//// RefreshBuffer

RefreshBuffer::RefreshBuffer() {

}



bool RefreshBuffer::nextBit() {
  Serial.print(speed); Serial.print(" ");
  return speed > 64;
}
