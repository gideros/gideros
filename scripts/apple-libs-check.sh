#!/bin/bash
# Shared by apple-libs-verify.sh and apple-plugins-verify.sh (source it, do not run it).
# Needs $T (a scratch dir) and $fail (set to 1 on any failure) from the caller.
IOS_MIN=${IOS_MIN:-15.0}; TVOS_MIN=${TVOS_MIN:-15.0}; MAC_MIN=${MAC_MIN:-12.0}

# check LABEL FILE PLATFORM "ARCHS" MINOS
check() {
  local label=$1 f=$2 wp=$3 wa=$4 wm=$5 have want a d got bad=0
  have=$(lipo -archs "$f" | tr ' ' '\n' | sort | tr '\n' ' ')
  want=$(echo $wa | tr ' ' '\n' | sort | tr '\n' ' ')
  [ "$have" = "$want" ] || { echo "FAIL $label: archs '$have' expected '$want'"; fail=1; bad=1; }
  for a in $(lipo -archs "$f"); do
    d=$T/x; rm -rf "$d"; mkdir -p "$d"
    if [ "$(lipo -archs "$f" | wc -w)" -gt 1 ]; then lipo "$f" -thin "$a" -output "$d/t.a"; else cp "$f" "$d/t.a"; fi
    # vtool needs an object file, not the archive
    got=$(cd "$d" && ar x t.a && xcrun vtool -show-build "$(ls *.o | grep -v '^__' | head -1)" | awk '/platform/{p=$2} /minos/{m=$2} END{print p" "m}')
    [ "$got" = "$wp $wm" ] || { echo "FAIL $label [$a]: $got, expected $wp $wm"; fail=1; bad=1; }
  done
  if [ $bad = 0 ]; then echo "ok   $label ($have) $wp $wm"; fi
}
