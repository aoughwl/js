# Package
version     = "0.1.0"
author      = "aoughwl"
description = "JS-value interop for the nimony JavaScript backend — an opaque handle to real JS values (strings, objects, functions, DOM nodes) across the linear-memory boundary, with GC-integrated ownership. The substrate aoughwl's client-side packages sit on."
license     = "MIT"
srcDir      = "."

# Pure interop — the Nim side is standard library only; the runtime bridges live
# in `runtime/jsffi.js` and are linked into the JS bundle by the backend.
