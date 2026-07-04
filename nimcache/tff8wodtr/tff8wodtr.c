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
NIM_THREADVAR NB8 LENGC_ERR_;
#include <stdio.h>
typedef struct LongString_0_sysvq0asl LongString_0_sysvq0asl;
typedef struct LongString_0_sysvq0asl{
  NI32 fullLen_0;
  NI32 rc_0;
  NI32 capImpl_0;
  NC8 data_0[];}
LongString_0_sysvq0asl;
typedef struct JsValue_0_jsfc0lwq21{
  NI32 h_0;}
JsValue_0_jsfc0lwq21;
typedef struct string_0_sysvq0asl{
  NU32 bytes_0;
  LongString_0_sysvq0asl* more_0;}
string_0_sysvq0asl;
extern void X60Qini_0_sysvq0asl(void);
extern void X60Qini_0_syn1lfpjv(void);
extern void X60Qini_0_jsfc0lwq21(void);
extern JsValue_0_jsfc0lwq21 global_0_jsfc0lwq21(string_0_sysvq0asl name_8);
extern JsValue_0_jsfc0lwq21 toJs_3_jsfc0lwq21(string_0_sysvq0asl s_0);
extern JsValue_0_jsfc0lwq21 call_1_jsfc0lwq21(JsValue_0_jsfc0lwq21 obj_11, string_0_sysvq0asl name_12, JsValue_0_jsfc0lwq21 a_6);
static inline JsValue_0_jsfc0lwq21 toJs_0_jsfc0lwq21(NI32 x_4);
extern JsValue_0_jsfc0lwq21 call_2_jsfc0lwq21(JsValue_0_jsfc0lwq21 obj_12, string_0_sysvq0asl name_13, JsValue_0_jsfc0lwq21 a_7, JsValue_0_jsfc0lwq21 b_6);
static inline NI32 toInt_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_5);
extern string_0_sysvq0asl dollarQ_2_sysvq0asl(NI32 x_226);
extern string_0_sysvq0asl ampQ_0_sysvq0asl(string_0_sysvq0asl a_54, string_0_sysvq0asl b_17);
extern void write_0_syn1lfpjv(FILE* f_4, string_0_sysvq0asl s_0);
extern void write_7_syn1lfpjv(FILE* f_11, NC8 c_1);
extern JsValue_0_jsfc0lwq21 call_3_jsfc0lwq21(JsValue_0_jsfc0lwq21 obj_13, string_0_sysvq0asl name_14, JsValue_0_jsfc0lwq21 a_8, JsValue_0_jsfc0lwq21 b_7, JsValue_0_jsfc0lwq21 c_1);
extern string_0_sysvq0asl toStr_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_8);
static inline JsValue_0_jsfc0lwq21 newJsObject_0_jsfc0lwq21(void);
extern void set_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 obj_9, string_0_sysvq0asl name_10, JsValue_0_jsfc0lwq21 val_3);
extern JsValue_0_jsfc0lwq21 get_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 obj_8, string_0_sysvq0asl name_9);
static inline JsValue_0_jsfc0lwq21 toJs_2_jsfc0lwq21(NB8 b_5);
static inline NB8 toBool_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_7);
extern string_0_sysvq0asl dollarQ_3_sysvq0asl(NB8 b_1);
static inline NI32 len_4_sysvq0asl(string_0_sysvq0asl s_34);
extern string_0_sysvq0asl newStringOfCap_0_sysvq0asl(NI32 len_5);
extern void add_2_sysvq0asl(string_0_sysvq0asl* s_50, string_0_sysvq0asl part_0);
static inline NB8 isNil_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_3);
static inline NB8 eqQ_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 a_5, JsValue_0_jsfc0lwq21 b_4);
static inline void nimStrDestroy(string_0_sysvq0asl s_41);
extern void eQdestroy_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 x_2);
extern void nimFlushStdStreams(void);
extern NI32 _numToJs(NI32 x_0);
extern NI32 _jsToNum(NI32 h_4);
extern NI32 _jsNewObject(void);
extern NI32 _boolToJs(NB8 b_1);
extern NB8 _jsToBool(NI32 h_6);
extern NB8 _jsStrictEq(NI32 a_0, NI32 b_0);
static inline NB8 arcDec_0_sysvq0asl(NI32* memLoc_1);
extern void dealloc_1_sysvq0asl(void* p_20);
LongString_0_sysvq0asl const strlit_0_I4512127154334175537_tff8wodtr = {
  .fullLen_0 = 7, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "console"}
