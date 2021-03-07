/*
Copyright © 2019, 2020, 2021 HackEDA, Inc.
Licensed under the WiPhone Public License v.1.0 (the "License"); you
may not use this file except in compliance with the License. You may
obtain a copy of the License at
https://wiphone.io/WiPhone_Public_License_v1.0.txt.

Unless required by applicable law or agreed to in writing, software,
hardware or documentation distributed under the License is distributed
on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#ifndef RTPACKET_H
#define RTPACKET_H

#include <byteswap.h>

// Taken from https://en.wikipedia.org/wiki/Real-time_Transport_Protocol
typedef struct __attribute__((packed)) {
  uint8_t vpxcc;
  uint8_t ptm;
  uint16_t sequence;
  uint32_t timestamp;
  uint32_t SSRC;
}
RTPacketHeader;

class RTPacket {
public:
  RTPacket() : version_(2), marker_(false), payload_type_(9), sequence_(0), timestamp_(0), ssrc_(0), csrc_(0) {};

  RTPacketHeader *generateHeader(uint32_t payloadLen) {
    ++sequence_;
    timestamp_ += payloadLen;

    header_.vpxcc = (sequence_ << 6);
    header_.ptm = payload_type_;
    header_.timestamp = __bswap_32(timestamp_);
    header_.sequence = __bswap_16(sequence_);
    header_.SSRC = __bswap_32(ssrc_);

    return &header_;
  };

  void newSession() {
    ++ssrc_;
    sequence_ = Random.random();
    timestamp_ = Random.random();
  };

  void setHeader(uint8_t *buff) {
    memcpy(&header_, buff, sizeof(header_));

    header_.sequence =  __bswap_16(header_.sequence);
    header_.timestamp = __bswap_32(header_.timestamp);
    header_.SSRC = __bswap_32(header_.SSRC);
  };

  uint8_t getPayloadType() {
    return header_.ptm & 0x7F;
  }

  uint32_t getSequenceNumber() {
    return header_.sequence;
  }

  uint32_t getSSRC() {
    return header_.SSRC;
  }

private:
  RTPacketHeader header_;

  uint8_t version_;
  bool marker_;
  uint8_t payload_type_;
  uint16_t sequence_;
  uint32_t timestamp_;
  uint32_t ssrc_;
  uint32_t csrc_;
};


#endif