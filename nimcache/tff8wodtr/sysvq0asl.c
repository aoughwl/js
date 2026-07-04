#define NIM_INTBITS 32
/* GENERATED CODE. DO NOT EDIT. */

#ifdef __cplusplus
#  if __cplusplus >= 201103L
#    /* nullptr is more type safe (less implicit conversions than 0) */
#    define NIM_NIL nullptr
#  else
#    // both `((void*)0)` and `NULL` would cause codegen to emit
#    // error: assigning to 'Foo *' from incompatible type 'void *'
#    // but codegen could be fixed if need. See also potential caveat regarding
#    // NULL.
#    // However, `0` causes other issues, see #13798
#    define NIM_NIL 0
#  endif
#else
#  include <stdbool.h>
#  define NIM_NIL NULL
#endif

#ifdef __cplusplus
#define NB8 bool
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901)
// see #13798: to avoid conflicts for code emitting `#include <stdbool.h>`
#define NB8 _Bool
#else
typedef unsigned char NB8; // best effort
#endif

typedef unsigned char NC8;

typedef float NF32;
typedef double NF64;
#if defined(__BORLANDC__) || defined(_MSC_VER)
typedef signed char NI8;
typedef signed short int NI16;
typedef signed int NI32;
typedef __int64 NI64;
/* XXX: Float128? */
typedef unsigned char NU8;
typedef unsigned short int NU16;
typedef unsigned int NU32;
typedef unsigned __int64 NU64;
#elif defined(HAVE_STDINT_H)
#ifndef USE_NIM_NAMESPACE
#  include <stdint.h>
#endif
typedef int8_t NI8;
typedef int16_t NI16;
typedef int32_t NI32;
typedef int64_t NI64;
typedef uint8_t NU8;
typedef uint16_t NU16;
typedef uint32_t NU32;
typedef uint64_t NU64;
#elif defined(HAVE_CSTDINT)
#ifndef USE_NIM_NAMESPACE
#  include <cstdint>
#endif
typedef std::int8_t NI8;
typedef std::int16_t NI16;
typedef std::int32_t NI32;
typedef std::int64_t NI64;
typedef std::uint8_t NU8;
typedef std::uint16_t NU16;
typedef std::uint32_t NU32;
typedef std::uint64_t NU64;
#else
/* Unknown compiler/version, do our best */
#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ NI8;
#else
typedef signed char NI8;
#endif
#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ NI16;
#else
typedef signed short int NI16;
#endif
#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ NI32;
#else
typedef signed int NI32;
#endif
#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ NI64;
#else
typedef long long int NI64;
#endif
/* XXX: Float128? */
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ NU8;
#else
typedef unsigned char NU8;
#endif
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ NU16;
#else
typedef unsigned short int NU16;
#endif
#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ NU32;
#else
typedef unsigned int NU32;
#endif
#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ NU64;
#else
typedef unsigned long long int NU64;
#endif
#endif

#ifdef NIM_INTBITS
#  if NIM_INTBITS == 64
typedef NI64 NI;
typedef NU64 NU;
#  elif NIM_INTBITS == 32
typedef NI32 NI;
typedef NU32 NU;
#  elif NIM_INTBITS == 16
typedef NI16 NI;
typedef NU16 NU;
#  elif NIM_INTBITS == 8
typedef NI8 NI;
typedef NU8 NU;
#  else
#    error "invalid bit width for int"
#  endif
#endif

#define NIM_TRUE true
#define NIM_FALSE false

#define _GNU_SOURCE

// Include math.h to use `NAN` that should be defined in C compilers supports C99.
#include <math.h>

// Define NAN in case math.h doesn't define it.
// NAN definition copied from math.h included in the Windows SDK version 10.0.14393.0
#ifndef NAN
#  ifndef _HUGE_ENUF
#    define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#  endif
#  define NAN_INFINITY ((float)(_HUGE_ENUF * _HUGE_ENUF))
#  define NAN ((float)(NAN_INFINITY * 0.0F))
#endif

#ifndef INF
#  ifdef INFINITY
#    define INF INFINITY
#  elif defined(HUGE_VAL)
#    define INF  HUGE_VAL
#  elif defined(_MSC_VER)
#    include <float.h>
#    define INF (DBL_MAX+DBL_MAX)
#  else
#    define INF (1.0 / 0.0)
#  endif
#endif

#if defined(__GNUC__) || defined(_MSC_VER)
#  define IL64(x) x##LL
#else /* works only without LL */
#  define IL64(x) ((NI64)x)
#endif


/* ------------ ignore typical warnings in Nim-generated files ------------- */
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wswitch-bool"
#  pragma GCC diagnostic ignored "-Wformat"
#  pragma GCC diagnostic ignored "-Wpointer-sign"
#  if defined(__clang__)
#    pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#  else
#    pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#  endif
#endif



/* ------------------------------------------------------------------- */
#ifdef  __cplusplus
#  define NIM_EXTERNC extern "C"
#else
#  define NIM_EXTERNC
#endif

#if defined(WIN32) || defined(_WIN32) /* only Windows has this mess... */
#  define N_LIB_PRIVATE
#  define N_CDECL(rettype, name) rettype __cdecl name
#  define N_STDCALL(rettype, name) rettype __stdcall name
#  define N_SYSCALL(rettype, name) rettype __syscall name
#  define N_FASTCALL(rettype, name) rettype __fastcall name
#  define N_THISCALL(rettype, name) rettype __thiscall name
#  define N_SAFECALL(rettype, name) rettype __stdcall name
/* function pointers with calling convention: */
#  define N_CDECL_PTR(rettype, name) rettype (__cdecl *name)
#  define N_STDCALL_PTR(rettype, name) rettype (__stdcall *name)
#  define N_SYSCALL_PTR(rettype, name) rettype (__syscall *name)
#  define N_FASTCALL_PTR(rettype, name) rettype (__fastcall *name)
#  define N_THISCALL_PTR(rettype, name) rettype (__thiscall *name)
#  define N_SAFECALL_PTR(rettype, name) rettype (__stdcall *name)

#  ifdef __EMSCRIPTEN__
#    define N_LIB_EXPORT  NIM_EXTERNC __declspec(dllexport) __attribute__((used))
#    define N_LIB_EXPORT_VAR  __declspec(dllexport) __attribute__((used))
#  else
#    define N_LIB_EXPORT  NIM_EXTERNC __declspec(dllexport)
#    define N_LIB_EXPORT_VAR  __declspec(dllexport)
#  endif
#  define N_LIB_IMPORT  extern __declspec(dllimport)
#else
#  define N_LIB_PRIVATE __attribute__((visibility("hidden")))
#  if defined(__GNUC__)
#    define N_CDECL(rettype, name) rettype name
#    define N_STDCALL(rettype, name) rettype name
#    define N_SYSCALL(rettype, name) rettype name
#    define N_FASTCALL(rettype, name) __attribute__((fastcall)) rettype name
#    define N_SAFECALL(rettype, name) rettype name
/*   function pointers with calling convention: */
#    define N_CDECL_PTR(rettype, name) rettype (*name)
#    define N_STDCALL_PTR(rettype, name) rettype (*name)
#    define N_SYSCALL_PTR(rettype, name) rettype (*name)
#    define N_FASTCALL_PTR(rettype, name) __attribute__((fastcall)) rettype (*name)
#    define N_SAFECALL_PTR(rettype, name) rettype (*name)
#  else
#    define N_CDECL(rettype, name) rettype name
#    define N_STDCALL(rettype, name) rettype name
#    define N_SYSCALL(rettype, name) rettype name
#    define N_FASTCALL(rettype, name) rettype name
#    define N_SAFECALL(rettype, name) rettype name
/*   function pointers with calling convention: */
#    define N_CDECL_PTR(rettype, name) rettype (*name)
#    define N_STDCALL_PTR(rettype, name) rettype (*name)
#    define N_SYSCALL_PTR(rettype, name) rettype (*name)
#    define N_FASTCALL_PTR(rettype, name) rettype (*name)
#    define N_SAFECALL_PTR(rettype, name) rettype (*name)
#  endif
#  ifdef __EMSCRIPTEN__
#    define N_LIB_EXPORT NIM_EXTERNC __attribute__((visibility("default"), used))
#    define N_LIB_EXPORT_VAR  __attribute__((visibility("default"), used))
#  else
#    define N_LIB_EXPORT NIM_EXTERNC __attribute__((visibility("default")))
#    define N_LIB_EXPORT_VAR  __attribute__((visibility("default")))
#  endif
#  define N_LIB_IMPORT  extern
#endif

#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
/* these compilers have a fastcall so use it: */
#  define N_NIMCALL(rettype, name) rettype __fastcall name
#  define N_NIMCALL_PTR(rettype, name) rettype (__fastcall *name)
#else
#  define N_NIMCALL(rettype, name) rettype name /* no modifier */
#  define N_NIMCALL_PTR(rettype, name) rettype (*name)
#endif

#define N_NOCONV(rettype, name) rettype name
/* specify no calling convention */
#define N_NOCONV_PTR(rettype, name) rettype (*name)

/* calling convention mess ----------------------------------------------- */
#if defined(__GNUC__) || defined(__TINYC__)
  /* these should support C99's inline */
#  define N_INLINE(rettype, name) inline rettype name
#elif defined(__BORLANDC__) || defined(_MSC_VER)
/* Borland's compiler is really STRANGE here; note that the __fastcall
   keyword cannot be before the return type, but __inline cannot be after
   the return type, so we do not handle this mess in the code generator
   but rather here. */
#  define N_INLINE(rettype, name) __inline rettype name
#else /* others are less picky: */
#  define N_INLINE(rettype, name) rettype __inline name
#endif

#define N_INLINE_PTR(rettype, name) rettype (*name)

#if defined(__GNUC__) || defined(__ICC__)
#  define N_NOINLINE __attribute__((__noinline__))
#elif defined(_MSC_VER)
#  define N_NOINLINE __declspec(noinline)
#else
#  define N_NOINLINE
#endif

#define N_NOINLINE_PTR(rettype, name) rettype (*name)

#if defined(_MSC_VER)
#  define NIM_ALIGN(x)  __declspec(align(x))
#  define NIM_ALIGNOF(x) __alignof(x)
#else
#  define NIM_ALIGN(x)  __attribute__((aligned(x)))
#  define NIM_ALIGNOF(x) __alignof__(x)
#endif

#include <stddef.h>


/*
  NIM_THREADVAR declaration based on
  https://stackoverflow.com/questions/18298280/how-to-declare-a-variable-as-thread-local-portably
*/
#if defined _WIN32
#  if defined _MSC_VER || defined __BORLANDC__
#    define NIM_THREADVAR __declspec(thread)
#  else
#    define NIM_THREADVAR __thread
#  endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define NIM_THREADVAR _Thread_local
#elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __BORLANDC__ )
#  define NIM_THREADVAR __declspec(thread)
#elif defined(__TINYC__) || defined(__GENODE__)
#  define NIM_THREADVAR
/* note that ICC (linux) and Clang are covered by __GNUC__ */
#elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define NIM_THREADVAR __thread
#else
#  error "Cannot define NIM_THREADVAR"
#endif

/* define NIM_STATIC_ASSERT */
#if defined(__cplusplus)
#define NIM_STATIC_ASSERT(x, msg) static_assert((x), msg)
#else
#define NIM_STATIC_ASSERT(x, msg) _Static_assert((x), msg)
#endif

// Test to see if Nim and the C compiler agree on the size of a pointer.
NIM_STATIC_ASSERT(sizeof(NI) == sizeof(void*) && NIM_INTBITS == sizeof(NI)*8, "Pointer size mismatch between Nim and C/C++ backend. You probably need to setup the backend compiler for target CPU.");

N_INLINE(NB8, _Qlengc_div_sll_overflow)(long long int a, long long int b, long long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  if (a == (long long int)(((unsigned long long int)1) << (sizeof(long long int) * 8 - 1)) && b == -1) {
    *res = a;
    return NIM_TRUE;
  }
  *res = a / b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_div_sl_overflow)(long int a, long int b, long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  if (a == (long int)(((unsigned long int)1) << (sizeof(long int) * 8 - 1)) && b == -1) {
    *res = a;
    return NIM_TRUE;
  }
  *res = a / b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_div_ull_overflow)(unsigned long long int a, unsigned long long int b, unsigned long long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE; /* Overflow: division by zero */
  }
  *res = a / b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_div_ul_overflow)(unsigned long int a, unsigned long int b, unsigned long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  *res = a / b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_mod_sll_overflow)(long long int a, long long int b, long long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  if (a == (long long int)(((unsigned long long int)1) << (sizeof(long long int) * 8 - 1)) && b == -1) {
    *res = 0;
    return NIM_TRUE;
  }
  *res = a % b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_mod_sl_overflow)(long int a, long int b, long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  if (a == (long int)(((unsigned long int)1) << (sizeof(long int) * 8 - 1)) && b == -1) {
    *res = 0;
    return NIM_TRUE;
  }
  *res = a % b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_mod_ull_overflow)(unsigned long long int a, unsigned long long int b, unsigned long long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  *res = a % b;
  return NIM_FALSE;
}

N_INLINE(NB8, _Qlengc_mod_ul_overflow)(unsigned long int a, unsigned long int b, unsigned long int *res) {
  if (b == 0) {
    *res = 0;
    return NIM_TRUE;
  }
  *res = a % b;
  return NIM_FALSE;
}
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <dlfcn.h>
typedef struct LongString_0_sysvq0asl LongString_0_sysvq0asl;
typedef struct Rtti_0_sysvq0asl Rtti_0_sysvq0asl;
typedef struct Trunk_0_sysvq0asl Trunk_0_sysvq0asl;
typedef struct LLChunk_0_sysvq0asl LLChunk_0_sysvq0asl;
typedef struct AvlNode_0_sysvq0asl AvlNode_0_sysvq0asl;
typedef struct FreeCell_0_sysvq0asl FreeCell_0_sysvq0asl;
typedef struct MemRegion_0_sysvq0asl MemRegion_0_sysvq0asl;
typedef struct SmallChunk_0_sysvq0asl SmallChunk_0_sysvq0asl;
typedef struct BigChunk_0_sysvq0asl BigChunk_0_sysvq0asl;
typedef struct HeapLinks_0_sysvq0asl HeapLinks_0_sysvq0asl;
typedef struct CoroutineBase_0_sysvq0asl CoroutineBase_0_sysvq0asl;
typedef struct Continuation_0_sysvq0asl Continuation_0_sysvq0asl;
typedef struct Exception_0_sysvq0asl Exception_0_sysvq0asl;
typedef struct LongString_0_sysvq0asl{
  NI32 fullLen_0;
  NI32 rc_0;
  NI32 capImpl_0;
  NC8 data_0[];}
LongString_0_sysvq0asl;
typedef struct string_0_sysvq0asl{
  NU32 bytes_0;
  LongString_0_sysvq0asl* more_0;}
string_0_sysvq0asl;
typedef struct RootObj_0_sysvq0asl{
  Rtti_0_sysvq0asl* vt_00;}
RootObj_0_sysvq0asl;
typedef struct X60Qt_0_IAarrayAuS32ZS16_sysvq0asl{
  NU32 a[16];}
X60Qt_0_IAarrayAuS32ZS16_sysvq0asl;
typedef struct Trunk_0_sysvq0asl{
  Trunk_0_sysvq0asl* next_0;
  NI32 key_0;
  X60Qt_0_IAarrayAuS32ZS16_sysvq0asl bits_0;}
Trunk_0_sysvq0asl;
typedef struct X60Qt_0_IAarrayAptrSX54runk0sysvq0aslZS256_sysvq0asl{
  Trunk_0_sysvq0asl* a[256];}
X60Qt_0_IAarrayAptrSX54runk0sysvq0aslZS256_sysvq0asl;
typedef struct IntSet_0_sysvq0asl{
  X60Qt_0_IAarrayAptrSX54runk0sysvq0aslZS256_sysvq0asl data_0;}
IntSet_0_sysvq0asl;
typedef struct LLChunk_0_sysvq0asl{
  NI32 size_0;
  NI32 acc_0;
  LLChunk_0_sysvq0asl* next_0;}
LLChunk_0_sysvq0asl;
typedef struct X60Qt_0_IAarrayAptrSX41vlX4eode0sysvq0aslZS2_sysvq0asl{
  AvlNode_0_sysvq0asl* a[2];}
X60Qt_0_IAarrayAptrSX41vlX4eode0sysvq0aslZS2_sysvq0asl;
typedef struct AvlNode_0_sysvq0asl{
  X60Qt_0_IAarrayAptrSX41vlX4eode0sysvq0aslZS2_sysvq0asl link_0;
  NI32 key_0;
  NI32 upperBound_0;
  NI32 level_0;}
AvlNode_0_sysvq0asl;
typedef struct FreeCell_0_sysvq0asl{
  FreeCell_0_sysvq0asl* next_0;
  NI32 alignment_0;}
FreeCell_0_sysvq0asl;
typedef struct BaseChunk_0_sysvq0asl{
  NI32 prevSize_0;
  NI32 size_0;
  MemRegion_0_sysvq0asl* owner_0;}
BaseChunk_0_sysvq0asl;
typedef struct SmallChunk_0_sysvq0asl{
  BaseChunk_0_sysvq0asl Q;
  SmallChunk_0_sysvq0asl* next_0;
  SmallChunk_0_sysvq0asl* prev_0;
  FreeCell_0_sysvq0asl* freeList_0;
  NI32 free_0;
  NU32 acc_0;
  NI32 foreignCells_0;
  NI32 chunkAlignOff_0;
   NIM_ALIGN(16)NU8 data_0[];}
