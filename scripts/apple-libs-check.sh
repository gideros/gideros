#!/bin/bash
# Shared code for apple-libs-verify.sh and apple-plugins-verify.sh
# Needs $T (a scratch dir) and $fail (set to 1 on any failure) from the caller.

# Minimum OS versions
_targets="$(dirname "${BASH_SOURCE[0]}")/GidAppleTargets.mk"

for _v in IOS_MIN TVOS_MIN MAC_MIN; do
  [ -n "${!_v:-}" ] || read -r "$_v" <<< "$(sed -n "s/^$_v?=//p" "$_targets")"
  [ -n "${!_v:-}" ] || { echo "cannot read $_v from $_targets" >&2; exit 1; }
done

# Checks that a static library (.a) was built for the right architectures,
# platform and minimum OS version. It prints ok or FAIL for each file and
# sets the caller's $fail flag. It doesn't exit on failure, so one run
# reports every problem.
#
# Example
#   check libfoo-sim libfoo.a IOSSIMULATOR "arm64 x86_64" $IOS_MIN.
check() {
  local label=$1 f=$2 wp=$3 wa=$4 wm=$5 have want a d got bad=0

  # Verify architecture
  have=$(lipo -archs "$f" | tr ' ' '\n' | sort | tr '\n' ' ')
  want=$(echo $wa | tr ' ' '\n' | sort | tr '\n' ' ')
  [ "$have" = "$want" ] || { echo "FAIL $label: archs '$have' expected '$want'"; fail=1; bad=1; }

  # Verify platform & minos for each architecture
  for a in $(lipo -archs "$f"); do
    d=$T/x; rm -rf "$d"; mkdir -p "$d"
    if [ "$(lipo -archs "$f" | wc -w)" -gt 1 ]; then lipo "$f" -thin "$a" -output "$d/t.a"; else cp "$f" "$d/t.a"; fi
    # vtool needs an object file, not the archive
    got=$(cd "$d" && ar x t.a && xcrun vtool -show-build "$(ls *.o | grep -v '^__' | head -1)" | awk '/platform/{p=$2} /minos/{m=$2} END{print p" "m}')
    [ "$got" = "$wp $wm" ] || { echo "FAIL $label [$a]: $got, expected $wp $wm"; fail=1; bad=1; }
  done

  if [ $bad = 0 ]; then echo "ok   $label ($have) $wp $wm"; fi
}
