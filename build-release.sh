#!/bin/bash
# Automated build script for Ciary releases
# Builds binaries for multiple platforms and creates release archives

set -e

# Configuration
PROJECT_NAME="ciary"
VERSION=${1:-"dev"}
BUILD_DIR="build"
DIST_DIR="$BUILD_DIR/dist"
RELEASE_DIR="$BUILD_DIR/release"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required tools are available
check_dependencies() {
    log_info "Checking dependencies..."
    
    # Check for make
    if ! command -v make &> /dev/null; then
        log_error "make is required but not installed"
        exit 1
    fi
    
    # Check for basic build tools
    if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
        log_error "gcc or clang is required but not installed"
        exit 1
    fi
    
    log_success "Dependencies check passed"
}

# Clean previous builds
clean_builds() {
    log_info "Cleaning previous builds..."
    make dist-clean > /dev/null 2>&1 || true
    rm -rf "$RELEASE_DIR"
    mkdir -p "$RELEASE_DIR"
    log_success "Clean completed"
}

# Build for a specific target
build_target() {
    local target=$1
    local binary_name=$2
    
    log_info "Building $target..."
    
    if make "$target" > /dev/null 2>&1; then
        if [ -f "$DIST_DIR/$binary_name" ]; then
            log_success "Built $target successfully"
            return 0
        else
            log_warn "Build succeeded but binary not found: $DIST_DIR/$binary_name"
            return 1
        fi
    else
        log_warn "Failed to build $target (cross-compiler may not be available)"
        return 1
    fi
}

# Create release archive
create_archive() {
    local binary_path=$1
    local platform=$2
    local arch=$3
    
    if [ ! -f "$binary_path" ]; then
        log_warn "Binary not found: $binary_path"
        return 1
    fi
    
    local archive_name="${PROJECT_NAME}-${VERSION}-${platform}-${arch}"
    local archive_dir="$RELEASE_DIR/$archive_name"
    
    # Create archive directory
    mkdir -p "$archive_dir"
    
    # Copy binary
    cp "$binary_path" "$archive_dir/$PROJECT_NAME"
    
    # Copy documentation
    cp README.md "$archive_dir/" 2>/dev/null || log_warn "README.md not found"
    cp LICENSE "$archive_dir/" 2>/dev/null || log_warn "LICENSE not found"
    
    # Create installation instructions
    cat > "$archive_dir/INSTALL.txt" << EOF
Ciary Installation Instructions
==============================

To install Ciary:

1. Make the binary executable:
   chmod +x ciary

2. (Optional) Move to your PATH:
   sudo mv ciary /usr/local/bin/

3. Install dependencies:
   - Ubuntu/Debian: sudo apt install libncurses-dev
   - macOS: brew install ncurses
   - Arch Linux: sudo pacman -S ncurses

4. Run Ciary:
   ./ciary  (or just 'ciary' if installed to PATH)

For more information, see README.md or visit:
https://github.com/git-akihakune/ciary
EOF
    
    # Create compressed archive
    cd "$RELEASE_DIR"
    if command -v tar &> /dev/null; then
        tar -czf "${archive_name}.tar.gz" "$archive_name"
        log_success "Created ${archive_name}.tar.gz"
    fi
    
    if command -v zip &> /dev/null; then
        zip -r "${archive_name}.zip" "$archive_name" > /dev/null
        log_success "Created ${archive_name}.zip"
    fi
    
    cd - > /dev/null
    
    # Clean up directory
    rm -rf "$archive_dir"
}

# Build native binary
build_native() {
    log_info "Building native binary..."
    
    if make native > /dev/null 2>&1; then
        # Detect current platform
        local platform=$(uname -s | tr '[:upper:]' '[:lower:]')
        local arch=$(uname -m)
        
        # Normalize architecture names
        case $arch in
            x86_64|amd64) arch="x86_64" ;;
            arm64) arch="aarch64" ;;
            i386|i686) arch="i386" ;;
        esac
        
        # Normalize platform names
        case $platform in
            darwin) platform="darwin" ;;
            linux) platform="linux" ;;
            freebsd) platform="freebsd" ;;
            openbsd) platform="openbsd" ;;
            netbsd) platform="netbsd" ;;
        esac
        
        local binary_path="$DIST_DIR/${PROJECT_NAME}-${platform}-${arch}"
        create_archive "$binary_path" "$platform" "$arch"
        log_success "Built native binary for $platform-$arch"
    else
        log_error "Failed to build native binary"
        return 1
    fi
}

# Main build function
main() {
    echo "=========================================="
    echo "  Ciary Release Builder v$VERSION"
    echo "=========================================="
    echo ""
    
    check_dependencies
    clean_builds
    
    # Build targets
    declare -A targets=(
        ["linux-x86_64"]="ciary-linux-x86_64"
        ["linux-aarch64"]="ciary-linux-aarch64"
        ["darwin-universal"]="ciary-darwin-universal"
        ["freebsd-x86_64"]="ciary-freebsd-x86_64"
        ["freebsd-aarch64"]="ciary-freebsd-aarch64"
        ["openbsd-x86_64"]="ciary-openbsd-x86_64"
        ["netbsd-x86_64"]="ciary-netbsd-x86_64"
    )
    
    # Build cross-compilation targets
    local success_count=0
    local total_count=${#targets[@]}
    
    for target in "${!targets[@]}"; do
        binary_name=${targets[$target]}
        if build_target "$target" "$binary_name"; then
            case $target in
                linux-x86_64) create_archive "$DIST_DIR/$binary_name" "linux" "x86_64" ;;
                linux-aarch64) create_archive "$DIST_DIR/$binary_name" "linux" "aarch64" ;;
                darwin-universal) create_archive "$DIST_DIR/$binary_name" "darwin" "universal" ;;
                freebsd-x86_64) create_archive "$DIST_DIR/$binary_name" "freebsd" "x86_64" ;;
                freebsd-aarch64) create_archive "$DIST_DIR/$binary_name" "freebsd" "aarch64" ;;
                openbsd-x86_64) create_archive "$DIST_DIR/$binary_name" "openbsd" "x86_64" ;;
                netbsd-x86_64) create_archive "$DIST_DIR/$binary_name" "netbsd" "x86_64" ;;
            esac
            ((success_count++))
        fi
    done
    
    # Build native binary as fallback/additional option
    build_native || log_warn "Native build failed"
    
    echo ""
    echo "=========================================="
    log_info "Build Summary:"
    log_info "Successfully built $success_count out of $total_count cross-compilation targets"
    
    if [ -d "$RELEASE_DIR" ] && [ "$(ls -A "$RELEASE_DIR")" ]; then
        log_success "Release archives created in: $RELEASE_DIR"
        echo ""
        log_info "Available releases:"
        ls -la "$RELEASE_DIR"/*.tar.gz "$RELEASE_DIR"/*.zip 2>/dev/null | awk '{print "  " $9 " (" $5 " bytes)"}' || true
    else
        log_warn "No release archives were created"
    fi
    echo "=========================================="
}

# Handle script arguments
case "${1:-}" in
    -h|--help)
        echo "Usage: $0 [VERSION]"
        echo ""
        echo "Build Ciary for multiple platforms and create release archives."
        echo ""
        echo "Arguments:"
        echo "  VERSION    Version string to use (default: 'dev')"
        echo ""
        echo "Examples:"
        echo "  $0                 # Build with version 'dev'"
        echo "  $0 1.0.0          # Build with version '1.0.0'"
        echo "  $0 v1.2.3-beta    # Build with version 'v1.2.3-beta'"
        exit 0
        ;;
    *)
        main
        ;;
esac