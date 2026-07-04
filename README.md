# js

**JS-value interop for the nimony JavaScript backend.**

The backend lays native Nim data out as byte offsets into one linear
`ArrayBuffer`; a genuine JS value — a string, object, function or DOM node —
can't live there. `JsValue` is an opaque **handle** into a runtime-side value
table, so real JS values cross that boundary. It is the substrate aoughwl's
client-side packages ([`web-state`](https://github.com/aoughwl/web-state), and
the DOM/`web` client runtime) sit on.

```nim
import jsffi

let math = global("Math")
echo math.call("max", toJs(3), toJs(7)).toInt      # 7

let o = newJsObject()
o.set("name", toJs("nimony"))
echo global("JSON").call("stringify", o)           # {"name":"nimony"}
```

## What you get

- **`JsValue`** — an owning handle to a JS value. GC-integrated: `=destroy`
  releases its table slot, and a copy allocates a fresh slot to the same value,
  so handles copy and free without ceremony and never double-free.
- **marshalling** — `toJs` / `toInt` / `toFloat` / `toBool` / `$`(stringify) for
  `int`/`float`/`bool`/`string` (strings cross as UTF-8).
- **access** — `global`, `get`/`set`, `call` (0–3 args) and variadic `apply`,
  `newJsObject`, `new`-construction (`newOf`), JS arrays (`newJsArray`/`add`/`[]`).
- **introspection** — `jsTypeof`, `hasProp`, `instanceOf`.
- **callbacks** — a Nim `proc {.nimcall.}` as a JS function (`toJs(JsProc0/1)`),
  the shape of an event handler.
- **`unsafeHandle` / `unsafeFromHandle`** — the raw-handle seam a higher layer
  (web-state, a DOM binding) uses to marshal a `JsValue` across its own runtime
  boundary without re-implementing the ARC hooks.

## Runtime

The Nim side is standard-library-only; the runtime bridges (the value table and
the `_jsGet`/`_jsCall`/`_strToJs`/… functions the `importc` procs name) live in
`runtime/jsffi.js`, linked into the JS bundle by the backend. Reading the raw FFI
seam is kept on plain `int32` handles, so no owning `JsValue` ever crosses into
an `importc`.

## Status

Prototype, tracking the nimony JS backend
([nim-lang/nimony#2043](https://github.com/nim-lang/nimony/pull/2043)). `tffi`
exercises the surface end-to-end (`./run.sh`, with the backend toolchain on
`BIN`). Once the backend lands `std/jsffi`, this becomes a thin shim over it.

## License

MIT.
