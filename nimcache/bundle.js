// Canonical JS runtime for the Leng JS backend: one ArrayBuffer as linear memory,
// plus the small set of C primitives the lowered code imports. The heap is Nim's
// OWN native allocator (`-d:nimNativeAlloc` — the ported `system/alloc.nim`),
// compiled to JS through lengjs like any other module; the runtime provides only
// `mmap`/`munmap` as the page primitives it sits on (Araq's boundary), so `alloc`/
// `dealloc`/`realloc` and their free-list reuse all run as real Nim code.
const _ab = new ArrayBuffer(1 << 26);           // 64 MiB linear memory
const _dv = new DataView(_ab);
const _u8 = new Uint8Array(_ab);
let _brk = 8;                                   // offset 0 reserved as nil

// `allocFixed(n)` is the codegen's own storage for value aggregates (a C-stack
// model: never freed), distinct from the Nim heap that sits on `mmap` below.
function allocFixed(n){ const p=(_brk+7)&~7; _brk=p+n; _u8.fill(0,p,p+n); return p; }

// Page primitives for `system/osalloc.nim`: `mmap` hands the Nim allocator a
// page-aligned, zero-filled region carved from the same buffer (MAP_FAILED = -1
// on exhaustion, which makes the allocator raise OutOfMem); `munmap` is a no-op
// (the bump arena does not reclaim whole pages — the Nim allocator still reuses
// cells within them). Signature matches posix `mmap(adr,len,prot,flags,fd,off)`.
const _PAGE = 4096;
function mmap(adr, len, prot, flags, fildes, off){
  len = Number(len);
  const p = (_brk + _PAGE - 1) & ~(_PAGE - 1);  // page-align
  if (p + len > _u8.length) return -1;          // MAP_FAILED
  _brk = p + len;
  _u8.fill(0, p, p + len);                      // MAP_ANONYMOUS: zero-filled
  return p;
}
function munmap(adr, len){ return 0; }

const mem = {
  setI8:(p,v)=>_dv.setInt8(p,v), i8:(p)=>_dv.getInt8(p),
  setU8:(p,v)=>_dv.setUint8(p,v), u8At:(p)=>_dv.getUint8(p),
  setI16:(p,v)=>_dv.setInt16(p,v,true), i16:(p)=>_dv.getInt16(p,true),
  setI32:(p,v)=>_dv.setInt32(p,v,true), i32:(p)=>_dv.getInt32(p,true),
  setU32:(p,v)=>_dv.setUint32(p,v,true), u32:(p)=>_dv.getUint32(p,true),
  setI64:(p,v)=>_dv.setBigInt64(p,BigInt(v),true), i64n:(p)=>Number(_dv.getBigInt64(p,true)),
  setU64:(p,v)=>_dv.setBigUint64(p,BigInt(v),true), u64n:(p)=>Number(_dv.getBigUint64(p,true)),
  i64b:(p)=>_dv.getBigInt64(p,true), u64b:(p)=>_dv.getBigUint64(p,true),   // exact 64-bit reads (int64/uint64 -> BigInt)
  setF64:(p,v)=>_dv.setFloat64(p,v,true), f64:(p)=>_dv.getFloat64(p,true),
  copy:(d,s,n)=>_u8.copyWithin(d,s,s+n),
  bytes:(p,n)=>_u8.subarray(p,p+n),
  writeStr:(p,s)=>{ for(let i=0;i<s.length;i++) _u8[p+i]=s.charCodeAt(i); },
};

function memcpy(d,s,n){ _u8.copyWithin(Number(d),Number(s),Number(s)+Number(n)); return d; }
function memset(p,v,n){ _u8.fill(v&0xff,Number(p),Number(p)+Number(n)); return p; }
function strlen(p){ let n=0; while(_u8[Number(p)+n]!==0) n++; return n; }
function memcmp(a,b,n){ a=Number(a);b=Number(b);n=Number(n); for(let i=0;i<n;i++){ const d=_u8[a+i]-_u8[b+i]; if(d!==0) return d<0?-1:1; } return 0; }

// Function table: a proc pointer in linear memory is an integer index into
// `_fns` (WASM's model — JS can't call an integer). `_fnid(fn)` interns a proc to
// its stable index when it's taken as a value; the codegen emits `_fns[idx](args)`
// for an indirect call (a proc variable / closure field). Index 0 is nil.
const _fns = [null];
const _fnmap = new Map();
function _fnid(fn){ let i=_fnmap.get(fn); if(i===undefined){ i=_fns.length; _fns.push(fn); _fnmap.set(fn,i); } return i; }

// C11 memory-order constants (imported by the atomic ops; ignored by the shims).
const __ATOMIC_RELAXED = 0, __ATOMIC_CONSUME = 1, __ATOMIC_ACQUIRE = 2,
      __ATOMIC_RELEASE = 3, __ATOMIC_ACQ_REL = 4, __ATOMIC_SEQ_CST = 5;

// C11 `__atomic_*_n` are generic over the slot type; on this `--bits:32` target
// both ARC refcounts (`rc: int`) and pointers are 4-byte, so every atomic slot
// is 32-bit. JS is single-threaded, so each is a plain read/modify/write. Signed
// `i32` for the fetch ops (the refcount `subFetch < 0` last-ref test), unsigned
// `u32` for the load/store/exchange the allocator's free-lists use for pointers.
function __atomic_add_fetch(p,v,o){ const n=(mem.i32(p)+Number(v))|0; mem.setI32(p,n); return n; }
function __atomic_sub_fetch(p,v,o){ const n=(mem.i32(p)-Number(v))|0; mem.setI32(p,n); return n; }
function __atomic_load_n(p,o){ return mem.u32(p); }
function __atomic_store_n(p,v,o){ mem.setU32(p,Number(v)); }
function __atomic_exchange_n(p,v,o){ const old=mem.u32(p); mem.setU32(p,Number(v)); return old; }
function __atomic_compare_exchange_n(p,exp,des,weak,so,fo){
  // if *p == *exp: *p = des, return true; else *exp = *p, return false
  const cur=mem.u32(p);
  if(cur===mem.u32(exp)){ mem.setU32(p,Number(des)); return true; }
  mem.setU32(exp,cur); return false;
}

// stdio — distinct stdout/stderr handles; the lowered code passes one as the
// `FILE*`, so route on identity (error/panic reporting goes to stderr).
const stdout = {}, stderr = {};
function _stream(f){ return f === stderr ? process.stderr : process.stdout; }
function fwrite(ptr,size,nmemb,f){ _stream(f).write(Buffer.from(_u8.subarray(ptr,ptr+size*nmemb))); return nmemb; }
function fprintf(f,fmt,...a){ let i=0; _stream(f).write(String(fmt).replace(/%ll[du]|%l[du]|%[dus]/g,()=>String(a[i++]))); }
function fputc(c,f){ _stream(f).write(Buffer.from([c&0xff])); return c; }
function nimFlushStdStreams(){}
function copyMem_0_sysvq0asl(d,s,n){ if(typeof d==='number'&&typeof s==='number') _u8.copyWithin(d,s,s+n); }
function exit(c){ process.exit(Number(c)||0); }

// ── JS-value interop bridge (std/jsffi) ──────────────────────────────────────
// Native Nim data lives in linear memory as byte offsets; a *JS* value (string,
// object, function, DOM node) can't. So Nim holds an integer HANDLE into this
// side table — the generalisation of the `_fns` proc-pointer table above. Slot 0
// is `undefined`/`null` (matches nil = offset 0), freed slots are recycled.
const _jsv = [undefined];
const _jsvFree = [];
function _jsNew(v){                                   // intern a JS value -> handle
  if (v === undefined || v === null) return 0;
  const i = _jsvFree.length ? _jsvFree.pop() : _jsv.length;
  _jsv[i] = v; return i;
}
function _jsRelease(h){ if (h > 0){ _jsv[h] = undefined; _jsvFree.push(h); } }
function _jsvDup(h){ return _jsNew(_jsv[h]); }        // a new slot to the same JS value
function _jsvLive(){ return _jsv.length - 1 - _jsvFree.length; }   // live slot count (leak tests)

// Strings cross the linear-memory boundary as UTF-8 bytes. `_strToJs` decodes a
// (ptr,len) slice of Nim string storage into a real JS string; the read-back is
// two calls (length, then copy) so no scratch region leaks — and since JS
// strings are immutable, both just encode the same handle (no cached state).
const _td = new TextDecoder(), _te = new TextEncoder();
function _strToJs(p, n){ return _jsNew(_td.decode(_u8.subarray(Number(p), Number(p) + Number(n)))); }
function _jsStrLen(h){ return _te.encode(String(_jsv[h])).length; }
function _jsStrInto(h, dst){ _u8.set(_te.encode(String(_jsv[h])), Number(dst)); }

// JS `===` (value/identity), so two distinct handles to the same value compare
// equal — handle-integer equality would not.
function _jsStrictEq(aH, bH){ return _jsv[aH] === _jsv[bH] ? 1 : 0; }

// Number/bool bridges: on --bits:32 a Nim int is already a JS Number.
function _numToJs(x){ return _jsNew(Number(x)); }
function _jsToNum(h){ return _jsv[h]; }
function _boolToJs(x){ return _jsNew(!!x); }
function _jsToBool(h){ return _jsv[h] ? 1 : 0; }

// Global lookup + property/method access, all keyed by JS-string handles so the
// member name itself rides the same marshalling path (no C string constants).
function _jsGlobalH(nameH){ return _jsNew(globalThis[_jsv[nameH]]); }
function _jsGetProp(oH, nameH){ return _jsNew(_jsv[oH][_jsv[nameH]]); }
function _jsSetProp(oH, nameH, vH){ _jsv[oH][_jsv[nameH]] = _jsv[vH]; }
function _jsCall0(oH, nameH){ const o = _jsv[oH]; return _jsNew(o[_jsv[nameH]]()); }
function _jsCall1(oH, nameH, aH){ const o = _jsv[oH]; return _jsNew(o[_jsv[nameH]](_jsv[aH])); }
function _jsCall2(oH, nameH, aH, bH){ const o = _jsv[oH]; return _jsNew(o[_jsv[nameH]](_jsv[aH], _jsv[bH])); }
function _jsCall3(oH, nameH, aH, bH, cH){ const o = _jsv[oH]; return _jsNew(o[_jsv[nameH]](_jsv[aH], _jsv[bH], _jsv[cH])); }
function _jsNewObject(){ return _jsNew({}); }

// `new Ctor(...)` construction.
function _jsCtor0(ctorH){ return _jsNew(new (_jsv[ctorH])()); }
function _jsCtor1(ctorH, aH){ return _jsNew(new (_jsv[ctorH])(_jsv[aH])); }
// `new Ctor(...args)` for any arity: args is a JS array handle, spread via Reflect.
function _jsCtorN(ctorH, argsH){ return _jsNew(Reflect.construct(_jsv[ctorH], _jsv[argsH])); }

// JS arrays. An array is just another JS value in the table; `_jsArrGet` interns
// a *new* handle to the element (owned by the returned JsValue), and `push`/set
// hand the array a direct reference to the element value — so releasing the Nim
// handle slot afterwards never disturbs the array's own reference (JS GC keeps
// the value alive as long as the array does). Floats need no bridge of their
// own: on --bits:32 a Nim float is already a JS Number, so `toJs(float)` reuses
// `_numToJs` and `toFloat` reuses `_jsToNum`.
function _jsNewArray(){ return _jsNew([]); }
function _jsArrLen(h){ return _jsv[h].length; }
function _jsArrPush(h, vH){ _jsv[h].push(_jsv[vH]); }
function _jsArrGet(h, i){ return _jsNew(_jsv[h][Number(i)]); }
function _jsArrSet(h, i, vH){ _jsv[h][Number(i)] = _jsv[vH]; }

// Introspection: `typeof`, `in`, `instanceof`. A DOM binding branches on these
// constantly (a node's type, whether a property exists, an Array vs a NodeList).
function _jsTypeof(h){ return _jsNew(typeof _jsv[h]); }
function _jsHasProp(oH, nameH){ return (_jsv[nameH] in _jsv[oH]) ? 1 : 0; }
function _jsInstanceOf(vH, ctorH){ return (_jsv[vH] instanceof _jsv[ctorH]) ? 1 : 0; }

// `obj.name(...args)` for any argument count (beyond the fixed _jsCall0..3): the
// Nim side marshals the args into a JS array, we spread it via Function.apply.
function _jsApply(oH, nameH, argsH){ const o = _jsv[oH]; return _jsNew(o[_jsv[nameH]].apply(o, _jsv[argsH])); }

// Nim proc -> JS function (the reverse of the _fns call table): a Nim proc used
// as a value lowers to an integer _fns index, so wrap that in a JS closure. The
// closure marshals each incoming JS argument to a `JsValue` — which the backend
// represents as a one-field `{h: int32}` object, i.e. 4 bytes in linear memory
// with the handle at offset 0 — and passes that object's byte offset (the ABI a
// Nim `proc(ev: JsValue)` expects). The Nim callback borrows the argument, so we
// release the handle after it returns; an event object is only valid for the
// duration of dispatch, matching the DOM contract.
function _fnToJs0(idx){ return _jsNew(() => { _fns[idx](); }); }
function _fnToJs1(idx){
  return _jsNew((a) => {
    const h = _jsNew(a);
    const p = allocFixed(4); mem.setI32(p, h);   // a JsValue {h} object for the ABI
    _fns[idx](p);
    _jsRelease(h);
  });
}

// generated by lengc (js backend) from jsfc0lwq21.c.nif
"use strict";

function eQdestroy_0_jsfc0lwq21(x_2) {
  _jsRelease(mem.i32(x_2));
}

function eqQ_0_jsfc0lwq21(a_5, b_4) {
  let result_1;
  let X60Qx_2 = _jsStrictEq(mem.i32(a_5), mem.i32(b_4));
  result_1 = X60Qx_2;
  return result_1;
}

function isNil_0_jsfc0lwq21(v_3) {
  let result_2;
  result_2 = (mem.i32(v_3) === 0);
  return result_2;
}

function toJs_0_jsfc0lwq21(x_4) {
  let result_6 = allocFixed(4);
  eQdestroy_0_jsfc0lwq21(result_6);
  let X60Qx_4 = _numToJs(x_4);
  mem.copy(result_6, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_4);
    return _o;
  })(), 4);
  return result_6;
}

function toInt_0_jsfc0lwq21(v_5) {
  let result_7;
  let X60Qx_5 = _jsToNum(mem.i32(v_5));
  result_7 = X60Qx_5;
  return result_7;
}

function toJs_2_jsfc0lwq21(b_5) {
  let result_10 = allocFixed(4);
  eQdestroy_0_jsfc0lwq21(result_10);
  let X60Qx_8 = _boolToJs(b_5);
  mem.copy(result_10, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_8);
    return _o;
  })(), 4);
  return result_10;
}

function toBool_0_jsfc0lwq21(v_7) {
  let result_11;
  let X60Qx_9 = _jsToBool(mem.i32(v_7));
  result_11 = X60Qx_9;
  return result_11;
}

function toJs_3_jsfc0lwq21(s_0) {
  let result_12 = allocFixed(4);
  let t_0 = allocFixed(8);
  mem.copy(t_0, nimStrDup(s_0), 8);
  eQdestroy_0_jsfc0lwq21(result_12);
  let X60Qx_10 = toCString_0_sysvq0asl(t_0);
  let X60Qx_11 = len_4_sysvq0asl(t_0);
  let X60Qx_12 = _strToJs(X60Qx_10, X60Qx_11);
  mem.copy(result_12, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_12);
    return _o;
  })(), 4);
  nimStrDestroy(t_0);
  return result_12;
  nimStrDestroy(t_0);
  return result_12;
}

function toStr_0_jsfc0lwq21(v_8) {
  let result_13 = allocFixed(8);
  nimStrWasMoved(result_13);
  let n_1 = _jsStrLen(mem.i32(v_8));
  if ((n_1 <= 0)) {
    return (() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 0);
      mem.setU32((_o + 4), 0);
      return _o;
    })();
  }
  nimStrDestroy(result_13);
  let X60Qx_13 = allocFixed(8);
  mem.copy(X60Qx_13, newString_0_sysvq0asl(n_1), 8);
  mem.copy(result_13, X60Qx_13, 8);
  let X60Qx_14 = toCString_0_sysvq0asl(result_13);
  _jsStrInto(mem.i32(v_8), X60Qx_14);
  return result_13;
}

function newJsObject_0_jsfc0lwq21() {
  let result_15 = allocFixed(4);
  eQdestroy_0_jsfc0lwq21(result_15);
  let X60Qx_16 = _jsNewObject();
  mem.copy(result_15, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_16);
    return _o;
  })(), 4);
  return result_15;
}