SmallChunk_0_sysvq0asl;
typedef struct BigChunk_0_sysvq0asl{
  BaseChunk_0_sysvq0asl Q;
  BigChunk_0_sysvq0asl* next_0;
  BigChunk_0_sysvq0asl* prev_0;
   NIM_ALIGN(16)NU8 data_0[];}
BigChunk_0_sysvq0asl;
typedef struct X60Qt_0_IAtupleAptrSX42igX43hunk0sysvq0aslZAiS32_sysvq0asl{
  BigChunk_0_sysvq0asl* fld_0;
  NI32 fld_1;}
X60Qt_0_IAtupleAptrSX42igX43hunk0sysvq0aslZAiS32_sysvq0asl;
typedef struct X60Qt_0_IAarrayAtupleAptrSX42igX43hunk0sysvq0aslZAiS32ZZS30_sysvq0asl{
  X60Qt_0_IAtupleAptrSX42igX43hunk0sysvq0aslZAiS32_sysvq0asl a[30];}
X60Qt_0_IAarrayAtupleAptrSX42igX43hunk0sysvq0aslZAiS32ZZS30_sysvq0asl;
typedef struct HeapLinks_0_sysvq0asl{
  NI32 count_0;
  X60Qt_0_IAarrayAtupleAptrSX42igX43hunk0sysvq0aslZAiS32ZZS30_sysvq0asl chunks_0;
  HeapLinks_0_sysvq0asl* next_0;}
HeapLinks_0_sysvq0asl;
typedef struct X60Qt_0_IAarrayAptrSX53mallX43hunk0sysvq0aslZS256_sysvq0asl{
  SmallChunk_0_sysvq0asl* a[256];}
X60Qt_0_IAarrayAptrSX53mallX43hunk0sysvq0aslZS256_sysvq0asl;
typedef struct X60Qt_0_IAarrayAptrSX46reeX43ell0sysvq0aslZS256_sysvq0asl{
  FreeCell_0_sysvq0asl* a[256];}
X60Qt_0_IAarrayAptrSX46reeX43ell0sysvq0aslZS256_sysvq0asl;
typedef struct X60Qt_0_IAarrayAuS32ZS24_sysvq0asl{
  NU32 a[24];}
X60Qt_0_IAarrayAuS32ZS24_sysvq0asl;
typedef struct X60Qt_0_IAarrayAptrSX42igX43hunk0sysvq0aslZS32_sysvq0asl{
  BigChunk_0_sysvq0asl* a[32];}
X60Qt_0_IAarrayAptrSX42igX43hunk0sysvq0aslZS32_sysvq0asl;
typedef struct X60Qt_0_IAarrayAR0AptrSX42igX43hunk0sysvq0aslZS32ZS24_sysvq0asl{
  X60Qt_0_IAarrayAptrSX42igX43hunk0sysvq0aslZS32_sysvq0asl a[24];}
X60Qt_0_IAarrayAR0AptrSX42igX43hunk0sysvq0aslZS32ZS24_sysvq0asl;
typedef struct MemRegion_0_sysvq0asl{
  X60Qt_0_IAarrayAptrSX53mallX43hunk0sysvq0aslZS256_sysvq0asl freeSmallChunks_0;
  X60Qt_0_IAarrayAptrSX46reeX43ell0sysvq0aslZS256_sysvq0asl sharedFreeLists_0;
  NU32 flBitmap_0;
  X60Qt_0_IAarrayAuS32ZS24_sysvq0asl slBitmap_0;
  X60Qt_0_IAarrayAR0AptrSX42igX43hunk0sysvq0aslZS32ZS24_sysvq0asl matrix_0;
  LLChunk_0_sysvq0asl* llmem_0;
  NI32 currMem_0;
  NI32 maxMem_0;
  NI32 freeMem_0;
  NI32 occ_0;
  NI32 lastSize_0;
  BigChunk_0_sysvq0asl* sharedFreeListBigChunks_0;
  IntSet_0_sysvq0asl chunkStarts_0;
  NB8 lockActive_0;
  NB8 locked_0;
  NB8 blockChunkSizeIncrease_0;
  NI32 nextChunkSize_0;
  HeapLinks_0_sysvq0asl heapLinks_0;}
MemRegion_0_sysvq0asl;
typedef struct Rtti_0_sysvq0asl{
  NI32 dl_0;
  NU32* dy_0;
  void* mt_0[];}
Rtti_0_sysvq0asl;
typedef struct Single_0_sysvq0asl{
  NU32 bits_0;}
Single_0_sysvq0asl;
typedef struct FloatingDecimal32_0_sysvq0asl{
  NU32 digits_0;
  NI32 exponent_0;}
FloatingDecimal32_0_sysvq0asl;
typedef struct Double_0_sysvq0asl{
  NU64 bits_0;}
Double_0_sysvq0asl;
typedef struct uint64x2_0_sysvq0asl{
  NU64 hi_0;
  NU64 lo_0;}
uint64x2_0_sysvq0asl;
typedef struct MulCmp_0_sysvq0asl{
  NU64 mul_0;
  NU64 cmp_0;}
MulCmp_0_sysvq0asl;
typedef struct FloatingDecimal64_0_sysvq0asl{
  NU64 significand_0;
  NI32 exponent_0;}
FloatingDecimal64_0_sysvq0asl;
typedef NU8 ErrorCode_0_sysvq0asl;

#define Success_0_sysvq0asl ((NU8)0)
#define OverflowError_0_sysvq0asl ((NU8)1)
#define Failure_0_sysvq0asl ((NU8)2)
#define BugError_0_sysvq0asl ((NU8)3)
#define IndexError_0_sysvq0asl ((NU8)4)
#define RangeError_0_sysvq0asl ((NU8)5)
#define OverlapError_0_sysvq0asl ((NU8)6)
#define SyntaxError_0_sysvq0asl ((NU8)7)
#define OutOfMemError_0_sysvq0asl ((NU8)8)
#define DiskFullError_0_sysvq0asl ((NU8)9)
#define StackOverflow_0_sysvq0asl ((NU8)10)
#define IOError_0_sysvq0asl ((NU8)11)
#define ValueError_0_sysvq0asl ((NU8)12)
#define KeyError_0_sysvq0asl ((NU8)13)
#define EndOfStreamError_0_sysvq0asl ((NU8)14)
#define SkipError_0_sysvq0asl ((NU8)15)
#define FullError_0_sysvq0asl ((NU8)16)
#define EmptyError_0_sysvq0asl ((NU8)17)
#define BusyError_0_sysvq0asl ((NU8)18)
#define DeadResource_0_sysvq0asl ((NU8)19)
#define ResourceExhaustedError_0_sysvq0asl ((NU8)20)
#define DescriptorExhaustedError_0_sysvq0asl ((NU8)21)
#define PermissionDenied_0_sysvq0asl ((NU8)22)
#define RetryError_0_sysvq0asl ((NU8)23)
#define TimeoutError_0_sysvq0asl ((NU8)24)
#define InterruptedError_0_sysvq0asl ((NU8)25)
#define DeadlockError_0_sysvq0asl ((NU8)26)
#define LockedError_0_sysvq0asl ((NU8)27)
#define FormatMismatch_0_sysvq0asl ((NU8)28)
#define AlreadyConnected_0_sysvq0asl ((NU8)29)
#define AddressNotAvailable_0_sysvq0asl ((NU8)30)
#define AddressFamilyUnsupported_0_sysvq0asl ((NU8)31)
#define BadOperation_0_sysvq0asl ((NU8)32)
#define AbortedOperation_0_sysvq0asl ((NU8)33)
#define UnimplementedOperation_0_sysvq0asl ((NU8)34)
#define AlreadyInProgress_0_sysvq0asl ((NU8)35)
#define NameTooLong_0_sysvq0asl ((NU8)36)
#define NameExists_0_sysvq0asl ((NU8)37)
#define NameNotFound_0_sysvq0asl ((NU8)38)
#define ContentTooLong_0_sysvq0asl ((NU8)39)
#define BadDescriptor_0_sysvq0asl ((NU8)40)
#define BadExecutable_0_sysvq0asl ((NU8)41)
#define BadLink_0_sysvq0asl ((NU8)42)
#define BadProtocol_0_sysvq0asl ((NU8)43)
#define ProtocolError_0_sysvq0asl ((NU8)44)
#define ReadonlyProtection_0_sysvq0asl ((NU8)45)
#define SegFault_0_sysvq0asl ((NU8)46)
#define DiskCorruption_0_sysvq0asl ((NU8)47)
#define Disconnected_0_sysvq0asl ((NU8)48)
#define RefusedConnection_0_sysvq0asl ((NU8)49)
#define UnreachableHost_0_sysvq0asl ((NU8)50)
#define UnrecoverableState_0_sysvq0asl ((NU8)51)
#define AuthenticationRequired_0_sysvq0asl ((NU8)52)
#define RedirectError_0_sysvq0asl ((NU8)53)
#define Reserved1_0_sysvq0asl ((NU8)54)
#define Reserved2_0_sysvq0asl ((NU8)55)
#define Reserved3_0_sysvq0asl ((NU8)56)
#define Reserved4_0_sysvq0asl ((NU8)57)
#define Reserved5_0_sysvq0asl ((NU8)58)
#define Reserved6_0_sysvq0asl ((NU8)59)
#define Reserved7_0_sysvq0asl ((NU8)60)
#define Reserved8_0_sysvq0asl ((NU8)61)
#define Reserved9_0_sysvq0asl ((NU8)62)
typedef N_NIMCALL_PTR(Continuation_0_sysvq0asl,  X60Qt_0_IAptrSX43oroutineX42ase0sysvq0aslZSX43ontinuation0R22AnimcallZAfalseZAR61_sysvq0asl)(CoroutineBase_0_sysvq0asl*);
typedef struct Continuation_0_sysvq0asl{
  X60Qt_0_IAptrSX43oroutineX42ase0sysvq0aslZSX43ontinuation0R22AnimcallZAfalseZAR61_sysvq0asl fn_0;
  CoroutineBase_0_sysvq0asl* env_0;}
Continuation_0_sysvq0asl;
typedef struct CoroutineBase_0_sysvq0asl{
  RootObj_0_sysvq0asl Q;
  Continuation_0_sysvq0asl caller_0;
  CoroutineBase_0_sysvq0asl* callee_0;}
CoroutineBase_0_sysvq0asl;
typedef NU8 TypeOfMode_0_sysvq0asl;

#define typeOfProc_0_sysvq0asl ((NU8)0)
#define typeOfIter_0_sysvq0asl ((NU8)1)
typedef struct Exception_0_sysvq0asl{
  RootObj_0_sysvq0asl Q;
  string_0_sysvq0asl msg_0;}
Exception_0_sysvq0asl;
typedef struct openArray_0_Ijk0jkw1_sysvq0asl{
  NC8* a_0;
  NI32 len_0;}
openArray_0_Ijk0jkw1_sysvq0asl;
typedef struct HSlice_0_Inzqy8d_sysvq0asl{
  NI32 a_0;
  NI32 b_0;}
HSlice_0_Inzqy8d_sysvq0asl;
typedef struct HSlice_0_I8tnmvh1_sysvq0asl{
  NI32 a_0;
  NI32 b_0;}
HSlice_0_I8tnmvh1_sysvq0asl;
typedef struct X60Qt_0_IArefSX52ootX4fbj0sysvq0asl_sysvq0asl{
  NI r_00;
  RootObj_0_sysvq0asl d_00;}
X60Qt_0_IArefSX52ootX4fbj0sysvq0asl_sysvq0asl;
typedef struct X60Qt_0_IAarraySstring0sysvq0aslS10_sysvq0asl{
  string_0_sysvq0asl a[10];}
X60Qt_0_IAarraySstring0sysvq0aslS10_sysvq0asl;
typedef N_NIMCALL_PTR(void,  X60Qt_0_ISEAnimcallZAfalseZAR11_jsfc0lwq21)(void);
typedef struct X60Qt_0_IAarrayAuS8ZS256_sysvq0asl{
  NU8 a[256];}
X60Qt_0_IAarrayAuS8ZS256_sysvq0asl;
typedef struct X60Qt_0_IAarrayAiS8ZS256_sysvq0asl{
  NI8 a[256];}
X60Qt_0_IAarrayAiS8ZS256_sysvq0asl;
typedef struct X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl{
  NI32 fld_0;
  NI32 fld_1;}
X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl;
typedef N_NIMCALL_PTR(void,  X60Qt_0_IAiS32ZSEAnimcallZAfalseZAR17_sysvq0asl)(NI32);
typedef struct X60Qt_0_IAarrayAiS8ZS100_sysvq0asl{
  NI8 a[100];}
X60Qt_0_IAarrayAiS8ZS100_sysvq0asl;
typedef struct X60Qt_0_IAarrayAcS8ZS200_sysvq0asl{
  NC8 a[200];}
X60Qt_0_IAarrayAcS8ZS200_sysvq0asl;
typedef struct X60Qt_0_IAarrayAuS64ZS77_sysvq0asl{
  NU64 a[77];}
X60Qt_0_IAarrayAuS64ZS77_sysvq0asl;
typedef struct X60Qt_0_IAarraySuint64x20sysvq0aslS619_sysvq0asl{
  uint64x2_0_sysvq0asl a[619];}
X60Qt_0_IAarraySuint64x20sysvq0aslS619_sysvq0asl;
typedef struct X60Qt_0_IAarraySX4dulX43mp0sysvq0aslS25_sysvq0asl{
  MulCmp_0_sysvq0asl a[25];}
X60Qt_0_IAarraySX4dulX43mp0sysvq0aslS25_sysvq0asl;
typedef struct X60Qt_0_IAarrayAcS8ZS16_sysvq0asl{
  NC8 a[16];}
X60Qt_0_IAarrayAcS8ZS16_sysvq0asl;
typedef struct X60Qt_0_IAarrayAcS8ZS65_sysvq0asl{
  NC8 a[65];}
X60Qt_0_IAarrayAcS8ZS65_sysvq0asl;
typedef N_NIMCALL_PTR(Continuation_0_sysvq0asl,  X60Qt_0_ISX43ontinuation0sysvq0aslSR0AnimcallZAfalseZAR37_sysvq0asl)(Continuation_0_sysvq0asl);
typedef N_NIMCALL_PTR(void,  X60Qt_0_IAptrSX43oroutineX42ase0sysvq0aslZSEAnimcallZAfalseZAR44_sysvq0asl)(CoroutineBase_0_sysvq0asl*);
typedef struct X60Qt_0_IArefSX45xception0sysvq0asl_sysvq0asl{
  NI r_00;
  Exception_0_sysvq0asl d_00;}
X60Qt_0_IArefSX45xception0sysvq0asl_sysvq0asl;
typedef N_NIMCALL_PTR(void,  X60Qt_0_ISX45xception0sysvq0aslSEAnimcallZAfalseZAR33_sysvq0asl)(Exception_0_sysvq0asl*);
static inline NI32 len_4_sysvq0asl(string_0_sysvq0asl s_34);
static inline void nimStrWasMoved(string_0_sysvq0asl* s_40);
static inline void nimStrDestroy(string_0_sysvq0asl s_41);
static inline string_0_sysvq0asl nimStrDup(string_0_sysvq0asl s_42);
void add_1_sysvq0asl(string_0_sysvq0asl* s_49, NC8 c_14);
static inline NC8 getQ_9_sysvq0asl(string_0_sysvq0asl s_54, NI32 i_14);
static inline void putQ_9_sysvq0asl(string_0_sysvq0asl* s_55, NI32 i_15, NC8 c_15);
string_0_sysvq0asl substr_0_sysvq0asl(string_0_sysvq0asl s_56, NI32 first_0, NI32 last_0);
string_0_sysvq0asl ampQ_0_sysvq0asl(string_0_sysvq0asl a_54, string_0_sysvq0asl b_17);
static inline void arcInc_0_sysvq0asl(NI32* memLoc_0);
static inline NB8 arcDec_0_sysvq0asl(NI32* memLoc_1);
static inline NB8 arcIsUnique_0_sysvq0asl(NI32* memLoc_2);
void panic_0_sysvq0asl(string_0_sysvq0asl s_70);
static inline NI32 nimIcheckB(NI32 i_19, NI32 b_22);
static inline NU32 nimUcheckB(NU32 i_21, NU32 b_24);
static inline void inc_0_Iloplki_sysvq0asl(NI32* x_374, NI32 y_215);
static inline void dec_0_Ig5i8xp_sysvq0asl(NI32* x_376, NI32 y_217);
static inline void dec_1_I0nzoz91_sysvq0asl(NI32* x_377);
static inline void listAdd_0_Ik4wxhz_sysvq0asl(SmallChunk_0_sysvq0asl** head_5, SmallChunk_0_sysvq0asl* c_38);
static inline void listRemove_0_Ibzev091_sysvq0asl(SmallChunk_0_sysvq0asl** head_6, SmallChunk_0_sysvq0asl* c_39);
void raiseIndexError3_0_I113jpc1_sysvq0asl(NI32 i_68, NI32 a_83, NI32 b_38);
void raiseIndexError3_0_Ic5mmkg_sysvq0asl(NU32 i_69, NU32 a_84, NU32 b_39);
static inline void inc_1_I6wjjge_jsfc0lwq21(NI32* x_11);
extern LongString_0_sysvq0asl const strlit_0_I8572766038233537570_syn1lfpjv;
extern LongString_0_sysvq0asl const strlit_0_I3372626016653902757_syn1lfpjv;
LongString_0_sysvq0asl const strlit_0_I1565838944098044620_sysvq0asl = {
  .fullLen_0 = 93, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "../nimony-js/lib/std/system/stringimpl.nim(403, 37): i < len(s) and 0 <= i [AssertionDefect]\012"}
