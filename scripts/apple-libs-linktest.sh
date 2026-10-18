#!/bin/bash
# Link smoke test: link a stub main that calls the public gdr_* entry points against every slice/arch of the four
# Apple libs in an installed template. A static lib compiles fine with missing symbols, so this is the check that
# the libs fit together. It uses the current SDK; it says nothing about older Xcode toolchains.
#
#   scripts/apple-libs-linktest.sh TEMPLATE_ROOT
set -u
ROOT=${1:?usage: $0 TEMPLATE_ROOT}
. "$(dirname "$0")/apple-libs-check.sh"   # IOS_MIN, TVOS_MIN, MAC_MIN (from apple-targets.mk)
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
fail=0

cat > "$W/main.cpp" <<'EOF'
extern "C" {
void gdr_initialize(void* view, int width, int height, bool player);
void gdr_drawFirstFrame();
void gdr_drawFrame(bool force);
void gdr_deinitialize();
bool gdr_isRunning();
}
// Globals the exported app normally provides (template EAGLView.m)
extern "C" { void* metalDevice = 0; void* metalFramebuffer = 0; }
int main() { gdr_initialize(0, 320, 480, false); gdr_drawFirstFrame(); gdr_drawFrame(true); (void)gdr_isRunning(); gdr_deinitialize(); return 0; }
EOF

FW_IOS="-framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore -framework AudioToolbox -framework AVFoundation -framework CoreAudio -framework CoreLocation -framework CoreMotion -framework GameController -framework GameKit -framework StoreKit -framework Metal -framework MetalKit -framework OpenGLES -framework Security -framework CoreFoundation -framework CoreVideo -framework CoreMedia -framework CoreText -framework ImageIO -framework UniformTypeIdentifiers -lsqlite3 -lz -lc++"
FW_TV="${FW_IOS//-framework CoreMotion /}"
FW_MAC="-framework Cocoa -framework AppKit -framework Foundation -framework CoreGraphics -framework QuartzCore -framework AudioToolbox -framework AVFoundation -framework CoreAudio -framework CoreLocation -framework CoreMotion -framework GameController -framework GameKit -framework StoreKit -framework Metal -framework MetalKit -framework OpenGL -framework IOKit -framework Security -framework CoreVideo -framework CoreMedia -framework CoreText -framework ImageIO -framework UniformTypeIdentifiers -framework Carbon -lsqlite3 -lz -lc++"

# run LABEL SDK ARCH MINFLAG FRAMEWORKS LIBS...
run() {
  local label=$1 sdk=$2 arch=$3 min=$4 fw=$5; shift 5
  if xcrun -sdk "$sdk" clang++ -arch "$arch" "$min" -o /dev/null "$W/main.cpp" "$@" $fw > "$W/$label.log" 2>&1; then
    echo "PASS  $label"
  else
    echo "FAIL  $label"; grep -E 'Undefined|^  "|error|not found' "$W/$label.log" | sort -u | head -8; fail=1
  fi
}

# slice DIR LIB sim|dev -> path of the .a for that slice
slice() {
  local x="$ROOT/$1/lib$2.xcframework" s
  for s in "$x"/*/; do
    s=${s%/}
    case $s in *-simulator) [ "$3" = sim ] && { echo "$s/lib$2.a"; return; } ;; *) [ "$3" = dev ] && { echo "$s/lib$2.a"; return; } ;; esac
  done
}

# need LABEL FILE... : fail (instead of silently testing nothing) when a library is missing or has no architectures
need() {
  local label=$1 f; shift
  for f in "$@"; do
    if [ ! -f "$f" ] || [ -z "$(lipo -archs "$f" 2>/dev/null)" ]; then echo "FAIL  $label: missing or unreadable library '$f'"; fail=1; return 1; fi
  done
}

# platform DIR DEVSDK DEVMIN SIMSDK SIMMIN FW LABEL
platform() {
  local dir=$1 dsdk=$2 dmin=$3 ssdk=$4 smin=$5 fw=$6 label=$7 kind sdk min a libs
  for kind in sim dev; do
    if [ $kind = sim ]; then sdk=$ssdk; min=$smin; else sdk=$dsdk; min=$dmin; fi
    # array: the template path contains a space
    libs=("$(slice "$dir" gideros $kind)" "$(slice "$dir" lua $kind)" "$(slice "$dir" gvfs $kind)" "$(slice "$dir" openal $kind)")
    need "$label-$kind" "${libs[@]}" || continue
    for a in $(lipo -archs "${libs[0]}"); do
      run "$label-$kind-$a" "$sdk" "$a" "$min" "$fw" "${libs[@]}"
    done
  done
}

platform "iOS Template" iphoneos "-miphoneos-version-min=$IOS_MIN" iphonesimulator "-mios-simulator-version-min=$IOS_MIN" "$FW_IOS" ios
platform AppleTV appletvos "-mtvos-version-min=$TVOS_MIN" appletvsimulator "-mtvos-simulator-version-min=$TVOS_MIN" "$FW_TV" tvos
MACLIBS=("$ROOT/Mac/libgideros.a" "$ROOT/Mac/liblua.a" "$ROOT/Mac/libgvfs.a" "$ROOT/Mac/libopenal.a")
if need mac "${MACLIBS[@]}"; then
for a in $(lipo -archs "$ROOT/Mac/libgideros.a"); do
  run "mac-$a" macosx "$a" "-mmacosx-version-min=$MAC_MIN" "$FW_MAC" "${MACLIBS[@]}"
done
fi

[ $fail = 0 ] && echo "ALL LINKS OK" || echo "LINK TEST FAILED"
exit $fail