function global_0_jsfc0lwq21(name_8) {
  let result_21 = allocFixed(4);
  let n_2 = allocFixed(4);
  mem.copy(n_2, toJs_3_jsfc0lwq21(name_8), 4);
  eQdestroy_0_jsfc0lwq21(result_21);
  let X60Qx_22 = _jsGlobalH(mem.i32(n_2));
  mem.copy(result_21, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_22);
    return _o;
  })(), 4);
  eQdestroy_0_jsfc0lwq21(n_2);
  return result_21;
  eQdestroy_0_jsfc0lwq21(n_2);
  return result_21;
}

function get_0_jsfc0lwq21(obj_8, name_9) {
  let result_22 = allocFixed(4);
  let n_3 = allocFixed(4);
  mem.copy(n_3, toJs_3_jsfc0lwq21(name_9), 4);
  eQdestroy_0_jsfc0lwq21(result_22);
  let X60Qx_23 = _jsGetProp(mem.i32(obj_8), mem.i32(n_3));
  mem.copy(result_22, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_23);
    return _o;
  })(), 4);
  eQdestroy_0_jsfc0lwq21(n_3);
  return result_22;
  eQdestroy_0_jsfc0lwq21(n_3);
  return result_22;
}

function set_0_jsfc0lwq21(obj_9, name_10, val_3) {
  let n_4 = allocFixed(4);
  mem.copy(n_4, toJs_3_jsfc0lwq21(name_10), 4);
  _jsSetProp(mem.i32(obj_9), mem.i32(n_4), mem.i32(val_3));
  eQdestroy_0_jsfc0lwq21(n_4);
}

function call_1_jsfc0lwq21(obj_11, name_12, a_6) {
  let result_24 = allocFixed(4);
  let n_6 = allocFixed(4);
  mem.copy(n_6, toJs_3_jsfc0lwq21(name_12), 4);
  eQdestroy_0_jsfc0lwq21(result_24);
  let X60Qx_25 = _jsCall1(mem.i32(obj_11), mem.i32(n_6), mem.i32(a_6));
  mem.copy(result_24, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_25);
    return _o;
  })(), 4);
  eQdestroy_0_jsfc0lwq21(n_6);
  return result_24;
  eQdestroy_0_jsfc0lwq21(n_6);
  return result_24;
}

function call_2_jsfc0lwq21(obj_12, name_13, a_7, b_6) {
  let result_25 = allocFixed(4);
  let n_7 = allocFixed(4);
  mem.copy(n_7, toJs_3_jsfc0lwq21(name_13), 4);
  eQdestroy_0_jsfc0lwq21(result_25);
  let X60Qx_26 = _jsCall2(mem.i32(obj_12), mem.i32(n_7), mem.i32(a_7), mem.i32(b_6));
  mem.copy(result_25, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_26);
    return _o;
  })(), 4);
  eQdestroy_0_jsfc0lwq21(n_7);
  return result_25;
  eQdestroy_0_jsfc0lwq21(n_7);
  return result_25;
}

function call_3_jsfc0lwq21(obj_13, name_14, a_8, b_7, c_1) {
  let result_26 = allocFixed(4);
  let n_8 = allocFixed(4);
  mem.copy(n_8, toJs_3_jsfc0lwq21(name_14), 4);
  eQdestroy_0_jsfc0lwq21(result_26);
  let X60Qx_27 = _jsCall3(mem.i32(obj_13), mem.i32(n_8), mem.i32(a_8), mem.i32(b_7), mem.i32(c_1));
  mem.copy(result_26, (() => {
    let _o = allocFixed(4);
    mem.setI32(_o, X60Qx_27);
    return _o;
  })(), 4);
  eQdestroy_0_jsfc0lwq21(n_8);
  return result_26;
  eQdestroy_0_jsfc0lwq21(n_8);
  return result_26;
}

function inc_1_I6wjjge_jsfc0lwq21(x_11) {
  mem.setI32(x_11, (mem.i32(x_11) + 1));
}

let X60QiniGuard_0_jsfc0lwq21 = allocFixed(1);

function X60Qini_0_jsfc0lwq21() {
  if (mem.u8At(X60QiniGuard_0_jsfc0lwq21)) {
    return;
  }
  mem.setU8(X60QiniGuard_0_jsfc0lwq21, true);
  X60Qini_0_sysvq0asl();
}

// generated by lengc (js backend) from for2ybv4p1.c.nif
"use strict";

let X60QiniGuard_0_for2ybv4p1 = allocFixed(1);

function X60Qini_0_for2ybv4p1() {
  if (mem.u8At(X60QiniGuard_0_for2ybv4p1)) {
    return;
  }
  mem.setU8(X60QiniGuard_0_for2ybv4p1, true);
  X60Qini_0_sysvq0asl();
}

// generated by lengc (js backend) from tff8wodtr.c.nif
"use strict";

let strlit_0_I4512127154334175537_tff8wodtr = allocFixed(19);

mem.setI32(strlit_0_I4512127154334175537_tff8wodtr, 7);

mem.setI32((strlit_0_I4512127154334175537_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I4512127154334175537_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I4512127154334175537_tff8wodtr + 12), "console");

let strlit_0_I5853397530369228807_tff8wodtr = allocFixed(51);

mem.setI32(strlit_0_I5853397530369228807_tff8wodtr, 39);

mem.setI32((strlit_0_I5853397530369228807_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I5853397530369228807_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I5853397530369228807_tff8wodtr + 12), "hello from Nim through a real JS string");

let strlit_0_I12935728853510326435_tff8wodtr = allocFixed(16);

mem.setI32(strlit_0_I12935728853510326435_tff8wodtr, 4);

mem.setI32((strlit_0_I12935728853510326435_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I12935728853510326435_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I12935728853510326435_tff8wodtr + 12), "Math");

let strlit_0_I2301975825215568994_tff8wodtr = allocFixed(32);

mem.setI32(strlit_0_I2301975825215568994_tff8wodtr, 20);

mem.setI32((strlit_0_I2301975825215568994_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I2301975825215568994_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I2301975825215568994_tff8wodtr + 12), "Math.max(3, 7)    = ");

let strlit_0_I6208868947491189530_tff8wodtr = allocFixed(32);

mem.setI32(strlit_0_I6208868947491189530_tff8wodtr, 20);

mem.setI32((strlit_0_I6208868947491189530_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I6208868947491189530_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I6208868947491189530_tff8wodtr + 12), "Math.max(3, 7, 5) = ");

let strlit_0_I12805258542531914732_tff8wodtr = allocFixed(22);

mem.setI32(strlit_0_I12805258542531914732_tff8wodtr, 10);

mem.setI32((strlit_0_I12805258542531914732_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I12805258542531914732_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I12805258542531914732_tff8wodtr + 12), "round-trip");

let strlit_0_I15421062535947656287_tff8wodtr = allocFixed(25);

mem.setI32(strlit_0_I15421062535947656287_tff8wodtr, 13);

mem.setI32((strlit_0_I15421062535947656287_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I15421062535947656287_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I15421062535947656287_tff8wodtr + 12), "back in Nim: ");

let strlit_0_I834269026986168554_tff8wodtr = allocFixed(18);

mem.setI32(strlit_0_I834269026986168554_tff8wodtr, 6);

mem.setI32((strlit_0_I834269026986168554_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I834269026986168554_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I834269026986168554_tff8wodtr + 12), "nimony");

let strlit_0_I407209193152762291_tff8wodtr = allocFixed(16);

mem.setI32(strlit_0_I407209193152762291_tff8wodtr, 4);

mem.setI32((strlit_0_I407209193152762291_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I407209193152762291_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I407209193152762291_tff8wodtr + 12), "name");

let strlit_0_I6393444175955872624_tff8wodtr = allocFixed(16);

mem.setI32(strlit_0_I6393444175955872624_tff8wodtr, 4);

mem.setI32((strlit_0_I6393444175955872624_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I6393444175955872624_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I6393444175955872624_tff8wodtr + 12), "year");

let strlit_0_I6644378417582681152_tff8wodtr = allocFixed(29);

mem.setI32(strlit_0_I6644378417582681152_tff8wodtr, 17);

mem.setI32((strlit_0_I6644378417582681152_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I6644378417582681152_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I6644378417582681152_tff8wodtr + 12), "o.year via get = ");

let strlit_0_I6373137695046429832_tff8wodtr = allocFixed(16);

mem.setI32(strlit_0_I6373137695046429832_tff8wodtr, 4);

mem.setI32((strlit_0_I6373137695046429832_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I6373137695046429832_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I6373137695046429832_tff8wodtr + 12), "JSON");

let strlit_0_I7519307887382905868_tff8wodtr = allocFixed(21);

mem.setI32(strlit_0_I7519307887382905868_tff8wodtr, 9);

mem.setI32((strlit_0_I7519307887382905868_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I7519307887382905868_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I7519307887382905868_tff8wodtr + 12), "stringify");

let strlit_0_I14551632781070332322_tff8wodtr = allocFixed(29);

mem.setI32(strlit_0_I14551632781070332322_tff8wodtr, 17);

mem.setI32((strlit_0_I14551632781070332322_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I14551632781070332322_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I14551632781070332322_tff8wodtr + 12), "bool round-trip: ");

let strlit_0_I933664836153361417_tff8wodtr = allocFixed(16);

mem.setI32(strlit_0_I933664836153361417_tff8wodtr, 4);

mem.setI32((strlit_0_I933664836153361417_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I933664836153361417_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I933664836153361417_tff8wodtr + 12), "nope");

let strlit_0_I6074055191300679588_tff8wodtr = allocFixed(32);

mem.setI32(strlit_0_I6074055191300679588_tff8wodtr, 20);

mem.setI32((strlit_0_I6074055191300679588_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I6074055191300679588_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I6074055191300679588_tff8wodtr + 12), "missing prop isNil: ");

let strlit_0_I11663264536865316071_tff8wodtr = allocFixed(27);

mem.setI32(strlit_0_I11663264536865316071_tff8wodtr, 15);

mem.setI32((strlit_0_I11663264536865316071_tff8wodtr + 4), 0);

mem.setI32((strlit_0_I11663264536865316071_tff8wodtr + 8), 0);

mem.writeStr((strlit_0_I11663264536865316071_tff8wodtr + 12), "self === self: ");

let console_0_tff8wodtr = allocFixed(4);

let m_0_tff8wodtr = allocFixed(4);

let o_0_tff8wodtr = allocFixed(4);

let X60QiniGuard_0_tff8wodtr = allocFixed(1);

function X60Qini_0_tff8wodtr() {
  if (mem.u8At(X60QiniGuard_0_tff8wodtr)) {
    return;
  }
  mem.setU8(X60QiniGuard_0_tff8wodtr, true);
  X60Qini_0_sysvq0asl();
  X60Qini_0_syn1lfpjv();
  X60Qini_0_jsfc0lwq21();
  mem.copy(console_0_tff8wodtr, global_0_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1852793854);
    mem.setU32((_o + 4), strlit_0_I4512127154334175537_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qtmp_1 = allocFixed(4);
  mem.copy(X60Qtmp_1, toJs_3_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1818585342);
    mem.setU32((_o + 4), strlit_0_I5853397530369228807_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qtmp_0 = allocFixed(4);
  mem.copy(X60Qtmp_0, call_1_jsfc0lwq21(console_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1735355395);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), X60Qtmp_1), 4);
  let X60Qx_0 = allocFixed(4);
  mem.copy(X60Qx_0, X60Qtmp_0, 4);
  mem.copy(m_0_tff8wodtr, global_0_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1952534014);
    mem.setU32((_o + 4), strlit_0_I12935728853510326435_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qtmp_5 = allocFixed(4);
  mem.copy(X60Qtmp_5, toJs_0_jsfc0lwq21(3), 4);
  let X60Qtmp_6 = allocFixed(4);
  mem.copy(X60Qtmp_6, toJs_0_jsfc0lwq21(7), 4);
  let X60Qtmp_4 = allocFixed(4);
  mem.copy(X60Qtmp_4, call_2_jsfc0lwq21(m_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2019650819);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), X60Qtmp_5, X60Qtmp_6), 4);
  let X60Qx_1 = toInt_0_jsfc0lwq21(X60Qtmp_4);
  let X60Qtmp_3 = allocFixed(8);
  mem.copy(X60Qtmp_3, dollarQ_2_sysvq0asl(X60Qx_1), 8);
  let X60Qtmp_2 = allocFixed(8);
  mem.copy(X60Qtmp_2, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1952534014);
    mem.setU32((_o + 4), strlit_0_I2301975825215568994_tff8wodtr);
    return _o;
  })(), X60Qtmp_3), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_2);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_10 = allocFixed(4);
  mem.copy(X60Qtmp_10, toJs_0_jsfc0lwq21(3), 4);
  let X60Qtmp_11 = allocFixed(4);
  mem.copy(X60Qtmp_11, toJs_0_jsfc0lwq21(7), 4);
  let X60Qtmp_12 = allocFixed(4);
  mem.copy(X60Qtmp_12, toJs_0_jsfc0lwq21(5), 4);
  let X60Qtmp_9 = allocFixed(4);
  mem.copy(X60Qtmp_9, call_3_jsfc0lwq21(m_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2019650819);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), X60Qtmp_10, X60Qtmp_11, X60Qtmp_12), 4);
  let X60Qx_2 = toInt_0_jsfc0lwq21(X60Qtmp_9);
  let X60Qtmp_8 = allocFixed(8);
  mem.copy(X60Qtmp_8, dollarQ_2_sysvq0asl(X60Qx_2), 8);
  let X60Qtmp_7 = allocFixed(8);
  mem.copy(X60Qtmp_7, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1952534014);
    mem.setU32((_o + 4), strlit_0_I6208868947491189530_tff8wodtr);
    return _o;
  })(), X60Qtmp_8), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_7);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_15 = allocFixed(4);
  mem.copy(X60Qtmp_15, toJs_3_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1970238206);
    mem.setU32((_o + 4), strlit_0_I12805258542531914732_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qtmp_14 = allocFixed(8);
  mem.copy(X60Qtmp_14, toStr_0_jsfc0lwq21(X60Qtmp_15), 8);
  let X60Qtmp_13 = allocFixed(8);
  mem.copy(X60Qtmp_13, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1667326718);
    mem.setU32((_o + 4), strlit_0_I15421062535947656287_tff8wodtr);
    return _o;
  })(), X60Qtmp_14), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_13);
  write_7_syn1lfpjv(stdout, 10);
  mem.copy(o_0_tff8wodtr, newJsObject_0_jsfc0lwq21(), 4);
  let X60Qtmp_16 = allocFixed(4);
  mem.copy(X60Qtmp_16, toJs_3_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1835626238);
    mem.setU32((_o + 4), strlit_0_I834269026986168554_tff8wodtr);
    return _o;
  })()), 4);
  set_0_jsfc0lwq21(o_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1835101950);
    mem.setU32((_o + 4), strlit_0_I407209193152762291_tff8wodtr);
    return _o;
  })(), X60Qtmp_16);
  let X60Qtmp_17 = allocFixed(4);
  mem.copy(X60Qtmp_17, toJs_0_jsfc0lwq21(2026), 4);
  set_0_jsfc0lwq21(o_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1634040318);
    mem.setU32((_o + 4), strlit_0_I6393444175955872624_tff8wodtr);
    return _o;
  })(), X60Qtmp_17);
  let X60Qtmp_20 = allocFixed(4);
  mem.copy(X60Qtmp_20, get_0_jsfc0lwq21(o_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1634040318);
    mem.setU32((_o + 4), strlit_0_I6393444175955872624_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qx_3 = toInt_0_jsfc0lwq21(X60Qtmp_20);
  let X60Qtmp_19 = allocFixed(8);
  mem.copy(X60Qtmp_19, dollarQ_2_sysvq0asl(X60Qx_3), 8);
  let X60Qtmp_18 = allocFixed(8);
  mem.copy(X60Qtmp_18, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2033086462);
    mem.setU32((_o + 4), strlit_0_I6644378417582681152_tff8wodtr);
    return _o;
  })(), X60Qtmp_19), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_18);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_23 = allocFixed(4);
  mem.copy(X60Qtmp_23, global_0_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1330858750);
    mem.setU32((_o + 4), strlit_0_I6373137695046429832_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qtmp_22 = allocFixed(4);
  mem.copy(X60Qtmp_22, call_1_jsfc0lwq21(X60Qtmp_23, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1920234494);
    mem.setU32((_o + 4), strlit_0_I7519307887382905868_tff8wodtr);
    return _o;
  })(), o_0_tff8wodtr), 4);
  let X60Qtmp_21 = allocFixed(8);
  mem.copy(X60Qtmp_21, toStr_0_jsfc0lwq21(X60Qtmp_22), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_21);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_24 = allocFixed(4);
  mem.copy(X60Qtmp_24, toJs_2_jsfc0lwq21(true), 4);
  let X60Qx_4 = toBool_0_jsfc0lwq21(X60Qtmp_24);
  let X60Qdesugar_0 = allocFixed(8);
  mem.copy(X60Qdesugar_0, dollarQ_3_sysvq0asl(X60Qx_4), 8);
  let X60Qtmp_25 = allocFixed(4);
  mem.copy(X60Qtmp_25, toJs_2_jsfc0lwq21(false), 4);
  let X60Qx_5 = toBool_0_jsfc0lwq21(X60Qtmp_25);
  let X60Qdesugar_1 = allocFixed(8);
  mem.copy(X60Qdesugar_1, dollarQ_3_sysvq0asl(X60Qx_5), 8);
  let X60Qx_6 = len_4_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1869570814);
    mem.setU32((_o + 4), strlit_0_I14551632781070332322_tff8wodtr);
    return _o;
  })());
  let X60Qx_7 = len_4_sysvq0asl(X60Qdesugar_0);
  let X60Qx_8 = len_4_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 8193);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  let X60Qx_9 = len_4_sysvq0asl(X60Qdesugar_1);
  let X60Qdesugar_2 = allocFixed(8);
  mem.copy(X60Qdesugar_2, newStringOfCap_0_sysvq0asl((((X60Qx_6 + X60Qx_7) + X60Qx_8) + X60Qx_9)), 8);
  add_2_sysvq0asl(X60Qdesugar_2, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1869570814);
    mem.setU32((_o + 4), strlit_0_I14551632781070332322_tff8wodtr);
    return _o;
  })());
  add_2_sysvq0asl(X60Qdesugar_2, X60Qdesugar_0);
  add_2_sysvq0asl(X60Qdesugar_2, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 8193);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  add_2_sysvq0asl(X60Qdesugar_2, X60Qdesugar_1);
  write_0_syn1lfpjv(stdout, X60Qdesugar_2);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_28 = allocFixed(4);
  mem.copy(X60Qtmp_28, get_0_jsfc0lwq21(o_0_tff8wodtr, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1886351102);
    mem.setU32((_o + 4), strlit_0_I933664836153361417_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qx_10 = isNil_0_jsfc0lwq21(X60Qtmp_28);
  let X60Qtmp_27 = allocFixed(8);
  mem.copy(X60Qtmp_27, dollarQ_3_sysvq0asl(X60Qx_10), 8);
  let X60Qtmp_26 = allocFixed(8);
  mem.copy(X60Qtmp_26, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1936289278);
    mem.setU32((_o + 4), strlit_0_I6074055191300679588_tff8wodtr);
    return _o;
  })(), X60Qtmp_27), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_26);
  write_7_syn1lfpjv(stdout, 10);
  let X60Qtmp_31 = allocFixed(4);
  mem.copy(X60Qtmp_31, global_0_jsfc0lwq21((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1952534014);
    mem.setU32((_o + 4), strlit_0_I12935728853510326435_tff8wodtr);
    return _o;
  })()), 4);
  let X60Qx_11 = eqQ_0_jsfc0lwq21(m_0_tff8wodtr, X60Qtmp_31);
  let X60Qtmp_30 = allocFixed(8);
  mem.copy(X60Qtmp_30, dollarQ_3_sysvq0asl(X60Qx_11), 8);
  let X60Qtmp_29 = allocFixed(8);
  mem.copy(X60Qtmp_29, ampQ_0_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1818588158);
    mem.setU32((_o + 4), strlit_0_I11663264536865316071_tff8wodtr);
    return _o;
  })(), X60Qtmp_30), 8);
  write_0_syn1lfpjv(stdout, X60Qtmp_29);
  write_7_syn1lfpjv(stdout, 10);
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
  eQdestroy_0_jsfc0lwq21(X60Qtmp_1);
}

