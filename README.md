# Ciary

**A minimalistic TUI (Terminal User Interface) diary application written in C**

Ciary helps you maintain a personal journal with a beautiful calendar interface, automatic time tracking, and seamless integration with your favorite text editor. Your entries are stored as plain Markdown files on your local machine - no cloud, no tracking, just your thoughts.

<img width="752" height="567" alt="Screenshot 2025-07-22 at 17 03 43" src="https://github.com/user-attachments/assets/b8203761-acbf-4ef4-bc0d-f580af884d3b" />

## Features

- **Beautiful Calendar View**: Navigate through months and years with intuitive keyboard controls
- **Smart Time Tracking**: Automatic timestamps for today's entries, custom times for other dates
- **Your Editor, Your Way**: Integrates with nvim, vim, nano, emacs, or vi
- **Read-Only Viewing**: Browse past entries without accidentally editing them
- **Personalized Experience**: Custom welcome messages with seasonal flair
- **Organized Storage**: One Markdown file per day with time-based entry sections
- **Configurable**: Customize directories, editors, and personalization settings
- **Private & Local**: All data stays on your machine
And more!

## Quick Start

### For End Users

1. **Download the latest release**:
   - Go to the [Releases page](https://github.com/git-akihakune/ciary/releases)
   - Download the `ciary` binary for your platform
   - Make it executable: `chmod +x ciary`
   - (Optional) Move to your PATH: `sudo mv ciary /usr/local/bin/`

2. **Install dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt install libncurses-dev
   
   # macOS (with Homebrew)
   brew install ncurses
   
   # Arch Linux
   sudo pacman -S ncurses
   ```

3. **Run Ciary**:
   ```bash
   ciary
   ```

4. **First-time setup**: Ciary will guide you through initial configuration on first run.

### Quick Usage

- **Navigate**: Use arrow keys to move through the calendar
- **Quick month/year navigation**: 
  - `[` / `]` or Page Up/Down for months
  - `<` / `>` or `,` / `.` for years
- **Create entry**: Press `Enter` (or `n` for non-nano editors) on any date
- **View entries**: Press `v` to read existing entries
- **Help**: Press `h` for full help

## How It Works

### Entry Format
Each day gets its own Markdown file (`YYYY-MM-DD.md`) with time-based sections:

```markdown
# 2024-01-15

## 09:30:45

Morning thoughts and plans for the day...

## 14:22:10

Afternoon update...

## 21:15:33

Evening reflection...
```

### Smart Time Handling
- **Today's entries**: Automatically use current time
- **Other dates**: Prompts you to specify the time (HH:MM or HH:MM:SS format)

### File Organization
- **Configuration**: `~/.config/ciary/config.conf`
- **Default journal location**: `~/Documents/journal` (customizable)
- **XDG compliant**: Follows Unix standards for file organization

## Configuration

Ciary creates a configuration file at `~/.config/ciary/config.conf`:

```ini
# Your preferred name (how Ciary addresses you)
preferred_name=Alice

# Directory where journal entries are stored
journal_directory=/home/alice/Documents/journal

# Preferred text editor (auto, nvim, vim, nano, emacs, vi)
editor_preference=auto

# Preferred file viewer (auto, less, more, cat)
viewer_preference=auto

# Show ASCII art on startup (true/false)
show_ascii_art=true

# Enable personalized messages (true/false)
enable_personalization=true
```

## Development

### Prerequisites

- C compiler (GCC or Clang)
- ncurses development library
- make

### Build from Source

1. **Clone the repository**:
   ```bash
   git clone https://github.com/username/ciary.git
   cd ciary
   ```

2. **Install development dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential libncurses-dev
   
   # macOS (with Homebrew)
   brew install ncurses
   xcode-select --install
   
   # Arch Linux
   sudo pacman -S base-devel ncurses
   ```

3. **Build**:
   ```bash
   make
   ```

4. **Run locally**:
   ```bash
   ./ciary
   ```

### Development Commands

```bash
# View all available build targets
make help

# Standard builds
make                    # Build for current platform
make debug             # Build with debug symbols
make release           # Build optimized release

# Cross-platform builds
make linux-x86_64      # Linux x86_64 (static)
make linux-aarch64     # Linux ARM64 (static)
make darwin-universal   # macOS Universal Binary (x86_64 + ARM64)
make freebsd-x86_64    # FreeBSD x86_64
make freebsd-aarch64   # FreeBSD ARM64
make openbsd-x86_64    # OpenBSD x86_64
make netbsd-x86_64     # NetBSD x86_64

# Distribution builds
make native            # Build native with platform suffix
make dist-all          # Build all cross-platform targets

# Maintenance
make clean             # Remove build artifacts
make dist-clean        # Remove build and distribution artifacts
sudo make install      # Install system-wide
sudo make uninstall    # Remove installation
```

### Release Building

For creating release archives:

```bash
# Build release archives for all platforms
./build-release.sh v1.0.0

# Build with custom version
./build-release.sh v1.2.3-beta

# Build with default version (dev)
./build-release.sh
```

This creates compressed archives in `build/release/` with:
- Binaries for all supported platforms
- Installation instructions
- Documentation (README, LICENSE)

### Automated Releases

Ciary uses GitHub Actions for automated releases:

- **Trigger**: Push a version tag (e.g., `git tag v1.0.0 && git push --tags`)
- **Builds**: Automatically builds for all supported platforms
- **Release**: Creates GitHub release with binaries and changelog
- **Formats**: Provides both `.tar.gz` and `.zip` archives

### Project Structure

```
ciary/
├── src/                    # Source code
│   ├── main.c              # Application entry point
│   ├── calendar.c          # Calendar view and navigation
│   ├── file_io.c           # File operations and editor integration
│   ├── config.c            # Configuration management
│   └── utils.c             # Utilities and helper functions
├── include/                # Header files
│   └── ciary.h             # Main header with declarations
├── build/                  # Build artifacts (created during build)
│   ├── obj/               # Object files
│   ├── dist/              # Cross-platform binaries
│   └── release/           # Release archives
├── .github/workflows/      # GitHub Actions CI/CD
│   └── release.yml        # Automated release workflow
├── Makefile               # Advanced cross-platform build system
├── build-release.sh       # Release building script
├── CLAUDE.md             # Project documentation for AI assistants
├── LICENSE               # MIT License
└── README.md             # This file
```

### Cross-Platform Support

Ciary supports multiple platforms and architectures:

#### Supported Platforms
- **Linux**: x86_64, ARM64 (aarch64) - Static binaries for maximum compatibility
- **macOS**: Universal binaries (Intel x86_64 + Apple Silicon ARM64)
- **FreeBSD**: x86_64, ARM64
- **OpenBSD**: x86_64
- **NetBSD**: x86_64

#### Build Requirements for Cross-Compilation
Cross-compilation requires appropriate toolchains:

```bash
# Ubuntu/Debian - Install cross-compilers
sudo apt install gcc-aarch64-linux-gnu

# The build system gracefully handles missing cross-compilers
# and builds what's available
```

### Architecture

Ciary follows a modular design:

- **Calendar Module**: Handles month display, date navigation, and user input
- **File I/O Module**: Manages diary files and external editor/viewer integration
- **Configuration Module**: Handles settings, first-run setup, and XDG compliance
- **Utilities Module**: Date/time functions and personalized messaging system
- **Build System**: Advanced Makefile with cross-platform support and CI/CD integration

## Contributing

All contributions welcomed! Here's how you can help:

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes**: Follow the existing code style
4. **Test thoroughly**: Ensure your changes don't break existing functionality
5. **Commit with clear messages**: `git commit -m "Add amazing feature"`
6. **Push and create a Pull Request**

### Code Guidelines

- Follow existing C style and conventions
- Keep functions focused and well-commented
- Ensure memory safety and proper error handling
- Test on multiple platforms when possible

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- **Documentation**: Check the built-in help (`h` key in Ciary)
- **Issues**: [GitHub Issues](https://github.com/git-akihakune/ciary/issues)
- **Discussions**: [GitHub Discussions](https://github.com/git-akihakune/ciary/discussions)

## Roadmap

- [x] **Cross-platform binary releases** - Automated builds for Linux, macOS, and BSD variants
- [ ] Weekly and yearly calendar views
- [ ] Entry search functionality
- [ ] Export capabilities (PDF, HTML)
- [ ] Entry templates
- [ ] Mood tracking integration
- [ ] Plugin system for extensions
- [ ] Encrypted storage option
- [ ] Entry synchronization between devices

...

---

**Happy journaling! 📝✨**

*Ciary - Your thoughts, your way, your privacy.*
