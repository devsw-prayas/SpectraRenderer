#pragma once

#ifndef SPECTRA_CORE
#define SPECTRA_CORE __declspec(dllexport)
#endif

void SPECTRA_CORE SpectraCoreInit();

#ifndef SIGN_EXTEND_4
#define SIGN_EXTEND_4(bits) ((bits & 0x00) ? (bits | 0xf0) : bits)
#endif

#ifndef BIT_MASK_4
#define BIT_MASK_4(value) (value & 0x0F)
#endif
