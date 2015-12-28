/*
 ---------------------------------------------------------------------
 Tunefish 4  -  http://tunefish-synth.com
 ---------------------------------------------------------------------
 This file is part of Tunefish.

 Tunefish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Tunefish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Tunefish.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#ifndef TYPES_HPP
#define TYPES_HPP

#ifndef _WIN32
#include <inttypes.h>
#endif

// macros to concatenate strings for pre-processor.
// two macros are used in order to expand any other
// macro feeded into eTOKENPASTE or eSTRINGIFY.
#define eTOKENPASTE_DEF(x, y)   x##y
#define eTOKENPASTE(x, y)       eTOKENPASTE_DEF(x, y)
#define eSTRINGIFY_DEF(x)       #x
#define eSTRINGIFY(x)           eSTRINGIFY_DEF(x)

// visual c++ specific stuff
#ifdef _WIN32
#define eFASTCALL               __fastcall
#define eFORCEINLINE            __forceinline
#define eINLINE                 __inline
#define eNORETURN               __declspec(noreturn)
#define eALIGN16                __declspec(align(16))
#define eNAKED                  __declspec(naked)
#define eTHREADLOCAL            __declspec(thread)
#define eCALLBACK               __stdcall
#define eCDECL                  __cdecl
#define eDEBUGBREAK             __debugbreak
#else
#define eFASTCALL
#define eFORCEINLINE            __inline
#define eINLINE                 __inline
#define eNORETURN
#define eALIGN16
#define eNAKED
#define eTHREADLOCAL
#define eCALLBACK
#define eCDECL
#define eDEBUGBREAK
#endif

// own types
typedef unsigned char           eU8;
typedef signed char             eS8;
typedef unsigned short          eU16;
typedef short                   eS16;
typedef unsigned int            eU32;
typedef int                     eS32;
typedef float                   eF32;
#ifdef _WIN32
typedef unsigned __int64        eU64;
typedef signed __int64          eS64;
#else
typedef uint64_t                eU64;
typedef int64_t                 eS64;
#endif
typedef double                  eF64;
typedef int                     eInt;
typedef char                    eChar;
typedef wchar_t                 eWChar;
typedef bool                    eBool;
typedef void *                  ePtr;
typedef const void *            eConstPtr;
typedef eU32                    eID;
typedef eU8                     BYTE; // required because of the shader headers

#ifdef eWIN64
typedef eU64                    eSize;
#else
typedef eU32                    eSize;
#endif

// numerical limits (dont't change into consts, it's a size thing!)
#define eU32_MAX                (0xffffffffU)
#define eS32_MIN                (-2147483647-1)
#define eS32_MAX                (2147483647)
#define eU16_MAX                (0xffffU)
#define eS16_MIN                (-32768)
#define eS16_MAX                (32767)
#define eU8_MAX                 (0xffU)
#define eS8_MIN                 (-128)
#define eS8_MAX                 (127)

#define eF32_MAX                (3.402823466e+38F)
#define eF32_MIN                (-eF32_MAX)
#define eF32_INF                (1e30f)

// some constants
#define eTRUE                   true
#define eFALSE                  false
#define eNOID                   0

#endif
