#pragma once

#if defined(SoundFloat24)
extern unsigned int FInv(unsigned int y);
extern unsigned int Fmul(unsigned int x, unsigned int y);
extern unsigned int SQRT(unsigned int x);
extern unsigned int SINCOS(unsigned int x, unsigned int mode);
extern unsigned int SIN(unsigned int x);
extern unsigned int COS(unsigned int x);
extern unsigned int FAdder(unsigned int x, unsigned int y, unsigned int mode);
extern unsigned int TFAdder(unsigned int x, unsigned int y, unsigned int z);
extern unsigned int UFAdder(unsigned int x, unsigned int y);
extern unsigned int UTFAdder(unsigned int x, unsigned int y, unsigned int z);
extern unsigned int Compare(unsigned int x, unsigned int y);
extern unsigned int ACOS(unsigned int x);
#endif