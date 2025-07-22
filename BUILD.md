# Building Ciary

Simple build instructions for the minimalistic TUI diary application.

## Quick Start

```bash
# Clone and build
git clone https://github.com/git-akihakune/ciary.git
cd ciary
make

# Check what's available
make deps-status
```

## Dependencies

### Required
- **ncurses**: For the TUI interface
- **gcc**: C compiler

### Optional (PDF Export)
- **libharu**: Native PDF generation (recommended)
- **wkhtmltopdf**: HTML-to-PDF converter
- **weasyprint**: Alternative PDF converter

## Installation by Platform

### Ubuntu/Debian
```bash
# Required
sudo apt install libncurses-dev build-essential

# Optional PDF support
sudo apt install libharu-dev wkhtmltopdf

# Build
make
```

### macOS
```bash
# Required
brew install ncurses

# Optional PDF support  
brew install libharu wkhtmltopdf

# Build
make
```

### FreeBSD
```bash
# Required
pkg install ncurses gmake gcc

# Optional PDF support
pkg install libharu wkhtmltopdf

# Build
gmake
```

### Arch Linux
```bash
# Required
sudo pacman -S ncurses base-devel

# Optional PDF support
sudo pacman -S libharu
# wkhtmltopdf from AUR: yay -S wkhtmltopdf

# Build
make
```

## Build Targets

```bash
make              # Default build
make debug        # Debug build with symbols
make release      # Optimized release build
make deps-status  # Check dependencies and show install hints
make clean        # Remove build artifacts
make install      # Install to /usr/local/bin
make help         # Show all available targets
```

## PDF Export Options

Ciary automatically detects and uses the best available PDF method:

1. **libharu** (best): Native generation, fast, no external tools
2. **wkhtmltopdf**: High quality, requires external tool
3. **weasyprint**: Good quality, requires Python
4. **HTML fallback**: Always available

Check what's available: `make deps-status`

## Cross-Platform Builds

```bash
make linux-x86_64     # Linux static binary
make darwin-universal  # macOS universal binary  
make freebsd-x86_64    # FreeBSD binary
```

## Minimal Build

Build with only HTML/Markdown export (no PDF):

```bash
# Install only ncurses
sudo apt install libncurses-dev  # Ubuntu
brew install ncurses             # macOS
pkg install ncurses              # FreeBSD

# Build
make
```

## Development

```bash
# Debug build
make debug

# Run tests
make test

# Check for issues
make deps-status
```

## Troubleshooting

### "ncurses not found"
```bash
# Ubuntu/Debian
sudo apt install libncurses-dev

# macOS
brew install ncurses

# FreeBSD  
pkg install ncurses
```

### "PDF export unavailable"
```bash
# Check status
make deps-status

# Install libharu (recommended)
sudo apt install libharu-dev     # Ubuntu
brew install libharu             # macOS
pkg install libharu              # FreeBSD

# Or install external tools
sudo apt install wkhtmltopdf     # Ubuntu
pip install weasyprint           # Any platform
```

### Build fails
```bash
# Make sure you have build tools
sudo apt install build-essential pkg-config  # Ubuntu
xcode-select --install                       # macOS
pkg install gmake gcc pkgconf                # FreeBSD
```

## What Gets Built

- `ciary`: Main executable
- PDF support: Automatically enabled if libharu is detected
- Export formats: HTML (always), Markdown (always), PDF (if available)

Run `make deps-status` to see exactly what's available on your system.

## Installation

```bash
# Install to system
sudo make install

# Remove from system  
sudo make uninstall

# Or run directly
./ciary
```

The build process is intentionally simple - no complex dependency management needed!