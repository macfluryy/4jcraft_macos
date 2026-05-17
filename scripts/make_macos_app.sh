#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# scripts/make_macos_app.sh
#
# Package the compiled Minecraft.Client binary together with its runtime
# assets into a standalone macOS .app bundle, then wrap that bundle into
# a distributable .dmg image.
#
# Prerequisites:
#   - Project is already built (build/targets/app/Minecraft.Client exists).
#   - Standard macOS command line tools: sips, iconutil, hdiutil, codesign.
#
# Usage:
#   scripts/make_macos_app.sh                  # default name and version
#   APP_NAME=4JCraft VERSION=1.0.0 scripts/make_macos_app.sh
#
# Outputs (in ./dist/):
#   <APP_NAME>.app
#   <APP_NAME>-<VERSION>.dmg
# -----------------------------------------------------------------------------

set -euo pipefail

# -----------------------------------------------------------------------------
# Config (override via env vars)
# -----------------------------------------------------------------------------
APP_NAME="${APP_NAME:-4JCraft}"
BUNDLE_ID="${BUNDLE_ID:-com.macfluryy.4jcraft}"
VERSION="${VERSION:-1.0.0}"
GAME_VERSION_DISPLAY="${GAME_VERSION_DISPLAY:-Console Edition 1.6}"
MIN_MACOS="${MIN_MACOS:-13.0}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_APP_DIR="${PROJECT_ROOT}/build/targets/app"
BIN_NAME="Minecraft.Client"
ICON_SRC="${PROJECT_ROOT}/targets/resources/Common/Media/Graphics/MinecraftIcon.png"

DIST_DIR="${PROJECT_ROOT}/dist"
APP_BUNDLE="${DIST_DIR}/${APP_NAME}.app"
DMG_PATH="${DIST_DIR}/${APP_NAME}-${VERSION}.dmg"

# -----------------------------------------------------------------------------
# Sanity checks
# -----------------------------------------------------------------------------
if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "error: this script must be run on macOS" >&2
  exit 1
fi

if [[ ! -x "${BUILD_APP_DIR}/${BIN_NAME}" ]]; then
  echo "error: ${BUILD_APP_DIR}/${BIN_NAME} not found." >&2
  echo "       run 'meson compile -C build' first." >&2
  exit 1
fi

for asset in Common Sound music; do
  if [[ ! -d "${BUILD_APP_DIR}/${asset}" ]]; then
    echo "error: missing runtime asset directory ${BUILD_APP_DIR}/${asset}" >&2
    echo "       did the post-build copy_assets_to_client step run?" >&2
    exit 1
  fi
done

if [[ ! -f "${ICON_SRC}" ]]; then
  echo "error: source icon ${ICON_SRC} not found" >&2
  exit 1
fi

# -----------------------------------------------------------------------------
# Reset output
# -----------------------------------------------------------------------------
echo "==> Cleaning ${DIST_DIR}"
rm -rf "${APP_BUNDLE}" "${DMG_PATH}"
mkdir -p "${DIST_DIR}"

# -----------------------------------------------------------------------------
# Build the .app skeleton
# -----------------------------------------------------------------------------
echo "==> Creating bundle skeleton: ${APP_BUNDLE}"
mkdir -p "${APP_BUNDLE}/Contents/MacOS"
mkdir -p "${APP_BUNDLE}/Contents/Resources"

# Real binary lives next to its assets inside MacOS/.
cp "${BUILD_APP_DIR}/${BIN_NAME}" "${APP_BUNDLE}/Contents/MacOS/${BIN_NAME}"
chmod +x "${APP_BUNDLE}/Contents/MacOS/${BIN_NAME}"

# Game data: Common/, Sound/, music/ all live next to the binary because
# the runtime loads them via CWD-relative paths.
cp -R "${BUILD_APP_DIR}/Common" "${APP_BUNDLE}/Contents/MacOS/"
cp -R "${BUILD_APP_DIR}/Sound"  "${APP_BUNDLE}/Contents/MacOS/"
cp -R "${BUILD_APP_DIR}/music"  "${APP_BUNDLE}/Contents/MacOS/"

# Bundle launcher: Finder execs CFBundleExecutable with CWD=/, so we need
# a tiny wrapper that chdirs into MacOS/ before launching the real binary.
cat > "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}" <<'WRAPPER_EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"
exec "./Minecraft.Client" "$@"
WRAPPER_EOF
chmod +x "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}"

