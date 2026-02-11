#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_linux"
DATA_DIR="$SCRIPT_DIR/data"
NPROC=$(nproc 2>/dev/null || echo 4)

RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

# --- Detect distro ---
if command -v apt-get &>/dev/null; then
    DISTRO="debian"
elif command -v pacman &>/dev/null; then
    DISTRO="arch"
elif command -v dnf &>/dev/null; then
    DISTRO="fedora"
else
    DISTRO="unknown"
fi

DEPS_APT="cmake g++ make pkg-config libgl-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libvulkan-dev"
DEPS_PAC="cmake gcc make pkgconf mesa libx11 libxrandr libxinerama libxcursor libxi libxkbcommon vulkan-icd-loader vulkan-headers"
DEPS_DNF="cmake gcc-c++ make pkgconf-pkg-config mesa-libGL-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libxkbcommon-devel vulkan-loader-devel"

# --- Install dependencies if requested ---
if [ "${1:-}" = "--deps" ]; then
    echo "Installing build dependencies..."
    case "$DISTRO" in
        debian) sudo apt-get update -qq && sudo apt-get install -y $DEPS_APT ;;
        arch)   sudo pacman -S --needed $DEPS_PAC ;;
        fedora) sudo dnf install -y $DEPS_DNF ;;
        *)      echo -e "${RED}Unknown distro. Install dependencies manually:${NC}"
                echo "  Tools: cmake, g++/gcc, make, pkg-config"
                echo "  Libs:  GL, X11, Xrandr, Xinerama, Xcursor, Xi, xkbcommon, vulkan"
                exit 1 ;;
    esac
    echo -e "${GREEN}Dependencies installed.${NC}"
    shift
fi

BUILD_TYPE="${1:-RelWithDebInfo}"

err=0

# --- Check build tools ---
for cmd in cmake g++ make pkg-config; do
    if ! command -v "$cmd" &>/dev/null; then
        echo -e "${RED}MISSING:${NC} $cmd not found. Run: ${YELLOW}$0 --deps${NC}"
        err=1
    fi
done

# --- Check system libraries (headers) ---
# header:ubuntu-pkg:arch-pkg:fedora-pkg
LIBS=(
    "GL/gl.h:libgl-dev:mesa:mesa-libGL-devel"
    "X11/Xlib.h:libx11-dev:libx11:libX11-devel"
    "X11/extensions/Xrandr.h:libxrandr-dev:libxrandr:libXrandr-devel"
    "X11/extensions/Xinerama.h:libxinerama-dev:libxinerama:libXinerama-devel"
    "X11/Xcursor/Xcursor.h:libxcursor-dev:libxcursor:libXcursor-devel"
    "X11/extensions/XInput2.h:libxi-dev:libxi:libXi-devel"
    "xkbcommon/xkbcommon.h:libxkbcommon-dev:libxkbcommon:libxkbcommon-devel"
)

for entry in "${LIBS[@]}"; do
    IFS=':' read -r header pkg_deb pkg_arch pkg_fed <<< "$entry"
    case "$DISTRO" in
        debian)  pkg="$pkg_deb" ;;
        arch)    pkg="$pkg_arch" ;;
        fedora)  pkg="$pkg_fed" ;;
        *)       pkg="$pkg_deb" ;;
    esac
    found=0
    for inc in /usr/include /usr/local/include; do
        if [ -f "$inc/$header" ]; then
            found=1
            break
        fi
    done
    if [ "$found" -eq 0 ]; then
        echo -e "${RED}MISSING:${NC} $header not found. Install: ${YELLOW}$pkg${NC}  (or run: $0 --deps)"
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
cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j"$NPROC"

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Run from the project root:"
echo "  ./build_linux/PDS/PDS"
