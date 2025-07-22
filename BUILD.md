# Building Ciary

This document provides comprehensive build instructions for Ciary across different platforms and dependency scenarios.

## Quick Start

```bash
# Clone the repository
git clone https://github.com/git-akihakune/ciary.git
cd ciary

# Basic build (HTML and Markdown export only)
make

# Build with native PDF support
make vcpkg-install  # Install dependencies
make               # Build with libHaru support
```

## Dependencies

### Required Dependencies

- **libncurses**: TUI interface
  - Ubuntu/Debian: `sudo apt install libncurses-dev`
  - macOS: `brew install ncurses`
  - FreeBSD: `pkg install ncurses`
  - Arch Linux: `sudo pacman -S ncurses`

### Optional Dependencies (PDF Export)

Ciary supports multiple approaches for PDF export, in order of preference:

1. **libHaru (Recommended)**: Native PDF generation, no external tools needed
2. **wkhtmltopdf**: HTML-to-PDF conversion tool
3. **weasyprint**: Alternative HTML-to-PDF converter

## Build Methods

### Method 1: Native PDF with vcpkg (Recommended)

This method provides the best user experience with native PDF generation.

```bash
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export PATH=$PATH:$(pwd)/vcpkg

# Install dependencies and build
make vcpkg-install
make
```

### Method 2: System Package Manager

Install libHaru through your system package manager:

```bash
# Ubuntu/Debian
sudo apt install libharu-dev libncurses-dev
make

# macOS (with Homebrew)
brew install libharu ncurses
make

# FreeBSD
pkg install libharu ncurses
gmake

# Arch Linux
sudo pacman -S libharu ncurses
make
```

### Method 3: External PDF Tools

Build without native PDF support and use external tools:

```bash
# Install base dependencies only
sudo apt install libncurses-dev  # Ubuntu/Debian
brew install ncurses              # macOS
pkg install ncurses               # FreeBSD

# Build Ciary
make

# Install PDF conversion tools (optional)
sudo apt install wkhtmltopdf      # Ubuntu/Debian
brew install wkhtmltopdf          # macOS
pkg install wkhtmltopdf           # FreeBSD

# Alternative: Python-based converter
pip install weasyprint
```

### Method 4: Minimal Build

Build with only HTML and Markdown export support:

```bash
# Install only ncurses
sudo apt install libncurses-dev

# Build
make
```

## Build Targets

```bash
# Standard builds
make                 # Default build
make debug          # Debug build with symbols
make release        # Optimized release build

# Dependency management
make vcpkg-install  # Install vcpkg dependencies
make vcpkg-status   # Check dependency status

# Cross-compilation
make linux-x86_64    # Linux x86_64 static binary
make darwin-universal # macOS universal binary
make freebsd-x86_64  # FreeBSD x86_64 binary

# Testing
make test           # Run all tests
make test-verbose   # Verbose test output

# Distribution
make dist-all       # Build all distribution targets
make native         # Build with platform suffix

# Maintenance
make clean          # Remove build artifacts
make dist-clean     # Remove all artifacts
make install        # Install to /usr/local/bin
make uninstall      # Remove from system
```

## Platform-Specific Notes

### Ubuntu/Debian

```bash
# Full installation with native PDF
sudo apt update
sudo apt install libncurses-dev pkg-config git cmake ninja-build curl zip unzip tar
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export PATH=$PATH:$(pwd)/vcpkg
make vcpkg-install
make
```

### macOS

```bash
# Install dependencies
brew install ncurses pkg-config git cmake ninja

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export PATH=$PATH:$(pwd)/vcpkg

# Build
make vcpkg-install
make
```

### FreeBSD

```bash
# Install dependencies
pkg install ncurses gmake gcc pkgconf git cmake ninja

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export PATH=$PATH:$(pwd)/vcpkg

# Build
gmake vcpkg-install
gmake
```

### Arch Linux

```bash
# Install dependencies
sudo pacman -S ncurses make gcc pkgconf git cmake ninja

# Option 1: Use AUR package for libharu
yay -S libharu

# Option 2: Use vcpkg
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export PATH=$PATH:$(pwd)/vcpkg
make vcpkg-install

# Build
make
```

## Troubleshooting

### Common Issues

1. **"vcpkg not found"**
   ```bash
   # Install vcpkg manually
   git clone https://github.com/Microsoft/vcpkg.git
   ./vcpkg/bootstrap-vcpkg.sh
   export PATH=$PATH:$(pwd)/vcpkg
   ```

2. **"libharu not found"**
   ```bash
   # Check status
   make vcpkg-status
   
   # Install dependencies
   make vcpkg-install
   ```

3. **"ncurses not found"**
   ```bash
   # Ubuntu/Debian
   sudo apt install libncurses-dev
   
   # macOS
   brew install ncurses
   
   # FreeBSD
   pkg install ncurses
   ```

4. **PDF export fails**
   ```bash
   # Check available PDF tools
   make vcpkg-status
   
   # Install alternative tools
   sudo apt install wkhtmltopdf  # or weasyprint
   ```

### Build Flags

- `HAVE_LIBHARU`: Enables native PDF generation
- `DEBUG`: Enables debug symbols and verbose output
- `NDEBUG`: Disables debug assertions (release builds)

### Environment Variables

- `VCPKG_ROOT`: Path to vcpkg installation
- `VCPKG_TARGET_TRIPLET`: Target platform for vcpkg
- `CC`: C compiler to use
- `CFLAGS`: Additional compiler flags
- `LDFLAGS`: Additional linker flags

## Performance Comparison

| Export Method | Speed | Dependencies | Quality |
|---------------|-------|--------------|---------|
| Native PDF (libHaru) | Fast | libHaru only | High |
| wkhtmltopdf | Medium | External tool | Very High |
| weasyprint | Slow | Python + deps | High |
| HTML only | Very Fast | None | N/A |

## Integration with Package Managers

### Homebrew Formula (macOS)
```ruby
class Ciary < Formula
  desc "Minimalistic TUI diary application"
  homepage "https://github.com/git-akihakune/ciary"
  url "https://github.com/git-akihakune/ciary/releases/latest"
  
  depends_on "ncurses"
  depends_on "libharu"
  
  def install
    system "make", "release"
    bin.install "ciary"
  end
end
```

### AUR Package (Arch Linux)
```bash
# PKGBUILD example
pkgname=ciary
pkgver=1.0.0
depends=('ncurses' 'libharu')
makedepends=('gcc' 'make')

build() {
  cd "$srcdir/$pkgname-$pkgver"
  make release
}

package() {
  cd "$srcdir/$pkgname-$pkgver"
  install -Dm755 ciary "$pkgdir/usr/bin/ciary"
}
```

## Contributing

When building for development:

```bash
# Development build with all features
make vcpkg-install
make debug

# Run tests
make test-all

# Check code with different configurations
make clean && make  # Without libHaru
make clean && make vcpkg-install && make  # With libHaru
```

For more information, see [CONTRIBUTING.md](CONTRIBUTING.md).