;
LongString_0_sysvq0asl const strlit_0_I7364560965974357967_sysvq0asl = {
  .fullLen_0 = 93, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "../nimony-js/lib/std/system/stringimpl.nim(407, 45): i < len(s) and 0 <= i [AssertionDefect]\012"}
;
LongString_0_sysvq0asl const strlit_0_I15539159382304113184_sysvq0asl = {
  .fullLen_0 = 27, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "invalid object conversion: "}
;
LongString_0_sysvq0asl const strlit_0_I14281474217946372742_sysvq0asl = {
  .fullLen_0 = 35, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "cannot dispatch; dispatcher is nil\012"}
;
LongString_0_sysvq0asl const strlit_0_I16690852185662743073_sysvq0asl = {
  .fullLen_0 = 16, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "could not load: "}
;
LongString_0_sysvq0asl const strlit_0_I10604297744791418982_sysvq0asl = {
  .fullLen_0 = 18, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "could not import: "}
;
LongString_0_sysvq0asl const strlit_0_I11614695157650328859_sysvq0asl = {
  .fullLen_0 = 21, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "index out of bounds: "}
;
LongString_0_sysvq0asl const strlit_0_I16845119709590674135_sysvq0asl = {
  .fullLen_0 = 7, .rc_0 = 0, .capImpl_0 = 0, .data_0 = " notin "}
;
X60Qt_0_IAarraySstring0sysvq0aslS10_sysvq0asl const NegTen_0_sysvq0asl = {
  .a = {
    {
      .bytes_0 = 3157250u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3222786u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3288322u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3353858u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3419394u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3484930u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3550466u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3616002u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3681538u, .more_0 = NIM_NIL}
    , {
      .bytes_0 = 3747074u, .more_0 = NIM_NIL}}}
;
X60Qt_0_ISEAnimcallZAfalseZAR11_jsfc0lwq21 gExitFlush_0_sysvq0asl;
X60Qt_0_IAarrayAiS8ZS256_sysvq0asl const fsLookupTable_0_sysvq0asl = {
  .a = {
    ((NI8)-1), ((NI8)0), ((NI8)1), ((NI8)1), ((NI8)2), ((NI8)2), ((NI8)2), ((NI8)2), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)3), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)4), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)5), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)6), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7), ((NI8)7)}}
;
__thread MemRegion_0_sysvq0asl allocator_0_sysvq0asl;
__thread NI32 missingBytes_0_sysvq0asl;
X60Qt_0_IAiS32ZSEAnimcallZAfalseZAR17_sysvq0asl oomHandler_0_sysvq0asl;
__thread X60Qt_0_IArefSX45xception0sysvq0asl_sysvq0asl* exc_0_sysvq0asl;
NB8 X60QiniGuard_0_sysvq0asl;
static inline NI32 min_2_sysvq0asl(NI32 x_204, NI32 y_161){
  NI32 result_5;
  NI32 X60Qx_2;
  if (x_204 <= y_161){
    X60Qx_2 = x_204;}
  else {
    X60Qx_2 = y_161;}
  result_5 = X60Qx_2;
  return result_5;}
static inline NI32 max_2_sysvq0asl(NI32 x_211, NI32 y_168){
  NI32 result_12;
  NI32 X60Qx_8;
  if (y_168 <= x_211){
    X60Qx_8 = x_211;}
  else {
    X60Qx_8 = y_168;}
  result_12 = X60Qx_8;
  return result_12;}
string_0_sysvq0asl dollarQ_0_sysvq0asl(NU64 x_224){
  string_0_sysvq0asl result_19;
  nimStrWasMoved((&result_19));
  nimStrDestroy(result_19);
  result_19 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  if (x_224 < ((NU64)10)){
    nimStrDestroy(result_19);
    NI32 X60Qx_57 = nimIcheckB(((NI32)x_224), 9);
    string_0_sysvq0asl X60Qx_58 = substr_0_sysvq0asl(NegTen_0_sysvq0asl.a[X60Qx_57], 1, 1);
    result_19 = X60Qx_58;}
  else {
    NU64 y_208 = x_224;
    {
      while (NIM_TRUE){
        add_1_sysvq0asl((&result_19), ((NC8)((NU64)(((NU64)(y_208 % ((NU64)10u))) + ((NU64)((NU32)(NC8)'0'))))));
        y_208 = ((NU64)(y_208 / ((NU64)10u)));
        if (y_208 == ((NU64)0u)){
          goto whileStmtLabel_0;}}}
    whileStmtLabel_0: ;
    NI32 X60Qx_59 = len_4_sysvq0asl(result_19);
    NI32 last_3 = ((NI32)(X60Qx_59 - 1));
    NI32 i_25 = 0;
    NI32 X60Qx_60 = len_4_sysvq0asl(result_19);
    NI32 b_29 = ((NI32)(X60Qx_60 / 2));
    {
      while (i_25 < b_29){
        NC8 ch_1 = getQ_9_sysvq0asl(result_19, i_25);
        NC8 X60Qx_61 = getQ_9_sysvq0asl(result_19, ((NI32)(last_3 - i_25)));
        putQ_9_sysvq0asl((&result_19), i_25, X60Qx_61);
        putQ_9_sysvq0asl((&result_19), ((NI32)(last_3 - i_25)), ch_1);
        inc_1_I6wjjge_jsfc0lwq21((&i_25));}}
    whileStmtLabel_1: ;}
  return result_19;}
string_0_sysvq0asl dollarQ_1_sysvq0asl(NI64 x_225){
  string_0_sysvq0asl result_20;
  nimStrWasMoved((&result_20));
  if (x_225 < ((NI64)0)){
    if (((NI64)-10) < x_225){
      nimStrDestroy(result_20);
      NI32 X60Qx_62 = nimIcheckB(((NI32)((NI64)-x_225)), 9);
      string_0_sysvq0asl X60Qx_63 = nimStrDup(NegTen_0_sysvq0asl.a[X60Qx_62]);
      result_20 = X60Qx_63;}
    else {
      if (x_225 == ((NI64)(IL64(-9223372036854775807) - IL64(1)))){
        nimStrDestroy(result_20);
        string_0_sysvq0asl X60Qtmp_0 = dollarQ_0_sysvq0asl(((NU64)x_225));
        string_0_sysvq0asl X60Qx_64 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
          .bytes_0 = 11521u, .more_0 = NIM_NIL}
        , X60Qtmp_0);
        result_20 = X60Qx_64;
        nimStrDestroy(X60Qtmp_0);}
      else {
        nimStrDestroy(result_20);
        string_0_sysvq0asl X60Qtmp_1 = dollarQ_1_sysvq0asl(((NI64)(((NI64)0) - x_225)));
        string_0_sysvq0asl X60Qx_65 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
          .bytes_0 = 11521u, .more_0 = NIM_NIL}
        , X60Qtmp_1);
        result_20 = X60Qx_65;
        nimStrDestroy(X60Qtmp_1);}}}
  else {
    if (x_225 < ((NI64)10)){
      nimStrDestroy(result_20);
      result_20 = (string_0_sysvq0asl){
        .bytes_0 = 0u, .more_0 = NIM_NIL}
      ;
      add_1_sysvq0asl((&result_20), ((NC8)((NI64)(x_225 + ((NI64)(NC8)'0')))));}
    else {
      nimStrDestroy(result_20);
      string_0_sysvq0asl X60Qx_66 = dollarQ_0_sysvq0asl(((NU64)x_225));
      result_20 = X60Qx_66;}}
  return result_20;}
string_0_sysvq0asl dollarQ_2_sysvq0asl(NI32 x_226){
  string_0_sysvq0asl result_21;
  nimStrWasMoved((&result_21));
  nimStrDestroy(result_21);
  string_0_sysvq0asl X60Qx_67 = dollarQ_1_sysvq0asl(((NI64)x_226));
  result_21 = X60Qx_67;
  return result_21;}
string_0_sysvq0asl dollarQ_3_sysvq0asl(NB8 b_1){
  string_0_sysvq0asl result_22;
  nimStrWasMoved((&result_22));
  string_0_sysvq0asl X60Qx_12;
  nimStrWasMoved((&X60Qx_12));
  if (b_1){
    nimStrDestroy(X60Qx_12);
    X60Qx_12 = (string_0_sysvq0asl){
      .bytes_0 = 1970435326u, .more_0 = (&strlit_0_I8572766038233537570_syn1lfpjv)}
    ;}
  else {
    nimStrDestroy(X60Qx_12);
    X60Qx_12 = (string_0_sysvq0asl){
      .bytes_0 = 1818322686u, .more_0 = (&strlit_0_I3372626016653902757_syn1lfpjv)}
    ;}
  nimStrDestroy(result_22);
  result_22 = X60Qx_12;
  nimStrWasMoved((&X60Qx_12));
  nimStrDestroy(X60Qx_12);
  return result_22;
  nimStrDestroy(X60Qx_12);
  return result_22;}
N_NIMCALL(void, nimNoopFlush_0_sysvq0asl)(void){
  }
void nimFlushStdStreams(void){
  gExitFlush_0_sysvq0asl();}
void cAbort_0_sysvq0asl(void){
  gExitFlush_0_sysvq0asl();
  abort();}
static inline void copyMem_0_sysvq0asl(void* dest_4, void* src_3, NI32 size_3){
  memcpy(dest_4, src_3, ((size_t)size_3));}
static inline void zeroMem_0_sysvq0asl(void* dest_6, NI32 size_6){
  memset(dest_6, 0, ((size_t)size_6));}
void raiseOutOfMem_0_sysvq0asl(void){
  cAbort_0_sysvq0asl();}
static inline NI32 align_0_sysvq0asl(NI32 address_0, NI32 alignment_0){
  NI32 result_30;
  result_30 = ((NI32)(((NI32)(address_0 + ((NI32)(alignment_0 - 1)))) & ((NI32) ~ ((NI32)(alignment_0 - 1)))));
  return result_30;}
static inline NI32 roundup_0_sysvq0asl(NI32 x_296, NI32 v_0){
  NI32 result_31;
  result_31 = ((NI32)(((NI32)(x_296 + ((NI32)(v_0 - 1)))) & ((NI32) ~ ((NI32)(v_0 - 1)))));
  return result_31;}
static inline void* osAllocPages_0_sysvq0asl(NI32 size_7){
  void* result_32;
  void* X60Qx_78 = mmap(NIM_NIL, ((size_t)size_7), ((int)((NI32)(((NI32)1) | ((NI32)2)))), ((int)((NI32)(((NI32)(((NI32)32) | ((NI32)2))) | ((NI32)0)))), ((int)-1), 0);
  result_32 = X60Qx_78;
  NB8 X60Qx_79;
  if (result_32 == NIM_NIL){
    X60Qx_79 = NIM_TRUE;}
  else {
    X60Qx_79 = (result_32 == ((void*)-1));}
  if (X60Qx_79){
    raiseOutOfMem_0_sysvq0asl();}
  return result_32;}
static inline void* osTryAllocPages_0_sysvq0asl(NI32 size_8){
  void* result_33;
  void* X60Qx_80 = mmap(NIM_NIL, ((size_t)size_8), ((int)((NI32)(((NI32)1) | ((NI32)2)))), ((int)((NI32)(((NI32)(((NI32)32) | ((NI32)2))) | ((NI32)0)))), ((int)-1), 0);
  result_33 = X60Qx_80;
  if (result_33 == ((void*)-1)){
    result_33 = NIM_NIL;}
  return result_33;}
static inline void osDeallocPages_0_sysvq0asl(void* p_9, NI32 size_9){
  }
static inline NI32 msbit_0_sysvq0asl(NU32 x_301){
  NI32 result_34;
  NI64 X60Qx_13;
  if (x_301 <= ((NU32)65535u)){
    NI64 X60Qx_14;
    if (x_301 <= ((NU32)255)){
      X60Qx_14 = 0;}
    else {
      X60Qx_14 = 8;}
    X60Qx_13 = X60Qx_14;}
  else {
    NI64 X60Qx_15;
    if (x_301 <= ((NU32)16777215u)){
      X60Qx_15 = 16;}
    else {
      X60Qx_15 = 24;}
    X60Qx_13 = X60Qx_15;}
  NI32 a_74 = X60Qx_13;
  NU32 X60Qx_81 = nimUcheckB(((NU8)((NU32)(x_301 >> a_74))), 255);
  result_34 = ((NI32)(((NI32)fsLookupTable_0_sysvq0asl.a[X60Qx_81]) + a_74));
  return result_34;}
static inline NI32 lsbit_0_sysvq0asl(NU32 x_302){
  NI32 result_35;
  NI32 X60Qx_82 = msbit_0_sysvq0asl(((NU32)(x_302 & ((NU32)(((NU32) ~ x_302) + ((NU32)1))))));
  result_35 = X60Qx_82;
  return result_35;}
static inline void setBit_0_sysvq0asl(NI32 nr_0, NU32* dest_7){
  (*dest_7) = ((NU32)((*dest_7) | ((NU32)(((NU32)1u) << ((NI32)(nr_0 & 31))))));}
static inline void clearBit_0_sysvq0asl(NI32 nr_1, NU32* dest_8){
  (*dest_8) = ((NU32)((*dest_8) & ((NU32) ~ ((NU32)(((NU32)1u) << ((NI32)(nr_1 & 31)))))));}
static inline void mappingSearch_0_sysvq0asl(NI32* r_0, NI32* fl_0, NI32* sl_0){
  NI32 X60Qx_83 = msbit_0_sysvq0asl(((NU32)(*r_0)));
  NI32 X60Qx_84 = roundup_0_sysvq0asl(((NI32)(1 << ((NI32)(X60Qx_83 - ((NI32)5))))), ((NI32)4096));
  NI32 t_3 = ((NI32)(X60Qx_84 - 1));
  (*r_0) = ((NI32)((*r_0) + t_3));
  (*r_0) = ((NI32)((*r_0) & ((NI32) ~ t_3)));
  NI32 X60Qx_85 = min_2_sysvq0asl((*r_0), ((NI32)1056964608));
  (*r_0) = ((NI32)X60Qx_85);
  NI32 X60Qx_86 = msbit_0_sysvq0asl(((NU32)(*r_0)));
  (*fl_0) = X60Qx_86;
  (*sl_0) = ((NI32)(((NI32)(((NI32)(*r_0)) >> ((NU32)((NI32)((*fl_0) - ((NI32)5)))))) - ((NI32)32)));
  dec_0_Ig5i8xp_sysvq0asl(fl_0, ((NI32)6));}
static inline X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl mappingInsert_0_sysvq0asl(NI32 r_1){
  X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl result_36;
  NI32 fl_4 = msbit_0_sysvq0asl(((NU32)r_1));
  NI32 sl_5 = ((NI32)(((NI32)(((NI32)r_1) >> ((NU32)((NI32)(fl_4 - ((NI32)5)))))) - ((NI32)32)));
  fl_4 = ((NI32)(fl_4 - ((NI32)6)));
  result_36 = (X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl){
    .fld_0 = fl_4, .fld_1 = sl_5}
  ;
  return result_36;}
static inline BigChunk_0_sysvq0asl* findSuitableBlock_0_sysvq0asl(MemRegion_0_sysvq0asl* a_6, NI32* fl_1, NI32* sl_1){
  BigChunk_0_sysvq0asl* result_37;
  NI32 X60Qx_87 = nimIcheckB((*fl_1), 23);
  NU32 tmp_2 = ((NU32)((*a_6).slBitmap_0.a[X60Qx_87] & ((NU32)(((NU32) ~ ((NU32)0u)) << (*sl_1)))));
  result_37 = NIM_NIL;
  if ((!(tmp_2 == ((NU32)0)))){
    NI32 X60Qx_88 = lsbit_0_sysvq0asl(tmp_2);
    (*sl_1) = X60Qx_88;
    NI32 X60Qx_89 = nimIcheckB((*fl_1), 23);
    NI32 X60Qx_90 = nimIcheckB((*sl_1), 31);
    result_37 = (*a_6).matrix_0.a[X60Qx_89].a[X60Qx_90];}
  else {
    NI32 X60Qx_91 = lsbit_0_sysvq0asl(((NU32)((*a_6).flBitmap_0 & ((NU32)(((NU32) ~ ((NU32)0u)) << ((NI32)((*fl_1) + 1)))))));
    (*fl_1) = X60Qx_91;
    if (0 < (*fl_1)){
      NI32 X60Qx_92 = nimIcheckB((*fl_1), 23);
      NI32 X60Qx_93 = lsbit_0_sysvq0asl((*a_6).slBitmap_0.a[X60Qx_92]);
      (*sl_1) = X60Qx_93;
      NI32 X60Qx_94 = nimIcheckB((*fl_1), 23);
      NI32 X60Qx_95 = nimIcheckB((*sl_1), 31);
      result_37 = (*a_6).matrix_0.a[X60Qx_94].a[X60Qx_95];}}
  return result_37;}
