// Canonical JS runtime for the Leng JS backend: one ArrayBuffer as linear memory,
// plus the small set of C primitives the lowered code imports. The heap is Nim's
// OWN native allocator (`-d:nimNativeAlloc` — the ported `system/alloc.nim`),
// compiled to JS through lengjs like any other module; the runtime provides only
// `mmap`/`munmap` as the page primitives it sits on (Araq's boundary), so `alloc`/
// `dealloc`/`realloc` and their free-list reuse all run as real Nim code.
const _ab = new ArrayBuffer(1 << 29);           // 512 MiB linear memory
const _dv = new DataView(_ab);
const _u8 = new Uint8Array(_ab);

// ── TWO ARENAS, NOT ONE BUMP POINTER ────────────────────────────────────────
//
// `allocFixed` is the codegen's storage for value aggregates and `mmap` hands
// pages to the Nim allocator. They used to share `_brk`, and that is what made
// this leak unfixable rather than merely present.
//
// The leak: `allocFixed` is described as "a C-stack model", but a C stack is
// RECLAIMED on return and this was not — every call to a Nim proc holding a
// value aggregate bumped the pointer forever. In an interactive page that is
// per EVENT, so linear memory ran out after a fixed number of interactions and
// the tab died with
//
//   RangeError: Offset is outside the bounds of the DataView
//
// which is a pointer past the end of memory with no bounds check in front of
// it. Measured on one such page: 30 MiB at boot, +15 MiB per interaction.
//
// The obvious fix — save the pointer at a call boundary and restore it after —
// was tried and crashed on a dangling reference. The reason was the SHARED
// pointer: restoring it also un-mapped heap pages the Nim allocator still
// owned, so the failure looked like "value aggregates escape their frame" when
// it was really "this rolls back the heap". Splitting the two makes a frame
// reset mean only what it says, and `_leaveFrame` below is then safe.
// The arena only has to hold what is allocated OUTSIDE a frame — module init,
// and whatever the main line does before it hands control to the event loop.
// Framed work returns its storage, so an interactive page reaches a steady
// state instead of growing; this bound therefore constrains straight-line code,
// which is why it is generous. A harness that reconciles ten thousand times
// from `main` with no callback in sight is the case that sets it.
const _FIXED_CAP = 1 << 27;                     // 128 MiB of value-aggregate arena
let _fbrk = 8;                                  // offset 0 reserved as nil
let _mbrk = _FIXED_CAP;                         // the Nim heap starts above it

function allocFixed(n){
  const p=(_fbrk+7)&~7; _fbrk=p+n;
  // A NAMED failure. Running off the end used to surface as a DataView range
  // error from whichever store happened to be first past the boundary, which
  // named neither the arena nor the cause.
  if(_fbrk>_FIXED_CAP) throw new RangeError(
    "lengjs: allocFixed arena exhausted ("+_FIXED_CAP+" bytes). A value "+
    "aggregate is being allocated outside any frame, or a frame is never left.");
  _u8.fill(0,p,p+n); return p;
}

// ── FRAMES ──────────────────────────────────────────────────────────────────
//
// The one place the codegen's stack model can be made true is the JS -> Nim
// callback boundary: the runtime ALREADY declares that lifetime, three lines
// down, where it releases the argument handle because "an event object is only
// valid for the duration of dispatch". Everything allocFixed'd during that
// dispatch has the same lifetime, so the arena pointer is saved on the way in
// and restored on the way out.
//
// Only at depth 0: a dispatch that occurs inside another dispatch must not
// reclaim storage its caller is still using.
let _frameDepth = 0;
function _enterFrame(){ _frameDepth++; return _fbrk; }
function _leaveFrame(mark){ if(--_frameDepth === 0) _fbrk = mark; }

// Page primitives for `system/osalloc.nim`: `mmap` hands the Nim allocator a
// page-aligned, zero-filled region carved from the same buffer (MAP_FAILED = -1
// on exhaustion, which makes the allocator raise OutOfMem); `munmap` is a no-op
// (the bump arena does not reclaim whole pages — the Nim allocator still reuses
// cells within them). Signature matches posix `mmap(adr,len,prot,flags,fd,off)`.
const _PAGE = 4096;
function mmap(adr, len, prot, flags, fildes, off){
  len = Number(len);
  const p = (_mbrk + _PAGE - 1) & ~(_PAGE - 1);  // page-align
  if (p + len > _u8.length) return -1;          // MAP_FAILED
  _mbrk = p + len;
  _u8.fill(0, p, p + len);                      // MAP_ANONYMOUS: zero-filled
  return p;
}
function munmap(adr, len){ return 0; }

const mem = {
  setI8:(p,v)=>_dv.setInt8(p,v), i8:(p)=>_dv.getInt8(p),
  setU8:(p,v)=>_dv.setUint8(p,v), u8At:(p)=>_dv.getUint8(p),
  setI16:(p,v)=>_dv.setInt16(p,v,true), i16:(p)=>_dv.getInt16(p,true),
  setU16:(p,v)=>_dv.setUint16(p,v,true), u16:(p)=>_dv.getUint16(p,true),
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
// Each of these is a FRAME (see `_enterFrame` above). The handle release below
// already says the argument's lifetime is the dispatch; the frame says the same
// thing about the linear memory the dispatch used, which is the only reason an
// interactive page does not grow without bound. `finally`, so a throw inside a
// Nim callback -- which is a normal thing for an event handler to do -- does not
// leak the frame and slowly reintroduce the problem.
function _fnToJs0(idx){ return _jsNew(() => {
  const m = _enterFrame();
  try { _fns[idx](); } finally { _leaveFrame(m); }
}); }
function _fnToJs1(idx){
  return _jsNew((a) => {
    const m = _enterFrame();
    const h = _jsNew(a);
    try {
      const p = allocFixed(4); mem.setI32(p, h); // a JsValue {h} object for the ABI
      _fns[idx](p);
    } finally { _jsRelease(h); _leaveFrame(m); }
  });
}
// A Nim proc BOUND to a context value, called as a zero-argument JS function.
//
// Nim callbacks here are `{.nimcall.}` — they carry no captured environment — so
// a callback that must act on a particular object (the DOM node this binding
// updates, say) has nowhere to put it. This is the C answer: an explicit
// userdata pointer instead of a closure. The context is captured BY VALUE, as
// `_effectNew` captures its function, so releasing the handle that carried it in
// never disturbs the live callback; each call hands Nim a fresh borrowed handle
// and releases it after, exactly like `_fnToJs1`.
function _fnToJsCtx(idx, ctxH){
  const ctx = _jsv[ctxH];
  return _jsNew(() => {
    const m = _enterFrame();
    const h = _jsNew(ctx);
    try {
      const p = allocFixed(4); mem.setI32(p, h);
      _fns[idx](p);
    } finally { _jsRelease(h); _leaveFrame(m); }
  });
}
