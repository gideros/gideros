# Minimum Apple OS Versions
#
# Defines the minimum OS platform versions that Gideros can target.  Defines can be 
# overriden by environment vars or on the make command (make IOS_MIN=16.0 ...).  Also
# known as the SDK floor these are the SDK floor of the oldest supported Xcode (13.0).
# `apple-libs-check.sh` uses these to verify all plugins and libraries we generate 
# meet the requirements, failing the build if not. These values must not exceed the
# exported Gideros project's deployment targets.
#
# NOTE - these are pased by sed - must start at column 0, do not add whitespace.

IOS_MIN?=15.0
TVOS_MIN?=15.0
MAC_MIN?=12.0

# Other Locations
# 
# The constants above are used only by the `apple-*.sh` files.  Here is a list of other 
# locations of these minimums.
# - ui/Templates/Xcode4/iOS Template/iOS Template.xcodeproj/project.pbxproj: 
#   - The floor of any exported Apple app, must be >= the values above;
#   - scripts/apple-libs-verify.sh fails if any value in it is lower, stopping drift
# - ios/iosplayer/iosplayer.xcodeproj:
#   - Has its own 15.0 / 12.0. Not read by these scripts; keep it consistent by hand.
# - The lib and plugin .xcodeproj files (luau/lua, libgvfs, openal, plugins/*/source)
#   - Do not change - we override them in GidiOS.mk
# - gdrexport/ExportBuiltin.cpp (LSMinimumSystemVersion 10.12.0) 
#   - Belongs to the Qt desktop macOS export and is unrelated.
# - Documentation (the wiki, PR text) mentions iOS 15 / tvOS 15 / macOS 12 and needs updating by hand when these change.
