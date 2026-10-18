#!/bin/bash
# Check every Apple lib slice in an installed template (Build.Mac/Templates/Xcode4/iOS Template):
# right architectures, Mach-O platform tag and minimum OS, and the library file inside each XCFramework
# slice named after the XCFramework (gdrexport patches keys into files by name).
#
#   scripts/apple-libs-verify.sh TEMPLATE_ROOT
set -u
ROOT=${1:?usage: $0 TEMPLATE_ROOT}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0
. "$(dirname "$0")/apple-libs-check.sh"

# Every expected library must be there: the loops below only look at what they happen to find
for n in gideros lua gvfs openal; do
  [ -d "$ROOT/iOS Template/lib$n.xcframework" ] || { echo "FAIL missing iOS Template/lib$n.xcframework"; fail=1; }
  [ -d "$ROOT/AppleTV/lib$n.xcframework" ] || { echo "FAIL missing AppleTV/lib$n.xcframework"; fail=1; }
  [ -f "$ROOT/Mac/lib$n.a" ] || { echo "FAIL missing Mac/lib$n.a"; fail=1; }
done

for entry in "iOS Template:IOS:IOSSIMULATOR:$IOS_MIN" "AppleTV:TVOS:TVOSSIMULATOR:$TVOS_MIN"; do
  IFS=: read -r dir dp sp mn <<<"$entry"
  found=0
  for x in "$ROOT/$dir"/*.xcframework; do
    [ -e "$x" ] || continue
    found=1
    n=$(basename "$x" .xcframework)
    slices=0
    for s in "$x"/*/; do
      s=${s%/}; sn=$(basename "$s"); slices=$((slices+1))
      [ -f "$s/$n.a" ] || { echo "FAIL $dir/$n/$sn: expected $n.a, found: $(ls "$s")"; fail=1; continue; }
      case $sn in
        *-simulator) check "$dir/$n/$sn" "$s/$n.a" "$sp" "arm64 x86_64" "$mn" ;;
        *)           check "$dir/$n/$sn" "$s/$n.a" "$dp" "arm64" "$mn" ;;
      esac
    done
    [ $slices = 2 ] || { echo "FAIL $dir/$n.xcframework: $slices slices, expected 2 (device + simulator)"; fail=1; }
  done
  for f in "$ROOT/$dir"/lib*.a; do
    [ -e "$f" ] && echo "WARN $dir/$(basename "$f"): stale flat library from an older install (the template links the XCFrameworks)"
  done
  [ $found = 1 ] || { echo "FAIL no .xcframework in $dir"; fail=1; }
done

found=0
for f in "$ROOT/Mac"/lib*.a; do
  [ -e "$f" ] || continue; found=1
  check "Mac/$(basename "$f")" "$f" MACOS "arm64 x86_64" "$MAC_MIN"
done
[ $found = 1 ] || { echo "FAIL no lib*.a in Mac"; fail=1; }

for d in "iOS Template" AppleTV Mac; do
  for m in default.metallib default-sim.metallib; do
    [ -f "$ROOT/$d/$m" ] || { echo "FAIL missing $d/$m"; fail=1; }
  done
done

[ $fail = 0 ] && echo "ALL SLICES OK" || echo "SLICE CHECK FAILED"
exit $fail