;
LongString_0_sysvq0asl const strlit_0_I5853397530369228807_tff8wodtr = {
  .fullLen_0 = 39, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "hello from Nim through a real JS string"}
;
LongString_0_sysvq0asl const strlit_0_I12935728853510326435_tff8wodtr = {
  .fullLen_0 = 4, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "Math"}
;
LongString_0_sysvq0asl const strlit_0_I2301975825215568994_tff8wodtr = {
  .fullLen_0 = 20, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "Math.max(3, 7)    = "}
;
LongString_0_sysvq0asl const strlit_0_I6208868947491189530_tff8wodtr = {
  .fullLen_0 = 20, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "Math.max(3, 7, 5) = "}
;
LongString_0_sysvq0asl const strlit_0_I12805258542531914732_tff8wodtr = {
  .fullLen_0 = 10, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "round-trip"}
;
LongString_0_sysvq0asl const strlit_0_I15421062535947656287_tff8wodtr = {
  .fullLen_0 = 13, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "back in Nim: "}
;
LongString_0_sysvq0asl const strlit_0_I834269026986168554_tff8wodtr = {
  .fullLen_0 = 6, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "nimony"}
;
LongString_0_sysvq0asl const strlit_0_I407209193152762291_tff8wodtr = {
  .fullLen_0 = 4, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "name"}
;
LongString_0_sysvq0asl const strlit_0_I6393444175955872624_tff8wodtr = {
  .fullLen_0 = 4, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "year"}
;
LongString_0_sysvq0asl const strlit_0_I6644378417582681152_tff8wodtr = {
  .fullLen_0 = 17, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "o.year via get = "}
;
LongString_0_sysvq0asl const strlit_0_I6373137695046429832_tff8wodtr = {
  .fullLen_0 = 4, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "JSON"}
;
LongString_0_sysvq0asl const strlit_0_I7519307887382905868_tff8wodtr = {
  .fullLen_0 = 9, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "stringify"}
;
LongString_0_sysvq0asl const strlit_0_I14551632781070332322_tff8wodtr = {
  .fullLen_0 = 17, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "bool round-trip: "}
;
LongString_0_sysvq0asl const strlit_0_I933664836153361417_tff8wodtr = {
  .fullLen_0 = 4, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "nope"}
;
LongString_0_sysvq0asl const strlit_0_I6074055191300679588_tff8wodtr = {
  .fullLen_0 = 20, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "missing prop isNil: "}
;
LongString_0_sysvq0asl const strlit_0_I11663264536865316071_tff8wodtr = {
  .fullLen_0 = 15, .rc_0 = 0, .capImpl_0 = 0, .data_0 = "self === self: "}