let cmdCount = allocFixed(4);

let cmdLine = allocFixed(4);

let nimEnviron = allocFixed(4);

function main(X60Qargc_0_tff8wodtr, X60Qargv_0_tff8wodtr, X60Qenvp_0_tff8wodtr) {
  mem.setI32(cmdCount, X60Qargc_0_tff8wodtr);
  mem.setU32(cmdLine, X60Qargv_0_tff8wodtr);
  mem.setU32(nimEnviron, X60Qenvp_0_tff8wodtr);
  X60Qini_0_tff8wodtr();
  nimFlushStdStreams();
  return 0;
}

// generated by lengc (js backend) from sysvq0asl.c.nif
"use strict";

let strlit_0_I1565838944098044620_sysvq0asl = allocFixed(105);

mem.setI32(strlit_0_I1565838944098044620_sysvq0asl, 93);

mem.setI32((strlit_0_I1565838944098044620_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I1565838944098044620_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I1565838944098044620_sysvq0asl + 12), "../nimony-js/lib/std/system/stringimpl.nim(403, 37): i < len(s) and 0 <= i [AssertionDefect]\n");

let strlit_0_I7364560965974357967_sysvq0asl = allocFixed(105);

mem.setI32(strlit_0_I7364560965974357967_sysvq0asl, 93);

mem.setI32((strlit_0_I7364560965974357967_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I7364560965974357967_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I7364560965974357967_sysvq0asl + 12), "../nimony-js/lib/std/system/stringimpl.nim(407, 45): i < len(s) and 0 <= i [AssertionDefect]\n");

let strlit_0_I15539159382304113184_sysvq0asl = allocFixed(39);

mem.setI32(strlit_0_I15539159382304113184_sysvq0asl, 27);

mem.setI32((strlit_0_I15539159382304113184_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I15539159382304113184_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I15539159382304113184_sysvq0asl + 12), "invalid object conversion: ");

let strlit_0_I14281474217946372742_sysvq0asl = allocFixed(47);

mem.setI32(strlit_0_I14281474217946372742_sysvq0asl, 35);

mem.setI32((strlit_0_I14281474217946372742_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I14281474217946372742_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I14281474217946372742_sysvq0asl + 12), "cannot dispatch; dispatcher is nil\n");

let strlit_0_I16690852185662743073_sysvq0asl = allocFixed(28);

mem.setI32(strlit_0_I16690852185662743073_sysvq0asl, 16);

mem.setI32((strlit_0_I16690852185662743073_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I16690852185662743073_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I16690852185662743073_sysvq0asl + 12), "could not load: ");

let strlit_0_I10604297744791418982_sysvq0asl = allocFixed(30);

mem.setI32(strlit_0_I10604297744791418982_sysvq0asl, 18);

mem.setI32((strlit_0_I10604297744791418982_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I10604297744791418982_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I10604297744791418982_sysvq0asl + 12), "could not import: ");

let strlit_0_I11614695157650328859_sysvq0asl = allocFixed(33);

mem.setI32(strlit_0_I11614695157650328859_sysvq0asl, 21);

mem.setI32((strlit_0_I11614695157650328859_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I11614695157650328859_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I11614695157650328859_sysvq0asl + 12), "index out of bounds: ");

let strlit_0_I16845119709590674135_sysvq0asl = allocFixed(19);

mem.setI32(strlit_0_I16845119709590674135_sysvq0asl, 7);

mem.setI32((strlit_0_I16845119709590674135_sysvq0asl + 4), 0);

mem.setI32((strlit_0_I16845119709590674135_sysvq0asl + 8), 0);

mem.writeStr((strlit_0_I16845119709590674135_sysvq0asl + 12), " notin ");

function min_2_sysvq0asl(x_204, y_161) {
  let result_5;
  let X60Qx_2;
  if ((x_204 <= y_161)) {
    X60Qx_2 = x_204;
  } else {
    X60Qx_2 = y_161;
  }
  result_5 = X60Qx_2;
  return result_5;
}

function max_2_sysvq0asl(x_211, y_168) {
  let result_12;
  let X60Qx_8;
  if ((y_168 <= x_211)) {
    X60Qx_8 = x_211;
  } else {
    X60Qx_8 = y_168;
  }
  result_12 = X60Qx_8;
  return result_12;
}

let NegTen_0_sysvq0asl = allocFixed(80);

mem.copy(NegTen_0_sysvq0asl, (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3157250);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 8), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3222786);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 16), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3288322);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 24), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3353858);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 32), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3419394);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 40), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3484930);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 48), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3550466);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 56), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3616002);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 64), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3681538);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

mem.copy((NegTen_0_sysvq0asl + 72), (() => {
  let _o = allocFixed(8);
  mem.setU32(_o, 3747074);
  mem.setU32((_o + 4), 0);
  return _o;
})(), 8);

function dollarQ_0_sysvq0asl(x_224) {
  var result_19 = allocFixed(8);
  nimStrWasMoved(result_19);
  nimStrDestroy(result_19);
  mem.copy(result_19, (() => {
    var _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  if ((x_224 < 10n)) {
    nimStrDestroy(result_19);
    var X60Qx_57 = nimIcheckB(Number(BigInt.asIntN(32, x_224)), 9);
    var X60Qx_58 = allocFixed(8);
    mem.copy(X60Qx_58, substr_0_sysvq0asl((NegTen_0_sysvq0asl + (X60Qx_57 * 8)), 1, 1), 8);
    mem.copy(result_19, X60Qx_58, 8);
  } else {
    whileStmtLabel_1: {
      whileStmtLabel_0: {
        var y_208 = x_224;
        {
          while (true) {
            add_1_sysvq0asl(result_19, Number(BigInt.asUintN(8, BigInt.asUintN(64, ((y_208 % 10n) + 48n)))));
            y_208 = (y_208 / 10n);
            if ((y_208 === 0n)) {
              break whileStmtLabel_0;
            }
          }
        }
      }
      var X60Qx_59 = len_4_sysvq0asl(result_19);
      var last_3 = (X60Qx_59 - 1);
      var i_25 = allocFixed(4);
      mem.setI32(i_25, 0);
      var X60Qx_60 = len_4_sysvq0asl(result_19);
      var b_29 = Math.trunc((X60Qx_60 / 2));
      {
        while ((mem.i32(i_25) < b_29)) {
          var ch_1 = getQ_9_sysvq0asl(result_19, mem.i32(i_25));
          var X60Qx_61 = getQ_9_sysvq0asl(result_19, (last_3 - mem.i32(i_25)));
          putQ_9_sysvq0asl(result_19, mem.i32(i_25), X60Qx_61);
          putQ_9_sysvq0asl(result_19, (last_3 - mem.i32(i_25)), ch_1);
          inc_1_I6wjjge_jsfc0lwq21(i_25);
        }
      }
    }
  }
  return result_19;
}

function dollarQ_1_sysvq0asl(x_225) {
  let result_20 = allocFixed(8);
  nimStrWasMoved(result_20);
  if ((x_225 < 0n)) {
    if ((-10n < x_225)) {
      nimStrDestroy(result_20);
      let X60Qx_62 = nimIcheckB(Number(BigInt.asIntN(32, BigInt.asIntN(64, (-x_225)))), 9);
      let X60Qx_63 = allocFixed(8);
      mem.copy(X60Qx_63, nimStrDup((NegTen_0_sysvq0asl + (X60Qx_62 * 8))), 8);
      mem.copy(result_20, X60Qx_63, 8);
    } else {
      if ((x_225 === -9223372036854775808n)) {
        nimStrDestroy(result_20);
        let X60Qtmp_0 = allocFixed(8);
        mem.copy(X60Qtmp_0, dollarQ_0_sysvq0asl(BigInt.asUintN(64, x_225)), 8);
        let X60Qx_64 = allocFixed(8);
        mem.copy(X60Qx_64, ampQ_0_sysvq0asl((() => {
          let _o = allocFixed(8);
          mem.setU32(_o, 11521);
          mem.setU32((_o + 4), 0);
          return _o;
        })(), X60Qtmp_0), 8);
        mem.copy(result_20, X60Qx_64, 8);
        nimStrDestroy(X60Qtmp_0);
      } else {
        nimStrDestroy(result_20);
        let X60Qtmp_1 = allocFixed(8);
        mem.copy(X60Qtmp_1, dollarQ_1_sysvq0asl(BigInt.asIntN(64, (0n - x_225))), 8);
        let X60Qx_65 = allocFixed(8);
        mem.copy(X60Qx_65, ampQ_0_sysvq0asl((() => {
          let _o = allocFixed(8);
          mem.setU32(_o, 11521);
          mem.setU32((_o + 4), 0);
          return _o;
        })(), X60Qtmp_1), 8);
        mem.copy(result_20, X60Qx_65, 8);
        nimStrDestroy(X60Qtmp_1);
      }
    }
  } else {
    if ((x_225 < 10n)) {
      nimStrDestroy(result_20);
      mem.copy(result_20, (() => {
        let _o = allocFixed(8);
        mem.setU32(_o, 0);
        mem.setU32((_o + 4), 0);
        return _o;
      })(), 8);
      add_1_sysvq0asl(result_20, Number(BigInt.asUintN(8, BigInt.asIntN(64, (x_225 + 48n)))));
    } else {
      nimStrDestroy(result_20);
      let X60Qx_66 = allocFixed(8);
      mem.copy(X60Qx_66, dollarQ_0_sysvq0asl(BigInt.asUintN(64, x_225)), 8);
      mem.copy(result_20, X60Qx_66, 8);
    }
  }
  return result_20;
}

function dollarQ_2_sysvq0asl(x_226) {
  let result_21 = allocFixed(8);
  nimStrWasMoved(result_21);
  nimStrDestroy(result_21);
  let X60Qx_67 = allocFixed(8);
  mem.copy(X60Qx_67, dollarQ_1_sysvq0asl(BigInt(x_226)), 8);
  mem.copy(result_21, X60Qx_67, 8);
  return result_21;
}

function dollarQ_3_sysvq0asl(b_1) {
  let result_22 = allocFixed(8);
  nimStrWasMoved(result_22);
  let X60Qx_12 = allocFixed(8);
  nimStrWasMoved(X60Qx_12);
  if (b_1) {
    nimStrDestroy(X60Qx_12);
    mem.copy(X60Qx_12, (() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 1970435326);
      mem.setU32((_o + 4), strlit_0_I8572766038233537570_syn1lfpjv);
      return _o;
    })(), 8);
  } else {
    nimStrDestroy(X60Qx_12);
    mem.copy(X60Qx_12, (() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 1818322686);
      mem.setU32((_o + 4), strlit_0_I3372626016653902757_syn1lfpjv);
      return _o;
    })(), 8);
  }
  nimStrDestroy(result_22);
  mem.copy(result_22, X60Qx_12, 8);
  nimStrWasMoved(X60Qx_12);
  nimStrDestroy(X60Qx_12);
  return result_22;
  nimStrDestroy(X60Qx_12);
  return result_22;
}

function nimNoopFlush_0_sysvq0asl() {
}

let gExitFlush_0_sysvq0asl = allocFixed(4);

mem.setU32(gExitFlush_0_sysvq0asl, _fnid(nimNoopFlush_0_sysvq0asl));

function nimFlushStdStreams() {
  _fns[mem.u32(gExitFlush_0_sysvq0asl)]();
}

function cAbort_0_sysvq0asl() {
  _fns[mem.u32(gExitFlush_0_sysvq0asl)]();
  abort();
}

function copyMem_0_sysvq0asl(dest_4, src_3, size_3) {
  memcpy(dest_4, src_3, size_3);
}

function zeroMem_0_sysvq0asl(dest_6, size_6) {
  memset(dest_6, 0, size_6);
}

function raiseOutOfMem_0_sysvq0asl() {
  cAbort_0_sysvq0asl();
}

function align_0_sysvq0asl(address_0, alignment_0) {
  let result_30;
  result_30 = ((address_0 + (alignment_0 - 1)) & (~(alignment_0 - 1)));
  return result_30;
}

function roundup_0_sysvq0asl(x_296, v_0) {
  let result_31;
  result_31 = ((x_296 + (v_0 - 1)) & (~(v_0 - 1)));
  return result_31;
}

function osAllocPages_0_sysvq0asl(size_7) {
  let result_32;
  let X60Qx_78 = mmap(0, size_7, (1 | 2), ((32 | 2) | 0), -1, 0);
  result_32 = X60Qx_78;
  let X60Qx_79;
  if ((result_32 === 0)) {
    X60Qx_79 = true;
  } else {
    X60Qx_79 = (result_32 === -1);
  }
  if (X60Qx_79) {
    raiseOutOfMem_0_sysvq0asl();
  }
  return result_32;
}

