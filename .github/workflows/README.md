# GitHub Actions CI/CD

This directory contains the GitHub Actions workflows for the Ciary project.

## Workflows Overview

### 1. **CI/CD Pipeline** (`ci.yml`)
**Comprehensive testing and quality assurance workflow**

- **Triggers**: All pushes to any branch, PRs to main/develop
- **Platforms**: Ubuntu, macOS
- **Jobs**:
  - `test-linux` - Ubuntu testing with all test suites
  - `test-macos` - macOS compatibility testing  
  - `code-quality` - Static analysis, formatting, documentation checks
  - `build-artifacts` - Release artifact creation (main branch only)
  - `integration-tests` - Full workflow testing
  - `performance-tests` - Performance benchmarking
  - `security-scan` - Security vulnerability scanning
  - `compatibility-test` - Multi-compiler testing (GCC, Clang)
  - `notify-success` - Success summary

### 2. **Tests** (`test.yml`)
**Fast testing workflow for rapid feedback**

- **Triggers**: Pushes to any branch, PRs to main/develop
- **Platforms**: Ubuntu, macOS  
- **Focus**: Quick build and test execution
- **Features**:
  - All 6 test suites (269 tests total)
  - Code quality checks with cppcheck
  - Multi-compiler support
  - Cross-compilation testing

### 3. **Release** (`release.yml`)
**Automated release builds for multiple platforms**

- **Triggers**: Git tags (`v*`), manual dispatch
- **Platforms**: Ubuntu (x86_64, ARM64), macOS (Universal), FreeBSD
- **Features**:
  - Cross-platform binary builds
  - Automatic release creation
  - Installation instructions included
  - Comprehensive platform support

### 4. **Status Check** (`status.yml`)
**Quick health monitoring**

- **Triggers**: All pushes, PRs, daily schedule
- **Purpose**: Fast build and test verification
- **Features**: Badge-compatible status reporting

## Workflow Features

### 🚀 **Automated Testing**
- **269 comprehensive test cases** across 6 test suites
- **100% test success rate** maintained
- **Multi-platform compatibility** (Linux, macOS, FreeBSD)
- **Cross-compilation support** for multiple architectures

### 🔍 **Quality Assurance**
- **Static analysis** with cppcheck
- **Security scanning** for common vulnerabilities
- **Memory safety** checks
- **Compiler warning** elimination
- **Code formatting** consistency

### 📦 **Release Management**
- **Multi-platform builds** (Linux x86_64/ARM64, macOS Universal, FreeBSD)
- **Automatic release creation** on version tags
- **Installation instructions** included with releases
- **Artifact preservation** for 30 days

### ⚡ **Performance Monitoring**
- **Build time** tracking
- **Test execution** performance
- **Resource usage** monitoring
- **Regression detection**

## Badge URLs

For use in README.md:

```markdown
![CI/CD](https://github.com/[username]/ciary/actions/workflows/ci.yml/badge.svg)
![Tests](https://github.com/[username]/ciary/actions/workflows/test.yml/badge.svg)
![Status](https://github.com/[username]/ciary/actions/workflows/status.yml/badge.svg)
```

## Local Testing

Before pushing, you can run the same tests locally:

```bash
# Quick test (mirrors status.yml)
make clean && make && make test

# Full test suite (mirrors test.yml)
make test-utils
make test-config  
make test-file-io
make test-integration
make test-ui
make test-personalization

# Quality checks
make clean && make CC=clang    # Multi-compiler test
make clean && make CFLAGS+="-Wall -Wextra -Werror"  # Strict warnings
```

## Workflow Triggers

| Event | CI/CD | Tests | Release | Status |
|-------|-------|-------|---------|--------|
| Push (any branch) | ✅ | ✅ | ❌ | ✅ |
| Push (main/develop) | ✅ | ✅ | ❌ | ✅ |
| Pull Request | ✅ | ✅ | ❌ | ✅ |
| Git Tag (`v*`) | ❌ | ❌ | ✅ | ❌ |
| Schedule (daily) | ❌ | ❌ | ❌ | ✅ |
| Manual Dispatch | ❌ | ❌ | ✅ | ❌ |

## Success Metrics

- ✅ **All workflows pass** on every commit
- ✅ **269/269 tests pass** (100% success rate)  
- ✅ **Zero compiler warnings** in all builds
- ✅ **Multi-platform compatibility** maintained
- ✅ **Automated releases** work seamlessly
- ✅ **Security scans** find no issues

This CI/CD setup ensures that Ciary maintains the highest quality standards while providing fast feedback to contributors and reliable releases for users.