;
JsValue_0_jsfc0lwq21 console_0_tff8wodtr;
JsValue_0_jsfc0lwq21 m_0_tff8wodtr;
JsValue_0_jsfc0lwq21 o_0_tff8wodtr;
NB8 X60QiniGuard_0_tff8wodtr;
NI32 cmdCount;
NC8** cmdLine;
NC8** nimEnviron;
void X60Qini_0_tff8wodtr(void){
  if (X60QiniGuard_0_tff8wodtr){
    return;}
  X60QiniGuard_0_tff8wodtr = NIM_TRUE;
  X60Qini_0_sysvq0asl();
  X60Qini_0_syn1lfpjv();
  X60Qini_0_jsfc0lwq21();
  console_0_tff8wodtr = global_0_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1852793854u, .more_0 = (&strlit_0_I4512127154334175537_tff8wodtr)}
  );
  JsValue_0_jsfc0lwq21 X60Qtmp_1 = toJs_3_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1818585342u, .more_0 = (&strlit_0_I5853397530369228807_tff8wodtr)}
  );
  JsValue_0_jsfc0lwq21 X60Qtmp_0 = call_1_jsfc0lwq21(console_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 1735355395u, .more_0 = NIM_NIL}
  , X60Qtmp_1);
  JsValue_0_jsfc0lwq21 X60Qx_0 = X60Qtmp_0;
  m_0_tff8wodtr = global_0_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1952534014u, .more_0 = (&strlit_0_I12935728853510326435_tff8wodtr)}
  );
  JsValue_0_jsfc0lwq21 X60Qtmp_5 = toJs_0_jsfc0lwq21(3);
  JsValue_0_jsfc0lwq21 X60Qtmp_6 = toJs_0_jsfc0lwq21(7);
  JsValue_0_jsfc0lwq21 X60Qtmp_4 = call_2_jsfc0lwq21(m_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 2019650819u, .more_0 = NIM_NIL}
  , X60Qtmp_5, X60Qtmp_6);
  NI32 X60Qx_1 = toInt_0_jsfc0lwq21(X60Qtmp_4);
  string_0_sysvq0asl X60Qtmp_3 = dollarQ_2_sysvq0asl(X60Qx_1);
  string_0_sysvq0asl X60Qtmp_2 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1952534014u, .more_0 = (&strlit_0_I2301975825215568994_tff8wodtr)}
  , X60Qtmp_3);
  write_0_syn1lfpjv(stdout, X60Qtmp_2);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_10 = toJs_0_jsfc0lwq21(3);
  JsValue_0_jsfc0lwq21 X60Qtmp_11 = toJs_0_jsfc0lwq21(7);
  JsValue_0_jsfc0lwq21 X60Qtmp_12 = toJs_0_jsfc0lwq21(5);
  JsValue_0_jsfc0lwq21 X60Qtmp_9 = call_3_jsfc0lwq21(m_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 2019650819u, .more_0 = NIM_NIL}
  , X60Qtmp_10, X60Qtmp_11, X60Qtmp_12);
  NI32 X60Qx_2 = toInt_0_jsfc0lwq21(X60Qtmp_9);
  string_0_sysvq0asl X60Qtmp_8 = dollarQ_2_sysvq0asl(X60Qx_2);
  string_0_sysvq0asl X60Qtmp_7 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1952534014u, .more_0 = (&strlit_0_I6208868947491189530_tff8wodtr)}
  , X60Qtmp_8);
  write_0_syn1lfpjv(stdout, X60Qtmp_7);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_15 = toJs_3_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1970238206u, .more_0 = (&strlit_0_I12805258542531914732_tff8wodtr)}
  );
  string_0_sysvq0asl X60Qtmp_14 = toStr_0_jsfc0lwq21(X60Qtmp_15);
  string_0_sysvq0asl X60Qtmp_13 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1667326718u, .more_0 = (&strlit_0_I15421062535947656287_tff8wodtr)}
  , X60Qtmp_14);
  write_0_syn1lfpjv(stdout, X60Qtmp_13);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  o_0_tff8wodtr = newJsObject_0_jsfc0lwq21();
  JsValue_0_jsfc0lwq21 X60Qtmp_16 = toJs_3_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1835626238u, .more_0 = (&strlit_0_I834269026986168554_tff8wodtr)}
  );
  set_0_jsfc0lwq21(o_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 1835101950u, .more_0 = (&strlit_0_I407209193152762291_tff8wodtr)}
  , X60Qtmp_16);
  JsValue_0_jsfc0lwq21 X60Qtmp_17 = toJs_0_jsfc0lwq21(2026);
  set_0_jsfc0lwq21(o_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 1634040318u, .more_0 = (&strlit_0_I6393444175955872624_tff8wodtr)}
  , X60Qtmp_17);
  JsValue_0_jsfc0lwq21 X60Qtmp_20 = get_0_jsfc0lwq21(o_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 1634040318u, .more_0 = (&strlit_0_I6393444175955872624_tff8wodtr)}
  );
  NI32 X60Qx_3 = toInt_0_jsfc0lwq21(X60Qtmp_20);
  string_0_sysvq0asl X60Qtmp_19 = dollarQ_2_sysvq0asl(X60Qx_3);
  string_0_sysvq0asl X60Qtmp_18 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 2033086462u, .more_0 = (&strlit_0_I6644378417582681152_tff8wodtr)}
  , X60Qtmp_19);
  write_0_syn1lfpjv(stdout, X60Qtmp_18);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_23 = global_0_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1330858750u, .more_0 = (&strlit_0_I6373137695046429832_tff8wodtr)}
  );
  JsValue_0_jsfc0lwq21 X60Qtmp_22 = call_1_jsfc0lwq21(X60Qtmp_23, (string_0_sysvq0asl){
    .bytes_0 = 1920234494u, .more_0 = (&strlit_0_I7519307887382905868_tff8wodtr)}
  , o_0_tff8wodtr);
  string_0_sysvq0asl X60Qtmp_21 = toStr_0_jsfc0lwq21(X60Qtmp_22);
  write_0_syn1lfpjv(stdout, X60Qtmp_21);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_24 = toJs_2_jsfc0lwq21(NIM_TRUE);
  NB8 X60Qx_4 = toBool_0_jsfc0lwq21(X60Qtmp_24);
  string_0_sysvq0asl X60Qdesugar_0 = dollarQ_3_sysvq0asl(X60Qx_4);
  JsValue_0_jsfc0lwq21 X60Qtmp_25 = toJs_2_jsfc0lwq21(NIM_FALSE);
  NB8 X60Qx_5 = toBool_0_jsfc0lwq21(X60Qtmp_25);
  string_0_sysvq0asl X60Qdesugar_1 = dollarQ_3_sysvq0asl(X60Qx_5);
  NI32 X60Qx_6 = len_4_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1869570814u, .more_0 = (&strlit_0_I14551632781070332322_tff8wodtr)}
  );
  NI32 X60Qx_7 = len_4_sysvq0asl(X60Qdesugar_0);
  NI32 X60Qx_8 = len_4_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 8193u, .more_0 = NIM_NIL}
  );
  NI32 X60Qx_9 = len_4_sysvq0asl(X60Qdesugar_1);
  string_0_sysvq0asl X60Qdesugar_2 = newStringOfCap_0_sysvq0asl(((NI)(((NI)(((NI)(X60Qx_6 + X60Qx_7)) + X60Qx_8)) + X60Qx_9)));
  add_2_sysvq0asl((&X60Qdesugar_2), (string_0_sysvq0asl){
    .bytes_0 = 1869570814u, .more_0 = (&strlit_0_I14551632781070332322_tff8wodtr)}
  );
  add_2_sysvq0asl((&X60Qdesugar_2), X60Qdesugar_0);
  add_2_sysvq0asl((&X60Qdesugar_2), (string_0_sysvq0asl){
    .bytes_0 = 8193u, .more_0 = NIM_NIL}
  );
  add_2_sysvq0asl((&X60Qdesugar_2), X60Qdesugar_1);
  write_0_syn1lfpjv(stdout, X60Qdesugar_2);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_28 = get_0_jsfc0lwq21(o_0_tff8wodtr, (string_0_sysvq0asl){
    .bytes_0 = 1886351102u, .more_0 = (&strlit_0_I933664836153361417_tff8wodtr)}
  );
  NB8 X60Qx_10 = isNil_0_jsfc0lwq21(X60Qtmp_28);
  string_0_sysvq0asl X60Qtmp_27 = dollarQ_3_sysvq0asl(X60Qx_10);
  string_0_sysvq0asl X60Qtmp_26 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1936289278u, .more_0 = (&strlit_0_I6074055191300679588_tff8wodtr)}
  , X60Qtmp_27);
  write_0_syn1lfpjv(stdout, X60Qtmp_26);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  JsValue_0_jsfc0lwq21 X60Qtmp_31 = global_0_jsfc0lwq21((string_0_sysvq0asl){
    .bytes_0 = 1952534014u, .more_0 = (&strlit_0_I12935728853510326435_tff8wodtr)}
  );
  NB8 X60Qx_11 = eqQ_0_jsfc0lwq21(m_0_tff8wodtr, X60Qtmp_31);
  string_0_sysvq0asl X60Qtmp_30 = dollarQ_3_sysvq0asl(X60Qx_11);
  string_0_sysvq0asl X60Qtmp_29 = ampQ_0_sysvq0asl((string_0_sysvq0asl){
    .bytes_0 = 1818588158u, .more_0 = (&strlit_0_I11663264536865316071_tff8wodtr)}
  , X60Qtmp_30);
  write_0_syn1lfpjv(stdout, X60Qtmp_29);
  write_7_syn1lfpjv(stdout, (NC8)'\012');
  nimStrDestroy(X60Qtmp_29);
  nimStrDestroy(X60Qtmp_30);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_31);
  nimStrDestroy(X60Qtmp_26);
  nimStrDestroy(X60Qtmp_27);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_28);
  nimStrDestroy(X60Qdesugar_2);
  nimStrDestroy(X60Qdesugar_1);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_25);
  nimStrDestroy(X60Qdesugar_0);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_24);
  nimStrDestroy(X60Qtmp_21);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_22);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_23);
  nimStrDestroy(X60Qtmp_18);
  nimStrDestroy(X60Qtmp_19);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_20);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_17);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_16);
  nimStrDestroy(X60Qtmp_13);
  nimStrDestroy(X60Qtmp_14);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_15);
  nimStrDestroy(X60Qtmp_7);
  nimStrDestroy(X60Qtmp_8);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_9);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_12);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_11);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_10);
  nimStrDestroy(X60Qtmp_2);
  nimStrDestroy(X60Qtmp_3);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_4);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_6);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_5);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_0);
  eQdestroy_0_jsfc0lwq21(X60Qtmp_1);}