function osTryAllocPages_0_sysvq0asl(size_8) {
  let result_33;
  let X60Qx_80 = mmap(0, size_8, (1 | 2), ((32 | 2) | 0), -1, 0);
  result_33 = X60Qx_80;
  if ((result_33 === -1)) {
    result_33 = 0;
  }
  return result_33;
}

function osDeallocPages_0_sysvq0asl(p_9, size_9) {
}

let fsLookupTable_0_sysvq0asl = allocFixed(256);

mem.setI8(fsLookupTable_0_sysvq0asl, -1);

mem.setI8((fsLookupTable_0_sysvq0asl + 1), 0);

mem.setI8((fsLookupTable_0_sysvq0asl + 2), 1);

mem.setI8((fsLookupTable_0_sysvq0asl + 3), 1);

mem.setI8((fsLookupTable_0_sysvq0asl + 4), 2);

mem.setI8((fsLookupTable_0_sysvq0asl + 5), 2);

mem.setI8((fsLookupTable_0_sysvq0asl + 6), 2);

mem.setI8((fsLookupTable_0_sysvq0asl + 7), 2);

mem.setI8((fsLookupTable_0_sysvq0asl + 8), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 9), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 10), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 11), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 12), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 13), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 14), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 15), 3);

mem.setI8((fsLookupTable_0_sysvq0asl + 16), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 17), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 18), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 19), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 20), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 21), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 22), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 23), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 24), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 25), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 26), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 27), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 28), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 29), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 30), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 31), 4);

mem.setI8((fsLookupTable_0_sysvq0asl + 32), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 33), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 34), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 35), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 36), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 37), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 38), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 39), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 40), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 41), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 42), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 43), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 44), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 45), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 46), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 47), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 48), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 49), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 50), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 51), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 52), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 53), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 54), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 55), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 56), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 57), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 58), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 59), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 60), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 61), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 62), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 63), 5);

mem.setI8((fsLookupTable_0_sysvq0asl + 64), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 65), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 66), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 67), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 68), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 69), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 70), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 71), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 72), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 73), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 74), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 75), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 76), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 77), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 78), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 79), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 80), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 81), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 82), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 83), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 84), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 85), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 86), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 87), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 88), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 89), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 90), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 91), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 92), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 93), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 94), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 95), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 96), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 97), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 98), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 99), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 100), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 101), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 102), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 103), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 104), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 105), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 106), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 107), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 108), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 109), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 110), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 111), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 112), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 113), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 114), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 115), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 116), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 117), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 118), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 119), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 120), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 121), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 122), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 123), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 124), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 125), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 126), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 127), 6);

mem.setI8((fsLookupTable_0_sysvq0asl + 128), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 129), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 130), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 131), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 132), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 133), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 134), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 135), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 136), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 137), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 138), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 139), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 140), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 141), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 142), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 143), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 144), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 145), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 146), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 147), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 148), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 149), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 150), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 151), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 152), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 153), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 154), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 155), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 156), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 157), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 158), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 159), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 160), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 161), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 162), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 163), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 164), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 165), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 166), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 167), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 168), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 169), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 170), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 171), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 172), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 173), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 174), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 175), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 176), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 177), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 178), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 179), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 180), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 181), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 182), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 183), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 184), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 185), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 186), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 187), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 188), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 189), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 190), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 191), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 192), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 193), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 194), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 195), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 196), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 197), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 198), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 199), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 200), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 201), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 202), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 203), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 204), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 205), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 206), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 207), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 208), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 209), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 210), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 211), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 212), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 213), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 214), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 215), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 216), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 217), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 218), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 219), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 220), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 221), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 222), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 223), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 224), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 225), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 226), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 227), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 228), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 229), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 230), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 231), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 232), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 233), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 234), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 235), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 236), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 237), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 238), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 239), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 240), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 241), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 242), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 243), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 244), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 245), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 246), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 247), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 248), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 249), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 250), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 251), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 252), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 253), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 254), 7);

mem.setI8((fsLookupTable_0_sysvq0asl + 255), 7);

function msbit_0_sysvq0asl(x_301) {
  let result_34;
  let X60Qx_13;
  if ((x_301 <= 65535)) {
    let X60Qx_14;
    if ((x_301 <= 255)) {
      X60Qx_14 = 0;
    } else {
      X60Qx_14 = 8;
    }
    X60Qx_13 = X60Qx_14;
  } else {
    let X60Qx_15;
    if ((x_301 <= 16777215)) {
      X60Qx_15 = 16;
    } else {
      X60Qx_15 = 24;
    }
    X60Qx_13 = X60Qx_15;
  }
  let a_74 = X60Qx_13;
  let X60Qx_81 = nimUcheckB(((x_301 >>> a_74) & 255), 255);
  result_34 = (mem.i8((fsLookupTable_0_sysvq0asl + X60Qx_81)) + a_74);
  return result_34;
}

function lsbit_0_sysvq0asl(x_302) {
  let result_35;
  let X60Qx_82 = msbit_0_sysvq0asl(((x_302 & (((~x_302) >>> 0) + 1)) >>> 0));
  result_35 = X60Qx_82;
  return result_35;
}

function setBit_0_sysvq0asl(nr_0, dest_7) {
  mem.setU32(dest_7, ((mem.u32(dest_7) | ((1 << (nr_0 & 31)) >>> 0)) >>> 0));
}

function clearBit_0_sysvq0asl(nr_1, dest_8) {
  mem.setU32(dest_8, ((mem.u32(dest_8) & ((~((1 << (nr_1 & 31)) >>> 0)) >>> 0)) >>> 0));
}

function mappingSearch_0_sysvq0asl(r_0, fl_0, sl_0) {
  let X60Qx_83 = msbit_0_sysvq0asl(mem.i32(r_0));
  let X60Qx_84 = roundup_0_sysvq0asl((1 << (X60Qx_83 - 5)), 4096);
  let t_3 = (X60Qx_84 - 1);
  mem.setI32(r_0, (mem.i32(r_0) + t_3));
  mem.setI32(r_0, (mem.i32(r_0) & (~t_3)));
  let X60Qx_85 = min_2_sysvq0asl(mem.i32(r_0), 1056964608);
  mem.setI32(r_0, X60Qx_85);
  let X60Qx_86 = msbit_0_sysvq0asl(mem.i32(r_0));
  mem.setI32(fl_0, X60Qx_86);
  mem.setI32(sl_0, ((mem.i32(r_0) >> (mem.i32(fl_0) - 5)) - 32));
  dec_0_Ig5i8xp_sysvq0asl(fl_0, 6);
}

function mappingInsert_0_sysvq0asl(r_1) {
  let result_36 = allocFixed(8);
  let fl_4 = msbit_0_sysvq0asl(r_1);
  let sl_5 = ((r_1 >> (fl_4 - 5)) - 32);
  fl_4 = (fl_4 - 6);
  mem.copy(result_36, (() => {
    let _o = allocFixed(8);
    mem.setI32(_o, fl_4);
    mem.setI32((_o + 4), sl_5);
    return _o;
  })(), 8);
  return result_36;
}

function findSuitableBlock_0_sysvq0asl(a_6, fl_1, sl_1) {
  let result_37;
  let X60Qx_87 = nimIcheckB(mem.i32(fl_1), 23);
  let tmp_2 = ((mem.u32(((a_6 + 2052) + (X60Qx_87 * 4))) & ((((~0) >>> 0) << mem.i32(sl_1)) >>> 0)) >>> 0);
  result_37 = 0;
  if ((!(tmp_2 === 0))) {
    let X60Qx_88 = lsbit_0_sysvq0asl(tmp_2);
    mem.setI32(sl_1, X60Qx_88);
    let X60Qx_89 = nimIcheckB(mem.i32(fl_1), 23);
    let X60Qx_90 = nimIcheckB(mem.i32(sl_1), 31);
    result_37 = mem.u32((((a_6 + 2148) + (X60Qx_89 * 128)) + (X60Qx_90 * 4)));
  } else {
    let X60Qx_91 = lsbit_0_sysvq0asl(((mem.u32((a_6 + 2048)) & ((((~0) >>> 0) << (mem.i32(fl_1) + 1)) >>> 0)) >>> 0));
    mem.setI32(fl_1, X60Qx_91);
    if ((0 < mem.i32(fl_1))) {
      let X60Qx_92 = nimIcheckB(mem.i32(fl_1), 23);
      let X60Qx_93 = lsbit_0_sysvq0asl(mem.u32(((a_6 + 2052) + (X60Qx_92 * 4))));
      mem.setI32(sl_1, X60Qx_93);
      let X60Qx_94 = nimIcheckB(mem.i32(fl_1), 23);
      let X60Qx_95 = nimIcheckB(mem.i32(sl_1), 31);
      result_37 = mem.u32((((a_6 + 2148) + (X60Qx_94 * 128)) + (X60Qx_95 * 4)));
    }
  }
  return result_37;
}

function removeChunkFromMatrix_0_sysvq0asl(a_7, b_7) {
  let X60Qtmptup_0 = allocFixed(8);
  mem.copy(X60Qtmptup_0, mappingInsert_0_sysvq0asl(mem.i32((b_7 + 4))), 8);
  let fl_5 = mem.i32(X60Qtmptup_0);
  let sl_6 = mem.i32((X60Qtmptup_0 + 4));
  if ((!(mem.u32((b_7 + 12)) === 0))) {
    mem.setU32((mem.u32((b_7 + 12)) + 16), mem.u32((b_7 + 16)));
  }
  if ((!(mem.u32((b_7 + 16)) === 0))) {
    mem.setU32((mem.u32((b_7 + 16)) + 12), mem.u32((b_7 + 12)));
  }
  let X60Qx_96 = nimIcheckB(fl_5, 23);
  let X60Qx_97 = nimIcheckB(sl_6, 31);
  if ((mem.u32((((a_7 + 2148) + (X60Qx_96 * 128)) + (X60Qx_97 * 4))) === b_7)) {
    let X60Qx_98 = nimIcheckB(fl_5, 23);
    let X60Qx_99 = nimIcheckB(sl_6, 31);
    mem.setU32((((a_7 + 2148) + (X60Qx_98 * 128)) + (X60Qx_99 * 4)), mem.u32((b_7 + 12)));
    let X60Qx_100 = nimIcheckB(fl_5, 23);
    let X60Qx_101 = nimIcheckB(sl_6, 31);
    if ((mem.u32((((a_7 + 2148) + (X60Qx_100 * 128)) + (X60Qx_101 * 4))) === 0)) {
      let X60Qx_102 = nimIcheckB(fl_5, 23);
      clearBit_0_sysvq0asl(sl_6, ((a_7 + 2052) + (X60Qx_102 * 4)));
      let X60Qx_103 = nimIcheckB(fl_5, 23);
      if ((mem.u32(((a_7 + 2052) + (X60Qx_103 * 4))) === 0)) {
        clearBit_0_sysvq0asl(fl_5, (a_7 + 2048));
      }
    }
  }
  mem.setU32((b_7 + 16), 0);
  mem.setU32((b_7 + 12), 0);
}

function removeChunkFromMatrix2_0_sysvq0asl(a_8, b_8, fl_3, sl_3) {
  let X60Qx_104 = nimIcheckB(fl_3, 23);
  let X60Qx_105 = nimIcheckB(sl_3, 31);
  mem.setU32((((a_8 + 2148) + (X60Qx_104 * 128)) + (X60Qx_105 * 4)), mem.u32((b_8 + 12)));
  let X60Qx_106 = nimIcheckB(fl_3, 23);
  let X60Qx_107 = nimIcheckB(sl_3, 31);
  if ((!(mem.u32((((a_8 + 2148) + (X60Qx_106 * 128)) + (X60Qx_107 * 4))) === 0))) {
    let X60Qx_108 = nimIcheckB(fl_3, 23);
    let X60Qx_109 = nimIcheckB(sl_3, 31);
    mem.setU32((mem.u32((((a_8 + 2148) + (X60Qx_108 * 128)) + (X60Qx_109 * 4))) + 16), 0);
  } else {
    let X60Qx_110 = nimIcheckB(fl_3, 23);
    clearBit_0_sysvq0asl(sl_3, ((a_8 + 2052) + (X60Qx_110 * 4)));
    let X60Qx_111 = nimIcheckB(fl_3, 23);
    if ((mem.u32(((a_8 + 2052) + (X60Qx_111 * 4))) === 0)) {
      clearBit_0_sysvq0asl(fl_3, (a_8 + 2048));
    }
  }
  mem.setU32((b_8 + 16), 0);
  mem.setU32((b_8 + 12), 0);
}

function addChunkToMatrix_0_sysvq0asl(a_9, b_9) {
  let X60Qtmptup_1 = allocFixed(8);
  mem.copy(X60Qtmptup_1, mappingInsert_0_sysvq0asl(mem.i32((b_9 + 4))), 8);
  let fl_6 = mem.i32(X60Qtmptup_1);
  let sl_7 = mem.i32((X60Qtmptup_1 + 4));
  mem.setU32((b_9 + 16), 0);
  let X60Qx_112 = nimIcheckB(fl_6, 23);
  let X60Qx_113 = nimIcheckB(sl_7, 31);
  mem.setU32((b_9 + 12), mem.u32((((a_9 + 2148) + (X60Qx_112 * 128)) + (X60Qx_113 * 4))));
  let X60Qx_114 = nimIcheckB(fl_6, 23);
  let X60Qx_115 = nimIcheckB(sl_7, 31);
  if ((!(mem.u32((((a_9 + 2148) + (X60Qx_114 * 128)) + (X60Qx_115 * 4))) === 0))) {
    let X60Qx_116 = nimIcheckB(fl_6, 23);
    let X60Qx_117 = nimIcheckB(sl_7, 31);
    mem.setU32((mem.u32((((a_9 + 2148) + (X60Qx_116 * 128)) + (X60Qx_117 * 4))) + 16), b_9);
  }
  let X60Qx_118 = nimIcheckB(fl_6, 23);
  let X60Qx_119 = nimIcheckB(sl_7, 31);
  mem.setU32((((a_9 + 2148) + (X60Qx_118 * 128)) + (X60Qx_119 * 4)), b_9);
  let X60Qx_120 = nimIcheckB(fl_6, 23);
  setBit_0_sysvq0asl(sl_7, ((a_9 + 2052) + (X60Qx_120 * 4)));
  setBit_0_sysvq0asl(fl_6, (a_9 + 2048));
}

function incCurrMem_0_sysvq0asl(a_10, bytes_0) {
  inc_0_Iloplki_sysvq0asl((a_10 + 5224), bytes_0);
}

function decCurrMem_0_sysvq0asl(a_11, bytes_1) {
  let X60Qx_121 = max_2_sysvq0asl(mem.i32((a_11 + 5228)), mem.i32((a_11 + 5224)));
  mem.setI32((a_11 + 5228), X60Qx_121);
  dec_0_Ig5i8xp_sysvq0asl((a_11 + 5224), bytes_1);
}

function allocPages_0_sysvq0asl(a_13, size_11) {
  let result_39;
  let X60Qx_123 = osAllocPages_0_sysvq0asl(size_11);
  result_39 = X60Qx_123;
  return result_39;
}

function tryAllocPages_0_sysvq0asl(a_14, size_12) {
  let result_40;
  let X60Qx_124 = osTryAllocPages_0_sysvq0asl(size_12);
  result_40 = X60Qx_124;
  return result_40;
}

function llAlloc_0_sysvq0asl(a_15, size_13) {
  let result_41;
  let X60Qx_125;
  if ((mem.u32((a_15 + 5220)) === 0)) {
    X60Qx_125 = true;
  } else {
    X60Qx_125 = (mem.i32(mem.u32((a_15 + 5220))) < size_13);
  }
  if (X60Qx_125) {
    let old_1 = mem.u32((a_15 + 5220));
    let X60Qx_126 = allocPages_0_sysvq0asl(a_15, 4096);
    mem.setU32((a_15 + 5220), X60Qx_126);
    incCurrMem_0_sysvq0asl(a_15, 4096);
    mem.setI32(mem.u32((a_15 + 5220)), 4084);
    mem.setI32((mem.u32((a_15 + 5220)) + 4), 12);
    mem.setU32((mem.u32((a_15 + 5220)) + 8), old_1);
  }
  result_41 = (mem.u32((a_15 + 5220)) + mem.i32((mem.u32((a_15 + 5220)) + 4)));
  dec_0_Ig5i8xp_sysvq0asl(mem.u32((a_15 + 5220)), size_13);
  inc_0_Iloplki_sysvq0asl((mem.u32((a_15 + 5220)) + 4), size_13);
  zeroMem_0_sysvq0asl(result_41, size_13);
  return result_41;
}