void removeChunkFromMatrix_0_sysvq0asl(MemRegion_0_sysvq0asl* a_7, BigChunk_0_sysvq0asl* b_7){
  X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl X60Qtmptup_0 = mappingInsert_0_sysvq0asl((*b_7).Q.size_0);
  NI32 fl_5 = X60Qtmptup_0.fld_0;
  NI32 sl_6 = X60Qtmptup_0.fld_1;
  if ((!((*b_7).next_0 == NIM_NIL))){
    (*(*b_7).next_0).prev_0 = (*b_7).prev_0;}
  if ((!((*b_7).prev_0 == NIM_NIL))){
    (*(*b_7).prev_0).next_0 = (*b_7).next_0;}
  NI32 X60Qx_96 = nimIcheckB(fl_5, 23);
  NI32 X60Qx_97 = nimIcheckB(sl_6, 31);
  if ((*a_7).matrix_0.a[X60Qx_96].a[X60Qx_97] == b_7){
    NI32 X60Qx_98 = nimIcheckB(fl_5, 23);
    NI32 X60Qx_99 = nimIcheckB(sl_6, 31);
    (*a_7).matrix_0.a[X60Qx_98].a[X60Qx_99] = (*b_7).next_0;
    NI32 X60Qx_100 = nimIcheckB(fl_5, 23);
    NI32 X60Qx_101 = nimIcheckB(sl_6, 31);
    if ((*a_7).matrix_0.a[X60Qx_100].a[X60Qx_101] == NIM_NIL){
      NI32 X60Qx_102 = nimIcheckB(fl_5, 23);
      clearBit_0_sysvq0asl(sl_6, (&(*a_7).slBitmap_0.a[X60Qx_102]));
      NI32 X60Qx_103 = nimIcheckB(fl_5, 23);
      if ((*a_7).slBitmap_0.a[X60Qx_103] == ((NU32)0u)){
        clearBit_0_sysvq0asl(fl_5, (&(*a_7).flBitmap_0));}}}
  (*b_7).prev_0 = NIM_NIL;
  (*b_7).next_0 = NIM_NIL;}
void removeChunkFromMatrix2_0_sysvq0asl(MemRegion_0_sysvq0asl* a_8, BigChunk_0_sysvq0asl* b_8, NI32 fl_3, NI32 sl_3){
  NI32 X60Qx_104 = nimIcheckB(fl_3, 23);
  NI32 X60Qx_105 = nimIcheckB(sl_3, 31);
  (*a_8).matrix_0.a[X60Qx_104].a[X60Qx_105] = (*b_8).next_0;
  NI32 X60Qx_106 = nimIcheckB(fl_3, 23);
  NI32 X60Qx_107 = nimIcheckB(sl_3, 31);
  if ((!((*a_8).matrix_0.a[X60Qx_106].a[X60Qx_107] == NIM_NIL))){
    NI32 X60Qx_108 = nimIcheckB(fl_3, 23);
    NI32 X60Qx_109 = nimIcheckB(sl_3, 31);
    (*(*a_8).matrix_0.a[X60Qx_108].a[X60Qx_109]).prev_0 = NIM_NIL;}
  else {
    NI32 X60Qx_110 = nimIcheckB(fl_3, 23);
    clearBit_0_sysvq0asl(sl_3, (&(*a_8).slBitmap_0.a[X60Qx_110]));
    NI32 X60Qx_111 = nimIcheckB(fl_3, 23);
    if ((*a_8).slBitmap_0.a[X60Qx_111] == ((NU32)0u)){
      clearBit_0_sysvq0asl(fl_3, (&(*a_8).flBitmap_0));}}
  (*b_8).prev_0 = NIM_NIL;
  (*b_8).next_0 = NIM_NIL;}
void addChunkToMatrix_0_sysvq0asl(MemRegion_0_sysvq0asl* a_9, BigChunk_0_sysvq0asl* b_9){
  X60Qt_0_IAtupleAiS32ZAiS32_sysvq0asl X60Qtmptup_1 = mappingInsert_0_sysvq0asl((*b_9).Q.size_0);
  NI32 fl_6 = X60Qtmptup_1.fld_0;
  NI32 sl_7 = X60Qtmptup_1.fld_1;
  (*b_9).prev_0 = NIM_NIL;
  NI32 X60Qx_112 = nimIcheckB(fl_6, 23);
  NI32 X60Qx_113 = nimIcheckB(sl_7, 31);
  (*b_9).next_0 = (*a_9).matrix_0.a[X60Qx_112].a[X60Qx_113];
  NI32 X60Qx_114 = nimIcheckB(fl_6, 23);
  NI32 X60Qx_115 = nimIcheckB(sl_7, 31);
  if ((!((*a_9).matrix_0.a[X60Qx_114].a[X60Qx_115] == NIM_NIL))){
    NI32 X60Qx_116 = nimIcheckB(fl_6, 23);
    NI32 X60Qx_117 = nimIcheckB(sl_7, 31);
    (*(*a_9).matrix_0.a[X60Qx_116].a[X60Qx_117]).prev_0 = b_9;}
  NI32 X60Qx_118 = nimIcheckB(fl_6, 23);
  NI32 X60Qx_119 = nimIcheckB(sl_7, 31);
  (*a_9).matrix_0.a[X60Qx_118].a[X60Qx_119] = b_9;
  NI32 X60Qx_120 = nimIcheckB(fl_6, 23);
  setBit_0_sysvq0asl(sl_7, (&(*a_9).slBitmap_0.a[X60Qx_120]));
  setBit_0_sysvq0asl(fl_6, (&(*a_9).flBitmap_0));}
static inline void incCurrMem_0_sysvq0asl(MemRegion_0_sysvq0asl* a_10, NI32 bytes_0){
  inc_0_Iloplki_sysvq0asl((&(*a_10).currMem_0), bytes_0);}
static inline void decCurrMem_0_sysvq0asl(MemRegion_0_sysvq0asl* a_11, NI32 bytes_1){
  NI32 X60Qx_121 = max_2_sysvq0asl((*a_11).maxMem_0, (*a_11).currMem_0);
  (*a_11).maxMem_0 = X60Qx_121;
  dec_0_Ig5i8xp_sysvq0asl((&(*a_11).currMem_0), bytes_1);}
void* allocPages_0_sysvq0asl(MemRegion_0_sysvq0asl* a_13, NI32 size_11){
  void* result_39;
  void* X60Qx_123 = osAllocPages_0_sysvq0asl(size_11);
  result_39 = X60Qx_123;
  return result_39;}
void* tryAllocPages_0_sysvq0asl(MemRegion_0_sysvq0asl* a_14, NI32 size_12){
  void* result_40;
  void* X60Qx_124 = osTryAllocPages_0_sysvq0asl(size_12);
  result_40 = X60Qx_124;
  return result_40;}
void* llAlloc_0_sysvq0asl(MemRegion_0_sysvq0asl* a_15, NI32 size_13){
  void* result_41;
  NB8 X60Qx_125;
  if ((*a_15).llmem_0 == NIM_NIL){
    X60Qx_125 = NIM_TRUE;}
  else {
    X60Qx_125 = ((*(*a_15).llmem_0).size_0 < size_13);}
  if (X60Qx_125){
    LLChunk_0_sysvq0asl* old_1 = (*a_15).llmem_0;
    void* X60Qx_126 = allocPages_0_sysvq0asl(a_15, ((NI32)4096));
    (*a_15).llmem_0 = ((LLChunk_0_sysvq0asl*)X60Qx_126);
    incCurrMem_0_sysvq0asl(a_15, ((NI32)4096));
    (*(*a_15).llmem_0).size_0 = ((NI32)(((NI32)4096) - sizeof(LLChunk_0_sysvq0asl)));
    (*(*a_15).llmem_0).acc_0 = sizeof(LLChunk_0_sysvq0asl);
    (*(*a_15).llmem_0).next_0 = old_1;}
  result_41 = ((void*)((NI32)(((NI32)(*a_15).llmem_0) + (*(*a_15).llmem_0).acc_0)));
  dec_0_Ig5i8xp_sysvq0asl((&(*(*a_15).llmem_0).size_0), size_13);
  inc_0_Iloplki_sysvq0asl((&(*(*a_15).llmem_0).acc_0), size_13);
  zeroMem_0_sysvq0asl(result_41, size_13);
  return result_41;}
HeapLinks_0_sysvq0asl* addHeapLink_0_sysvq0asl(MemRegion_0_sysvq0asl* a_16, BigChunk_0_sysvq0asl* p_10, NI32 size_14){
  HeapLinks_0_sysvq0asl* result_42;
  HeapLinks_0_sysvq0asl* it_0 = (&(*a_16).heapLinks_0);
  {
    while (NIM_TRUE){
      NB8 X60Qx_127;
      if ((!(it_0 == NIM_NIL))){
        X60Qx_127 = (((NI32)(((NI32)(((NI32)((NI32)29)) - ((NI32)((NI32)0)))) + 1)) <= (*it_0).count_0);}
      else {
        X60Qx_127 = NIM_FALSE;}
      if (X60Qx_127){
        it_0 = (*it_0).next_0;}
      else {
        break;}}}
  whileStmtLabel_0: ;
  if (it_0 == NIM_NIL){
    void* X60Qx_128 = llAlloc_0_sysvq0asl(a_16, sizeof(HeapLinks_0_sysvq0asl));
    HeapLinks_0_sysvq0asl* n_7 = ((HeapLinks_0_sysvq0asl*)X60Qx_128);
    (*n_7).next_0 = (*a_16).heapLinks_0.next_0;
    (*a_16).heapLinks_0.next_0 = n_7;
    NI32 X60Qx_129 = nimIcheckB(0, 29);
    (*n_7).chunks_0.a[X60Qx_129] = (X60Qt_0_IAtupleAptrSX42igX43hunk0sysvq0aslZAiS32_sysvq0asl){
      .fld_0 = p_10, .fld_1 = size_14}
    ;
    (*n_7).count_0 = 1;
    result_42 = n_7;}
  else {
    NI32 L_0 = (*it_0).count_0;
    NI32 X60Qx_130 = nimIcheckB(L_0, 29);
    (*it_0).chunks_0.a[X60Qx_130] = (X60Qt_0_IAtupleAptrSX42igX43hunk0sysvq0aslZAiS32_sysvq0asl){
      .fld_0 = p_10, .fld_1 = size_14}
    ;
    inc_1_I6wjjge_jsfc0lwq21((&(*it_0).count_0));
    result_42 = it_0;}
  return result_42;}
Trunk_0_sysvq0asl* intSetGet_0_sysvq0asl(IntSet_0_sysvq0asl* t_0, NI32 key_0){
  Trunk_0_sysvq0asl* result_43;
  NI32 X60Qx_131 = nimIcheckB(((NI32)(key_0 & ((NI32)255))), 255);
  Trunk_0_sysvq0asl* it_2 = (*t_0).data_0.a[X60Qx_131];
  {
    while ((!(it_2 == NIM_NIL))){
      if ((*it_2).key_0 == key_0){
        return it_2;}
      it_2 = (*it_2).next_0;}}
  whileStmtLabel_0: ;
  result_43 = NIM_NIL;
  return result_43;}
Trunk_0_sysvq0asl* intSetPut_0_sysvq0asl(MemRegion_0_sysvq0asl* a_18, NI32 key_1){
  Trunk_0_sysvq0asl* result_44;
  Trunk_0_sysvq0asl* X60Qx_132 = intSetGet_0_sysvq0asl((&(*a_18).chunkStarts_0), key_1);
  result_44 = X60Qx_132;
  if (result_44 == NIM_NIL){
    void* X60Qx_133 = llAlloc_0_sysvq0asl(a_18, sizeof(Trunk_0_sysvq0asl));
    result_44 = ((Trunk_0_sysvq0asl*)X60Qx_133);
    NI32 X60Qx_134 = nimIcheckB(((NI32)(key_1 & ((NI32)255))), 255);
    (*result_44).next_0 = (*a_18).chunkStarts_0.data_0.a[X60Qx_134];
    NI32 X60Qx_135 = nimIcheckB(((NI32)(key_1 & ((NI32)255))), 255);
    (*a_18).chunkStarts_0.data_0.a[X60Qx_135] = result_44;
    (*result_44).key_0 = key_1;}
  return result_44;}
NB8 contains_1_sysvq0asl(IntSet_0_sysvq0asl* s_4, NI32 key_2){
  NB8 result_45;
  Trunk_0_sysvq0asl* t_4 = intSetGet_0_sysvq0asl((&(*s_4)), ((NI32)(((NI32)key_2) >> ((NU32)((NI32)9)))));
  if ((!(t_4 == NIM_NIL))){
    NI32 u_1 = ((NI32)(key_2 & ((NI32)511)));
    NI32 X60Qx_136 = nimIcheckB(((NI32)(((NI32)u_1) >> ((NU32)((NI32)5)))), 15);
    result_45 = (!(((NU32)((*t_4).bits_0.a[X60Qx_136] & ((NU32)(((NU32)1) << ((NI32)(u_1 & ((NI32)31))))))) == ((NU32)0)));}
  else {
    result_45 = NIM_FALSE;}
  return result_45;}
void incl_2_sysvq0asl(MemRegion_0_sysvq0asl* a_19, NI32 key_3){
  Trunk_0_sysvq0asl* t_5 = intSetPut_0_sysvq0asl(a_19, ((NI32)(((NI32)key_3) >> ((NU32)((NI32)9)))));
  NI32 u_2 = ((NI32)(key_3 & ((NI32)511)));
  NI32 X60Qx_137 = nimIcheckB(((NI32)(((NI32)u_2) >> ((NU32)((NI32)5)))), 15);
  NI32 X60Qx_138 = nimIcheckB(((NI32)(((NI32)u_2) >> ((NU32)((NI32)5)))), 15);
  (*t_5).bits_0.a[X60Qx_137] = ((NU32)((*t_5).bits_0.a[X60Qx_138] | ((NU32)(((NU32)1) << ((NI32)(u_2 & ((NI32)31)))))));}
void excl_2_sysvq0asl(IntSet_0_sysvq0asl* s_5, NI32 key_4){
  Trunk_0_sysvq0asl* t_6 = intSetGet_0_sysvq0asl((&(*s_5)), ((NI32)(((NI32)key_4) >> ((NU32)((NI32)9)))));
  if ((!(t_6 == NIM_NIL))){
    NI32 u_3 = ((NI32)(key_4 & ((NI32)511)));
    NI32 X60Qx_139 = nimIcheckB(((NI32)(((NI32)u_3) >> ((NU32)((NI32)5)))), 15);
    NI32 X60Qx_140 = nimIcheckB(((NI32)(((NI32)u_3) >> ((NU32)((NI32)5)))), 15);
    (*t_6).bits_0.a[X60Qx_139] = ((NU32)((*t_6).bits_0.a[X60Qx_140] & ((NU32) ~ ((NU32)(((NU32)1) << ((NI32)(u_3 & ((NI32)31))))))));}}
static inline NB8 isSmallChunk_0_sysvq0asl(BaseChunk_0_sysvq0asl* c_0){
  NB8 result_46;
  result_46 = ((*c_0).size_0 <= ((NI32)(((NI32)4096) - sizeof(SmallChunk_0_sysvq0asl))));
  return result_46;}
static inline NB8 chunkUnused_0_sysvq0asl(BaseChunk_0_sysvq0asl* c_1){
  NB8 result_47;
  result_47 = (((NI32)((*c_1).prevSize_0 & 1)) == 0);
  return result_47;}
static inline NI32 pageIndex_0_sysvq0asl(BaseChunk_0_sysvq0asl* c_2){
  NI32 result_48;
  result_48 = ((NI32)(((NI32)((NI32)c_2)) >> ((NU32)((NI32)12))));
  return result_48;}
static inline NI32 pageIndex_1_sysvq0asl(void* p_11){
  NI32 result_49;
  result_49 = ((NI32)(((NI32)((NI32)p_11)) >> ((NU32)((NI32)12))));
  return result_49;}
static inline BaseChunk_0_sysvq0asl* pageAddr_0_sysvq0asl(void* p_12){
  BaseChunk_0_sysvq0asl* result_50;
  result_50 = ((BaseChunk_0_sysvq0asl*)((NI32)(((NI32)p_12) & ((NI32) ~ ((NI32)4095)))));
  return result_50;}
