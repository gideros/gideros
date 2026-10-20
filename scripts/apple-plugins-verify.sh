#!/bin/bash
# Check the Apple plugin binaries installed under Build.Mac/All Plugins/<Name>/bin/iOS.
# The same rules as the core libs, expects, per plugin, lib<n>.ios.xcframework and/or
# lib<n>.atv.xcframework (device arm64 + fat simulator) and/or a fat lib<n>.mac.a.
# Flat lib<n>.ios.a / .atv.a files are reported as stale (they are x86_64-only simulator
# builds from the old rules and are ignored by the exporter when an XCFramework exists).
#
#   scripts/apple-plugins-verify.sh "Build.Mac/All Plugins"
set -u

# Directory holding one folder per plugin; scratch dir (T) is removed on exit
ROOT=${1:?usage: $0 ALL_PLUGINS_DIR}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0

# Load utils and platform minimum versions
. "$(dirname "$0")/apple-libs-check.sh"

total=0

# Visit every plugin's bin/iOS folder
for bin in "$ROOT"/*/bin/iOS; do
  [ -d "$bin" ] || continue

  # <ROOT>/<plugin>/bin/iOS -> plugin name
  pn=$(basename "$(dirname "$(dirname "$bin")")")
  count=0

  # iOS and tvOS plugins ship as XCFrameworks
  for x in "$bin"/lib*.xcframework; do
    [ -e "$x" ] || continue
    count=$((count+1))

    # Split the name: libogg.ios.xcframework -> plat=ios, n=libogg
    base=$(basename "$x" .xcframework)            # libogg.ios
    plat=${base##*.}; n=${base%.*}                 # ios, libogg

    # Device platform, simulator platform and minimum OS to expect for this suffix
    case $plat in
      ios) dp=IOS;  sp=IOSSIMULATOR;  mn=$IOS_MIN ;;
      atv) dp=TVOS; sp=TVOSSIMULATOR; mn=$TVOS_MIN ;;
      *)   echo "FAIL $pn/$base.xcframework: platform suffix '$plat' is not ios or atv"; fail=1; continue ;;
    esac

    # Check each slice's library: simulator must be arm64 + x86_64, device arm64 only
    slices=0
    for s in "$x"/*/; do
      s=${s%/}; sn=$(basename "$s"); slices=$((slices+1))
      [ -f "$s/$n.a" ] || { echo "FAIL $pn/$base/$sn: expected $n.a, found: $(ls "$s")"; fail=1; continue; }
      case $sn in
        *-simulator) check "$pn/$base/$sn" "$s/$n.a" "$sp" "arm64 x86_64" "$mn" ;;
        *)           check "$pn/$base/$sn" "$s/$n.a" "$dp" "arm64" "$mn" ;;
      esac
    done

    # Exactly one device slice and one simulator slice
    [ $slices = 2 ] || { echo "FAIL $pn/$base.xcframework: $slices slices, expected 2 (device + simulator)"; fail=1; }
  done

  # macOS plugins ship as a fat static library
  for f in "$bin"/lib*.mac.a; do
    [ -e "$f" ] || continue; count=$((count+1))
    check "$pn/$(basename "$f")" "$f" MACOS "arm64 x86_64" "$MAC_MIN"
  done

  # Old flat iOS/tvOS libraries: warn only
  for f in "$bin"/lib*.ios.a "$bin"/lib*.atv.a; do
    [ -e "$f" ] && echo "WARN $pn/$(basename "$f"): stale flat library (pre-XCFramework build)"
  done

  # A plugin with no binaries at all is an error
  [ $count -gt 0 ] || { echo "FAIL $pn/bin/iOS has no plugin binaries"; fail=1; }
  total=$((total+count))
done

# No plugins found anywhere usually means a wrong ROOT
[ $total -gt 0 ] || { echo "FAIL no plugin binaries under $ROOT"; fail=1; }

# Summary and exit status
[ $fail = 0 ] && echo "ALL PLUGIN SLICES OK ($total binaries)" || echo "PLUGIN SLICE CHECK FAILED"
exit $fail
