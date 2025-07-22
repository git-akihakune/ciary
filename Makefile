# Cross-platform Makefile for Ciary
# Supports native builds and cross-compilation for multiple platforms

# Default compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lncurses

# Build directories
SRCDIR = src
INCDIR = include
TESTDIR = tests
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
DISTDIR = $(BUILDDIR)/dist
TESTOBJDIR = $(BUILDDIR)/test_obj

# Source files and objects
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = ciary

# Test files and objects
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTOBJDIR)/%.o)
TEST_TARGET = $(BUILDDIR)/test_runner

# Library objects (exclude main.o for testing)
LIB_SOURCES = $(filter-out $(SRCDIR)/main.c, $(SOURCES))
LIB_OBJECTS = $(LIB_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Platform-specific settings
ifeq ($(UNAME_S),Darwin)
    PLATFORM = darwin
    # macOS specific flags
    LDFLAGS += -L/opt/homebrew/lib -L/usr/local/lib
    CFLAGS += -I/opt/homebrew/include -I/usr/local/include
else ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
else ifeq ($(UNAME_S),FreeBSD)
    PLATFORM = freebsd
else ifeq ($(UNAME_S),OpenBSD)
    PLATFORM = openbsd
else ifeq ($(UNAME_S),NetBSD)
    PLATFORM = netbsd
else
    PLATFORM = unknown
endif

# Architecture detection
ifeq ($(UNAME_M),x86_64)
    ARCH = x86_64
else ifeq ($(UNAME_M),amd64)
    ARCH = x86_64
else ifeq ($(UNAME_M),arm64)
    ARCH = aarch64
else ifeq ($(UNAME_M),aarch64)
    ARCH = aarch64
else ifeq ($(UNAME_M),i386)
    ARCH = i386
else ifeq ($(UNAME_M),i686)
    ARCH = i386
else
    ARCH = $(UNAME_M)
endif

.PHONY: all clean install uninstall debug release dist-all dist-clean
.PHONY: linux-x86_64 linux-aarch64 darwin-universal freebsd-x86_64 freebsd-aarch64 openbsd-x86_64 netbsd-x86_64
.PHONY: test test-utils test-config test-file-io test-integration test-ui test-personalization test-clean test-all

# Default target
all: $(TARGET)

# Standard build
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TESTOBJDIR):
	mkdir -p $(TESTOBJDIR)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build with optimizations
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

# Test builds and execution
$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c | $(TESTOBJDIR)
	$(CC) $(CFLAGS) -I$(TESTDIR) -c $< -o $@

