#!/usr/bin/env bash
# Compile + run the JS-backend tests: nimony c --bits:32 -> lengjs each module
# -> bundle with runtime/jsffi.js -> node -> diff against <test>.output.
#
# Needs the nimony JS-backend toolchain (bin/nimony, bin/lengjs). Point BIN at it:
#   BIN=~/nimony-js/bin ./run.sh            # run every tests/t*.nim
#   BIN=~/nimony-js/bin ./run.sh tffi       # run one
set -u
ROOT="$(cd "$(dirname "$0")" && pwd)"
BIN="${BIN:-$HOME/nimony-js/bin}"
RUNTIME="$ROOT/runtime/jsffi.js"
CACHE="$ROOT/nimcache"
NIMONY="$BIN/nimony"; LENGJS="$BIN/lengjs"
[ -x "$NIMONY" ] || { echo "no nimony at $NIMONY (set BIN=path/to/nimony-js/bin)"; exit 2; }

pass=0; fail=0
run_one() {
  local nim="$1" name; name="$(basename "$nim" .nim)"
  local out="$ROOT/tests/$name.output"
  rm -rf "$CACHE"; mkdir -p "$CACHE"
  # front end + hexer -> .c.nif per module (the C link failing on a 64-bit host
  # is expected and ignored; success = at least one .c.nif was produced).
  "$NIMONY" c --bits:32 --define:nimNativeAlloc --path:"$ROOT" --nimcache:"$CACHE" "$nim" >/dev/null 2>&1
  mapfile -t cnifs < <(find "$CACHE" -name '*.c.nif')
  if [ "${#cnifs[@]}" -eq 0 ]; then echo "FAIL $name: no .c.nif (compile error)"; fail=$((fail+1)); return; fi
  local bundle="$CACHE/bundle.js"; cat "$RUNTIME" > "$bundle"; echo >> "$bundle"
  for c in "${cnifs[@]}"; do "$LENGJS" "$c" "${c%.c.nif}.js" >/dev/null 2>&1 || { echo "FAIL $name: lengjs $c"; fail=$((fail+1)); return; }; cat "${c%.c.nif}.js" >> "$bundle"; echo >> "$bundle"; done
  echo "main(0, []);" >> "$bundle"
  local got; got="$(node "$bundle" 2>&1)"
  if [ -f "$out" ] && [ "$got" == "$(cat "$out")" ]; then echo "SUCCESS $name"; pass=$((pass+1));
  else echo "FAIL $name"; diff <(echo "$got") "$out" 2>/dev/null | head -20; fail=$((fail+1)); fi
}

if [ $# -ge 1 ]; then run_one "$ROOT/tests/$1.nim"
else for f in "$ROOT"/tests/t*.nim; do run_one "$f"; done; fi
echo "-- $pass passed, $fail failed --"
[ "$fail" -eq 0 ]
