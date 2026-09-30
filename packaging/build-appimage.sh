#!/bin/sh

set -eu

PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
APPDIR="$BUILD_DIR/AthenaPM.AppDir"

rm -rf "$APPDIR"

cmake --install "$BUILD_DIR" --prefix "$APPDIR"

cp "$PROJECT_ROOT/packaging/AppDir/AppRun" \
   "$APPDIR/AppRun"

chmod +x "$APPDIR/AppRun"

cp "$PROJECT_ROOT/packaging/AppDir/athena-pm.desktop" \
   "$APPDIR/athena-pm.desktop"

cp "$PROJECT_ROOT/packaging/AppDir/athena.png" \
   "$APPDIR/athena.png"

echo "AppDir creata: $APPDIR"

APPIMAGETOOL="${APPIMAGETOOL:-$HOME/Applications/appimagetool/appimagetool-x86_64.AppImage}"
ATHENA_VERSION="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\).*/\1/p' "$PROJECT_ROOT/CMakeLists.txt" | head -n 1)"
DIST_DIR="$PROJECT_ROOT/dist"
OUTPUT="$DIST_DIR/AthenaPM-${ATHENA_VERSION}-x86_64.AppImage"

mkdir -p "$DIST_DIR"

"$APPIMAGETOOL" \
    "$APPDIR" \
    "$OUTPUT"

echo "AppImage creata: $OUTPUT"