BigChunk_0_sysvq0asl* requestOsChunks_0_sysvq0asl(MemRegion_0_sysvq0asl* a_20, NI32 size_15){
  BigChunk_0_sysvq0asl* result_51;
  if ((!(*a_20).blockChunkSizeIncrease_0)){
    NI32 usedMem_0 = (*a_20).occ_0;
    if (usedMem_0 < ((NI32)(64 * 1024))){
      (*a_20).nextChunkSize_0 = ((NI32)(((NI32)4096) * 4));}
    else {
      NI32 X60Qx_141 = roundup_0_sysvq0asl(((NI32)(((NI32)usedMem_0) >> ((NU32)2))), ((NI32)4096));
      NI32 X60Qx_142 = min_2_sysvq0asl(X60Qx_141, ((NI32)((*a_20).nextChunkSize_0 * 2)));
      (*a_20).nextChunkSize_0 = X60Qx_142;
      NI32 X60Qx_143 = min_2_sysvq0asl((*a_20).nextChunkSize_0, ((NI32)1056964608));
      (*a_20).nextChunkSize_0 = ((NI32)X60Qx_143);}}
  NI32 size_36 = size_15;
  if ((*a_20).nextChunkSize_0 < size_36){
    void* X60Qx_144 = allocPages_0_sysvq0asl(a_20, size_36);
    result_51 = ((BigChunk_0_sysvq0asl*)X60Qx_144);}
  else {
    void* X60Qx_145 = tryAllocPages_0_sysvq0asl(a_20, (*a_20).nextChunkSize_0);
    result_51 = ((BigChunk_0_sysvq0asl*)X60Qx_145);
    if (result_51 == NIM_NIL){
      void* X60Qx_146 = allocPages_0_sysvq0asl(a_20, size_36);
      result_51 = ((BigChunk_0_sysvq0asl*)X60Qx_146);
      (*a_20).blockChunkSizeIncrease_0 = NIM_TRUE;}
    else {
      size_36 = (*a_20).nextChunkSize_0;}}
  incCurrMem_0_sysvq0asl(a_20, size_36);
  inc_0_Iloplki_sysvq0asl((&(*a_20).freeMem_0), size_36);
  HeapLinks_0_sysvq0asl* heapLink_0 = addHeapLink_0_sysvq0asl(a_20, result_51, size_36);
  (*result_51).next_0 = NIM_NIL;
  (*result_51).prev_0 = NIM_NIL;
  (*result_51).Q.size_0 = size_36;
  NI32 nxt_0 = ((NI32)((NU32)(((NU32)((NI32)result_51)) + ((NU32)size_36))));
  BaseChunk_0_sysvq0asl* next_1 = ((BaseChunk_0_sysvq0asl*)nxt_0);
  NI32 X60Qx_147 = pageIndex_0_sysvq0asl(next_1);
  NB8 X60Qx_148 = contains_1_sysvq0asl((&(*a_20).chunkStarts_0), X60Qx_147);
  if (X60Qx_148){
    (*next_1).prevSize_0 = ((NI32)(size_36 | ((NI32)((*next_1).prevSize_0 & 1))));}
  NI32 X60Qx_16;
  if ((!((*a_20).lastSize_0 == 0))){
    X60Qx_16 = (*a_20).lastSize_0;}
  else {
    X60Qx_16 = ((NI32)4096);}
  NI32 lastSize_0 = X60Qx_16;
  NI32 prv_0 = ((NI32)((NU32)(((NU32)((NI32)result_51)) - ((NU32)lastSize_0))));
  BaseChunk_0_sysvq0asl* prev_1 = ((BaseChunk_0_sysvq0asl*)prv_0);
  NB8 X60Qx_149;
  NI32 X60Qx_150 = pageIndex_0_sysvq0asl(prev_1);
  NB8 X60Qx_151 = contains_1_sysvq0asl((&(*a_20).chunkStarts_0), X60Qx_150);
  if (X60Qx_151){
    X60Qx_149 = ((*prev_1).size_0 == lastSize_0);}
  else {
    X60Qx_149 = NIM_FALSE;}
  if (X60Qx_149){
    (*result_51).Q.prevSize_0 = ((NI32)(lastSize_0 | ((NI32)((*result_51).Q.prevSize_0 & 1))));}
  else {
    (*result_51).Q.prevSize_0 = ((NI32)(0 | ((NI32)((*result_51).Q.prevSize_0 & 1))));}
  (*a_20).lastSize_0 = size_36;
  return result_51;}
static inline NB8 isAccessible_0_sysvq0asl(MemRegion_0_sysvq0asl* a_21, void* p_13){
  NB8 result_52;
  NI32 X60Qx_152 = pageIndex_1_sysvq0asl(p_13);
  NB8 X60Qx_153 = contains_1_sysvq0asl((&(*a_21).chunkStarts_0), X60Qx_152);
  result_52 = X60Qx_153;
  return result_52;}
static inline void updatePrevSize_0_sysvq0asl(MemRegion_0_sysvq0asl* a_22, BigChunk_0_sysvq0asl* c_5, NI32 prevSize_0){
  BaseChunk_0_sysvq0asl* ri_0 = ((BaseChunk_0_sysvq0asl*)((NI32)((NU32)(((NU32)((NI32)c_5)) + ((NU32)(*c_5).Q.size_0)))));
  NB8 X60Qx_154 = isAccessible_0_sysvq0asl((&(*a_22)), ((void*)ri_0));
  if (X60Qx_154){
    (*ri_0).prevSize_0 = ((NI32)(prevSize_0 | ((NI32)((*ri_0).prevSize_0 & 1))));}}
BigChunk_0_sysvq0asl* splitChunk2_0_sysvq0asl(MemRegion_0_sysvq0asl* a_23, BigChunk_0_sysvq0asl* c_6, NI32 size_16){
  BigChunk_0_sysvq0asl* result_53;
  result_53 = ((BigChunk_0_sysvq0asl*)((NI32)((NU32)(((NU32)((NI32)c_6)) + ((NU32)size_16)))));
  (*result_53).Q.size_0 = ((NI32)((*c_6).Q.size_0 - size_16));
  (*result_53).next_0 = NIM_NIL;
  (*result_53).prev_0 = NIM_NIL;
  (*result_53).Q.prevSize_0 = size_16;
  (*result_53).Q.owner_0 = (&(*a_23));
  updatePrevSize_0_sysvq0asl(a_23, c_6, (*result_53).Q.size_0);
  (*c_6).Q.size_0 = size_16;
  NI32 X60Qx_155 = pageIndex_0_sysvq0asl(((BaseChunk_0_sysvq0asl*)result_53));
  incl_2_sysvq0asl(a_23, X60Qx_155);
  return result_53;}
void splitChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_24, BigChunk_0_sysvq0asl* c_7, NI32 size_17){
  BigChunk_0_sysvq0asl* rest_0 = splitChunk2_0_sysvq0asl(a_24, c_7, size_17);
  addChunkToMatrix_0_sysvq0asl(a_24, rest_0);}
void freeBigChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_25, BigChunk_0_sysvq0asl* c_8){
  BigChunk_0_sysvq0asl* c_28 = c_8;
  inc_0_Iloplki_sysvq0asl((&(*a_25).freeMem_0), (*c_28).Q.size_0);
  (*c_28).Q.prevSize_0 = ((NI32)((*c_28).Q.prevSize_0 & ((NI32) ~ 1)));
  NI32 prevSize_1 = (*c_28).Q.prevSize_0;
  if ((!(prevSize_1 == 0))){
    BaseChunk_0_sysvq0asl* le_0 = ((BaseChunk_0_sysvq0asl*)((NI32)((NU32)(((NU32)((NI32)c_28)) - ((NU32)prevSize_1)))));
    NB8 X60Qx_156;
    NB8 X60Qx_157 = isAccessible_0_sysvq0asl((&(*a_25)), ((void*)le_0));
    if (X60Qx_157){
      NB8 X60Qx_158 = chunkUnused_0_sysvq0asl(le_0);
      X60Qx_156 = X60Qx_158;}
    else {
      X60Qx_156 = NIM_FALSE;}
    if (X60Qx_156){
      NB8 X60Qx_159;
      NB8 X60Qx_160 = isSmallChunk_0_sysvq0asl(le_0);
      if ((!X60Qx_160)){
        X60Qx_159 = ((*le_0).size_0 < ((NI32)1056964608));}
      else {
        X60Qx_159 = NIM_FALSE;}
      if (X60Qx_159){
        removeChunkFromMatrix_0_sysvq0asl(a_25, ((BigChunk_0_sysvq0asl*)le_0));
        inc_0_Iloplki_sysvq0asl((&(*le_0).size_0), (*c_28).Q.size_0);
        NI32 X60Qx_161 = pageIndex_0_sysvq0asl(((BaseChunk_0_sysvq0asl*)c_28));
        excl_2_sysvq0asl((&(*a_25).chunkStarts_0), X60Qx_161);
        c_28 = ((BigChunk_0_sysvq0asl*)le_0);
        if (((NI32)1056964608) < (*c_28).Q.size_0){
          BigChunk_0_sysvq0asl* rest_1 = splitChunk2_0_sysvq0asl(a_25, c_28, ((NI32)1056964608));
          addChunkToMatrix_0_sysvq0asl(a_25, c_28);
          c_28 = rest_1;}}}}
  BaseChunk_0_sysvq0asl* ri_1 = ((BaseChunk_0_sysvq0asl*)((NI32)((NU32)(((NU32)((NI32)c_28)) + ((NU32)(*c_28).Q.size_0)))));
  NB8 X60Qx_162;
  NB8 X60Qx_163 = isAccessible_0_sysvq0asl((&(*a_25)), ((void*)ri_1));
  if (X60Qx_163){
    NB8 X60Qx_164 = chunkUnused_0_sysvq0asl(ri_1);
    X60Qx_162 = X60Qx_164;}
  else {
    X60Qx_162 = NIM_FALSE;}
  if (X60Qx_162){
    NB8 X60Qx_165;
    NB8 X60Qx_166 = isSmallChunk_0_sysvq0asl(ri_1);
    if ((!X60Qx_166)){
      X60Qx_165 = ((*c_28).Q.size_0 < ((NI32)1056964608));}
    else {
      X60Qx_165 = NIM_FALSE;}
    if (X60Qx_165){
      removeChunkFromMatrix_0_sysvq0asl(a_25, ((BigChunk_0_sysvq0asl*)ri_1));
      inc_0_Iloplki_sysvq0asl((&(*c_28).Q.size_0), (*ri_1).size_0);
      NI32 X60Qx_167 = pageIndex_0_sysvq0asl(ri_1);
      excl_2_sysvq0asl((&(*a_25).chunkStarts_0), X60Qx_167);
      if (((NI32)1056964608) < (*c_28).Q.size_0){
        BigChunk_0_sysvq0asl* rest_2 = splitChunk2_0_sysvq0asl(a_25, c_28, ((NI32)1056964608));
        addChunkToMatrix_0_sysvq0asl(a_25, rest_2);}}}
  addChunkToMatrix_0_sysvq0asl(a_25, c_28);}
BigChunk_0_sysvq0asl* getBigChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_26, NI32 size_18){
  BigChunk_0_sysvq0asl* result_54;
  NI32 size_37 = size_18;
  NI32 fl_7 = 0;
  NI32 sl_8 = 0;
  mappingSearch_0_sysvq0asl((&size_37), (&fl_7), (&sl_8));
  BigChunk_0_sysvq0asl* X60Qx_168 = findSuitableBlock_0_sysvq0asl((&(*a_26)), (&fl_7), (&sl_8));
  result_54 = X60Qx_168;
  if (result_54 == NIM_NIL){
    if (size_37 < ((NI32)(((NI32)128) * ((NI32)4096)))){
      BigChunk_0_sysvq0asl* X60Qx_169 = requestOsChunks_0_sysvq0asl(a_26, ((NI32)(((NI32)128) * ((NI32)4096))));
      result_54 = X60Qx_169;
      splitChunk_0_sysvq0asl(a_26, result_54, size_37);}
    else {
      BigChunk_0_sysvq0asl* X60Qx_170 = requestOsChunks_0_sysvq0asl(a_26, size_37);
      result_54 = X60Qx_170;
      if (size_37 < (*result_54).Q.size_0){
        splitChunk_0_sysvq0asl(a_26, result_54, size_37);}}
    (*result_54).Q.owner_0 = (&(*a_26));}
  else {
    removeChunkFromMatrix2_0_sysvq0asl(a_26, result_54, fl_7, sl_8);
    if (((NI32)(size_37 + ((NI32)4096))) <= (*result_54).Q.size_0){
      splitChunk_0_sysvq0asl(a_26, result_54, size_37);}}
  (*result_54).Q.prevSize_0 = 1;
  NI32 X60Qx_171 = pageIndex_0_sysvq0asl(((BaseChunk_0_sysvq0asl*)result_54));
  incl_2_sysvq0asl(a_26, X60Qx_171);
  dec_0_Ig5i8xp_sysvq0asl((&(*a_26).freeMem_0), size_37);
  return result_54;}
BigChunk_0_sysvq0asl* getHugeChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_27, NI32 size_19){
  BigChunk_0_sysvq0asl* result_55;
  void* X60Qx_172 = allocPages_0_sysvq0asl(a_27, size_19);
  result_55 = ((BigChunk_0_sysvq0asl*)X60Qx_172);
  incCurrMem_0_sysvq0asl(a_27, size_19);
  (*result_55).next_0 = NIM_NIL;
  (*result_55).prev_0 = NIM_NIL;
  (*result_55).Q.size_0 = size_19;
  (*result_55).Q.prevSize_0 = 1;
  (*result_55).Q.owner_0 = (&(*a_27));
  NI32 X60Qx_173 = pageIndex_0_sysvq0asl(((BaseChunk_0_sysvq0asl*)result_55));
  incl_2_sysvq0asl(a_27, X60Qx_173);
  return result_55;}
void freeHugeChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_28, BigChunk_0_sysvq0asl* c_9){
  NI32 size_38 = (*c_9).Q.size_0;
  NI32 X60Qx_174 = pageIndex_0_sysvq0asl(((BaseChunk_0_sysvq0asl*)c_9));
  excl_2_sysvq0asl((&(*a_28).chunkStarts_0), X60Qx_174);
  decCurrMem_0_sysvq0asl(a_28, size_38);
  osDeallocPages_0_sysvq0asl(((void*)c_9), size_38);}
SmallChunk_0_sysvq0asl* getSmallChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_29){
  SmallChunk_0_sysvq0asl* result_56;
  BigChunk_0_sysvq0asl* res_1 = getBigChunk_0_sysvq0asl(a_29, ((NI32)4096));
  result_56 = ((SmallChunk_0_sysvq0asl*)res_1);
  return result_56;}
void deallocBigChunk_0_sysvq0asl(MemRegion_0_sysvq0asl* a_31, BigChunk_0_sysvq0asl* c_10){
  dec_0_Ig5i8xp_sysvq0asl((&(*a_31).occ_0), (*c_10).Q.size_0);
  (*c_10).prev_0 = NIM_NIL;
  if (((NI32)1056964609) <= (*c_10).Q.size_0){
    freeHugeChunk_0_sysvq0asl(a_31, c_10);}
  else {
    freeBigChunk_0_sysvq0asl(a_31, c_10);}}
