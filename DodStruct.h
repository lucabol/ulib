#ifndef DODSTRUCT_H
#define DODSTRUCT_H

#include "Utils.h"

#ifndef SIDXT
#define SIDXT Size
#endif

#define Struct1(sn,N,t1,n1) t1 sn##n1[N]; SIDXT sn##Idx = 0;
#define Struct2(sn,N,t1,n1,t2,n2) Struct1(sn,N,t1,n1) t2 sn##n2[N];
#define Struct3(sn,N,t1,n1,t2,n2,t3,n3) Struct2(sn,N,t1,n1,t2,n2) t3 sn##n3[N];
#define Struct4(sn,N,t1,n1,t2,n2,t3,n3,t4,n4) Struct3(sn,N,t1,n1,t2,n2,t3,n3) t4 sn##n4[N];
#define Struct5(sn,N,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5) Struct4(sn,N,t1,n1,t2,n2,t3,n3,t4,n4) t5 sn##n5[N];

#endif