function addHeapLink_0_sysvq0asl(a_16, p_10, size_14) {
  whileStmtLabel_0: {
    var result_42;
    var it_0 = (a_16 + 6280);
    {
      while (true) {
        var X60Qx_127;
        if ((!(it_0 === 0))) {
          X60Qx_127 = (30 <= mem.i32(it_0));
        } else {
          X60Qx_127 = false;
        }
        if (X60Qx_127) {
          it_0 = mem.u32((it_0 + 244));
        } else {
          break;
        }
      }
    }
  }
  if ((it_0 === 0)) {
    var X60Qx_128 = llAlloc_0_sysvq0asl(a_16, 248);
    var n_7 = X60Qx_128;
    mem.setU32((n_7 + 244), mem.u32(((a_16 + 6280) + 244)));
    mem.setU32(((a_16 + 6280) + 244), n_7);
    var X60Qx_129 = nimIcheckB(0, 29);
    mem.copy(((n_7 + 4) + (X60Qx_129 * 8)), (() => {
      var _o = allocFixed(8);
      mem.setU32(_o, p_10);
      mem.setI32((_o + 4), size_14);
      return _o;
    })(), 8);
    mem.setI32(n_7, 1);
    result_42 = n_7;
  } else {
    var L_0 = mem.i32(it_0);
    var X60Qx_130 = nimIcheckB(L_0, 29);
    mem.copy(((it_0 + 4) + (X60Qx_130 * 8)), (() => {
      var _o = allocFixed(8);
      mem.setU32(_o, p_10);
      mem.setI32((_o + 4), size_14);
      return _o;
    })(), 8);
    inc_1_I6wjjge_jsfc0lwq21(it_0);
    result_42 = it_0;
  }
  return result_42;
}

function intSetGet_0_sysvq0asl(t_0, key_0) {
  whileStmtLabel_0: {
    var result_43;
    var X60Qx_131 = nimIcheckB((key_0 & 255), 255);
    var it_2 = mem.u32((t_0 + (X60Qx_131 * 4)));
    {
      while ((!(it_2 === 0))) {
        if ((mem.i32((it_2 + 4)) === key_0)) {
          return it_2;
        }
        it_2 = mem.u32(it_2);
      }
    }
  }
  result_43 = 0;
  return result_43;
}

function intSetPut_0_sysvq0asl(a_18, key_1) {
  let result_44;
  let X60Qx_132 = intSetGet_0_sysvq0asl((a_18 + 5248), key_1);
  result_44 = X60Qx_132;
  if ((result_44 === 0)) {
    let X60Qx_133 = llAlloc_0_sysvq0asl(a_18, 72);
    result_44 = X60Qx_133;
    let X60Qx_134 = nimIcheckB((key_1 & 255), 255);
    mem.setU32(result_44, mem.u32(((a_18 + 5248) + (X60Qx_134 * 4))));
    let X60Qx_135 = nimIcheckB((key_1 & 255), 255);
    mem.setU32(((a_18 + 5248) + (X60Qx_135 * 4)), result_44);
    mem.setI32((result_44 + 4), key_1);
  }
  return result_44;
}

function contains_1_sysvq0asl(s_4, key_2) {
  let result_45;
  let t_4 = intSetGet_0_sysvq0asl(s_4, (key_2 >> 9));
  if ((!(t_4 === 0))) {
    let u_1 = (key_2 & 511);
    let X60Qx_136 = nimIcheckB((u_1 >> 5), 15);
    result_45 = (!(((mem.u32(((t_4 + 8) + (X60Qx_136 * 4))) & ((1 << (u_1 & 31)) >>> 0)) >>> 0) === 0));
  } else {
    result_45 = false;
  }
  return result_45;
}

function incl_2_sysvq0asl(a_19, key_3) {
  let t_5 = intSetPut_0_sysvq0asl(a_19, (key_3 >> 9));
  let u_2 = (key_3 & 511);
  let X60Qx_137 = nimIcheckB((u_2 >> 5), 15);
  let X60Qx_138 = nimIcheckB((u_2 >> 5), 15);
  mem.setU32(((t_5 + 8) + (X60Qx_137 * 4)), ((mem.u32(((t_5 + 8) + (X60Qx_138 * 4))) | ((1 << (u_2 & 31)) >>> 0)) >>> 0));
}

function excl_2_sysvq0asl(s_5, key_4) {
  let t_6 = intSetGet_0_sysvq0asl(s_5, (key_4 >> 9));
  if ((!(t_6 === 0))) {
    let u_3 = (key_4 & 511);
    let X60Qx_139 = nimIcheckB((u_3 >> 5), 15);
    let X60Qx_140 = nimIcheckB((u_3 >> 5), 15);
    mem.setU32(((t_6 + 8) + (X60Qx_139 * 4)), ((mem.u32(((t_6 + 8) + (X60Qx_140 * 4))) & ((~((1 << (u_3 & 31)) >>> 0)) >>> 0)) >>> 0));
  }
}

function isSmallChunk_0_sysvq0asl(c_0) {
  let result_46;
  result_46 = (mem.i32((c_0 + 4)) <= 4056);
  return result_46;
}

function chunkUnused_0_sysvq0asl(c_1) {
  let result_47;
  result_47 = ((mem.i32(c_1) & 1) === 0);
  return result_47;
}

function pageIndex_0_sysvq0asl(c_2) {
  let result_48;
  result_48 = (c_2 >> 12);
  return result_48;
}

function pageIndex_1_sysvq0asl(p_11) {
  let result_49;
  result_49 = (p_11 >> 12);
  return result_49;
}

function pageAddr_0_sysvq0asl(p_12) {
  let result_50;
  result_50 = (p_12 & (~4095));
  return result_50;
}

function requestOsChunks_0_sysvq0asl(a_20, size_15) {
  let result_51;
  if ((!mem.u8At((a_20 + 6274)))) {
    let usedMem_0 = mem.i32((a_20 + 5236));
    if ((usedMem_0 < 65536)) {
      mem.setI32((a_20 + 6276), 16384);
    } else {
      let X60Qx_141 = roundup_0_sysvq0asl((usedMem_0 >> 2), 4096);
      let X60Qx_142 = min_2_sysvq0asl(X60Qx_141, (mem.i32((a_20 + 6276)) * 2));
      mem.setI32((a_20 + 6276), X60Qx_142);
      let X60Qx_143 = min_2_sysvq0asl(mem.i32((a_20 + 6276)), 1056964608);
      mem.setI32((a_20 + 6276), X60Qx_143);
    }
  }
  let size_36 = size_15;
  if ((mem.i32((a_20 + 6276)) < size_36)) {
    let X60Qx_144 = allocPages_0_sysvq0asl(a_20, size_36);
    result_51 = X60Qx_144;
  } else {
    let X60Qx_145 = tryAllocPages_0_sysvq0asl(a_20, mem.i32((a_20 + 6276)));
    result_51 = X60Qx_145;
    if ((result_51 === 0)) {
      let X60Qx_146 = allocPages_0_sysvq0asl(a_20, size_36);
      result_51 = X60Qx_146;
      mem.setU8((a_20 + 6274), true);
    } else {
      size_36 = mem.i32((a_20 + 6276));
    }
  }
  incCurrMem_0_sysvq0asl(a_20, size_36);
  inc_0_Iloplki_sysvq0asl((a_20 + 5232), size_36);
  let heapLink_0 = addHeapLink_0_sysvq0asl(a_20, result_51, size_36);
  mem.setU32((result_51 + 12), 0);
  mem.setU32((result_51 + 16), 0);
  mem.setI32((result_51 + 4), size_36);
  let nxt_0 = (result_51 + size_36);
  let next_1 = nxt_0;
  let X60Qx_147 = pageIndex_0_sysvq0asl(next_1);
  let X60Qx_148 = contains_1_sysvq0asl((a_20 + 5248), X60Qx_147);
  if (X60Qx_148) {
    mem.setI32(next_1, (size_36 | (mem.i32(next_1) & 1)));
  }
  let X60Qx_16;
  if ((!(mem.i32((a_20 + 5240)) === 0))) {
    X60Qx_16 = mem.i32((a_20 + 5240));
  } else {
    X60Qx_16 = 4096;
  }
  let lastSize_0 = X60Qx_16;
  let prv_0 = (result_51 - lastSize_0);
  let prev_1 = prv_0;
  let X60Qx_149;
  let X60Qx_150 = pageIndex_0_sysvq0asl(prev_1);
  let X60Qx_151 = contains_1_sysvq0asl((a_20 + 5248), X60Qx_150);
  if (X60Qx_151) {
    X60Qx_149 = (mem.i32((prev_1 + 4)) === lastSize_0);
  } else {
    X60Qx_149 = false;
  }
  if (X60Qx_149) {
    mem.setI32(result_51, (lastSize_0 | (mem.i32(result_51) & 1)));
  } else {
    mem.setI32(result_51, (0 | (mem.i32(result_51) & 1)));
  }
  mem.setI32((a_20 + 5240), size_36);
  return result_51;
}

function isAccessible_0_sysvq0asl(a_21, p_13) {
  let result_52;
  let X60Qx_152 = pageIndex_1_sysvq0asl(p_13);
  let X60Qx_153 = contains_1_sysvq0asl((a_21 + 5248), X60Qx_152);
  result_52 = X60Qx_153;
  return result_52;
}

function updatePrevSize_0_sysvq0asl(a_22, c_5, prevSize_0) {
  let ri_0 = (c_5 + mem.i32((c_5 + 4)));
  let X60Qx_154 = isAccessible_0_sysvq0asl(a_22, ri_0);
  if (X60Qx_154) {
    mem.setI32(ri_0, (prevSize_0 | (mem.i32(ri_0) & 1)));
  }
}

function splitChunk2_0_sysvq0asl(a_23, c_6, size_16) {
  let result_53;
  result_53 = (c_6 + size_16);
  mem.setI32((result_53 + 4), (mem.i32((c_6 + 4)) - size_16));
  mem.setU32((result_53 + 12), 0);
  mem.setU32((result_53 + 16), 0);
  mem.setI32(result_53, size_16);
  mem.setU32((result_53 + 8), a_23);
  updatePrevSize_0_sysvq0asl(a_23, c_6, mem.i32((result_53 + 4)));
  mem.setI32((c_6 + 4), size_16);
  let X60Qx_155 = pageIndex_0_sysvq0asl(result_53);
  incl_2_sysvq0asl(a_23, X60Qx_155);
  return result_53;
}

function splitChunk_0_sysvq0asl(a_24, c_7, size_17) {
  let rest_0 = splitChunk2_0_sysvq0asl(a_24, c_7, size_17);
  addChunkToMatrix_0_sysvq0asl(a_24, rest_0);
}

function freeBigChunk_0_sysvq0asl(a_25, c_8) {
  let c_28 = c_8;
  inc_0_Iloplki_sysvq0asl((a_25 + 5232), mem.i32((c_28 + 4)));
  mem.setI32(c_28, (mem.i32(c_28) & (~1)));
  let prevSize_1 = mem.i32(c_28);
  if ((!(prevSize_1 === 0))) {
    let le_0 = (c_28 - prevSize_1);
    let X60Qx_156;
    let X60Qx_157 = isAccessible_0_sysvq0asl(a_25, le_0);
    if (X60Qx_157) {
      let X60Qx_158 = chunkUnused_0_sysvq0asl(le_0);
      X60Qx_156 = X60Qx_158;
    } else {
      X60Qx_156 = false;
    }
    if (X60Qx_156) {
      let X60Qx_159;
      let X60Qx_160 = isSmallChunk_0_sysvq0asl(le_0);
      if ((!X60Qx_160)) {
        X60Qx_159 = (mem.i32((le_0 + 4)) < 1056964608);
      } else {
        X60Qx_159 = false;
      }
      if (X60Qx_159) {
        removeChunkFromMatrix_0_sysvq0asl(a_25, le_0);
        inc_0_Iloplki_sysvq0asl((le_0 + 4), mem.i32((c_28 + 4)));
        let X60Qx_161 = pageIndex_0_sysvq0asl(c_28);
        excl_2_sysvq0asl((a_25 + 5248), X60Qx_161);
        c_28 = le_0;
        if ((1056964608 < mem.i32((c_28 + 4)))) {
          let rest_1 = splitChunk2_0_sysvq0asl(a_25, c_28, 1056964608);
          addChunkToMatrix_0_sysvq0asl(a_25, c_28);
          c_28 = rest_1;
        }
      }
    }
  }
  let ri_1 = (c_28 + mem.i32((c_28 + 4)));
  let X60Qx_162;
  let X60Qx_163 = isAccessible_0_sysvq0asl(a_25, ri_1);
  if (X60Qx_163) {
    let X60Qx_164 = chunkUnused_0_sysvq0asl(ri_1);
    X60Qx_162 = X60Qx_164;
  } else {
    X60Qx_162 = false;
  }
  if (X60Qx_162) {
    let X60Qx_165;
    let X60Qx_166 = isSmallChunk_0_sysvq0asl(ri_1);
    if ((!X60Qx_166)) {
      X60Qx_165 = (mem.i32((c_28 + 4)) < 1056964608);
    } else {
      X60Qx_165 = false;
    }
    if (X60Qx_165) {
      removeChunkFromMatrix_0_sysvq0asl(a_25, ri_1);
      inc_0_Iloplki_sysvq0asl((c_28 + 4), mem.i32((ri_1 + 4)));
      let X60Qx_167 = pageIndex_0_sysvq0asl(ri_1);
      excl_2_sysvq0asl((a_25 + 5248), X60Qx_167);
      if ((1056964608 < mem.i32((c_28 + 4)))) {
        let rest_2 = splitChunk2_0_sysvq0asl(a_25, c_28, 1056964608);
        addChunkToMatrix_0_sysvq0asl(a_25, rest_2);
      }
    }
  }
  addChunkToMatrix_0_sysvq0asl(a_25, c_28);
}

function getBigChunk_0_sysvq0asl(a_26, size_18) {
  let result_54;
  let size_37 = allocFixed(4);
  mem.setI32(size_37, size_18);
  let fl_7 = allocFixed(4);
  mem.setI32(fl_7, 0);
  let sl_8 = allocFixed(4);
  mem.setI32(sl_8, 0);
  mappingSearch_0_sysvq0asl(size_37, fl_7, sl_8);
  let X60Qx_168 = findSuitableBlock_0_sysvq0asl(a_26, fl_7, sl_8);
  result_54 = X60Qx_168;
  if ((result_54 === 0)) {
    if ((mem.i32(size_37) < 524288)) {
      let X60Qx_169 = requestOsChunks_0_sysvq0asl(a_26, 524288);
      result_54 = X60Qx_169;
      splitChunk_0_sysvq0asl(a_26, result_54, mem.i32(size_37));
    } else {
      let X60Qx_170 = requestOsChunks_0_sysvq0asl(a_26, mem.i32(size_37));
      result_54 = X60Qx_170;
      if ((mem.i32(size_37) < mem.i32((result_54 + 4)))) {
        splitChunk_0_sysvq0asl(a_26, result_54, mem.i32(size_37));
      }
    }
    mem.setU32((result_54 + 8), a_26);
  } else {
    removeChunkFromMatrix2_0_sysvq0asl(a_26, result_54, mem.i32(fl_7), mem.i32(sl_8));
    if (((mem.i32(size_37) + 4096) <= mem.i32((result_54 + 4)))) {
      splitChunk_0_sysvq0asl(a_26, result_54, mem.i32(size_37));
    }
  }
  mem.setI32(result_54, 1);
  let X60Qx_171 = pageIndex_0_sysvq0asl(result_54);
  incl_2_sysvq0asl(a_26, X60Qx_171);
  dec_0_Ig5i8xp_sysvq0asl((a_26 + 5232), mem.i32(size_37));
  return result_54;
}

function getHugeChunk_0_sysvq0asl(a_27, size_19) {
  let result_55;
  let X60Qx_172 = allocPages_0_sysvq0asl(a_27, size_19);
  result_55 = X60Qx_172;
  incCurrMem_0_sysvq0asl(a_27, size_19);
  mem.setU32((result_55 + 12), 0);
  mem.setU32((result_55 + 16), 0);
  mem.setI32((result_55 + 4), size_19);
  mem.setI32(result_55, 1);
  mem.setU32((result_55 + 8), a_27);
  let X60Qx_173 = pageIndex_0_sysvq0asl(result_55);
  incl_2_sysvq0asl(a_27, X60Qx_173);
  return result_55;
}

function freeHugeChunk_0_sysvq0asl(a_28, c_9) {
  let size_38 = mem.i32((c_9 + 4));
  let X60Qx_174 = pageIndex_0_sysvq0asl(c_9);
  excl_2_sysvq0asl((a_28 + 5248), X60Qx_174);
  decCurrMem_0_sysvq0asl(a_28, size_38);
  osDeallocPages_0_sysvq0asl(c_9, size_38);
}

