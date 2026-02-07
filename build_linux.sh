#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_linux"
DATA_DIR="$SCRIPT_DIR/data"
NPROC=$(nproc 2>/dev/null || echo 4)
BUILD_TYPE="${1:-RelWithDebInfo}"

RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

err=0

# --- Check build tools ---
for cmd in cmake g++ make pkg-config; do
    if ! command -v "$cmd" &>/dev/null; then
        echo -e "${RED}MISSING:${NC} $cmd not found. Install it with your package manager."
        err=1
    fi
done

# --- Check system libraries (headers) ---
LIBS=(
    "GL/gl.h:libgl-dev (or mesa-dev)"
    "X11/Xlib.h:libx11-dev"
    "X11/extensions/Xrandr.h:libxrandr-dev"
    "X11/extensions/Xinerama.h:libxinerama-dev"
    "X11/Xcursor/Xcursor.h:libxcursor-dev"
    "X11/extensions/XInput2.h:libxi-dev"
    "xkbcommon/xkbcommon.h:libxkbcommon-dev"
)

for entry in "${LIBS[@]}"; do
    header="${entry%%:*}"
    pkg="${entry##*:}"
    found=0
    for inc in /usr/include /usr/local/include; do
        if [ -f "$inc/$header" ]; then
            found=1
            break
        fi
    done
    if [ "$found" -eq 0 ]; then
        echo -e "${RED}MISSING:${NC} $header not found. Install: ${YELLOW}$pkg${NC}"
        err=1
    fi
done

# --- Check submodules ---
SUBMODULES=(
    "ThirdParty/SDL2"
    "ThirdParty/bgfx.cmake"
    "ThirdParty/soloud/soloud"
    "ThirdParty/glm"
    "ThirdParty/imgui"
    "ThirdParty/tracy"
    "ThirdParty/ImGuizmo"
)

missing_subs=0
for sub in "${SUBMODULES[@]}"; do
    if [ ! -f "$SCRIPT_DIR/$sub/CMakeLists.txt" ] && [ ! -f "$SCRIPT_DIR/$sub/include" ]; then
        # Some submodules don't have CMakeLists.txt at root, just check if dir has files
        if [ ! -d "$SCRIPT_DIR/$sub" ] || [ -z "$(ls -A "$SCRIPT_DIR/$sub" 2>/dev/null)" ]; then
            echo -e "${RED}MISSING:${NC} Submodule $sub is empty or not initialized."
            missing_subs=1
        fi
    fi
done

if [ "$missing_subs" -eq 1 ]; then
    echo -e "${YELLOW}FIX:${NC} Run: git submodule update --init --recursive"
    err=1
fi

# --- Check game data ---
if [ ! -d "$DATA_DIR" ]; then
    echo -e "${RED}MISSING:${NC} data/ directory not found."
    echo -e "${YELLOW}FIX:${NC} Extract Panzer Dragoon Saga CD1 contents into: $DATA_DIR"
    err=1
elif [ -z "$(ls "$DATA_DIR"/*.DAT 2>/dev/null)" ]; then
    echo -e "${YELLOW}WARNING:${NC} data/ exists but no .DAT files found. Game may not run."
    echo -e "         Expected CD1 files like:INGS_A3.DAT, COMMON.DAT, etc."
fi

if [ "$err" -ne 0 ]; then
    echo ""
    echo -e "${RED}Build aborted. Fix the issues above and try again.${NC}"
    exit 1
fi

echo -e "${GREEN}All dependencies found.${NC}"
echo "Building ($BUILD_TYPE) with $NPROC jobs..."
echo ""

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
make -j"$NPROC"

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Run from the project root:"
echo "  ./build_linux/PDS/PDS"
