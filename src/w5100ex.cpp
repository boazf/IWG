/*
 * This file was copied and modified in order to add functionality that 
 * was removed from w5100 library.
 * 
 * Original Copyright 2018 Paul Stoffregen
 * Original Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef USE_WIFI
#include <Arduino.h>
#include <w5100ex.h>

void W5100ClassEx::send_data_processing(SOCKET s, const uint8_t *data, uint16_t len)
{
  // This is same as having no offset in a call to send_data_processing_offset
  send_data_processing_offset(s, 0, data, len);
}

void W5100ClassEx::send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len)
{
  uint16_t ptr = readSnTX_WR(s);
  ptr += data_offset;
  uint16_t offset = ptr & SMASK;
  uint16_t dstAddr = offset + SBASE(s);
  if (offset + len > SSIZE) 
  {
    // Wrap around circular buffer
    uint16_t size = SSIZE - offset;
    write(dstAddr, data, size);
    write(SBASE(s), data + size, len - size);
  } 
  else {
    write(dstAddr, data, len);
  }

  ptr += len;
  writeSnTX_WR(s, ptr);
}

uint16_t W5100ClassEx::getRXReceivedSize(SOCKET s)
{
  uint16_t val=0,val1=0;
  do {
    val1 = readSnRX_RSR(s);
    if (val1 != 0)
      val = readSnRX_RSR(s);
  } 
  while (val != val1);
  return val;
}

void W5100ClassEx::read_data(SOCKET s, volatile uint16_t src, volatile uint8_t *dst, uint16_t len)
{
  uint16_t size;
  uint16_t src_mask;
  uint16_t src_ptr;
  uint8_t *dst_ptr = const_cast<uint8_t*>(reinterpret_cast<volatile uint8_t *>(dst));

  src_mask = src & RMASK;
  src_ptr = RBASE(s) + src_mask;

  if( (src_mask + len) > RSIZE ) 
  {
    size = RSIZE - src_mask;
    read(src_ptr, dst_ptr, size);
    dst_ptr += size;
    read(RBASE(s), dst_ptr, len - size);
  } 
  else
    read(src_ptr, dst_ptr, len);
}

W5100ClassEx W5100Ex;
#endif // USE_WIFI