function getSmallChunk_0_sysvq0asl(a_29) {
  let result_56;
  let res_1 = getBigChunk_0_sysvq0asl(a_29, 4096);
  result_56 = res_1;
  return result_56;
}

function deallocBigChunk_0_sysvq0asl(a_31, c_10) {
  dec_0_Ig5i8xp_sysvq0asl((a_31 + 5236), mem.i32((c_10 + 4)));
  mem.setU32((c_10 + 16), 0);
  if ((1056964609 <= mem.i32((c_10 + 4)))) {
    freeHugeChunk_0_sysvq0asl(a_31, c_10);
  } else {
    freeBigChunk_0_sysvq0asl(a_31, c_10);
  }
}

function addToSharedFreeListBigChunks_0_sysvq0asl(a_32, c_11) {
  whileStmtLabel_0: {
    {
      while (true) {
        var X60Qx_175 = __atomic_load_n((a_32 + 5244), __ATOMIC_RELAXED);
        __atomic_store_n((c_11 + 12), X60Qx_175, __ATOMIC_RELAXED);
        var X60Qx_176 = __atomic_compare_exchange_n((a_32 + 5244), (c_11 + 12), c_11, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
        if (X60Qx_176) {
          break whileStmtLabel_0;
        }
      }
    }
  }
}

function addToSharedFreeList_0_sysvq0asl(c_12, f_0, size_20) {
  whileStmtLabel_0: {
    {
      while (true) {
        var X60Qx_177 = nimIcheckB(size_20, 255);
        var X60Qx_178 = __atomic_load_n(((mem.u32((c_12 + 8)) + 1024) + (X60Qx_177 * 4)), __ATOMIC_RELAXED);
        __atomic_store_n(f_0, X60Qx_178, __ATOMIC_RELAXED);
        var X60Qx_179 = nimIcheckB(size_20, 255);
        var X60Qx_180 = __atomic_compare_exchange_n(((mem.u32((c_12 + 8)) + 1024) + (X60Qx_179 * 4)), f_0, f_0, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
        if (X60Qx_180) {
          break whileStmtLabel_0;
        }
      }
    }
  }
}

function compensateCounters_0_sysvq0asl(a_33, c_13, size_21) {
  whileStmtLabel_0: {
    var it_3 = mem.u32((c_13 + 20));
    var total_0 = allocFixed(4);
    mem.setI32(total_0, 0);
    {
      while ((!(it_3 === 0))) {
        inc_0_Iloplki_sysvq0asl(total_0, size_21);
        var X60Qx_181 = pageAddr_0_sysvq0asl(it_3);
        var chunk_0 = X60Qx_181;
        if ((!(c_13 === chunk_0))) {
          mem.setI32((c_13 + 32), (mem.i32((c_13 + 32)) + 1));
        }
        it_3 = mem.u32(it_3);
      }
    }
  }
  mem.setI32((c_13 + 24), (mem.i32((c_13 + 24)) + mem.i32(total_0)));
  dec_0_Ig5i8xp_sysvq0asl((a_33 + 5236), mem.i32(total_0));
}

function freeDeferredObjects_0_sysvq0asl(a_34, root_0) {
  whileStmtLabel_0: {
    var it_4 = root_0;
    var maxIters_0 = allocFixed(4);
    mem.setI32(maxIters_0, 20);
    {
      while (true) {
        var rest_3 = __atomic_load_n((it_4 + 12), __ATOMIC_RELAXED);
        __atomic_store_n((it_4 + 12), 0, __ATOMIC_RELAXED);
        deallocBigChunk_0_sysvq0asl(a_34, it_4);
        if ((mem.i32(maxIters_0) === 0)) {
          if ((!(rest_3 === 0))) {
            addToSharedFreeListBigChunks_0_sysvq0asl(a_34, rest_3);
          }
          break whileStmtLabel_0;
        }
        it_4 = rest_3;
        dec_1_I0nzoz91_sysvq0asl(maxIters_0);
        if ((it_4 === 0)) {
          break whileStmtLabel_0;
        }
      }
    }
  }
}

function smallChunkAlignOffset_0_sysvq0asl(alignment_1) {
  let result_57;
  if ((alignment_1 <= 16)) {
    result_57 = 0;
  } else {
    let X60Qx_182 = align_0_sysvq0asl(48, alignment_1);
    result_57 = ((X60Qx_182 - 40) - 8);
  }
  return result_57;
}

function bigChunkAlignOffset_0_sysvq0asl(alignment_2) {
  let result_58;
  if ((alignment_2 === 0)) {
    result_58 = 0;
  } else {
    let X60Qx_183 = align_0_sysvq0asl(28, alignment_2);
    result_58 = ((X60Qx_183 - 20) - 8);
  }
  return result_58;
}

function rawAlloc_0_sysvq0asl(a_35, requestedSize_0, alignment_3) {
  let result_59;
  let X60Qx_184 = max_2_sysvq0asl(16, alignment_3);
  let size_39 = roundup_0_sysvq0asl(requestedSize_0, X60Qx_184);
  let alignOff_0 = smallChunkAlignOffset_0_sysvq0asl(alignment_3);
  if (((size_39 + alignOff_0) <= 4056)) {
    let s_82 = Math.trunc((size_39 / 16));
    let X60Qx_185 = nimIcheckB(s_82, 255);
    let c_29 = mem.u32((a_35 + (X60Qx_185 * 4)));
    let X60Qx_186;
    if ((!(c_29 === 0))) {
      X60Qx_186 = (!(mem.i32((c_29 + 36)) === alignOff_0));
    } else {
      X60Qx_186 = false;
    }
    if (X60Qx_186) {
      c_29 = 0;
    }
    if ((c_29 === 0)) {
      let X60Qx_187 = getSmallChunk_0_sysvq0asl(a_35);
      c_29 = X60Qx_187;
      mem.setU32((c_29 + 20), 0);
      mem.setI32((c_29 + 32), 0);
      mem.setI32((c_29 + 36), alignOff_0);
      mem.setI32((c_29 + 4), size_39);
      mem.setU32((c_29 + 28), (alignOff_0 + size_39));
      mem.setI32((c_29 + 24), ((4056 - alignOff_0) - size_39));
      mem.setU32((c_29 + 12), 0);
      mem.setU32((c_29 + 16), 0);
      if ((mem.u32((c_29 + 20)) === 0)) {
        let X60Qx_188 = nimIcheckB(s_82, 255);
        let X60Qx_189 = __atomic_exchange_n(((a_35 + 1024) + (X60Qx_188 * 4)), 0, __ATOMIC_RELAXED);
        mem.setU32((c_29 + 20), X60Qx_189);
        compensateCounters_0_sysvq0asl(a_35, c_29, size_39);
      }
      if ((size_39 <= mem.i32((c_29 + 24)))) {
        let X60Qx_190 = nimIcheckB(s_82, 255);
        listAdd_0_Ik4wxhz_sysvq0asl((a_35 + (X60Qx_190 * 4)), c_29);
      }
      result_59 = ((c_29 + 40) + alignOff_0);
    } else {
      if ((mem.u32((c_29 + 20)) === 0)) {
        result_59 = ((c_29 + 40) + mem.u32((c_29 + 28)));
        mem.setU32((c_29 + 28), (mem.u32((c_29 + 28)) + size_39));
      } else {
        result_59 = mem.u32((c_29 + 20));
        mem.setU32((c_29 + 20), mem.u32(mem.u32((c_29 + 20))));
        let X60Qx_191 = pageAddr_0_sysvq0asl(result_59);
        if ((!(X60Qx_191 === c_29))) {
          mem.setI32((c_29 + 32), (mem.i32((c_29 + 32)) - 1));
        } else {
        }
      }
      mem.setI32((c_29 + 24), (mem.i32((c_29 + 24)) - size_39));
      if ((mem.u32((c_29 + 20)) === 0)) {
        let X60Qx_192 = nimIcheckB(s_82, 255);
        let X60Qx_193 = __atomic_exchange_n(((a_35 + 1024) + (X60Qx_192 * 4)), 0, __ATOMIC_RELAXED);
        mem.setU32((c_29 + 20), X60Qx_193);
        compensateCounters_0_sysvq0asl(a_35, c_29, size_39);
      }
      if ((mem.i32((c_29 + 24)) < size_39)) {
        let X60Qx_194 = nimIcheckB(s_82, 255);
        listRemove_0_Ibzev091_sysvq0asl((a_35 + (X60Qx_194 * 4)), c_29);
      }
    }
    inc_0_Iloplki_sysvq0asl((a_35 + 5236), size_39);
  } else {
    let deferredFrees_0 = __atomic_exchange_n((a_35 + 5244), 0, __ATOMIC_RELAXED);
    if ((!(deferredFrees_0 === 0))) {
      freeDeferredObjects_0_sysvq0asl(a_35, deferredFrees_0);
    }
    let alignPad_0 = bigChunkAlignOffset_0_sysvq0asl(alignment_3);
    size_39 = ((requestedSize_0 + 20) + alignPad_0);
    let X60Qx_17;
    if ((1056964609 <= size_39)) {
      let X60Qx_195 = getHugeChunk_0_sysvq0asl(a_35, size_39);
      X60Qx_17 = X60Qx_195;
    } else {
      let X60Qx_196 = getBigChunk_0_sysvq0asl(a_35, size_39);
      X60Qx_17 = X60Qx_196;
    }
    let c_32 = X60Qx_17;
    result_59 = ((c_32 + 20) + alignPad_0);
    mem.setU32((c_32 + 16), result_59);
    inc_0_Iloplki_sysvq0asl((a_35 + 5236), mem.i32((c_32 + 4)));
  }
  return result_59;
}

function rawDealloc_0_sysvq0asl(a_37, p_14) {
  let c_33 = pageAddr_0_sysvq0asl(p_14);
  let X60Qx_198 = isSmallChunk_0_sysvq0asl(c_33);
  if (X60Qx_198) {
    let c_34 = c_33;
    let s_83 = mem.i32((c_34 + 4));
    let f_3 = p_14;
    if ((mem.u32((c_34 + 8)) === a_37)) {
      dec_0_Ig5i8xp_sysvq0asl((a_37 + 5236), s_83);
      let X60Qx_199 = nimIcheckB(Math.trunc((s_83 / 16)), 255);
      let activeChunk_0 = mem.u32((a_37 + (X60Qx_199 * 4)));
      let X60Qx_200;
      let X60Qx_201;
      if ((!(activeChunk_0 === 0))) {
        X60Qx_201 = (!(c_34 === activeChunk_0));
      } else {
        X60Qx_201 = false;
      }
      if (X60Qx_201) {
        X60Qx_200 = (mem.i32((activeChunk_0 + 36)) === mem.i32((c_34 + 36)));
      } else {
        X60Qx_200 = false;
      }
      if (X60Qx_200) {
        mem.setU32(f_3, mem.u32((activeChunk_0 + 20)));
        mem.setU32((activeChunk_0 + 20), f_3);
        mem.setI32((activeChunk_0 + 24), (mem.i32((activeChunk_0 + 24)) + s_83));
        mem.setI32((activeChunk_0 + 32), (mem.i32((activeChunk_0 + 32)) + 1));
      } else {
        mem.setU32(f_3, mem.u32((c_34 + 20)));
        mem.setU32((c_34 + 20), f_3);
        if ((mem.i32((c_34 + 24)) < s_83)) {
          let X60Qx_202 = nimIcheckB(Math.trunc((s_83 / 16)), 255);
          listAdd_0_Ik4wxhz_sysvq0asl((a_37 + (X60Qx_202 * 4)), c_34);
          mem.setI32((c_34 + 24), (mem.i32((c_34 + 24)) + s_83));
        } else {
          mem.setI32((c_34 + 24), (mem.i32((c_34 + 24)) + s_83));
        }
      }
    } else {
      addToSharedFreeList_0_sysvq0asl(c_34, f_3, Math.trunc((s_83 / 16)));
    }
  } else {
    if ((mem.u32((c_33 + 8)) === a_37)) {
      deallocBigChunk_0_sysvq0asl(a_37, c_33);
    } else {
      addToSharedFreeListBigChunks_0_sysvq0asl(mem.u32((c_33 + 8)), c_33);
    }
  }
}

function alloc_0_sysvq0asl(allocator_0, size_22) {
  let result_62;
  let X60Qx_204 = rawAlloc_0_sysvq0asl(allocator_0, size_22, 0);
  result_62 = X60Qx_204;
  return result_62;
}

function dealloc_0_sysvq0asl(allocator_2, p_16) {
  rawDealloc_0_sysvq0asl(allocator_2, p_16);
}

let allocator_0_sysvq0asl = allocFixed(6528);

function alloc_1_sysvq0asl(size_24) {
  let result_69;
  let X60Qx_211 = alloc_0_sysvq0asl(allocator_0_sysvq0asl, size_24);
  result_69 = X60Qx_211;
  return result_69;
}

function dealloc_1_sysvq0asl(p_20) {
  dealloc_0_sysvq0asl(allocator_0_sysvq0asl, p_20);
}

let missingBytes_0_sysvq0asl = allocFixed(4);

function continueAfterOutOfMem_0_sysvq0asl(size_28) {
  if ((mem.i32(missingBytes_0_sysvq0asl) < (2147483647 - size_28))) {
    mem.setI32(missingBytes_0_sysvq0asl, (mem.i32(missingBytes_0_sysvq0asl) + size_28));
  } else {
    mem.setI32(missingBytes_0_sysvq0asl, 2147483647);
  }
}

let oomHandler_0_sysvq0asl = allocFixed(4);

mem.setU32(oomHandler_0_sysvq0asl, _fnid(continueAfterOutOfMem_0_sysvq0asl));

function ssLenOf_0_sysvq0asl(bytes_2) {
  let result_95;
  result_95 = ((bytes_2 & 255) >>> 0);
  return result_95;
}

function rawData_1_sysvq0asl(s_33) {
  let result_96;
  if ((6 < mem.u8At(s_33))) {
    result_96 = (mem.u32((s_33 + 4)) + 12);
  } else {
    result_96 = (s_33 + 1);
  }
  return result_96;
}

function len_4_sysvq0asl(s_34) {
  let result_98;
  result_98 = mem.u8At(s_34);
  if ((6 < result_98)) {
    result_98 = mem.i32(mem.u32((s_34 + 4)));
  }
  return result_98;
}

function readRawData_0_sysvq0asl(s_39, start_0) {
  let result_103;
  if ((6 < mem.u8At(s_39))) {
    result_103 = ((mem.u32((s_39 + 4)) + 12) + start_0);
  } else {
    result_103 = ((s_39 + 1) + start_0);
  }
  return result_103;
}

function nimStrWasMoved(s_40) {
  mem.setU32(s_40, 0);
}

function nimStrDestroy(s_41) {
  if ((mem.u8At(s_41) === 255)) {
    let X60Qx_221 = arcDec_0_sysvq0asl((mem.u32((s_41 + 4)) + 4));
    if (X60Qx_221) {
      dealloc_1_sysvq0asl(mem.u32((s_41 + 4)));
    }
  }
}

function nimStrCopy(dest_11, src_6) {
  let ssrc_0 = mem.u8At(src_6);
  if ((ssrc_0 <= 6)) {
    let sdest_0 = mem.u8At(dest_11);
    if ((sdest_0 === 255)) {
      let X60Qx_222 = arcDec_0_sysvq0asl((mem.u32((dest_11 + 4)) + 4));
      if (X60Qx_222) {
        dealloc_1_sysvq0asl(mem.u32((dest_11 + 4)));
      }
    }
    copyMem_0_sysvq0asl(dest_11, src_6, 8);
  } else {
    if ((dest_11 === src_6)) {
      return;
    }
    let sdest_1 = mem.u8At(dest_11);
    if ((sdest_1 === 255)) {
      let X60Qx_223 = arcDec_0_sysvq0asl((mem.u32((dest_11 + 4)) + 4));
      if (X60Qx_223) {
        dealloc_1_sysvq0asl(mem.u32((dest_11 + 4)));
      }
    }
    if ((ssrc_0 === 255)) {
      arcInc_0_sysvq0asl((mem.u32((src_6 + 4)) + 4));
    }
    copyMem_0_sysvq0asl(dest_11, src_6, 8);
  }
}

function nimStrDup(s_42) {
  let result_104 = allocFixed(8);
  let X60Qx_224 = ssLenOf_0_sysvq0asl(mem.u32(s_42));
  if ((X60Qx_224 === 255)) {
    arcInc_0_sysvq0asl((mem.u32((s_42 + 4)) + 4));
  }
  mem.copy(result_104, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, mem.u32(s_42));
    mem.setU32((_o + 4), mem.u32((s_42 + 4)));
    return _o;
  })(), 8);
  return result_104;
}