$(TEST_TARGET): $(LIB_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(LIB_OBJECTS) $(TEST_OBJECTS) -o $@ $(LDFLAGS)

test: $(TEST_TARGET)
	@echo "Running all tests..."
	@$(TEST_TARGET) all

test-utils: $(TEST_TARGET)
	@echo "Running utility function tests..."
	@$(TEST_TARGET) utils

test-config: $(TEST_TARGET)
	@echo "Running configuration system tests..."
	@$(TEST_TARGET) config

test-file-io: $(TEST_TARGET)
	@echo "Running file I/O tests..."
	@$(TEST_TARGET) file_io

test-integration: $(TEST_TARGET)
	@echo "Running integration tests..."
	@$(TEST_TARGET) integration

test-ui: $(TEST_TARGET)
	@echo "Running UI/UX tests..."
	@$(TEST_TARGET) ui

test-personalization: $(TEST_TARGET)
	@echo "Running personalization system tests..."
	@$(TEST_TARGET) personalization

test-verbose: $(TEST_TARGET)
	@echo "Running all tests (verbose)..."
	@$(TEST_TARGET) -v all

test-clean:
	rm -rf $(TESTOBJDIR) $(TEST_TARGET)

test-all: clean test

# Cross-compilation targets

# Linux x86_64
linux-x86_64: CC = x86_64-linux-gnu-gcc
linux-x86_64: LDFLAGS = -lncurses -static
linux-x86_64: TARGET = $(DISTDIR)/ciary-linux-x86_64
linux-x86_64: CFLAGS += -O2 -DNDEBUG
linux-x86_64: $(DISTDIR)/ciary-linux-x86_64

# Linux aarch64
linux-aarch64: CC = aarch64-linux-gnu-gcc
linux-aarch64: LDFLAGS = -lncurses
linux-aarch64: TARGET = $(DISTDIR)/ciary-linux-aarch64
linux-aarch64: CFLAGS += -O2 -DNDEBUG
linux-aarch64: $(DISTDIR)/ciary-linux-aarch64

# macOS universal binary (x86_64 + arm64)
darwin-universal: $(DISTDIR)/ciary-darwin-universal

$(DISTDIR)/ciary-darwin-universal: $(SOURCES) | $(DISTDIR)
	# Build x86_64 version
	$(CC) -arch x86_64 $(CFLAGS) -O2 -DNDEBUG $(SOURCES) -o $(BUILDDIR)/ciary-x86_64 $(LDFLAGS)
	# Build arm64 version
	$(CC) -arch arm64 $(CFLAGS) -O2 -DNDEBUG $(SOURCES) -o $(BUILDDIR)/ciary-arm64 $(LDFLAGS)
	# Create universal binary
	lipo -create $(BUILDDIR)/ciary-x86_64 $(BUILDDIR)/ciary-arm64 -output $@
	# Clean up intermediate files
	rm -f $(BUILDDIR)/ciary-x86_64 $(BUILDDIR)/ciary-arm64

# FreeBSD x86_64
freebsd-x86_64: CC = x86_64-unknown-freebsd-gcc
freebsd-x86_64: LDFLAGS = -lncurses
freebsd-x86_64: TARGET = $(DISTDIR)/ciary-freebsd-x86_64
freebsd-x86_64: CFLAGS += -O2 -DNDEBUG
freebsd-x86_64: $(DISTDIR)/ciary-freebsd-x86_64

# FreeBSD aarch64
freebsd-aarch64: CC = aarch64-unknown-freebsd-gcc
freebsd-aarch64: LDFLAGS = -lncurses
freebsd-aarch64: TARGET = $(DISTDIR)/ciary-freebsd-aarch64
freebsd-aarch64: CFLAGS += -O2 -DNDEBUG
freebsd-aarch64: $(DISTDIR)/ciary-freebsd-aarch64

# OpenBSD x86_64
openbsd-x86_64: CC = x86_64-unknown-openbsd-gcc
openbsd-x86_64: LDFLAGS = -lncurses
openbsd-x86_64: TARGET = $(DISTDIR)/ciary-openbsd-x86_64
openbsd-x86_64: CFLAGS += -O2 -DNDEBUG
openbsd-x86_64: $(DISTDIR)/ciary-openbsd-x86_64

# NetBSD x86_64
netbsd-x86_64: CC = x86_64-unknown-netbsd-gcc
netbsd-x86_64: LDFLAGS = -lncurses
netbsd-x86_64: TARGET = $(DISTDIR)/ciary-netbsd-x86_64
netbsd-x86_64: CFLAGS += -O2 -DNDEBUG
netbsd-x86_64: $(DISTDIR)/ciary-netbsd-x86_64

# Generic cross-compilation rule
$(DISTDIR)/ciary-%: $(SOURCES) | $(DISTDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

# Create distribution directory
$(DISTDIR):
	mkdir -p $(DISTDIR)

# Build all distribution targets
dist-all: linux-x86_64 linux-aarch64 darwin-universal freebsd-x86_64 freebsd-aarch64 openbsd-x86_64 netbsd-x86_64

# Native build with platform suffix
native: release | $(DISTDIR)
	cp $(TARGET) $(DISTDIR)/ciary-$(PLATFORM)-$(ARCH)

# Clean targets
clean:
	rm -rf $(BUILDDIR) $(TARGET)

dist-clean: clean
	rm -rf $(DISTDIR)

# Installation targets
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Help target
help:
	@echo "Ciary Build System"
	@echo "=================="
	@echo ""
	@echo "Native builds:"
	@echo "  all           - Build for current platform"
	@echo "  debug         - Build with debug symbols"
	@echo "  release       - Build optimized release"
	@echo "  native        - Build release with platform suffix"
	@echo ""
	@echo "Testing:"
	@echo "  test          - Run all tests"
	@echo "  test-utils    - Run utility function tests"
	@echo "  test-config   - Run configuration tests"
	@echo "  test-file-io  - Run file I/O tests"
	@echo "  test-integration - Run integration tests"
	@echo "  test-ui       - Run UI/UX tests"
	@echo "  test-personalization - Run personalization tests"
	@echo "  test-verbose  - Run all tests with verbose output"
	@echo "  test-clean    - Clean test artifacts"
	@echo "  test-all      - Clean build and run all tests"
	@echo ""
	@echo "Cross-compilation:"
	@echo "  linux-x86_64    - Build for Linux x86_64"
	@echo "  linux-aarch64   - Build for Linux ARM64"
	@echo "  darwin-universal - Build universal macOS binary"
	@echo "  freebsd-x86_64  - Build for FreeBSD x86_64"
	@echo "  freebsd-aarch64 - Build for FreeBSD ARM64"
	@echo "  openbsd-x86_64  - Build for OpenBSD x86_64"
	@echo "  netbsd-x86_64   - Build for NetBSD x86_64"
	@echo ""
	@echo "Distribution:"
	@echo "  dist-all      - Build all distribution targets"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean         - Remove build artifacts"
	@echo "  dist-clean    - Remove build and dist artifacts"
	@echo "  install       - Install to /usr/local/bin"
	@echo "  uninstall     - Remove from /usr/local/bin"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Current platform: $(PLATFORM)-$(ARCH)"