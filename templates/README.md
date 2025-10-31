# C++ Source Templates

These templates ensure consistency across all source files in the fontlift-win-cli project.

## Usage

### For Header Files (.h)

```bash
cp templates/header.h src/new_module.h
```

Then replace:
- `[PATH_TO_FILE]` → `src/new_module.h`
- `[HEADER_GUARD]` → `FONTLIFT_NEW_MODULE_H` (uppercase, underscores)

### For Source Files (.cpp)

```bash
cp templates/source.cpp src/new_module.cpp
```

Then replace:
- `[PATH_TO_FILE]` → `src/new_module.cpp`
- `[HEADER_NAME]` → `new_module` (without .h extension)

## Requirements

All source files must include:
1. `this_file` comment with relative path from project root
2. Project description comment
3. Copyright and license notice
4. Appropriate includes

## Example

**header.h after customization:**
```cpp
// this_file: src/font_ops.h
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#ifndef FONTLIFT_FONT_OPS_H
#define FONTLIFT_FONT_OPS_H

void ListFonts(bool showPaths, bool showNames);

#endif // FONTLIFT_FONT_OPS_H
```

**source.cpp after customization:**
```cpp
// this_file: src/font_ops.cpp
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "font_ops.h"
#include <windows.h>
#include <iostream>

void ListFonts(bool showPaths, bool showNames) {
    // Implementation
}
```
