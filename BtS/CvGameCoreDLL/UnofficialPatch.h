#pragma once

// UnofficialPatch.h -- Extra defines for versioning and custom compilation.

#ifndef CIV4_UNOFFICIAL_PATCH_H
#define CIV4_UNOFFICIAL_PATCH_H

// Version numbers. Note this is an "integer times 100" since floats are prone to roundoff error.
#define UP_PATCH_VERSION (22)

// Compilation Option: _USE_UNOFFICIALPATCH
//
// If this symbol is defined, all Unofficial Patch changes will be in effect.
//
// If it is not, the only differences from Official 3.17 will be a BTS version
// string on the flag hover and the CvGlobals functions isUnofficialPatch() and
// getUnofficialPatchVersion() which will return false and 0 respectively.
#define _USE_UNOFFICIALPATCH

// Compilation Option: _USE_AIAUTOPLAY
//
// If this symbol is defined, jdog5000's AIAutoPlay SDK changes will be in effect.
#define _USE_AIAUTOPLAY

#endif