# -----------------------------------------------------------------------------
# Build the .icns icon from the in-game 64x64 PNG.
# Upscaling 64 -> 1024 with sips is bilinear and slightly soft, but the
# Minecraft logo is pixel art so it still reads fine. Replace the icon
# manually if you want crisper nearest-neighbour scaling.
# -----------------------------------------------------------------------------
echo "==> Generating AppIcon.icns from MinecraftIcon.png"
ICONSET_DIR="$(mktemp -d)/AppIcon.iconset"
mkdir -p "${ICONSET_DIR}"

declare -a SIZES=(
  "16:icon_16x16.png"
  "32:icon_16x16@2x.png"
  "32:icon_32x32.png"
  "64:icon_32x32@2x.png"
  "128:icon_128x128.png"
  "256:icon_128x128@2x.png"
  "256:icon_256x256.png"
  "512:icon_256x256@2x.png"
  "512:icon_512x512.png"
  "1024:icon_512x512@2x.png"
)
for entry in "${SIZES[@]}"; do
  size="${entry%%:*}"
  name="${entry##*:}"
  sips -z "${size}" "${size}" "${ICON_SRC}" --out "${ICONSET_DIR}/${name}" \
    >/dev/null
done

iconutil -c icns "${ICONSET_DIR}" \
  -o "${APP_BUNDLE}/Contents/Resources/AppIcon.icns"
rm -rf "${ICONSET_DIR}"

# -----------------------------------------------------------------------------
# Info.plist
# -----------------------------------------------------------------------------
echo "==> Writing Info.plist"
cat > "${APP_BUNDLE}/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>CFBundleIdentifier</key>
    <string>${BUNDLE_ID}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundleDisplayName</key>
    <string>${APP_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${VERSION}</string>
    <key>LSMinimumSystemVersion</key>
    <string>${MIN_MACOS}</string>
    <key>LSApplicationCategoryType</key>
    <string>public.app-category.games</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
    <key>NSHumanReadableCopyright</key>
    <string>Unofficial macOS port of Minecraft ${GAME_VERSION_DISPLAY}. Not affiliated with Mojang, Microsoft, or 4J Studios. Game assets remain the property of their respective owners.</string>
</dict>
</plist>
EOF

# -----------------------------------------------------------------------------
# Ad-hoc code signing.
# Without this, Gatekeeper blocks newly-built bundles with
# "App is damaged and can't be opened" once they pick up a quarantine
# attribute (e.g. after being downloaded from a DMG).
# This signs with the local "-" identity, which Apple accepts for
# personal use but NOT for distribution outside your machine.
# -----------------------------------------------------------------------------
echo "==> Ad-hoc signing the bundle"
codesign --force --deep --sign - "${APP_BUNDLE}"
codesign --verify --deep --strict --verbose=2 "${APP_BUNDLE}" || true

echo "==> ${APP_BUNDLE} ready ($(du -sh "${APP_BUNDLE}" | awk '{print $1}'))"

# -----------------------------------------------------------------------------
# DMG packaging.
# Layout: <volume>/<APP_NAME>.app and a symlink to /Applications so users
# can drag-install. UDZO compression keeps the image small.
# -----------------------------------------------------------------------------
echo "==> Creating DMG: ${DMG_PATH}"
DMG_STAGE="$(mktemp -d)/${APP_NAME}-stage"
mkdir -p "${DMG_STAGE}"
cp -R "${APP_BUNDLE}" "${DMG_STAGE}/"
ln -s /Applications "${DMG_STAGE}/Applications"

hdiutil create \
  -volname "${APP_NAME}" \
  -srcfolder "${DMG_STAGE}" \
  -ov \
  -format UDZO \
  "${DMG_PATH}" >/dev/null

rm -rf "${DMG_STAGE}"
echo "==> ${DMG_PATH} ready ($(du -sh "${DMG_PATH}" | awk '{print $1}'))"

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
echo
echo "Done. Output:"
echo "  ${APP_BUNDLE}"
echo "  ${DMG_PATH}"
echo
echo "Test by running:  open '${APP_BUNDLE}'"
echo
echo "If macOS complains 'app is damaged' on another machine, that machine"
echo "needs to either right-click -> Open the bundle once, or run:"
echo "  xattr -d com.apple.quarantine '${APP_NAME}.app'"
