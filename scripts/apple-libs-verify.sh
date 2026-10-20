#!/bin/bash
# Check every Apple lib slice in an installed template (Build.Mac/Templates/Xcode4/iOS Template):
# right architectures, Mach-O platform tag and minimum OS, and the library file inside each XCFramework
# slice named after the XCFramework (gdrexport patches keys into files by name).
#
#   scripts/apple-libs-verify.sh TEMPLATE_ROOT
set -u

# Template to check; scratch dir (T) is removed on exit
ROOT=${1:?usage: $0 TEMPLATE_ROOT}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0

# Load check() and the minimum OS versions (IOS_MIN, TVOS_MIN, MAC_MIN)
. "$(dirname "$0")/apple-libs-check.sh"

# Ensure every expected library is present
for n in gideros lua gvfs openal; do
  [ -d "$ROOT/iOS Template/lib$n.xcframework" ] || { echo "FAIL missing iOS Template/lib$n.xcframework"; fail=1; }
  [ -d "$ROOT/AppleTV/lib$n.xcframework" ] || { echo "FAIL missing AppleTV/lib$n.xcframework"; fail=1; }
  [ -f "$ROOT/Mac/lib$n.a" ] || { echo "FAIL missing Mac/lib$n.a"; fail=1; }
done

# iOS and tvOS: each entry is dir:devicePlatform:simulatorPlatform:minOS
for entry in "iOS Template:IOS:IOSSIMULATOR:$IOS_MIN" "AppleTV:TVOS:TVOSSIMULATOR:$TVOS_MIN"; do
  IFS=: read -r dir dp sp mn <<<"$entry"
  found=0

  # Check every XCFramework in the directory
  for x in "$ROOT/$dir"/*.xcframework; do
    [ -e "$x" ] || continue
    found=1
    n=$(basename "$x" .xcframework)

    # Check each slice's library: simulator must be arm64 + x86_64, device arm64 only
    slices=0
    for s in "$x"/*/; do
      s=${s%/}; sn=$(basename "$s"); slices=$((slices+1))
      [ -f "$s/$n.a" ] || { echo "FAIL $dir/$n/$sn: expected $n.a, found: $(ls "$s")"; fail=1; continue; }
      case $sn in
        *-simulator) check "$dir/$n/$sn" "$s/$n.a" "$sp" "arm64 x86_64" "$mn" ;;
        *)           check "$dir/$n/$sn" "$s/$n.a" "$dp" "arm64" "$mn" ;;
      esac
    done

    # Exactly one device slice and one simulator slice
    [ $slices = 2 ] || { echo "FAIL $dir/$n.xcframework: $slices slices, expected 2 (device + simulator)"; fail=1; }
  done

  # Old flat libraries left by an earlier install: warn only
  for f in "$ROOT/$dir"/lib*.a; do
    [ -e "$f" ] && echo "WARN $dir/$(basename "$f"): stale flat library from an older install (the template links the XCFrameworks)"
  done

  # A directory with no XCFrameworks at all is an error
  [ $found = 1 ] || { echo "FAIL no .xcframework in $dir"; fail=1; }
done

# macOS: flat fat libraries
found=0
for f in "$ROOT/Mac"/lib*.a; do
  [ -e "$f" ] || continue; found=1
  check "Mac/$(basename "$f")" "$f" MACOS "arm64 x86_64" "$MAC_MIN"
done
[ $found = 1 ] || { echo "FAIL no lib*.a in Mac"; fail=1; }

# Every platform needs both Metal shader libraries
for d in "iOS Template" AppleTV Mac; do
  for m in default.metallib default-sim.metallib; do
    [ -f "$ROOT/$d/$m" ] || { echo "FAIL missing $d/$m"; fail=1; }
  done
done

# The exported template declares the deployment target of an app: the libs must not be built for a newer OS than that
PBX="$ROOT/iOS Template.xcodeproj/project.pbxproj"
if [ ! -f "$PBX" ]; then
  echo "FAIL missing $PBX (cannot compare the libs' minimum OS with the template)"; fail=1
else
  # Compare each platform's deployment target(s) in the project with the libs' minimum
  for entry in "IPHONEOS:$IOS_MIN" "TVOS:$TVOS_MIN" "MACOSX:$MAC_MIN"; do
    key=${entry%%:*}_DEPLOYMENT_TARGET; min=${entry#*:}

    # All distinct values of this key in the project file
    vals=$(grep -o "$key = [0-9.]*;" "$PBX" | sed -E 's/.* = ([0-9.]*);/\1/' | sort -u)
    [ -n "$vals" ] || { echo "FAIL no $key in the template project"; fail=1; continue; }

    # Fail on any value lower than the libs' minimum (compares major, then minor)
    bad=0
    for v in $vals; do
      if awk -v a="$v" -v b="$min" 'BEGIN{split(a,x,".");split(b,y,".");exit !((x[1]+0<y[1]+0)||(x[1]+0==y[1]+0&&x[2]+0<y[2]+0))}'; then
        echo "FAIL template $key = $v is lower than the libs' minimum $min (scripts/GidAppleTargets.mk): raise the template or lower the libs"; fail=1; bad=1
      fi
    done
    [ $bad = 1 ] || echo "ok   template $key ($(echo $vals)) >= $min"
  done
fi

# Summary and exit status
[ $fail = 0 ] && echo "ALL SLICES OK" || echo "SLICE CHECK FAILED"
exit $fail