static inline void addToSharedFreeListBigChunks_0_sysvq0asl(MemRegion_0_sysvq0asl* a_32, BigChunk_0_sysvq0asl* c_11){
  {
    while (NIM_TRUE){
      BigChunk_0_sysvq0asl* X60Qx_175 = __atomic_load_n((&(*a_32).sharedFreeListBigChunks_0), __ATOMIC_RELAXED);
      __atomic_store_n((&(*c_11).next_0), X60Qx_175, __ATOMIC_RELAXED);
      NB8 X60Qx_176 = __atomic_compare_exchange_n((&(*a_32).sharedFreeListBigChunks_0), (&(*c_11).next_0), c_11, NIM_TRUE, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
      if (X60Qx_176){
        goto whileStmtLabel_0;}}}
  whileStmtLabel_0: ;}
static inline void addToSharedFreeList_0_sysvq0asl(SmallChunk_0_sysvq0asl* c_12, FreeCell_0_sysvq0asl* f_0, NI32 size_20){
  {
    while (NIM_TRUE){
      NI32 X60Qx_177 = nimIcheckB(size_20, 255);
      FreeCell_0_sysvq0asl* X60Qx_178 = __atomic_load_n((&(*(*c_12).Q.owner_0).sharedFreeLists_0.a[X60Qx_177]), __ATOMIC_RELAXED);
      __atomic_store_n((&(*f_0).next_0), X60Qx_178, __ATOMIC_RELAXED);
      NI32 X60Qx_179 = nimIcheckB(size_20, 255);
      NB8 X60Qx_180 = __atomic_compare_exchange_n((&(*(*c_12).Q.owner_0).sharedFreeLists_0.a[X60Qx_179]), (&(*f_0).next_0), f_0, NIM_TRUE, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
      if (X60Qx_180){
        goto whileStmtLabel_0;}}}
  whileStmtLabel_0: ;}
void compensateCounters_0_sysvq0asl(MemRegion_0_sysvq0asl* a_33, SmallChunk_0_sysvq0asl* c_13, NI32 size_21){
  FreeCell_0_sysvq0asl* it_3 = (*c_13).freeList_0;
  NI32 total_0 = 0;
  {
    while ((!(it_3 == NIM_NIL))){
      inc_0_Iloplki_sysvq0asl((&total_0), size_21);
      BaseChunk_0_sysvq0asl* X60Qx_181 = pageAddr_0_sysvq0asl(((void*)it_3));
      SmallChunk_0_sysvq0asl* chunk_0 = ((SmallChunk_0_sysvq0asl*)X60Qx_181);
      if ((!(c_13 == chunk_0))){
        (*c_13).foreignCells_0 = ((NI32)((*c_13).foreignCells_0 + ((NI32)1)));}
      it_3 = (*it_3).next_0;}}
  whileStmtLabel_0: ;
  (*c_13).free_0 = ((NI32)((*c_13).free_0 + ((NI32)total_0)));
  dec_0_Ig5i8xp_sysvq0asl((&(*a_33).occ_0), total_0);}
void freeDeferredObjects_0_sysvq0asl(MemRegion_0_sysvq0asl* a_34, BigChunk_0_sysvq0asl* root_0){
  BigChunk_0_sysvq0asl* it_4 = root_0;
  NI32 maxIters_0 = ((NI32)20);
  {
    while (NIM_TRUE){
      BigChunk_0_sysvq0asl* rest_3 = __atomic_load_n((&(*it_4).next_0), __ATOMIC_RELAXED);
      __atomic_store_n((&(*it_4).next_0), NIM_NIL, __ATOMIC_RELAXED);
      deallocBigChunk_0_sysvq0asl(a_34, ((BigChunk_0_sysvq0asl*)it_4));
      if (maxIters_0 == 0){
        if ((!(rest_3 == NIM_NIL))){
          addToSharedFreeListBigChunks_0_sysvq0asl((&(*a_34)), rest_3);}
        goto whileStmtLabel_0;}
      it_4 = rest_3;
      dec_1_I0nzoz91_sysvq0asl((&maxIters_0));
      if (it_4 == NIM_NIL){
        goto whileStmtLabel_0;}}}
  whileStmtLabel_0: ;}
static inline NI32 smallChunkAlignOffset_0_sysvq0asl(NI32 alignment_1){
  NI32 result_57;
  if (alignment_1 <= ((NI32)16)){
    result_57 = 0;}
  else {
    NI32 X60Qx_182 = align_0_sysvq0asl(((NI32)(sizeof(SmallChunk_0_sysvq0asl) + sizeof(FreeCell_0_sysvq0asl))), alignment_1);
    result_57 = ((NI32)(((NI32)(X60Qx_182 - sizeof(SmallChunk_0_sysvq0asl))) - sizeof(FreeCell_0_sysvq0asl)));}
  return result_57;}
static inline NI32 bigChunkAlignOffset_0_sysvq0asl(NI32 alignment_2){
  NI32 result_58;
  if (alignment_2 == 0){
    result_58 = 0;}
  else {
    NI32 X60Qx_183 = align_0_sysvq0asl(((NI32)(sizeof(BigChunk_0_sysvq0asl) + sizeof(FreeCell_0_sysvq0asl))), alignment_2);
    result_58 = ((NI32)(((NI32)(X60Qx_183 - sizeof(BigChunk_0_sysvq0asl))) - sizeof(FreeCell_0_sysvq0asl)));}
  return result_58;}
void* rawAlloc_0_sysvq0asl(MemRegion_0_sysvq0asl* a_35, NI32 requestedSize_0, NI32 alignment_3){
  void* result_59;
  NI32 X60Qx_184 = max_2_sysvq0asl(((NI32)16), alignment_3);
  NI32 size_39 = roundup_0_sysvq0asl(requestedSize_0, X60Qx_184);
  NI32 alignOff_0 = smallChunkAlignOffset_0_sysvq0asl(alignment_3);
  if (((NI32)(size_39 + alignOff_0)) <= ((NI32)(((NI32)4096) - sizeof(SmallChunk_0_sysvq0asl)))){
    NI32 s_82 = ((NI32)(size_39 / ((NI32)16)));
    NI32 X60Qx_185 = nimIcheckB(s_82, 255);
    SmallChunk_0_sysvq0asl* c_29 = (*a_35).freeSmallChunks_0.a[X60Qx_185];
    NB8 X60Qx_186;
    if ((!(c_29 == NIM_NIL))){
      X60Qx_186 = (!((*c_29).chunkAlignOff_0 == ((NI32)alignOff_0)));}
    else {
      X60Qx_186 = NIM_FALSE;}
    if (X60Qx_186){
      c_29 = NIM_NIL;}
    if (c_29 == NIM_NIL){
      SmallChunk_0_sysvq0asl* X60Qx_187 = getSmallChunk_0_sysvq0asl(a_35);
      c_29 = X60Qx_187;
      (*c_29).freeList_0 = NIM_NIL;
      (*c_29).foreignCells_0 = 0;
      (*c_29).chunkAlignOff_0 = ((NI32)alignOff_0);
      (*c_29).Q.size_0 = size_39;
      (*c_29).acc_0 = ((NU32)((NI32)(alignOff_0 + size_39)));
      (*c_29).free_0 = ((NI32)((NI32)(((NI32)(((NI32)(((NI32)4096) - sizeof(SmallChunk_0_sysvq0asl))) - alignOff_0)) - size_39)));
      (*c_29).next_0 = NIM_NIL;
      (*c_29).prev_0 = NIM_NIL;
      if ((*c_29).freeList_0 == NIM_NIL){
        NI32 X60Qx_188 = nimIcheckB(s_82, 255);
        FreeCell_0_sysvq0asl* X60Qx_189 = __atomic_exchange_n((&(*a_35).sharedFreeLists_0.a[X60Qx_188]), NIM_NIL, __ATOMIC_RELAXED);
        (*c_29).freeList_0 = X60Qx_189;
        compensateCounters_0_sysvq0asl(a_35, c_29, size_39);}
      if (size_39 <= (*c_29).free_0){
        NI32 X60Qx_190 = nimIcheckB(s_82, 255);
        listAdd_0_Ik4wxhz_sysvq0asl((&(*a_35).freeSmallChunks_0.a[X60Qx_190]), c_29);}
      result_59 = ((void*)((NI32)(((NI32)((void*)((void*)(&(*c_29).data_0)))) + alignOff_0)));}
    else {
      if ((*c_29).freeList_0 == NIM_NIL){
        result_59 = ((void*)((NI32)((NU32)(((NU32)((NI32)(&(*c_29).data_0))) + ((NU32)((NI32)(*c_29).acc_0))))));
        (*c_29).acc_0 = ((NU32)((*c_29).acc_0 + ((NU32)size_39)));}
      else {
        result_59 = ((void*)(*c_29).freeList_0);
        (*c_29).freeList_0 = (*(*c_29).freeList_0).next_0;
        BaseChunk_0_sysvq0asl* X60Qx_191 = pageAddr_0_sysvq0asl(result_59);
        if ((!(((SmallChunk_0_sysvq0asl*)X60Qx_191) == c_29))){
          (*c_29).foreignCells_0 = ((NI32)((*c_29).foreignCells_0 - ((NI32)1)));}
        else {
          }}
      (*c_29).free_0 = ((NI32)((*c_29).free_0 - ((NI32)size_39)));
      if ((*c_29).freeList_0 == NIM_NIL){
        NI32 X60Qx_192 = nimIcheckB(s_82, 255);
        FreeCell_0_sysvq0asl* X60Qx_193 = __atomic_exchange_n((&(*a_35).sharedFreeLists_0.a[X60Qx_192]), NIM_NIL, __ATOMIC_RELAXED);
        (*c_29).freeList_0 = X60Qx_193;
        compensateCounters_0_sysvq0asl(a_35, c_29, size_39);}
      if ((*c_29).free_0 < size_39){
        NI32 X60Qx_194 = nimIcheckB(s_82, 255);
        listRemove_0_Ibzev091_sysvq0asl((&(*a_35).freeSmallChunks_0.a[X60Qx_194]), c_29);}}
    inc_0_Iloplki_sysvq0asl((&(*a_35).occ_0), size_39);}
  else {
    BigChunk_0_sysvq0asl* deferredFrees_0 = __atomic_exchange_n((&(*a_35).sharedFreeListBigChunks_0), NIM_NIL, __ATOMIC_RELAXED);
    if ((!(deferredFrees_0 == NIM_NIL))){
      freeDeferredObjects_0_sysvq0asl(a_35, deferredFrees_0);}
    NI32 alignPad_0 = bigChunkAlignOffset_0_sysvq0asl(alignment_3);
    size_39 = ((NI32)(((NI32)(requestedSize_0 + sizeof(BigChunk_0_sysvq0asl))) + alignPad_0));
    BigChunk_0_sysvq0asl* X60Qx_17;
    if (((NI32)1056964609) <= size_39){
      BigChunk_0_sysvq0asl* X60Qx_195 = getHugeChunk_0_sysvq0asl(a_35, size_39);
      X60Qx_17 = X60Qx_195;}
    else {
      BigChunk_0_sysvq0asl* X60Qx_196 = getBigChunk_0_sysvq0asl(a_35, size_39);
      X60Qx_17 = X60Qx_196;}
    BigChunk_0_sysvq0asl* c_32 = X60Qx_17;
    result_59 = ((void*)((NI32)(((NI32)((void*)((void*)(&(*c_32).data_0)))) + alignPad_0)));
    (*c_32).prev_0 = ((BigChunk_0_sysvq0asl*)result_59);
    inc_0_Iloplki_sysvq0asl((&(*a_35).occ_0), (*c_32).Q.size_0);}
  return result_59;}
void rawDealloc_0_sysvq0asl(MemRegion_0_sysvq0asl* a_37, void* p_14){
  BaseChunk_0_sysvq0asl* c_33 = pageAddr_0_sysvq0asl(p_14);
  NB8 X60Qx_198 = isSmallChunk_0_sysvq0asl(c_33);
  if (X60Qx_198){
    SmallChunk_0_sysvq0asl* c_34 = ((SmallChunk_0_sysvq0asl*)c_33);
    NI32 s_83 = (*c_34).Q.size_0;
    FreeCell_0_sysvq0asl* f_3 = ((FreeCell_0_sysvq0asl*)p_14);
    if ((*c_34).Q.owner_0 == (&(*a_37))){
      dec_0_Ig5i8xp_sysvq0asl((&(*a_37).occ_0), s_83);
      NI32 X60Qx_199 = nimIcheckB(((NI32)(s_83 / ((NI32)16))), 255);
      SmallChunk_0_sysvq0asl* activeChunk_0 = (*a_37).freeSmallChunks_0.a[X60Qx_199];
      NB8 X60Qx_200;
      NB8 X60Qx_201;
      if ((!(activeChunk_0 == NIM_NIL))){
        X60Qx_201 = (!(c_34 == activeChunk_0));}
      else {
        X60Qx_201 = NIM_FALSE;}
      if (X60Qx_201){
        X60Qx_200 = ((*activeChunk_0).chunkAlignOff_0 == (*c_34).chunkAlignOff_0);}
      else {
        X60Qx_200 = NIM_FALSE;}
      if (X60Qx_200){
        (*f_3).next_0 = (*activeChunk_0).freeList_0;
        (*activeChunk_0).freeList_0 = f_3;
        (*activeChunk_0).free_0 = ((NI32)((*activeChunk_0).free_0 + ((NI32)s_83)));
        (*activeChunk_0).foreignCells_0 = ((NI32)((*activeChunk_0).foreignCells_0 + ((NI32)1)));}
      else {
        (*f_3).next_0 = (*c_34).freeList_0;
        (*c_34).freeList_0 = f_3;
        if ((*c_34).free_0 < s_83){
          NI32 X60Qx_202 = nimIcheckB(((NI32)(s_83 / ((NI32)16))), 255);
          listAdd_0_Ik4wxhz_sysvq0asl((&(*a_37).freeSmallChunks_0.a[X60Qx_202]), c_34);
          (*c_34).free_0 = ((NI32)((*c_34).free_0 + ((NI32)s_83)));}
        else {
          (*c_34).free_0 = ((NI32)((*c_34).free_0 + ((NI32)s_83)));}}}
    else {
      addToSharedFreeList_0_sysvq0asl(c_34, f_3, ((NI32)(s_83 / ((NI32)16))));}}
  else {
    if ((*c_33).owner_0 == (&(*a_37))){
      deallocBigChunk_0_sysvq0asl(a_37, ((BigChunk_0_sysvq0asl*)c_33));}
    else {
      addToSharedFreeListBigChunks_0_sysvq0asl((*c_33).owner_0, ((BigChunk_0_sysvq0asl*)c_33));}}}
void* alloc_0_sysvq0asl(MemRegion_0_sysvq0asl* allocator_0, NI32 size_22){
  void* result_62;
  void* X60Qx_204 = rawAlloc_0_sysvq0asl(allocator_0, size_22, 0);
  result_62 = X60Qx_204;
  return result_62;}
void dealloc_0_sysvq0asl(MemRegion_0_sysvq0asl* allocator_2, void* p_16){
  rawDealloc_0_sysvq0asl(allocator_2, p_16);}
void* alloc_1_sysvq0asl(NI32 size_24){
  void* result_69;
  void* X60Qx_211 = alloc_0_sysvq0asl((&allocator_0_sysvq0asl), size_24);
  result_69 = X60Qx_211;
  return result_69;}
void dealloc_1_sysvq0asl(void* p_20){
  dealloc_0_sysvq0asl((&allocator_0_sysvq0asl), p_20);}
N_NIMCALL(void, continueAfterOutOfMem_0_sysvq0asl)(NI32 size_28){
  if (missingBytes_0_sysvq0asl < ((NI32)(((NI32)2147483647) - size_28))){
    missingBytes_0_sysvq0asl = ((NI32)(missingBytes_0_sysvq0asl + size_28));}
  else {
    missingBytes_0_sysvq0asl = ((NI32)2147483647);}}
static inline NI32 ssLenOf_0_sysvq0asl(NU32 bytes_2){
  NI32 result_95;
  result_95 = ((NI32)((NU32)(bytes_2 & 255u)));
  return result_95;}
static inline NC8* rawData_1_sysvq0asl(string_0_sysvq0asl* s_33){
  NC8* result_96;
  if (((NI32)6) < ((NI32)(*((NU8*)(&(*s_33).bytes_0))))){
    result_96 = ((NC8*)(&(*(*s_33).more_0).data_0[0]));}
  else {
    result_96 = ((NC8*)((NU32)(((NU32)(&(*s_33).bytes_0)) + 1u)));}
  return result_96;}
static inline NI32 len_4_sysvq0asl(string_0_sysvq0asl s_34){
  NI32 result_98;
  result_98 = ((NI32)(*((NU8*)(&s_34.bytes_0))));
  if (((NI32)6) < result_98){
    result_98 = (*s_34.more_0).fullLen_0;}
  return result_98;}
static inline NC8* readRawData_0_sysvq0asl(string_0_sysvq0asl* s_39, NI32 start_0){
  NC8* result_103;
  if (((NI32)6) < ((NI32)(*((NU8*)(&(*s_39).bytes_0))))){
    result_103 = ((NC8*)((NU32)(((NU32)(&(*(*s_39).more_0).data_0[0])) + ((NU32)start_0))));}
  else {
    result_103 = ((NC8*)((NU32)(((NU32)((NC8*)((NU32)(((NU32)(&(*s_39).bytes_0)) + 1u)))) + ((NU32)start_0))));}
  return result_103;}
static inline void nimStrWasMoved(string_0_sysvq0asl* s_40){
  (*s_40).bytes_0 = ((NU32)0);}
static inline void nimStrDestroy(string_0_sysvq0asl s_41){
  if (((NI32)(*((NU8*)(&s_41.bytes_0)))) == ((NI32)255)){
    NB8 X60Qx_221 = arcDec_0_sysvq0asl((&(*s_41.more_0).rc_0));
    if (X60Qx_221){
      dealloc_1_sysvq0asl(((void*)s_41.more_0));}}}
void nimStrCopy(string_0_sysvq0asl* dest_11, string_0_sysvq0asl src_6){
  NI32 ssrc_0 = ((NI32)(*((NU8*)(&src_6.bytes_0))));
  if (ssrc_0 <= ((NI32)6)){
    NI32 sdest_0 = ((NI32)(*((NU8*)(&(*dest_11).bytes_0))));
    if (sdest_0 == ((NI32)255)){
      NB8 X60Qx_222 = arcDec_0_sysvq0asl((&(*(*dest_11).more_0).rc_0));
      if (X60Qx_222){
        dealloc_1_sysvq0asl(((void*)(*dest_11).more_0));}}
    copyMem_0_sysvq0asl(((void*)(&(*dest_11).bytes_0)), ((void*)(&src_6.bytes_0)), sizeof(string_0_sysvq0asl));}
  else {
    if ((&(*dest_11)) == (&src_6)){
      return;}
    NI32 sdest_1 = ((NI32)(*((NU8*)(&(*dest_11).bytes_0))));
    if (sdest_1 == ((NI32)255)){
      NB8 X60Qx_223 = arcDec_0_sysvq0asl((&(*(*dest_11).more_0).rc_0));
      if (X60Qx_223){
        dealloc_1_sysvq0asl(((void*)(*dest_11).more_0));}}
    if (ssrc_0 == ((NI32)255)){
      arcInc_0_sysvq0asl((&(*src_6.more_0).rc_0));}
    copyMem_0_sysvq0asl(((void*)(&(*dest_11).bytes_0)), ((void*)(&src_6.bytes_0)), sizeof(string_0_sysvq0asl));}}
static inline string_0_sysvq0asl nimStrDup(string_0_sysvq0asl s_42){
  string_0_sysvq0asl result_104;
  NI32 X60Qx_224 = ssLenOf_0_sysvq0asl(s_42.bytes_0);
  if (X60Qx_224 == ((NI32)255)){
    arcInc_0_sysvq0asl((&(*s_42.more_0).rc_0));}
  result_104 = (string_0_sysvq0asl){
    .bytes_0 = s_42.bytes_0, .more_0 = s_42.more_0}
  ;
  return result_104;}
static inline NI32 len_5_sysvq0asl(NC8* a_46){
  NI32 result_105;
  NI64 X60Qx_19;
  if (((void*)a_46) == NIM_NIL){
    X60Qx_19 = 0;}
  else {
    size_t X60Qx_225 = strlen(a_46);
    X60Qx_19 = ((NI32)X60Qx_225);}
  result_105 = X60Qx_19;
  return result_105;}
static inline NI32 ssResize_0_sysvq0asl(NI32 old_0){
  NI32 result_106;
  NI64 X60Qx_20;
  if (old_0 <= 0){
    X60Qx_20 = 4;}
  else {
    if (old_0 <= 32767){
      X60Qx_20 = ((NI32)(old_0 * 2));}
    else {
      X60Qx_20 = ((NI32)(((NI32)(old_0 / 2)) + old_0));}}
  result_106 = X60Qx_20;
  return result_106;}
void ensureUniqueLong_0_sysvq0asl(string_0_sysvq0asl* s_43, NI32 oldLen_0, NI32 newLen_5){
  NI32 sl_10 = ((NI32)(*((NU8*)(&(*s_43).bytes_0))));
  NB8 isHeap_0 = (sl_10 == ((NI32)255));
  NI32 X60Qx_21;
  if (isHeap_0){
    X60Qx_21 = (*(*s_43).more_0).capImpl_0;}
  else {
    X60Qx_21 = 0;}
  NI32 cap_1 = X60Qx_21;
  NB8 X60Qx_226;
  NB8 X60Qx_227;
  if (isHeap_0){
    NB8 X60Qx_228 = arcIsUnique_0_sysvq0asl((&(*(*s_43).more_0).rc_0));
    X60Qx_227 = X60Qx_228;}
  else {
    X60Qx_227 = NIM_FALSE;}
  if (X60Qx_227){
    X60Qx_226 = (newLen_5 <= cap_1);}
  else {
    X60Qx_226 = NIM_FALSE;}
  if (X60Qx_226){
    (*(*s_43).more_0).fullLen_0 = newLen_5;
    NI32 X60Qx_229 = min_2_sysvq0asl(oldLen_0, ((NI32)3));
    copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_43).bytes_0)) + 1u)))), ((void*)(&(*(*s_43).more_0).data_0[0])), X60Qx_229);}
  else {
    NI32 X60Qx_22;
    if (cap_1 < newLen_5){
      NI32 X60Qx_230 = ssResize_0_sysvq0asl(cap_1);
      NI32 X60Qx_231 = max_2_sysvq0asl(newLen_5, X60Qx_230);
      X60Qx_22 = X60Qx_231;}
    else {
      X60Qx_22 = cap_1;}
    NI32 newCap_2 = X60Qx_22;
    void* X60Qx_232 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + newCap_2)));
    LongString_0_sysvq0asl* p_35 = ((LongString_0_sysvq0asl*)X60Qx_232);
    if ((!(p_35 == NIM_NIL))){
      (*p_35).rc_0 = 0;
      (*p_35).fullLen_0 = newLen_5;
      (*p_35).capImpl_0 = newCap_2;
      if (isHeap_0){
        LongString_0_sysvq0asl* old_2 = (*s_43).more_0;
        NI32 X60Qx_233 = min_2_sysvq0asl(oldLen_0, newCap_2);
        copyMem_0_sysvq0asl(((void*)(&(*p_35).data_0[0])), ((void*)(&(*old_2).data_0[0])), X60Qx_233);
        NB8 X60Qx_234 = arcDec_0_sysvq0asl((&(*old_2).rc_0));
        if (X60Qx_234){
          dealloc_1_sysvq0asl(((void*)old_2));}}
      else {
        NI32 X60Qx_235 = min_2_sysvq0asl(oldLen_0, newCap_2);
        copyMem_0_sysvq0asl(((void*)(&(*p_35).data_0[0])), ((void*)(&(*(*s_43).more_0).data_0[0])), X60Qx_235);}
      (*s_43).more_0 = p_35;
      (*((NU8*)(&(*s_43).bytes_0))) = ((NU8)((NI32)255));
      NI32 X60Qx_236 = min_2_sysvq0asl(oldLen_0, ((NI32)3));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_43).bytes_0)) + 1u)))), ((void*)(&(*p_35).data_0[0])), X60Qx_236);}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + newCap_2)));
      (*s_43).bytes_0 = ((NU32)21760775509248519ull);
      (*s_43).more_0 = NIM_NIL;}}}
