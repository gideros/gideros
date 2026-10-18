# Minimum OS versions of the Apple static libraries and plugins we build (iOS, tvOS, macOS).
# This is the ONE definition: scripts/GidiOS.mk includes it, and apple-libs-check.sh / apple-libs-linktest.sh (so also
# apple-libs-verify.sh and apple-plugins-verify.sh) read it from here. A value in the environment or on the make command
# line overrides it, e.g. `make IOS_MIN=16.0 ...`.
#
# 15 / 15 / 12 are the SDK floor of the oldest supported Xcode (13.0: iOS 15, tvOS 15, macOS 12; newer SDKs reject lower targets).
# They must not be HIGHER than the deployment targets of the exported project, or apps linking the libs get "built for newer OS"
# warnings.
#
# Other places that carry these versions, and what to do about them:
#  - ui/Templates/Xcode4/iOS Template/iOS Template.xcodeproj/project.pbxproj: IPHONEOS_DEPLOYMENT_TARGET (iOS), TVOS_DEPLOYMENT_TARGET
#    (tvOS), MACOSX_DEPLOYMENT_TARGET (macOS). This is the floor of an exported app. KEEP IT >= the values below;
#    scripts/apple-libs-verify.sh fails if any value in it is lower.
#  - ios/iosplayer/iosplayer.xcodeproj: carries its own 15.0 / 12.0. Not read by these scripts; keep it consistent by hand.
#  - The lib and plugin .xcodeproj files (luau/lua, libgvfs, openal, plugins/*/source) still say 9.0 / 10.11 on purpose: the xcodebuild
#    command lines in GidiOS.mk override them through APPLE_DEPLOY, so do not edit them to match.
#  - gdrexport/ExportBuiltin.cpp (LSMinimumSystemVersion 10.12.0) belongs to the Qt desktop macOS export and is unrelated.
#  - Documentation (the wiki, PR text) mentions iOS 15 / tvOS 15 / macOS 12 and needs updating by hand when these change.
IOS_MIN?=15.0
TVOS_MIN?=15.0
MAC_MIN?=12.0