NI32 main(NI32 X60Qargc_0_tff8wodtr, char** X60Qargv_0_tff8wodtr, char** X60Qenvp_0_tff8wodtr){
  cmdCount = X60Qargc_0_tff8wodtr;
  cmdLine = ((NC8**)X60Qargv_0_tff8wodtr);
  nimEnviron = ((NC8**)X60Qenvp_0_tff8wodtr);
  X60Qini_0_tff8wodtr();
  nimFlushStdStreams();
  return 0;}
static inline JsValue_0_jsfc0lwq21 toJs_0_jsfc0lwq21(NI32 x_4){
  JsValue_0_jsfc0lwq21 result_6;
  eQdestroy_0_jsfc0lwq21(result_6);
  NI32 X60Qx_4 = _numToJs(x_4);
  result_6 = (JsValue_0_jsfc0lwq21){
    .h_0 = X60Qx_4}
  ;
  return result_6;}
static inline NI32 toInt_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_5){
  NI32 result_7;
  NI32 X60Qx_5 = _jsToNum(v_5.h_0);
  result_7 = X60Qx_5;
  return result_7;}
static inline JsValue_0_jsfc0lwq21 newJsObject_0_jsfc0lwq21(void){
  JsValue_0_jsfc0lwq21 result_15;
  eQdestroy_0_jsfc0lwq21(result_15);
  NI32 X60Qx_16 = _jsNewObject();
  result_15 = (JsValue_0_jsfc0lwq21){
    .h_0 = X60Qx_16}
  ;
  return result_15;}