void transitionToLong_0_sysvq0asl(string_0_sysvq0asl* s_44, NI32 sl_4, NI32 newLen_6){
  NI32 X60Qx_237 = ssResize_0_sysvq0asl(newLen_6);
  NI32 newCap_3 = max_2_sysvq0asl(newLen_6, X60Qx_237);
  void* X60Qx_238 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + newCap_3)));
  LongString_0_sysvq0asl* p_36 = ((LongString_0_sysvq0asl*)X60Qx_238);
  if ((!(p_36 == NIM_NIL))){
    (*p_36).rc_0 = 0;
    (*p_36).fullLen_0 = newLen_6;
    (*p_36).capImpl_0 = newCap_3;
    copyMem_0_sysvq0asl(((void*)(&(*p_36).data_0[0])), ((void*)((NC8*)((NU32)(((NU32)(&(*s_44).bytes_0)) + 1u)))), sl_4);
    (*s_44).more_0 = p_36;
    (*((NU8*)(&(*s_44).bytes_0))) = ((NU8)((NI32)255));
    NI32 X60Qx_239 = min_2_sysvq0asl(sl_4, ((NI32)3));
    copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_44).bytes_0)) + 1u)))), ((void*)(&(*p_36).data_0[0])), X60Qx_239);}
  else {
    oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + newCap_3)));
    (*s_44).bytes_0 = ((NU32)21760775509248519ull);
    (*s_44).more_0 = NIM_NIL;}}
void prepareMutation_0_sysvq0asl(string_0_sysvq0asl* s_46){
  NI32 sl_12 = ((NI32)(*((NU8*)(&(*s_46).bytes_0))));
  NB8 X60Qx_242;
  if (sl_12 == ((NI32)254)){
    X60Qx_242 = NIM_TRUE;}
  else {
    NB8 X60Qx_243;
    if (sl_12 == ((NI32)255)){
      NB8 X60Qx_244 = arcIsUnique_0_sysvq0asl((&(*(*s_46).more_0).rc_0));
      X60Qx_243 = (!X60Qx_244);}
    else {
      X60Qx_243 = NIM_FALSE;}
    X60Qx_242 = X60Qx_243;}
  if (X60Qx_242){
    if (sl_12 == ((NI32)255)){
      NB8 X60Qx_245 = arcDec_0_sysvq0asl((&(*(*s_46).more_0).rc_0));}
    LongString_0_sysvq0asl* old_3 = (*s_46).more_0;
    NI32 oldLen_1 = (*old_3).fullLen_0;
    void* X60Qx_246 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + oldLen_1)));
    LongString_0_sysvq0asl* p_37 = ((LongString_0_sysvq0asl*)X60Qx_246);
    if ((!(p_37 == NIM_NIL))){
      (*p_37).rc_0 = 0;
      (*p_37).fullLen_0 = oldLen_1;
      (*p_37).capImpl_0 = oldLen_1;
      copyMem_0_sysvq0asl(((void*)(&(*p_37).data_0[0])), ((void*)(&(*old_3).data_0[0])), oldLen_1);
      (*s_46).more_0 = p_37;
      (*((NU8*)(&(*s_46).bytes_0))) = ((NU8)((NI32)255));}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + oldLen_1)));
      (*s_46).bytes_0 = ((NU32)21760775509248519ull);
      (*s_46).more_0 = NIM_NIL;}}}
void add_1_sysvq0asl(string_0_sysvq0asl* s_49, NC8 c_14){
  NI32 sl_14 = ((NI32)(*((NU8*)(&(*s_49).bytes_0))));
  if (sl_14 < ((NI32)6)){
    NI32 newLen_14 = ((NI32)(sl_14 + 1));
    ((NC8*)((NU32)(((NU32)(&(*s_49).bytes_0)) + 1u)))[sl_14] = c_14;
    (*((NU8*)(&(*s_49).bytes_0))) = ((NU8)newLen_14);}
  else {
    if (((NI32)6) < sl_14){
      NI32 l_1 = (*(*s_49).more_0).fullLen_0;
      NB8 X60Qx_248;
      NB8 X60Qx_249;
      if (sl_14 == ((NI32)255)){
        NB8 X60Qx_250 = arcIsUnique_0_sysvq0asl((&(*(*s_49).more_0).rc_0));
        X60Qx_249 = X60Qx_250;}
      else {
        X60Qx_249 = NIM_FALSE;}
      if (X60Qx_249){
        X60Qx_248 = (l_1 < (*(*s_49).more_0).capImpl_0);}
      else {
        X60Qx_248 = NIM_FALSE;}
      if (X60Qx_248){
        (*(*s_49).more_0).data_0[l_1] = c_14;
        (*(*s_49).more_0).fullLen_0 = ((NI32)(l_1 + 1));
        if (l_1 < ((NI32)3)){
          ((NC8*)((NU32)(((NU32)(&(*s_49).bytes_0)) + 1u)))[l_1] = c_14;}}
      else {
        NI32 oldLen_2 = (*(*s_49).more_0).fullLen_0;
        ensureUniqueLong_0_sysvq0asl(s_49, oldLen_2, ((NI32)(oldLen_2 + 1)));
        if (((NI32)(*((NU8*)(&(*s_49).bytes_0)))) == ((NI32)255)){
          (*(*s_49).more_0).data_0[oldLen_2] = c_14;
          if (oldLen_2 < ((NI32)3)){
            ((NC8*)((NU32)(((NU32)(&(*s_49).bytes_0)) + 1u)))[oldLen_2] = c_14;}}}}
    else {
      transitionToLong_0_sysvq0asl(s_49, sl_14, ((NI32)(sl_14 + 1)));
      if (((NI32)(*((NU8*)(&(*s_49).bytes_0)))) == ((NI32)255)){
        (*(*s_49).more_0).data_0[sl_14] = c_14;}}}}
void add_2_sysvq0asl(string_0_sysvq0asl* s_50, string_0_sysvq0asl part_0){
  NI32 partLen_0 = len_4_sysvq0asl(part_0);
  if (partLen_0 == 0){
    return;}
  NC8* partData_0 = rawData_1_sysvq0asl((&part_0));
  NI32 sl_15 = ((NI32)(*((NU8*)(&(*s_50).bytes_0))));
  if (sl_15 <= ((NI32)6)){
    NI32 sLen_0 = sl_15;
    NI32 newLen_15 = ((NI32)(sLen_0 + partLen_0));
    if (newLen_15 <= ((NI32)6)){
      copyMem_0_sysvq0asl(((void*)((NU32)(((NU32)(((NU32)(&(*s_50).bytes_0)) + 1u)) + ((NU32)sLen_0)))), ((void*)partData_0), partLen_0);
      (*((NU8*)(&(*s_50).bytes_0))) = ((NU8)newLen_15);}
    else {
      transitionToLong_0_sysvq0asl(s_50, sLen_0, newLen_15);
      if (((NI32)(*((NU8*)(&(*s_50).bytes_0)))) == ((NI32)255)){
        copyMem_0_sysvq0asl(((void*)((NU32)(((NU32)(&(*(*s_50).more_0).data_0[0])) + ((NU32)sLen_0)))), ((void*)partData_0), partLen_0);
        copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_50).bytes_0)) + 1u)))), ((void*)(&(*(*s_50).more_0).data_0[0])), ((NI32)3));}}}
  else {
    NI32 sLen_1 = (*(*s_50).more_0).fullLen_0;
    NI32 newLen_16 = ((NI32)(sLen_1 + partLen_0));
    ensureUniqueLong_0_sysvq0asl(s_50, sLen_1, newLen_16);
    if (((NI32)(*((NU8*)(&(*s_50).bytes_0)))) == ((NI32)255)){
      copyMem_0_sysvq0asl(((void*)((NU32)(((NU32)(&(*(*s_50).more_0).data_0[0])) + ((NU32)sLen_1)))), ((void*)partData_0), partLen_0);
      if (sLen_1 < ((NI32)3)){
        copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_50).bytes_0)) + 1u)))), ((void*)(&(*(*s_50).more_0).data_0[0])), ((NI32)3));}}}}
static inline NU32 zeroSwarPadImplLE_0_sysvq0asl(NU32 bytes_4, NI32 newLen_9){
  NU32 result_110;
  NU32 keepBits_0 = ((NU32)((NI32)(((NI32)(newLen_9 + 1)) * 8)));
  NU32 X60Qx_25;
  if (((NU32)((NI32)(sizeof(NU32) * 8))) <= keepBits_0){
    X60Qx_25 = ((NU32) ~ 0u);}
  else {
    X60Qx_25 = ((NU32)(((NU32)(((NU32)1) << keepBits_0)) - 1u));}
  NU32 mask_0 = X60Qx_25;
  result_110 = ((NU32)(((NU32)(bytes_4 & ((NU32)(mask_0 & ((NU32) ~ 255u))))) | ((NU32)newLen_9)));
  return result_110;}
static inline NU32 zeroSwarPadImpl_0_sysvq0asl(NU32 bytes_5, NI32 newLen_10){
  NU32 result_111;
  NU32 X60Qx_251 = zeroSwarPadImplLE_0_sysvq0asl(bytes_5, newLen_10);
  result_111 = X60Qx_251;
  return result_111;}
void shrink_1_sysvq0asl(string_0_sysvq0asl* s_52, NI32 newLen_12){
  NI32 X60Qx_252 = len_4_sysvq0asl((*s_52));
  if (newLen_12 <= X60Qx_252){
    NI32 sl_16 = ((NI32)(*((NU8*)(&(*s_52).bytes_0))));
    if (sl_16 <= ((NI32)6)){
      if (newLen_12 <= ((NI32)3)){
        NU32 X60Qx_253 = zeroSwarPadImpl_0_sysvq0asl((*s_52).bytes_0, newLen_12);
        (*s_52).bytes_0 = X60Qx_253;}
      else {
        (*((NU8*)(&(*s_52).bytes_0))) = ((NU8)newLen_12);}}
    else {
      prepareMutation_0_sysvq0asl(s_52);
      (*(*s_52).more_0).fullLen_0 = newLen_12;
      NI32 X60Qx_254 = min_2_sysvq0asl(newLen_12, ((NI32)3));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&(*s_52).bytes_0)) + 1u)))), ((void*)(&(*(*s_52).more_0).data_0[0])), X60Qx_254);}}}
static inline NC8 getQ_9_sysvq0asl(string_0_sysvq0asl s_54, NI32 i_14){
  NB8 X60Qx_257;
  NI32 X60Qx_258 = len_4_sysvq0asl(s_54);
  if (i_14 < X60Qx_258){
    X60Qx_257 = (0 <= i_14);}
  else {
    X60Qx_257 = NIM_FALSE;}
  if ((!X60Qx_257)){
    panic_0_sysvq0asl((string_0_sysvq0asl){
      .bytes_0 = 791555838u, .more_0 = (&strlit_0_I1565838944098044620_sysvq0asl)}
    );}
  NC8 result_112;
  NC8 X60Qx_26;
  if (((NI32)6) < ((NI32)(*((NU8*)(&s_54.bytes_0))))){
    X60Qx_26 = (*s_54.more_0).data_0[i_14];}
  else {
    X60Qx_26 = ((NC8*)((NU32)(((NU32)(&s_54.bytes_0)) + 1u)))[i_14];}
  result_112 = X60Qx_26;
  return result_112;}
static inline void putQ_9_sysvq0asl(string_0_sysvq0asl* s_55, NI32 i_15, NC8 c_15){
  NB8 X60Qx_259;
  NI32 X60Qx_260 = len_4_sysvq0asl((*s_55));
  if (i_15 < X60Qx_260){
    X60Qx_259 = (0 <= i_15);}
  else {
    X60Qx_259 = NIM_FALSE;}
  if ((!X60Qx_259)){
    panic_0_sysvq0asl((string_0_sysvq0asl){
      .bytes_0 = 791555838u, .more_0 = (&strlit_0_I7364560965974357967_sysvq0asl)}
    );}
  prepareMutation_0_sysvq0asl(s_55);
  if (((NI32)6) < ((NI32)(*((NU8*)(&(*s_55).bytes_0))))){
    (*(*s_55).more_0).data_0[i_15] = c_15;
    if (i_15 < ((NI32)3)){
      ((NC8*)((NU32)(((NU32)(&(*s_55).bytes_0)) + 1u)))[i_15] = c_15;}}
  else {
    ((NC8*)((NU32)(((NU32)(&(*s_55).bytes_0)) + 1u)))[i_15] = c_15;}}
string_0_sysvq0asl substr_0_sysvq0asl(string_0_sysvq0asl s_56, NI32 first_0, NI32 last_0){
  string_0_sysvq0asl result_113;
  nimStrWasMoved((&result_113));
  nimStrDestroy(result_113);
  result_113 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  NI32 sLen_2 = len_4_sysvq0asl(s_56);
  NI32 f_4 = max_2_sysvq0asl(first_0, 0);
  NI32 X60Qx_261 = min_2_sysvq0asl(last_0, ((NI32)(sLen_2 - 1)));
  NI32 l_2 = ((NI32)(X60Qx_261 + 1));
  if (l_2 <= f_4){
    return result_113;}
  NI32 newLen_17 = ((NI32)(l_2 - f_4));
  NC8* src_7 = rawData_1_sysvq0asl((&s_56));
  if (newLen_17 <= ((NI32)6)){
    (*((NU8*)(&result_113.bytes_0))) = ((NU8)newLen_17);
    copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_113.bytes_0)) + 1u)))), ((void*)((NU32)(((NU32)src_7) + ((NU32)f_4)))), newLen_17);}
  else {
    void* X60Qx_262 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + newLen_17)));
    LongString_0_sysvq0asl* p_38 = ((LongString_0_sysvq0asl*)X60Qx_262);
    if ((!(p_38 == NIM_NIL))){
      (*p_38).rc_0 = 0;
      (*p_38).fullLen_0 = newLen_17;
      (*p_38).capImpl_0 = newLen_17;
      copyMem_0_sysvq0asl(((void*)(&(*p_38).data_0[0])), ((void*)((NU32)(((NU32)src_7) + ((NU32)f_4)))), newLen_17);
      result_113.more_0 = p_38;
      (*((NU8*)(&result_113.bytes_0))) = ((NU8)((NI32)255));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_113.bytes_0)) + 1u)))), ((void*)(&(*p_38).data_0[0])), ((NI32)3));}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + newLen_17)));
      result_113.bytes_0 = ((NU32)21760775509248519ull);
      result_113.more_0 = NIM_NIL;}}
  return result_113;}
