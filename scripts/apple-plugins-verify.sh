#!/bin/bash
# Check the Apple plugin binaries installed under Build.Mac/All Plugins/<Name>/bin/iOS: the same rules as the core
# libs (architectures, Mach-O platform tag, minimum OS, library file named after the XCFramework without its platform
# suffix). Expects, per plugin, lib<n>.ios.xcframework and/or lib<n>.atv.xcframework (device arm64 + fat simulator)
# and/or a fat lib<n>.mac.a. Flat lib<n>.ios.a / .atv.a files are reported as stale (they are x86_64-only simulator
# builds from the old rules and are ignored by the exporter when an XCFramework exists).
#
#   scripts/apple-plugins-verify.sh "Build.Mac/All Plugins"
set -u
ROOT=${1:?usage: $0 ALL_PLUGINS_DIR}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0
. "$(dirname "$0")/apple-libs-check.sh"

total=0
for bin in "$ROOT"/*/bin/iOS; do
  [ -d "$bin" ] || continue
  pn=$(basename "$(dirname "$(dirname "$bin")")")
  count=0
  for x in "$bin"/lib*.xcframework; do
    [ -e "$x" ] || continue
    count=$((count+1))
    base=$(basename "$x" .xcframework)            # libogg.ios
    plat=${base##*.}; n=${base%.*}                 # ios, libogg
    case $plat in
      ios) dp=IOS;  sp=IOSSIMULATOR;  mn=$IOS_MIN ;;
      atv) dp=TVOS; sp=TVOSSIMULATOR; mn=$TVOS_MIN ;;
      *)   echo "FAIL $pn/$base.xcframework: platform suffix '$plat' is not ios or atv"; fail=1; continue ;;
    esac
    slices=0
    for s in "$x"/*/; do
      s=${s%/}; sn=$(basename "$s"); slices=$((slices+1))
      [ -f "$s/$n.a" ] || { echo "FAIL $pn/$base/$sn: expected $n.a, found: $(ls "$s")"; fail=1; continue; }
      case $sn in
        *-simulator) check "$pn/$base/$sn" "$s/$n.a" "$sp" "arm64 x86_64" "$mn" ;;
        *)           check "$pn/$base/$sn" "$s/$n.a" "$dp" "arm64" "$mn" ;;
      esac
    done
    [ $slices = 2 ] || { echo "FAIL $pn/$base.xcframework: $slices slices, expected 2 (device + simulator)"; fail=1; }
  done
  for f in "$bin"/lib*.mac.a; do
    [ -e "$f" ] || continue; count=$((count+1))
    check "$pn/$(basename "$f")" "$f" MACOS "arm64 x86_64" "$MAC_MIN"
  done
  for f in "$bin"/lib*.ios.a "$bin"/lib*.atv.a; do
    [ -e "$f" ] && echo "WARN $pn/$(basename "$f"): stale flat library (pre-XCFramework build)"
  done
  [ $count -gt 0 ] || { echo "FAIL $pn/bin/iOS has no plugin binaries"; fail=1; }
  total=$((total+count))
done
[ $total -gt 0 ] || { echo "FAIL no plugin binaries under $ROOT"; fail=1; }

[ $fail = 0 ] && echo "ALL PLUGIN SLICES OK ($total binaries)" || echo "PLUGIN SLICE CHECK FAILED"
exit $fail
