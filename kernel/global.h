/*
 T his program is free s*oftware; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.
 
 ---
 Copyright (C) 2015, ufan <>
 */

#ifndef _global_h_
#define _global_h_
#include "Rtypes.h"
#include <vector>
#include <map>
//
typedef std::map< UInt_t,std::vector<int> > ChannelMap;
typedef std::vector<int> TDCContainer;
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
}
//
const int g_range_precision=524288;
const int g_range_highprecision=2097152;
#endif // _global_h_