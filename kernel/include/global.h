// Copyright (C) 2016  Yong Zhou

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _global_h_
#define _global_h_

#include "Rtypes.h"
#include <vector>
#include <map>
//
typedef std::map< UInt_t,std::vector<int> > ChannelMap;//The Key is the global "encoded_wireid" in merged data and the front-panel channel id in raw data; the value is TDCContainer
typedef std::vector<int> TDCContainer;//All the measured leading/trailing ADC counts of the corresponding "encoded_wireid"
//
enum EBoardType {
    EMWDC=0,
    ETOF,
    EUNKNOWN
};
enum ELocation {
  EDOWN=0,
  EUP
};
enum EDirection {
  EX=0,
  EY,
  EU
};

// The Encoding namespace contains the utility functions for "encoded_wireid" encoding and decoding.
// Both the MWDC and TOF detector readout channels are encoded as unique global 32-bits word, which will
// reperent its corresponding detector channel in the data.
// The rules are as follows:
//    1) Detector Type(8 bits,24-31): MWDC=0, TOF=1, NULL=0xFF (NULL represents invalid/unconnected front-panel channel)
//    2) Detector Location(4 bits, 20-23): Up=1, Down=0
//    3) Wire Plane(4 bits,16-19, this field is only valid for MWDC): X=0, Y=1, U=2 ; TOF is 0
//    4) Channel Index(8 bits,0-15): MWDC X/Y plane: 0-79, MWDC U plane: 0-105, TOF corner PMT: 0-3    
namespace Encoding {
    inline UInt_t EncodeType(UChar_t type){
      return (type&0xFF)<<24;
    }
    inline UInt_t EncodeLocation(UChar_t location){
      return (location&0xF)<<20;
    }
    inline UInt_t EncodeDirection(UChar_t direction){
      return (direction&0xF)<<16;
    }
    inline UInt_t Encode(UChar_t type,UChar_t location,UChar_t direction,UShort_t index){
      return ((type&0xFF)<<24)+((location&0xF)<<20)+((direction&0xF)<<16)+index;
    }
    //
    inline UChar_t DecodeType(UInt_t encoded_id){
      return encoded_id>>24;
    }
    inline UChar_t DecodeLocation(UInt_t encoded_id){
      return (encoded_id>>20)&0xF;
    }
    inline UChar_t DecodeDirection(UInt_t encoded_id){
      return (encoded_id>>16)&0xF;
    }
    inline UShort_t DecodeIndex(UInt_t encoded_id){
      return encoded_id&0xFFFF;
    }
    inline void Decode(UInt_t encoded_id,UChar_t& type,UChar_t& location,UChar_t& direction,UShort_t& index){
      type = (encoded_id>>24);
      location = (encoded_id>>20)&0xF;
      direction = (encoded_id>>16)&0xF;
      index = encoded_id&0xFFFF;
    }

    inline Bool_t IsChannelValid(UInt_t encoded_id){
      UChar_t type=Encoding::DecodeType(encoded_id);
      if(type == 0xFF){
        return false;
      }
      return true;
    }
}
//
const int g_range_precision=524288;//2^19,the maximum ADC value of high resolution mode, the unit is 25us/256
const int g_range_highprecision=2097152;//2^21, the maximum ADC value of very high resolution mode, the unit is 25us/256/4
const int g_range_bunchid=4096;
const int g_range_eventid=4096;

const double g_timeunit_precision=25000./256;
const double g_timeunit_highprecision=25000./256/4;
//
const int g_mwdc_location=2;
const int g_mwdc_wireplane=3;
const int g_mwdc_wireindex[3]={80,80,106};

const int g_tof_location=2;
const int g_tof_wireindex=4;

const char g_str_location[2][10]={"Down","Up"};
const char g_str_plane[3][10]={"X","Y","U"};

const UInt_t g_invalid_channel=Encoding::Encode(0xFF,0,0,0);
#endif // _global_h_