function len_5_sysvq0asl(a_46) {
  let result_105;
  let X60Qx_19;
  if ((a_46 === 0)) {
    X60Qx_19 = 0;
  } else {
    let X60Qx_225 = strlen(a_46);
    X60Qx_19 = X60Qx_225;
  }
  result_105 = X60Qx_19;
  return result_105;
}

function ssResize_0_sysvq0asl(old_0) {
  let result_106;
  let X60Qx_20;
  if ((old_0 <= 0)) {
    X60Qx_20 = 4;
  } else {
    if ((old_0 <= 32767)) {
      X60Qx_20 = (old_0 * 2);
    } else {
      X60Qx_20 = (Math.trunc((old_0 / 2)) + old_0);
    }
  }
  result_106 = X60Qx_20;
  return result_106;
}

function ensureUniqueLong_0_sysvq0asl(s_43, oldLen_0, newLen_5) {
  let sl_10 = mem.u8At(s_43);
  let isHeap_0 = (sl_10 === 255);
  let X60Qx_21;
  if (isHeap_0) {
    X60Qx_21 = mem.i32((mem.u32((s_43 + 4)) + 8));
  } else {
    X60Qx_21 = 0;
  }
  let cap_1 = X60Qx_21;
  let X60Qx_226;
  let X60Qx_227;
  if (isHeap_0) {
    let X60Qx_228 = arcIsUnique_0_sysvq0asl((mem.u32((s_43 + 4)) + 4));
    X60Qx_227 = X60Qx_228;
  } else {
    X60Qx_227 = false;
  }
  if (X60Qx_227) {
    X60Qx_226 = (newLen_5 <= cap_1);
  } else {
    X60Qx_226 = false;
  }
  if (X60Qx_226) {
    mem.setI32(mem.u32((s_43 + 4)), newLen_5);
    let X60Qx_229 = min_2_sysvq0asl(oldLen_0, 3);
    copyMem_0_sysvq0asl((s_43 + 1), (mem.u32((s_43 + 4)) + 12), X60Qx_229);
  } else {
    let X60Qx_22;
    if ((cap_1 < newLen_5)) {
      let X60Qx_230 = ssResize_0_sysvq0asl(cap_1);
      let X60Qx_231 = max_2_sysvq0asl(newLen_5, X60Qx_230);
      X60Qx_22 = X60Qx_231;
    } else {
      X60Qx_22 = cap_1;
    }
    let newCap_2 = X60Qx_22;
    let X60Qx_232 = alloc_1_sysvq0asl((12 + newCap_2));
    let p_35 = X60Qx_232;
    if ((!(p_35 === 0))) {
      mem.setI32((p_35 + 4), 0);
      mem.setI32(p_35, newLen_5);
      mem.setI32((p_35 + 8), newCap_2);
      if (isHeap_0) {
        let old_2 = mem.u32((s_43 + 4));
        let X60Qx_233 = min_2_sysvq0asl(oldLen_0, newCap_2);
        copyMem_0_sysvq0asl((p_35 + 12), (old_2 + 12), X60Qx_233);
        let X60Qx_234 = arcDec_0_sysvq0asl((old_2 + 4));
        if (X60Qx_234) {
          dealloc_1_sysvq0asl(old_2);
        }
      } else {
        let X60Qx_235 = min_2_sysvq0asl(oldLen_0, newCap_2);
        copyMem_0_sysvq0asl((p_35 + 12), (mem.u32((s_43 + 4)) + 12), X60Qx_235);
      }
      mem.setU32((s_43 + 4), p_35);
      mem.setU8(s_43, (255 & 255));
      let X60Qx_236 = min_2_sysvq0asl(oldLen_0, 3);
      copyMem_0_sysvq0asl((s_43 + 1), (p_35 + 12), X60Qx_236);
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + newCap_2));
      mem.setU32(s_43, 21760775509248519n);
      mem.setU32((s_43 + 4), 0);
    }
  }
}

function transitionToLong_0_sysvq0asl(s_44, sl_4, newLen_6) {
  let X60Qx_237 = ssResize_0_sysvq0asl(newLen_6);
  let newCap_3 = max_2_sysvq0asl(newLen_6, X60Qx_237);
  let X60Qx_238 = alloc_1_sysvq0asl((12 + newCap_3));
  let p_36 = X60Qx_238;
  if ((!(p_36 === 0))) {
    mem.setI32((p_36 + 4), 0);
    mem.setI32(p_36, newLen_6);
    mem.setI32((p_36 + 8), newCap_3);
    copyMem_0_sysvq0asl((p_36 + 12), (s_44 + 1), sl_4);
    mem.setU32((s_44 + 4), p_36);
    mem.setU8(s_44, (255 & 255));
    let X60Qx_239 = min_2_sysvq0asl(sl_4, 3);
    copyMem_0_sysvq0asl((s_44 + 1), (p_36 + 12), X60Qx_239);
  } else {
    _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + newCap_3));
    mem.setU32(s_44, 21760775509248519n);
    mem.setU32((s_44 + 4), 0);
  }
}

function prepareMutation_0_sysvq0asl(s_46) {
  let sl_12 = mem.u8At(s_46);
  let X60Qx_242;
  if ((sl_12 === 254)) {
    X60Qx_242 = true;
  } else {
    let X60Qx_243;
    if ((sl_12 === 255)) {
      let X60Qx_244 = arcIsUnique_0_sysvq0asl((mem.u32((s_46 + 4)) + 4));
      X60Qx_243 = (!X60Qx_244);
    } else {
      X60Qx_243 = false;
    }
    X60Qx_242 = X60Qx_243;
  }
  if (X60Qx_242) {
    if ((sl_12 === 255)) {
      let X60Qx_245 = arcDec_0_sysvq0asl((mem.u32((s_46 + 4)) + 4));
    }
    let old_3 = mem.u32((s_46 + 4));
    let oldLen_1 = mem.i32(old_3);
    let X60Qx_246 = alloc_1_sysvq0asl((12 + oldLen_1));
    let p_37 = X60Qx_246;
    if ((!(p_37 === 0))) {
      mem.setI32((p_37 + 4), 0);
      mem.setI32(p_37, oldLen_1);
      mem.setI32((p_37 + 8), oldLen_1);
      copyMem_0_sysvq0asl((p_37 + 12), (old_3 + 12), oldLen_1);
      mem.setU32((s_46 + 4), p_37);
      mem.setU8(s_46, (255 & 255));
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + oldLen_1));
      mem.setU32(s_46, 21760775509248519n);
      mem.setU32((s_46 + 4), 0);
    }
  }
}

function add_1_sysvq0asl(s_49, c_14) {
  let sl_14 = mem.u8At(s_49);
  if ((sl_14 < 6)) {
    let newLen_14 = (sl_14 + 1);
    mem.setU8(((s_49 + 1) + sl_14), c_14);
    mem.setU8(s_49, (newLen_14 & 255));
  } else {
    if ((6 < sl_14)) {
      let l_1 = mem.i32(mem.u32((s_49 + 4)));
      let X60Qx_248;
      let X60Qx_249;
      if ((sl_14 === 255)) {
        let X60Qx_250 = arcIsUnique_0_sysvq0asl((mem.u32((s_49 + 4)) + 4));
        X60Qx_249 = X60Qx_250;
      } else {
        X60Qx_249 = false;
      }
      if (X60Qx_249) {
        X60Qx_248 = (l_1 < mem.i32((mem.u32((s_49 + 4)) + 8)));
      } else {
        X60Qx_248 = false;
      }
      if (X60Qx_248) {
        mem.setU8(((mem.u32((s_49 + 4)) + 12) + l_1), c_14);
        mem.setI32(mem.u32((s_49 + 4)), (l_1 + 1));
        if ((l_1 < 3)) {
          mem.setU8(((s_49 + 1) + l_1), c_14);
        }
      } else {
        let oldLen_2 = mem.i32(mem.u32((s_49 + 4)));
        ensureUniqueLong_0_sysvq0asl(s_49, oldLen_2, (oldLen_2 + 1));
        if ((mem.u8At(s_49) === 255)) {
          mem.setU8(((mem.u32((s_49 + 4)) + 12) + oldLen_2), c_14);
          if ((oldLen_2 < 3)) {
            mem.setU8(((s_49 + 1) + oldLen_2), c_14);
          }
        }
      }
    } else {
      transitionToLong_0_sysvq0asl(s_49, sl_14, (sl_14 + 1));
      if ((mem.u8At(s_49) === 255)) {
        mem.setU8(((mem.u32((s_49 + 4)) + 12) + sl_14), c_14);
      }
    }
  }
}

function add_2_sysvq0asl(s_50, part_0) {
  let partLen_0 = len_4_sysvq0asl(part_0);
  if ((partLen_0 === 0)) {
    return;
  }
  let partData_0 = rawData_1_sysvq0asl(part_0);
  let sl_15 = mem.u8At(s_50);
  if ((sl_15 <= 6)) {
    let sLen_0 = sl_15;
    let newLen_15 = (sLen_0 + partLen_0);
    if ((newLen_15 <= 6)) {
      copyMem_0_sysvq0asl(((s_50 + 1) + sLen_0), partData_0, partLen_0);
      mem.setU8(s_50, (newLen_15 & 255));
    } else {
      transitionToLong_0_sysvq0asl(s_50, sLen_0, newLen_15);
      if ((mem.u8At(s_50) === 255)) {
        copyMem_0_sysvq0asl(((mem.u32((s_50 + 4)) + 12) + sLen_0), partData_0, partLen_0);
        copyMem_0_sysvq0asl((s_50 + 1), (mem.u32((s_50 + 4)) + 12), 3);
      }
    }
  } else {
    let sLen_1 = mem.i32(mem.u32((s_50 + 4)));
    let newLen_16 = (sLen_1 + partLen_0);
    ensureUniqueLong_0_sysvq0asl(s_50, sLen_1, newLen_16);
    if ((mem.u8At(s_50) === 255)) {
      copyMem_0_sysvq0asl(((mem.u32((s_50 + 4)) + 12) + sLen_1), partData_0, partLen_0);
      if ((sLen_1 < 3)) {
        copyMem_0_sysvq0asl((s_50 + 1), (mem.u32((s_50 + 4)) + 12), 3);
      }
    }
  }
}

function zeroSwarPadImplLE_0_sysvq0asl(bytes_4, newLen_9) {
  let result_110;
  let keepBits_0 = ((newLen_9 + 1) * 8);
  let X60Qx_25;
  if ((32 <= keepBits_0)) {
    X60Qx_25 = ((~0) >>> 0);
  } else {
    X60Qx_25 = (((1 << keepBits_0) >>> 0) - 1);
  }
  let mask_0 = X60Qx_25;
  result_110 = ((((bytes_4 & ((mask_0 & ((~255) >>> 0)) >>> 0)) >>> 0) | newLen_9) >>> 0);
  return result_110;
}

function zeroSwarPadImpl_0_sysvq0asl(bytes_5, newLen_10) {
  let result_111;
  let X60Qx_251 = zeroSwarPadImplLE_0_sysvq0asl(bytes_5, newLen_10);
  result_111 = X60Qx_251;
  return result_111;
}

function shrink_1_sysvq0asl(s_52, newLen_12) {
  let X60Qx_252 = len_4_sysvq0asl(s_52);
  if ((newLen_12 <= X60Qx_252)) {
    let sl_16 = mem.u8At(s_52);
    if ((sl_16 <= 6)) {
      if ((newLen_12 <= 3)) {
        let X60Qx_253 = zeroSwarPadImpl_0_sysvq0asl(mem.u32(s_52), newLen_12);
        mem.setU32(s_52, X60Qx_253);
      } else {
        mem.setU8(s_52, (newLen_12 & 255));
      }
    } else {
      prepareMutation_0_sysvq0asl(s_52);
      mem.setI32(mem.u32((s_52 + 4)), newLen_12);
      let X60Qx_254 = min_2_sysvq0asl(newLen_12, 3);
      copyMem_0_sysvq0asl((s_52 + 1), (mem.u32((s_52 + 4)) + 12), X60Qx_254);
    }
  }
}

function getQ_9_sysvq0asl(s_54, i_14) {
  let X60Qx_257;
  let X60Qx_258 = len_4_sysvq0asl(s_54);
  if ((i_14 < X60Qx_258)) {
    X60Qx_257 = (0 <= i_14);
  } else {
    X60Qx_257 = false;
  }
  if ((!X60Qx_257)) {
    panic_0_sysvq0asl((() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 791555838);
      mem.setU32((_o + 4), strlit_0_I1565838944098044620_sysvq0asl);
      return _o;
    })());
  }
  let result_112;
  let X60Qx_26;
  if ((6 < mem.u8At(s_54))) {
    X60Qx_26 = mem.u8At(((mem.u32((s_54 + 4)) + 12) + i_14));
  } else {
    X60Qx_26 = mem.u8At(((s_54 + 1) + i_14));
  }
  result_112 = X60Qx_26;
  return result_112;
}

function putQ_9_sysvq0asl(s_55, i_15, c_15) {
  let X60Qx_259;
  let X60Qx_260 = len_4_sysvq0asl(s_55);
  if ((i_15 < X60Qx_260)) {
    X60Qx_259 = (0 <= i_15);
  } else {
    X60Qx_259 = false;
  }
  if ((!X60Qx_259)) {
    panic_0_sysvq0asl((() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 791555838);
      mem.setU32((_o + 4), strlit_0_I7364560965974357967_sysvq0asl);
      return _o;
    })());
  }
  prepareMutation_0_sysvq0asl(s_55);
  if ((6 < mem.u8At(s_55))) {
    mem.setU8(((mem.u32((s_55 + 4)) + 12) + i_15), c_15);
    if ((i_15 < 3)) {
      mem.setU8(((s_55 + 1) + i_15), c_15);
    }
  } else {
    mem.setU8(((s_55 + 1) + i_15), c_15);
  }
}

function substr_0_sysvq0asl(s_56, first_0, last_0) {
  let result_113 = allocFixed(8);
  nimStrWasMoved(result_113);
  nimStrDestroy(result_113);
  mem.copy(result_113, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  let sLen_2 = len_4_sysvq0asl(s_56);
  let f_4 = max_2_sysvq0asl(first_0, 0);
  let X60Qx_261 = min_2_sysvq0asl(last_0, (sLen_2 - 1));
  let l_2 = (X60Qx_261 + 1);
  if ((l_2 <= f_4)) {
    return result_113;
  }
  let newLen_17 = (l_2 - f_4);
  let src_7 = rawData_1_sysvq0asl(s_56);
  if ((newLen_17 <= 6)) {
    mem.setU8(result_113, (newLen_17 & 255));
    copyMem_0_sysvq0asl((result_113 + 1), (src_7 + f_4), newLen_17);
  } else {
    let X60Qx_262 = alloc_1_sysvq0asl((12 + newLen_17));
    let p_38 = X60Qx_262;
    if ((!(p_38 === 0))) {
      mem.setI32((p_38 + 4), 0);
      mem.setI32(p_38, newLen_17);
      mem.setI32((p_38 + 8), newLen_17);
      copyMem_0_sysvq0asl((p_38 + 12), (src_7 + f_4), newLen_17);
      mem.setU32((result_113 + 4), p_38);
      mem.setU8(result_113, (255 & 255));
      copyMem_0_sysvq0asl((result_113 + 1), (p_38 + 12), 3);
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + newLen_17));
      mem.setU32(result_113, 21760775509248519n);
      mem.setU32((result_113 + 4), 0);
    }
  }
  return result_113;
}

function newString_0_sysvq0asl(len_4) {
  let result_132 = allocFixed(8);
  nimStrWasMoved(result_132);
  nimStrDestroy(result_132);
  mem.copy(result_132, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  if ((len_4 <= 0)) {
    return result_132;
  }
  if ((len_4 <= 6)) {
    mem.setU8(result_132, (len_4 & 255));
    zeroMem_0_sysvq0asl((result_132 + 1), len_4);
  } else {
    let X60Qx_294 = alloc_1_sysvq0asl((12 + len_4));
    let p_39 = X60Qx_294;
    if ((!(p_39 === 0))) {
      zeroMem_0_sysvq0asl(p_39, (12 + len_4));
      mem.setI32((p_39 + 4), 0);
      mem.setI32(p_39, len_4);
      mem.setI32((p_39 + 8), len_4);
      mem.setU32((result_132 + 4), p_39);
      mem.setU8(result_132, (255 & 255));
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + len_4));
      mem.setU32(result_132, 21760775509248519n);
      mem.setU32((result_132 + 4), 0);
    }
  }
  return result_132;
}

