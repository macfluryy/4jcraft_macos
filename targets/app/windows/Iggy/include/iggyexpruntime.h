#ifndef __RAD_INCLUDE_IGGYEXPRUNTIME_H__
#define __RAD_INCLUDE_IGGYEXPRUNTIME_H__

#include "rrCore.h"

#define IDOC

RADDEFSTART

#ifndef __RAD_HIGGYEXP_
#define __RAD_HIGGYEXP_
typedef void* HIGGYEXP;
#endif



#define IGGYEXP_MIN_STORAGE 1024 IDOC


IDOC RADEXPFUNC HIGGYEXP RADEXPLINK IggyExpCreate(char* ip_address, S32 port,
                                                  void* storage,
                                                  S32 storage_size_in_bytes);















IDOC RADEXPFUNC void RADEXPLINK IggyExpDestroy(HIGGYEXP p);


IDOC RADEXPFUNC rrbool RADEXPLINK IggyExpCheckValidity(HIGGYEXP p);











RADDEFEND

#endif  