static inline JsValue_0_jsfc0lwq21 toJs_2_jsfc0lwq21(NB8 b_5){
  JsValue_0_jsfc0lwq21 result_10;
  eQdestroy_0_jsfc0lwq21(result_10);
  NI32 X60Qx_8 = _boolToJs(b_5);
  result_10 = (JsValue_0_jsfc0lwq21){
    .h_0 = X60Qx_8}
  ;
  return result_10;}
static inline NB8 toBool_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_7){
  NB8 result_11;
  NB8 X60Qx_9 = _jsToBool(v_7.h_0);
  result_11 = X60Qx_9;
  return result_11;}
static inline NI32 len_4_sysvq0asl(string_0_sysvq0asl s_34){
  NI32 result_98;
  result_98 = ((NI32)(*((NU8*)(&s_34.bytes_0))));
  if (((NI32)6) < result_98){
    result_98 = (*s_34.more_0).fullLen_0;}
  return result_98;}
static inline NB8 isNil_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 v_3){
  NB8 result_2;
  result_2 = (v_3.h_0 == 0);
  return result_2;}
static inline NB8 eqQ_0_jsfc0lwq21(JsValue_0_jsfc0lwq21 a_5, JsValue_0_jsfc0lwq21 b_4){
  NB8 result_1;
  NB8 X60Qx_2 = _jsStrictEq(a_5.h_0, b_4.h_0);
  result_1 = X60Qx_2;
  return result_1;}
static inline void nimStrDestroy(string_0_sysvq0asl s_41){
  if (((NI32)(*((NU8*)(&s_41.bytes_0)))) == ((NI32)255)){
    NB8 X60Qx_221 = arcDec_0_sysvq0asl((&(*s_41.more_0).rc_0));
    if (X60Qx_221){
      dealloc_1_sysvq0asl(((void*)s_41.more_0));}}}
static inline NB8 arcDec_0_sysvq0asl(NI32* memLoc_1){
  NB8 result_156;
  NI32 X60Qx_319 = __atomic_sub_fetch((&(*memLoc_1)), 1, __ATOMIC_SEQ_CST);
  result_156 = (X60Qx_319 < 0);
  return result_156;}
