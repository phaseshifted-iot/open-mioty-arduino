# Contributing to OpenMioty

Thank you for your interest in contributing to OpenMioty! This document provides guidelines for contributing to the project.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [License](#license)

## Code of Conduct

This project aims to be welcoming and inclusive. Please be respectful and constructive in all interactions.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates.

**Good bug reports include:**
- Clear, descriptive title
- Steps to reproduce the issue
- Expected vs actual behavior
- Hardware/software versions (board, Arduino IDE/PlatformIO, library version)
- Code snippets or examples demonstrating the issue
- Serial output or error messages

**Create a bug report:** [New Issue](https://github.com/phaseshifted-iot/open-mioty-arduino/issues/new)

### Suggesting Enhancements

Enhancement suggestions are welcome! Please provide:
- Clear use case and benefit
- Proposed implementation approach (if applicable)
- Compatibility considerations

### Code Contributions

We welcome contributions in these areas:

1. **Platform Support**
   - RP2040, STM32, nRF52, ATmega implementations
   - See ARCHITECTURE.md for porting guide

2. **Radio Drivers**
   - RFM69HW, CC1101, or other mioty-compatible chips
   - Must follow existing driver interface

3. **Board Configurations**
   - New board pin mappings and templates
   - Located in `src/platform/<mcu>/<board>/`

4. **Examples**
   - Additional use cases and demonstrations
   - Well-commented, tested code

5. **Documentation**
   - Improvements to README, ARCHITECTURE, or code comments
   - Tutorials and guides

6. **Testing**
   - Validation on different hardware
   - Regional profile testing (EU, US, IN)

## Development Setup

### Prerequisites

- **Arduino IDE** 1.8.x or 2.x, or **PlatformIO**
- **Hardware**: Supported board (Arduino Nesso N1 recommended)
- **Git** for version control

### Getting Started

1. **Fork the repository**
   ```bash
   # Click "Fork" on GitHub, then:
   git clone https://github.com/YOUR-USERNAME/open-mioty-arduino.git
   cd open-mioty-arduino
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow coding standards below
   - Test thoroughly on hardware if possible

4. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add feature: brief description"
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create Pull Request**
   - Go to your fork on GitHub
   - Click "New Pull Request"
   - Fill in the template

## Coding Standards

### C++ Style

- **Indentation**: 4 spaces (no tabs)
- **Naming**:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Variables: `camelCase` or `snake_case`
- **Comments**: Use Doxygen-style for public APIs
- **Line length**: Prefer < 100 characters

### Example:
```cpp
/**
 * @brief Brief function description
 * 
 * Detailed description if needed.
 * 
 * @param paramName Parameter description
 * @return Return value description
 */
int16_t myFunction(uint8_t paramName) {
    // Implementation
    return 0;
}
```

### Architecture Guidelines

- **Layered design**: Respect the existing layer separation
  - Platform (MCU-specific)
  - Board (pin mappings)
  - Driver (radio chips)
  - Regional (frequency profiles)
  - Application (user API)

- **No hardcoded values**: Use regional profile constants
- **Template-based configuration**: Follow existing patterns
- **Minimal dependencies**: Keep platform layer independent

### Logging

- **Production code**: NO verbose Serial.print statements
- **Errors only**: Log critical failures
- **Examples**: User-facing logging is OK
- **Optional debug**: Use `#ifdef OPENMIOTY_DEBUG` if needed

### Testing

- **Hardware testing**: Test on actual hardware when possible
- **Compilation**: Verify compilation for all supported platforms
- **Examples**: Ensure examples compile and run
- **No regressions**: Verify existing functionality still works

## Pull Request Process

1. **Update documentation**
   - Update README.md if adding features
   - Add comments to new code
   - Update ARCHITECTURE.md for structural changes

2. **Update CHANGELOG.md**
   - Add entry under "Unreleased" section
   - Use format: `### Added/Changed/Fixed`

3. **Ensure clean commit history**
   - Squash trivial commits if needed
   - Use descriptive commit messages

4. **Fill PR template**
   - Describe what and why
   - Reference any related issues
   - List testing performed

5. **Address review feedback**
   - Respond to comments
   - Make requested changes
   - Re-request review when ready

6. **Merge criteria**
   - At least one approving review
   - All CI checks passing (if applicable)
   - No merge conflicts
   - Follows contribution guidelines

## File Structure

```
open-mioty-arduino/
├── src/
│   ├── OpenMioty.h          # Main header
│   ├── core/                # Fraunhofer TS-UNB protocol (do not modify)
│   ├── drivers/             # Radio chip drivers
│   ├── platform/            # MCU-specific code
│   │   └── <mcu>/
│   │       └── <board>/     # Board configurations
│   └── regional/            # Frequency profiles
├── examples/                # Example sketches
├── extras/                  # Documentation, datasheets
├── ARCHITECTURE.md          # Technical design docs
├── CHANGELOG.md             # Version history
└── README.md                # Main documentation
```

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (Third-Party Modified Version of the Fraunhofer TS-UNB-Lib).

**Important**: This library is for academic and non-commercial use only. Commercial use of mioty® technology requires patent licenses from [Sisvel International S.A.](https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)

## Questions?

- **GitHub Discussions**: For questions and community support
- **GitHub Issues**: For bug reports and feature requests
- **Email**: Not monitored - please use GitHub

## Recognition

Contributors will be acknowledged in:
- README.md acknowledgments section
- Git commit history
- GitHub contributors page

Thank you for helping improve OpenMioty! 🎉