string_0_sysvq0asl newString_0_sysvq0asl(NI32 len_4){
  string_0_sysvq0asl result_132;
  nimStrWasMoved((&result_132));
  nimStrDestroy(result_132);
  result_132 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  if (len_4 <= 0){
    return result_132;}
  if (len_4 <= ((NI32)6)){
    (*((NU8*)(&result_132.bytes_0))) = ((NU8)len_4);
    zeroMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_132.bytes_0)) + 1u)))), len_4);}
  else {
    void* X60Qx_294 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + len_4)));
    LongString_0_sysvq0asl* p_39 = ((LongString_0_sysvq0asl*)X60Qx_294);
    if ((!(p_39 == NIM_NIL))){
      zeroMem_0_sysvq0asl(((void*)p_39), ((NI32)(((NI32)12) + len_4)));
      (*p_39).rc_0 = 0;
      (*p_39).fullLen_0 = len_4;
      (*p_39).capImpl_0 = len_4;
      result_132.more_0 = p_39;
      (*((NU8*)(&result_132.bytes_0))) = ((NU8)((NI32)255));}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + len_4)));
      result_132.bytes_0 = ((NU32)21760775509248519ull);
      result_132.more_0 = NIM_NIL;}}
  return result_132;}
string_0_sysvq0asl newStringOfCap_0_sysvq0asl(NI32 len_5){
  string_0_sysvq0asl result_133;
  nimStrWasMoved((&result_133));
  nimStrDestroy(result_133);
  result_133 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  if (len_5 <= ((NI32)6)){
    return result_133;}
  void* X60Qx_295 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + len_5)));
  LongString_0_sysvq0asl* p_40 = ((LongString_0_sysvq0asl*)X60Qx_295);
  if ((!(p_40 == NIM_NIL))){
    zeroMem_0_sysvq0asl(((void*)p_40), ((NI32)(((NI32)12) + len_5)));
    (*p_40).rc_0 = 0;
    (*p_40).fullLen_0 = 0;
    (*p_40).capImpl_0 = len_5;
    result_133.more_0 = p_40;
    (*((NU8*)(&result_133.bytes_0))) = ((NU8)((NI32)255));}
  else {
    oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + len_5)));
    result_133.bytes_0 = ((NU32)21760775509248519ull);
    result_133.more_0 = NIM_NIL;}
  return result_133;}
string_0_sysvq0asl ampQ_0_sysvq0asl(string_0_sysvq0asl a_54, string_0_sysvq0asl b_17){
  string_0_sysvq0asl result_134;
  nimStrWasMoved((&result_134));
  nimStrDestroy(result_134);
  result_134 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  NI32 X60Qx_296 = len_4_sysvq0asl(a_54);
  NI32 X60Qx_297 = len_4_sysvq0asl(b_17);
  NI32 rlen_0 = ((NI32)(X60Qx_296 + X60Qx_297));
  if (rlen_0 == 0){
    return result_134;}
  if (rlen_0 <= ((NI32)6)){
    NI32 al_0 = len_4_sysvq0asl(a_54);
    (*((NU8*)(&result_134.bytes_0))) = ((NU8)rlen_0);
    if (0 < al_0){
      NC8* X60Qx_298 = rawData_1_sysvq0asl((&a_54));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_134.bytes_0)) + 1u)))), ((void*)X60Qx_298), al_0);}
    NI32 X60Qx_299 = len_4_sysvq0asl(b_17);
    if (0 < X60Qx_299){
      NC8* X60Qx_300 = rawData_1_sysvq0asl((&b_17));
      NI32 X60Qx_301 = len_4_sysvq0asl(b_17);
      copyMem_0_sysvq0asl(((void*)((NU32)(((NU32)(((NU32)(&result_134.bytes_0)) + 1u)) + ((NU32)al_0)))), ((void*)X60Qx_300), X60Qx_301);}}
  else {
    void* X60Qx_302 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + rlen_0)));
    LongString_0_sysvq0asl* p_41 = ((LongString_0_sysvq0asl*)X60Qx_302);
    if ((!(p_41 == NIM_NIL))){
      (*p_41).rc_0 = 0;
      (*p_41).fullLen_0 = rlen_0;
      (*p_41).capImpl_0 = rlen_0;
      NI32 al_1 = len_4_sysvq0asl(a_54);
      if (0 < al_1){
        NC8* X60Qx_303 = rawData_1_sysvq0asl((&a_54));
        copyMem_0_sysvq0asl(((void*)(&(*p_41).data_0[0])), ((void*)X60Qx_303), al_1);}
      NI32 X60Qx_304 = len_4_sysvq0asl(b_17);
      if (0 < X60Qx_304){
        NC8* X60Qx_305 = rawData_1_sysvq0asl((&b_17));
        NI32 X60Qx_306 = len_4_sysvq0asl(b_17);
        copyMem_0_sysvq0asl(((void*)((NU32)(((NU32)(&(*p_41).data_0[0])) + ((NU32)al_1)))), ((void*)X60Qx_305), X60Qx_306);}
      result_134.more_0 = p_41;
      (*((NU8*)(&result_134.bytes_0))) = ((NU8)((NI32)255));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_134.bytes_0)) + 1u)))), ((void*)(&(*p_41).data_0[0])), ((NI32)3));}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + rlen_0)));
      result_134.bytes_0 = ((NU32)21760775509248519ull);
      result_134.more_0 = NIM_NIL;}}
  return result_134;}
string_0_sysvq0asl borrowCStringUnsafe_0_sysvq0asl(NC8* s_61, NI32 l_0){
  string_0_sysvq0asl result_139;
  nimStrWasMoved((&result_139));
  nimStrDestroy(result_139);
  result_139 = (string_0_sysvq0asl){
    .bytes_0 = 0u, .more_0 = NIM_NIL}
  ;
  if (l_0 <= 0){
    return result_139;}
  if (l_0 <= ((NI32)6)){
    (*((NU8*)(&result_139.bytes_0))) = ((NU8)l_0);
    copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_139.bytes_0)) + 1u)))), ((void*)s_61), l_0);}
  else {
    void* X60Qx_311 = alloc_1_sysvq0asl(((NI32)(((NI32)12) + l_0)));
    LongString_0_sysvq0asl* p_42 = ((LongString_0_sysvq0asl*)X60Qx_311);
    if ((!(p_42 == NIM_NIL))){
      (*p_42).rc_0 = 0;
      (*p_42).fullLen_0 = l_0;
      (*p_42).capImpl_0 = l_0;
      copyMem_0_sysvq0asl(((void*)(&(*p_42).data_0[0])), ((void*)s_61), l_0);
      result_139.more_0 = p_42;
      (*((NU8*)(&result_139.bytes_0))) = ((NU8)((NI32)255));
      copyMem_0_sysvq0asl(((void*)((NC8*)((NU32)(((NU32)(&result_139.bytes_0)) + 1u)))), ((void*)(&(*p_42).data_0[0])), ((NI32)3));}
    else {
      oomHandler_0_sysvq0asl(((NI32)(((NI32)12) + l_0)));
      result_139.bytes_0 = ((NU32)21760775509248519ull);
      result_139.more_0 = NIM_NIL;}}
  return result_139;}
string_0_sysvq0asl nimBorrowCStringUnsafe(NC8* s_62){
  string_0_sysvq0asl result_140;
  nimStrWasMoved((&result_140));
  nimStrDestroy(result_140);
  NI32 X60Qx_312 = len_5_sysvq0asl(s_62);
  string_0_sysvq0asl X60Qx_313 = borrowCStringUnsafe_0_sysvq0asl(s_62, X60Qx_312);
  result_140 = X60Qx_313;
  return result_140;}
void ensureTerminatingZero_0_sysvq0asl(string_0_sysvq0asl* s_63){
  NI32 oldLen_3 = len_4_sysvq0asl((*s_63));
  add_1_sysvq0asl(s_63, (NC8)'\000');
  shrink_1_sysvq0asl(s_63, oldLen_3);}
NC8* toCString_0_sysvq0asl(string_0_sysvq0asl* s_64){
  NC8* result_141;
  ensureTerminatingZero_0_sysvq0asl(s_64);
  NC8* X60Qx_314 = rawData_1_sysvq0asl((&(*s_64)));
  result_141 = ((NC8*)X60Qx_314);
  return result_141;}
static inline void arcInc_0_sysvq0asl(NI32* memLoc_0){
  NI32 X60Qx_318 = __atomic_add_fetch((&(*memLoc_0)), 1, __ATOMIC_SEQ_CST);}
static inline NB8 arcDec_0_sysvq0asl(NI32* memLoc_1){
  NB8 result_156;
  NI32 X60Qx_319 = __atomic_sub_fetch((&(*memLoc_1)), 1, __ATOMIC_SEQ_CST);
  result_156 = (X60Qx_319 < 0);
  return result_156;}
static inline NB8 arcIsUnique_0_sysvq0asl(NI32* memLoc_2){
  NB8 result_157;
  NI32 X60Qx_320 = __atomic_load_n((&(*memLoc_2)), __ATOMIC_ACQUIRE);
  result_157 = (X60Qx_320 == 0);
  return result_157;}
void writeErr_0_sysvq0asl(NI64 x_329){
  fprintf(stderr, "%lld", x_329);}
void writeErr_1_sysvq0asl(NU64 x_330){
  fprintf(stderr, "%llu", x_330);}
void writeErr_2_sysvq0asl(string_0_sysvq0asl s_68){
  NC8* X60Qx_321 = readRawData_0_sysvq0asl((&s_68), 0);
  NI32 X60Qx_322 = len_4_sysvq0asl(s_68);
  NU32 X60Qx_323 = fwrite(((void*)X60Qx_321), 1u, ((NU32)X60Qx_322), stderr);}
void writeErr_3_sysvq0asl(NC8* s_69){
  NI32 X60Qx_324 = len_5_sysvq0asl(s_69);
  NU32 X60Qx_325 = fwrite(((void*)s_69), 1u, ((NU32)X60Qx_324), stderr);}
void panic_0_sysvq0asl(string_0_sysvq0asl s_70){
  writeErr_2_sysvq0asl(s_70);
  exit(((NI32)1));}
static inline NI32 nimIcheckAB(NI32 i_18, NI32 a_68, NI32 b_21){
  NI32 result_158;
  NB8 X60Qx_326;
  if (a_68 <= i_18){
    X60Qx_326 = (i_18 <= b_21);}
  else {
    X60Qx_326 = NIM_FALSE;}
  if (X60Qx_326){
    result_158 = ((NI32)(i_18 - a_68));}
  else {
    result_158 = 0;
    raiseIndexError3_0_I113jpc1_sysvq0asl(i_18, a_68, b_21);}
  return result_158;}
static inline NI32 nimIcheckB(NI32 i_19, NI32 b_22){
  NI32 result_159;
  NB8 X60Qx_327;
  if (0 <= i_19){
    X60Qx_327 = (i_19 <= b_22);}
  else {
    X60Qx_327 = NIM_FALSE;}
  if (X60Qx_327){
    result_159 = i_19;}
  else {
    result_159 = 0;
    raiseIndexError3_0_I113jpc1_sysvq0asl(i_19, 0, b_22);}
  return result_159;}
static inline NU32 nimUcheckAB(NU32 i_20, NU32 a_69, NU32 b_23){
  NU32 result_160;
  result_160 = ((NU32)(i_20 - a_69));
  if (b_23 < result_160){
    raiseIndexError3_0_Ic5mmkg_sysvq0asl(i_20, a_69, b_23);}
  return result_160;}
static inline NU32 nimUcheckB(NU32 i_21, NU32 b_24){
  NU32 result_161;
  result_161 = i_21;
  if (b_24 < result_161){
    raiseIndexError3_0_Ic5mmkg_sysvq0asl(i_21, ((NU32)0), b_24);}
  return result_161;}
static inline void nimInvalidObjConv(string_0_sysvq0asl name_0){
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1986947582u, .more_0 = (&strlit_0_I15539159382304113184_sysvq0asl)}
  );
  writeErr_2_sysvq0asl(name_0);
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 2561u, .more_0 = NIM_NIL}
  );
  exit(((NI32)1));}
static inline void nimChckNilDisp(void* p_25){
  if (p_25 == NIM_NIL){
    writeErr_2_sysvq0asl((string_0_sysvq0asl){
      .bytes_0 = 1851876350u, .more_0 = (&strlit_0_I14281474217946372742_sysvq0asl)}
    );
    exit(((NI32)1));}}
void procAddrError_0_sysvq0asl(NC8* name_1){
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1970234366u, .more_0 = (&strlit_0_I10604297744791418982_sysvq0asl)}
  );
  writeErr_3_sysvq0asl(name_1);
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 2561u, .more_0 = NIM_NIL}
  );
  exit(1);}
void* nimLoadLibrary(NC8* path_2){
  void* result_162;
  int flags_1 = ((NI32)2);
  void* X60Qx_328 = dlopen(path_2, flags_1);
  result_162 = X60Qx_328;
  return result_162;}
void* nimGetProcAddr(void* lib_3, NC8* name_3){
  void* result_163;
  void* X60Qx_329 = dlsym(lib_3, name_3);
  result_163 = X60Qx_329;
  if (result_163 == NIM_NIL){
    procAddrError_0_sysvq0asl(name_3);}
  return result_163;}
void* nimDynlibLoadStep(void* prev_0, NC8* cand_0){
  void* result_164;
  if ((!(prev_0 == NIM_NIL))){
    result_164 = prev_0;}
  else {
    void* X60Qx_330 = nimLoadLibrary(cand_0);
    result_164 = X60Qx_330;}
  return result_164;}
void* nimDynlibCheck(void* lib_4, NC8* path_3){
  void* result_165;
  if (lib_4 == NIM_NIL){
    writeErr_2_sysvq0asl((string_0_sysvq0asl){
      .bytes_0 = 1970234366u, .more_0 = (&strlit_0_I16690852185662743073_sysvq0asl)}
    );
    writeErr_3_sysvq0asl(path_3);
    writeErr_2_sysvq0asl((string_0_sysvq0asl){
      .bytes_0 = 2561u, .more_0 = NIM_NIL}
    );
    exit(1);}
  result_165 = lib_4;
  return result_165;}
static inline void inc_0_Iloplki_sysvq0asl(NI32* x_374, NI32 y_215){
  (*x_374) = ((NI32)((*x_374) + y_215));}
static inline void dec_0_Ig5i8xp_sysvq0asl(NI32* x_376, NI32 y_217){
  (*x_376) = ((NI32)((*x_376) - y_217));}
static inline void dec_1_I0nzoz91_sysvq0asl(NI32* x_377){
  (*x_377) = ((NI32)((*x_377) - ((NI32)1)));}
static inline void listAdd_0_Ik4wxhz_sysvq0asl(SmallChunk_0_sysvq0asl** head_5, SmallChunk_0_sysvq0asl* c_38){
  (*c_38).next_0 = (*head_5);
  if ((!((*head_5) == NIM_NIL))){
    (*(*head_5)).prev_0 = c_38;}
  (*head_5) = c_38;}
static inline void listRemove_0_Ibzev091_sysvq0asl(SmallChunk_0_sysvq0asl** head_6, SmallChunk_0_sysvq0asl* c_39){
  if (c_39 == (*head_6)){
    (*head_6) = (*c_39).next_0;
    if ((!((*head_6) == NIM_NIL))){
      (*(*head_6)).prev_0 = NIM_NIL;}}
  else {
    (*(*c_39).prev_0).next_0 = (*c_39).next_0;
    if ((!((*c_39).next_0 == NIM_NIL))){
      (*(*c_39).next_0).prev_0 = (*c_39).prev_0;}}
  (*c_39).next_0 = NIM_NIL;
  (*c_39).prev_0 = NIM_NIL;}
void raiseIndexError3_0_I113jpc1_sysvq0asl(NI32 i_68, NI32 a_83, NI32 b_38){
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1684957694u, .more_0 = (&strlit_0_I11614695157650328859_sysvq0asl)}
  );
  writeErr_0_sysvq0asl(((NI64)i_68));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1869488382u, .more_0 = (&strlit_0_I16845119709590674135_sysvq0asl)}
  );
  writeErr_0_sysvq0asl(((NI64)a_83));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 3026434u, .more_0 = NIM_NIL}
  );
  writeErr_0_sysvq0asl(((NI64)b_38));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 2561u, .more_0 = NIM_NIL}
  );
  exit(((NI32)1));}
void raiseIndexError3_0_Ic5mmkg_sysvq0asl(NU32 i_69, NU32 a_84, NU32 b_39){
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1684957694u, .more_0 = (&strlit_0_I11614695157650328859_sysvq0asl)}
  );
  writeErr_1_sysvq0asl(((NU64)i_69));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1869488382u, .more_0 = (&strlit_0_I16845119709590674135_sysvq0asl)}
  );
  writeErr_1_sysvq0asl(((NU64)a_84));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 3026434u, .more_0 = NIM_NIL}
  );
  writeErr_1_sysvq0asl(((NU64)b_39));
  writeErr_2_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 2561u, .more_0 = NIM_NIL}
  );
  exit(((NI32)1));}
void eQwasmovedQ_ArefSX45xception0sysvq0asl_0_sysvq0asl(X60Qt_0_IArefSX45xception0sysvq0asl_sysvq0asl** dest_0){
  (*dest_0) = NIM_NIL;}
void X60Qini_0_sysvq0asl(void){
  if (X60QiniGuard_0_sysvq0asl){
    return;}
  X60QiniGuard_0_sysvq0asl = NIM_TRUE;
  eQwasmovedQ_ArefSX45xception0sysvq0asl_0_sysvq0asl((&exc_0_sysvq0asl));}
static inline void inc_1_I6wjjge_jsfc0lwq21(NI32* x_11){
  (*x_11) = ((NI32)((*x_11) + ((NI32)1)));}
static void __attribute__((constructor)) init(void) {gExitFlush_0_sysvq0asl = nimNoopFlush_0_sysvq0asl;
oomHandler_0_sysvq0asl = continueAfterOutOfMem_0_sysvq0asl;
}

