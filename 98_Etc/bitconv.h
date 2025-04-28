#pragma once

#include "../Common/DataType.h"

extern float f_f32f24(float x);
#if defined(SoundFloat24)
extern __declspec(dllexport) unsigned int x_f32f24(float x);
extern __declspec(dllexport) float f24f32(int x);
extern unsigned int f24Int(unsigned int x);
#endif

extern unsigned int x_f32f24_test(float x);
extern unsigned int x_f32f16(float x);
extern unsigned int x_x32x24(unsigned int x);
extern int f24x32(int x);

extern float f_f32f24_old(float x);
extern unsigned int x_f32f24_old(float x);
extern unsigned int x_f32f16_old(float x);
extern float f24f32_old(int x);
extern unsigned int x_x32x24_old(unsigned int x);

