# Contributing to fontlift-win-cli

Thank you for your interest in contributing to fontlift-win-cli! This document provides guidelines for contributing to the project.

## Code of Conduct

- Be respectful and constructive
- Focus on what is best for the project and community
- Show empathy towards other contributors
- Accept constructive criticism gracefully

## How to Contribute

### Reporting Bugs

Before creating a bug report:
1. Check existing issues to avoid duplicates
2. Test with the latest version
3. Verify the bug on a clean Windows installation if possible

When creating a bug report, include:
- Windows version (e.g., Windows 10, Windows 11)
- fontlift-win version
- Exact command used
- Expected behavior
- Actual behavior
- Error messages (if any)
- Steps to reproduce

### Suggesting Features

Feature requests are welcome, but remember:
- fontlift-win is intentionally minimal and focused
- New features must align with the project's core purpose
- Complex features may be rejected to maintain simplicity

When suggesting a feature:
- Explain the problem it solves
- Describe the proposed solution
- Consider whether it fits the project scope
- Check if existing features can solve the problem

### Pull Requests

#### Before You Start

1. Check open issues and PRs to avoid duplicate work
2. For major changes, open an issue first to discuss
3. Read PLAN.md and TODO.md to understand the roadmap
4. Ensure you have a Windows development environment

#### Development Setup

```cmd
# Clone the repository
git clone https://github.com/fontlaborg/fontlift-win-cli.git
cd fontlift-win-cli

# Build the project
build.cmd
```

**Requirements:**
- Windows 7 or later
- Visual Studio 2017 or later (for MSVC)
- Git

#### Coding Standards

**C++ Code Style:**
- C++17 standard
- Functions: maximum 20 lines
- Files: maximum 200 lines
- Indentation: maximum 3 levels
- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters

**Naming Conventions:**
- Functions: `PascalCase()` (e.g., `ListFonts()`)
- Variables: `camelCase` (e.g., `hasPathFlag`)
- Constants: `UPPER_CASE` (e.g., `EXIT_SUCCESS_CODE`)

**File Headers:**
All source files must include:
```cpp
// this_file: path/to/file.cpp
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0
```

Use templates in `templates/` directory for new files.

**Code Quality:**
- No dynamic memory allocation (use stack variables)
- Prefer Windows API over third-party libraries
- Add comments for complex logic
- Handle error cases explicitly
- Validate all inputs

**What NOT to Add:**
- External dependencies (keep it dependency-free)
- Configuration files (use command-line arguments only)
- Logging frameworks
- Analytics or telemetry
- Auto-update mechanisms
- GUI components

See PRINCIPLES.md for complete design philosophy.

#### Testing

Before submitting:
1. Build successfully with no warnings
2. Test all commands manually:
   - `fontlift-win list` with various flags
   - `fontlift-win install` with .ttf, .otf, .ttc files
   - `fontlift-win uninstall` by path and name
   - `fontlift-win remove` by path and name
3. Test edge cases:
   - Filenames with spaces
   - Invalid font files
   - Without administrator privileges
   - Fonts in non-standard locations
4. Verify fonts persist after reboot
5. Check code follows style guidelines

#### Commit Messages

Format:
```
Brief summary (50 chars or less)

Detailed explanation if needed:
- What changed and why
- Impact on existing functionality
- Breaking changes (if any)

Fixes #123
```

**Examples:**
```
Fix font parsing for TTC collections

- Handle multiple fonts in single .ttc file
- Extract all font names correctly
- Add fallback for malformed name tables

Fixes #45
```

```
Add Unicode path support

- Use wide character APIs for file operations
- Handle paths with non-ASCII characters
- Test with Japanese and Chinese filenames
```

#### Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make your changes following the guidelines
4. Update documentation if needed:
   - README.md for user-facing changes
   - CHANGELOG.md for version history
   - Code comments for implementation details
5. Test thoroughly on Windows
6. Commit with clear messages
7. Push to your fork
8. Open a Pull Request

**PR Description Should Include:**
- What: Brief description of changes
- Why: Problem being solved or feature added
- How: Technical approach taken
- Testing: What was tested and how
- Screenshots: For UI changes (if applicable)

#### Review Process

- PRs are reviewed by project maintainers
- Address feedback promptly
- Keep PRs focused and small (easier to review)
- Be patient and respectful during review

## Documentation

### Code Documentation

- Add `this_file` comment to all source files
- Document complex algorithms with comments
- Explain "why" not just "what"
- Keep comments concise and clear

### User Documentation

- Update README.md for new commands or options
- Add examples for new features
- Update troubleshooting section if needed
- Keep language clear and simple

## Project Structure

```
fontlift-win-cli/
├── src/              # Source code
│   └── main.cpp      # Main entry point
├── templates/        # Code templates
├── build.cmd         # Build script
├── publish.cmd       # Distribution packaging
├── README.md         # User documentation
├── PLAN.md           # Implementation roadmap
├── TODO.md           # Task list
├── DEPENDENCIES.md   # Dependency documentation
└── CHANGELOG.md      # Version history
```

## Getting Help

- Check documentation: README.md, PLAN.md, PRINCIPLES.md
- Search existing issues
- Ask in issue discussions
- Be specific about your problem

## License

By contributing, you agree that your contributions will be licensed under the Apache License 2.0.

## Questions?

Open an issue with the `question` label or contact the maintainers.

---

Thank you for contributing to fontlift-win-cli!