function newStringOfCap_0_sysvq0asl(len_5) {
  let result_133 = allocFixed(8);
  nimStrWasMoved(result_133);
  nimStrDestroy(result_133);
  mem.copy(result_133, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  if ((len_5 <= 6)) {
    return result_133;
  }
  let X60Qx_295 = alloc_1_sysvq0asl((12 + len_5));
  let p_40 = X60Qx_295;
  if ((!(p_40 === 0))) {
    zeroMem_0_sysvq0asl(p_40, (12 + len_5));
    mem.setI32((p_40 + 4), 0);
    mem.setI32(p_40, 0);
    mem.setI32((p_40 + 8), len_5);
    mem.setU32((result_133 + 4), p_40);
    mem.setU8(result_133, (255 & 255));
  } else {
    _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + len_5));
    mem.setU32(result_133, 21760775509248519n);
    mem.setU32((result_133 + 4), 0);
  }
  return result_133;
}

function ampQ_0_sysvq0asl(a_54, b_17) {
  let result_134 = allocFixed(8);
  nimStrWasMoved(result_134);
  nimStrDestroy(result_134);
  mem.copy(result_134, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  let X60Qx_296 = len_4_sysvq0asl(a_54);
  let X60Qx_297 = len_4_sysvq0asl(b_17);
  let rlen_0 = (X60Qx_296 + X60Qx_297);
  if ((rlen_0 === 0)) {
    return result_134;
  }
  if ((rlen_0 <= 6)) {
    let al_0 = len_4_sysvq0asl(a_54);
    mem.setU8(result_134, (rlen_0 & 255));
    if ((0 < al_0)) {
      let X60Qx_298 = rawData_1_sysvq0asl(a_54);
      copyMem_0_sysvq0asl((result_134 + 1), X60Qx_298, al_0);
    }
    let X60Qx_299 = len_4_sysvq0asl(b_17);
    if ((0 < X60Qx_299)) {
      let X60Qx_300 = rawData_1_sysvq0asl(b_17);
      let X60Qx_301 = len_4_sysvq0asl(b_17);
      copyMem_0_sysvq0asl(((result_134 + 1) + al_0), X60Qx_300, X60Qx_301);
    }
  } else {
    let X60Qx_302 = alloc_1_sysvq0asl((12 + rlen_0));
    let p_41 = X60Qx_302;
    if ((!(p_41 === 0))) {
      mem.setI32((p_41 + 4), 0);
      mem.setI32(p_41, rlen_0);
      mem.setI32((p_41 + 8), rlen_0);
      let al_1 = len_4_sysvq0asl(a_54);
      if ((0 < al_1)) {
        let X60Qx_303 = rawData_1_sysvq0asl(a_54);
        copyMem_0_sysvq0asl((p_41 + 12), X60Qx_303, al_1);
      }
      let X60Qx_304 = len_4_sysvq0asl(b_17);
      if ((0 < X60Qx_304)) {
        let X60Qx_305 = rawData_1_sysvq0asl(b_17);
        let X60Qx_306 = len_4_sysvq0asl(b_17);
        copyMem_0_sysvq0asl(((p_41 + 12) + al_1), X60Qx_305, X60Qx_306);
      }
      mem.setU32((result_134 + 4), p_41);
      mem.setU8(result_134, (255 & 255));
      copyMem_0_sysvq0asl((result_134 + 1), (p_41 + 12), 3);
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + rlen_0));
      mem.setU32(result_134, 21760775509248519n);
      mem.setU32((result_134 + 4), 0);
    }
  }
  return result_134;
}

function borrowCStringUnsafe_0_sysvq0asl(s_61, l_0) {
  let result_139 = allocFixed(8);
  nimStrWasMoved(result_139);
  nimStrDestroy(result_139);
  mem.copy(result_139, (() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 0);
    mem.setU32((_o + 4), 0);
    return _o;
  })(), 8);
  if ((l_0 <= 0)) {
    return result_139;
  }
  if ((l_0 <= 6)) {
    mem.setU8(result_139, (l_0 & 255));
    copyMem_0_sysvq0asl((result_139 + 1), s_61, l_0);
  } else {
    let X60Qx_311 = alloc_1_sysvq0asl((12 + l_0));
    let p_42 = X60Qx_311;
    if ((!(p_42 === 0))) {
      mem.setI32((p_42 + 4), 0);
      mem.setI32(p_42, l_0);
      mem.setI32((p_42 + 8), l_0);
      copyMem_0_sysvq0asl((p_42 + 12), s_61, l_0);
      mem.setU32((result_139 + 4), p_42);
      mem.setU8(result_139, (255 & 255));
      copyMem_0_sysvq0asl((result_139 + 1), (p_42 + 12), 3);
    } else {
      _fns[mem.u32(oomHandler_0_sysvq0asl)]((12 + l_0));
      mem.setU32(result_139, 21760775509248519n);
      mem.setU32((result_139 + 4), 0);
    }
  }
  return result_139;
}

function nimBorrowCStringUnsafe(s_62) {
  let result_140 = allocFixed(8);
  nimStrWasMoved(result_140);
  nimStrDestroy(result_140);
  let X60Qx_312 = len_5_sysvq0asl(s_62);
  let X60Qx_313 = allocFixed(8);
  mem.copy(X60Qx_313, borrowCStringUnsafe_0_sysvq0asl(s_62, X60Qx_312), 8);
  mem.copy(result_140, X60Qx_313, 8);
  return result_140;
}

function ensureTerminatingZero_0_sysvq0asl(s_63) {
  let oldLen_3 = len_4_sysvq0asl(s_63);
  add_1_sysvq0asl(s_63, 0);
  shrink_1_sysvq0asl(s_63, oldLen_3);
}

function toCString_0_sysvq0asl(s_64) {
  let result_141;
  ensureTerminatingZero_0_sysvq0asl(s_64);
  let X60Qx_314 = rawData_1_sysvq0asl(s_64);
  result_141 = X60Qx_314;
  return result_141;
}

function arcInc_0_sysvq0asl(memLoc_0) {
  let X60Qx_318 = __atomic_add_fetch(memLoc_0, 1, __ATOMIC_SEQ_CST);
}

function arcDec_0_sysvq0asl(memLoc_1) {
  let result_156;
  let X60Qx_319 = __atomic_sub_fetch(memLoc_1, 1, __ATOMIC_SEQ_CST);
  result_156 = (X60Qx_319 < 0);
  return result_156;
}

function arcIsUnique_0_sysvq0asl(memLoc_2) {
  let result_157;
  let X60Qx_320 = __atomic_load_n(memLoc_2, __ATOMIC_ACQUIRE);
  result_157 = (X60Qx_320 === 0);
  return result_157;
}

function writeErr_0_sysvq0asl(x_329) {
  fprintf(stderr, "%lld", x_329);
}

function writeErr_1_sysvq0asl(x_330) {
  fprintf(stderr, "%llu", x_330);
}

function writeErr_2_sysvq0asl(s_68) {
  let X60Qx_321 = readRawData_0_sysvq0asl(s_68, 0);
  let X60Qx_322 = len_4_sysvq0asl(s_68);
  let X60Qx_323 = fwrite(X60Qx_321, 1, X60Qx_322, stderr);
}

function writeErr_3_sysvq0asl(s_69) {
  let X60Qx_324 = len_5_sysvq0asl(s_69);
  let X60Qx_325 = fwrite(s_69, 1, X60Qx_324, stderr);
}

function panic_0_sysvq0asl(s_70) {
  writeErr_2_sysvq0asl(s_70);
  exit(1);
}

function nimIcheckAB(i_18, a_68, b_21) {
  let result_158;
  let X60Qx_326;
  if ((a_68 <= i_18)) {
    X60Qx_326 = (i_18 <= b_21);
  } else {
    X60Qx_326 = false;
  }
  if (X60Qx_326) {
    result_158 = (i_18 - a_68);
  } else {
    result_158 = 0;
    raiseIndexError3_0_I113jpc1_sysvq0asl(i_18, a_68, b_21);
  }
  return result_158;
}

function nimIcheckB(i_19, b_22) {
  let result_159;
  let X60Qx_327;
  if ((0 <= i_19)) {
    X60Qx_327 = (i_19 <= b_22);
  } else {
    X60Qx_327 = false;
  }
  if (X60Qx_327) {
    result_159 = i_19;
  } else {
    result_159 = 0;
    raiseIndexError3_0_I113jpc1_sysvq0asl(i_19, 0, b_22);
  }
  return result_159;
}

function nimUcheckAB(i_20, a_69, b_23) {
  let result_160;
  result_160 = (i_20 - a_69);
  if ((b_23 < result_160)) {
    raiseIndexError3_0_Ic5mmkg_sysvq0asl(i_20, a_69, b_23);
  }
  return result_160;
}

function nimUcheckB(i_21, b_24) {
  let result_161;
  result_161 = i_21;
  if ((b_24 < result_161)) {
    raiseIndexError3_0_Ic5mmkg_sysvq0asl(i_21, 0, b_24);
  }
  return result_161;
}

function nimInvalidObjConv(name_0) {
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1986947582);
    mem.setU32((_o + 4), strlit_0_I15539159382304113184_sysvq0asl);
    return _o;
  })());
  writeErr_2_sysvq0asl(name_0);
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2561);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  exit(1);
}

function nimChckNilDisp(p_25) {
  if ((p_25 === 0)) {
    writeErr_2_sysvq0asl((() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 1851876350);
      mem.setU32((_o + 4), strlit_0_I14281474217946372742_sysvq0asl);
      return _o;
    })());
    exit(1);
  }
}

function procAddrError_0_sysvq0asl(name_1) {
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1970234366);
    mem.setU32((_o + 4), strlit_0_I10604297744791418982_sysvq0asl);
    return _o;
  })());
  writeErr_3_sysvq0asl(name_1);
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2561);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  exit(1);
}

function nimLoadLibrary(path_2) {
  let result_162;
  let flags_1 = 2;
  let X60Qx_328 = dlopen(path_2, flags_1);
  result_162 = X60Qx_328;
  return result_162;
}

function nimGetProcAddr(lib_3, name_3) {
  let result_163;
  let X60Qx_329 = dlsym(lib_3, name_3);
  result_163 = X60Qx_329;
  if ((result_163 === 0)) {
    procAddrError_0_sysvq0asl(name_3);
  }
  return result_163;
}

function nimDynlibLoadStep(prev_0, cand_0) {
  let result_164;
  if ((!(prev_0 === 0))) {
    result_164 = prev_0;
  } else {
    let X60Qx_330 = nimLoadLibrary(cand_0);
    result_164 = X60Qx_330;
  }
  return result_164;
}

function nimDynlibCheck(lib_4, path_3) {
  let result_165;
  if ((lib_4 === 0)) {
    writeErr_2_sysvq0asl((() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 1970234366);
      mem.setU32((_o + 4), strlit_0_I16690852185662743073_sysvq0asl);
      return _o;
    })());
    writeErr_3_sysvq0asl(path_3);
    writeErr_2_sysvq0asl((() => {
      let _o = allocFixed(8);
      mem.setU32(_o, 2561);
      mem.setU32((_o + 4), 0);
      return _o;
    })());
    exit(1);
  }
  result_165 = lib_4;
  return result_165;
}

let exc_0_sysvq0asl = allocFixed(4);

function inc_0_Iloplki_sysvq0asl(x_374, y_215) {
  mem.setI32(x_374, (mem.i32(x_374) + y_215));
}

function dec_0_Ig5i8xp_sysvq0asl(x_376, y_217) {
  mem.setI32(x_376, (mem.i32(x_376) - y_217));
}

function dec_1_I0nzoz91_sysvq0asl(x_377) {
  mem.setI32(x_377, (mem.i32(x_377) - 1));
}

function listAdd_0_Ik4wxhz_sysvq0asl(head_5, c_38) {
  mem.setU32((c_38 + 12), mem.u32(head_5));
  if ((!(mem.u32(head_5) === 0))) {
    mem.setU32((mem.u32(head_5) + 16), c_38);
  }
  mem.setU32(head_5, c_38);
}

function listRemove_0_Ibzev091_sysvq0asl(head_6, c_39) {
  if ((c_39 === mem.u32(head_6))) {
    mem.setU32(head_6, mem.u32((c_39 + 12)));
    if ((!(mem.u32(head_6) === 0))) {
      mem.setU32((mem.u32(head_6) + 16), 0);
    }
  } else {
    mem.setU32((mem.u32((c_39 + 16)) + 12), mem.u32((c_39 + 12)));
    if ((!(mem.u32((c_39 + 12)) === 0))) {
      mem.setU32((mem.u32((c_39 + 12)) + 16), mem.u32((c_39 + 16)));
    }
  }
  mem.setU32((c_39 + 12), 0);
  mem.setU32((c_39 + 16), 0);
}

function raiseIndexError3_0_I113jpc1_sysvq0asl(i_68, a_83, b_38) {
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1684957694);
    mem.setU32((_o + 4), strlit_0_I11614695157650328859_sysvq0asl);
    return _o;
  })());
  writeErr_0_sysvq0asl(BigInt(i_68));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1869488382);
    mem.setU32((_o + 4), strlit_0_I16845119709590674135_sysvq0asl);
    return _o;
  })());
  writeErr_0_sysvq0asl(BigInt(a_83));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 3026434);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  writeErr_0_sysvq0asl(BigInt(b_38));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2561);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  exit(1);
}

function raiseIndexError3_0_Ic5mmkg_sysvq0asl(i_69, a_84, b_39) {
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1684957694);
    mem.setU32((_o + 4), strlit_0_I11614695157650328859_sysvq0asl);
    return _o;
  })());
  writeErr_1_sysvq0asl(BigInt(i_69));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 1869488382);
    mem.setU32((_o + 4), strlit_0_I16845119709590674135_sysvq0asl);
    return _o;
  })());
  writeErr_1_sysvq0asl(BigInt(a_84));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 3026434);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  writeErr_1_sysvq0asl(BigInt(b_39));
  writeErr_2_sysvq0asl((() => {
    let _o = allocFixed(8);
    mem.setU32(_o, 2561);
    mem.setU32((_o + 4), 0);
    return _o;
  })());
  exit(1);
}

function eQwasmovedQ_ArefSX45xception0sysvq0asl_0_sysvq0asl(dest_0) {
  mem.setU32(dest_0, 0);
}

let X60QiniGuard_0_sysvq0asl = allocFixed(1);

function X60Qini_0_sysvq0asl() {
  if (mem.u8At(X60QiniGuard_0_sysvq0asl)) {
    return;
  }
  mem.setU8(X60QiniGuard_0_sysvq0asl, true);
  eQwasmovedQ_ArefSX45xception0sysvq0asl_0_sysvq0asl(exc_0_sysvq0asl);
}

// generated by lengc (js backend) from syn1lfpjv.c.nif
"use strict";

let strlit_0_I8572766038233537570_syn1lfpjv = allocFixed(16);

mem.setI32(strlit_0_I8572766038233537570_syn1lfpjv, 4);

mem.setI32((strlit_0_I8572766038233537570_syn1lfpjv + 4), 0);

mem.setI32((strlit_0_I8572766038233537570_syn1lfpjv + 8), 0);

mem.writeStr((strlit_0_I8572766038233537570_syn1lfpjv + 12), "true");

let strlit_0_I3372626016653902757_syn1lfpjv = allocFixed(17);

mem.setI32(strlit_0_I3372626016653902757_syn1lfpjv, 5);

mem.setI32((strlit_0_I3372626016653902757_syn1lfpjv + 4), 0);

mem.setI32((strlit_0_I3372626016653902757_syn1lfpjv + 8), 0);

mem.writeStr((strlit_0_I3372626016653902757_syn1lfpjv + 12), "false");

function write_0_syn1lfpjv(f_4, s_0) {
  let X60Qx_1 = readRawData_0_sysvq0asl(s_0, 0);
  let X60Qx_2 = len_4_sysvq0asl(s_0);
  let X60Qx_3 = fwrite(X60Qx_1, 1, X60Qx_2, f_4);
}

function write_7_syn1lfpjv(f_11, c_1) {
  let X60Qx_4 = fputc(c_1, f_11);
}

let X60QiniGuard_0_syn1lfpjv = allocFixed(1);

function X60Qini_0_syn1lfpjv() {
  if (mem.u8At(X60QiniGuard_0_syn1lfpjv)) {
    return;
  }
  mem.setU8(X60QiniGuard_0_syn1lfpjv, true);
  X60Qini_0_sysvq0asl();
  X60Qini_0_for2ybv4p1();
}

main(0, []);
