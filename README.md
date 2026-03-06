# libvbscript

A cross-platform VBScript engine extracted from [Wine](https://www.winehq.org/), packaged as a standalone shared/static library.

## Background

[Visual Pinball](https://github.com/vpinball/vpinball) uses VBScript as its scripting engine for table logic. On Windows, this is handled natively, but cross-platform support (macOS, Linux, iOS, Android) required an alternative. The Wine project's VBScript engine provided a solid foundation, but needed significant bug fixes and missing feature implementations to handle the breadth of real-world VBScript used in pinball tables.

`libvbscript` packages this work as a reusable C library with a clean callback-based API, decoupled from both Wine and Visual Pinball internals.

## What's Included

- **Wine VBScript engine** — compiler, interpreter, lexer, and parser
- **OLE Automation (oleaut32)** — `VARIANT`, `SAFEARRAY`, type coercion, and `VarFormat`
- **ATL support** — minimal ATL stubs for COM class registration
- **VBScript RegExp** — `RegExp`, `Match`, and `Matches` objects via Wine's vbregexp implementation
- **Scripting Runtime (scrrun)** — `Scripting.Dictionary` and `Scripting.FileSystemObject` via Wine's scrrun implementation
- **Locale support (kernelbase)** — Wine's locale handling for string operations

## API

The library exposes a minimal callback-based API:

```c
#include "libvbscript.h"

libvbscript_callbacks_t callbacks = {};
callbacks.log = my_log_handler;
callbacks.create_object = my_create_object;
libvbscript_init(&callbacks);

// ... use IActiveScript / IActiveScriptParse interfaces ...

libvbscript_shutdown();
```

Including `libvbscript.h` automatically defines `__LIBVBSCRIPT__`, which configures the Wine headers for library mode.

## Building

### Prerequisites

- CMake 3.25+
- Bison 3.8.2+
- C99 compiler (gcc, clang, or mingw)

### macOS / Linux

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Windows (MSYS2/mingw)

```bash
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DPLATFORM=win -DARCH=x64 -B build
cmake --build build
```

This produces `vbscript64.dll` and `vbscript64.lib` (MSVC-compatible import library via gendef/dlltool).

### Static Library

```bash
cmake -DBUILD_SHARED=OFF -B build-static
cmake --build build-static
```

### Install

```bash
cmake --install build --prefix /usr/local
```

## Wine Bug Fixes

The following Wine bugs have been fixed (either upstream in Wine or in this library):

- ~~[Bug 53644](https://bugs.winehq.org/show_bug.cgi?id=53644)~~ — vbscript can not compile classes with lists of private / public / dim declarations
- ~~[Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670)~~ — vbscript can not compile if expressions with reversed gte, lte, (=>, =<)
- ~~[Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676)~~ — vbscript can not exec_script - invalid number of arguments for Randomize
- ~~[Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678)~~ — vbscript can not compile CaseClausules that do not use a colon
- ~~[Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766)~~ — vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds
- ~~[Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767)~~ — vbscript fails to handle ReDim when variable is not yet created
- ~~[Bug 53782](https://bugs.winehq.org/show_bug.cgi?id=53782)~~ — vbscript can not compile ReDim with list of variables
- ~~[Bug 53783](https://bugs.winehq.org/show_bug.cgi?id=53783)~~ — vbscript can not compile private const expressions
- ~~[Bug 53807](https://bugs.winehq.org/show_bug.cgi?id=53807)~~ — vbscript fails to redim original array in function when passed byref
- ~~[Bug 53866](https://bugs.winehq.org/show_bug.cgi?id=53866)~~ — vbscript fails to handle SAFEARRAY in for...each
- ~~[Bug 53867](https://bugs.winehq.org/show_bug.cgi?id=53867)~~ — vbscript fails to retrieve property array by index
- ~~[Bug 53868](https://bugs.winehq.org/show_bug.cgi?id=53868)~~ — vbscript fails to return TypeName for VT_DISPATCH
- ~~[Bug 53873](https://bugs.winehq.org/show_bug.cgi?id=53873)~~ — vbscript fails to compile Else If when If is on same line
- ~~[Bug 53888](https://bugs.winehq.org/show_bug.cgi?id=53888)~~ — vbscript does not allow Mid on non VT_BSTR
- ~~[Bug 54234](https://bugs.winehq.org/show_bug.cgi?id=54234)~~ — vbscript fails to compile when colon follows Else in If...Else
- ~~[Bug 54456](https://bugs.winehq.org/show_bug.cgi?id=54456)~~ — vbscript memory leak in For Each with SafeArray as group
- ~~[Bug 54457](https://bugs.winehq.org/show_bug.cgi?id=54457)~~ — vbscript memory leaks in interp_redim_preserve
- ~~[Bug 54458](https://bugs.winehq.org/show_bug.cgi?id=54458)~~ — vbscript memory leaks in Global_Split
- ~~[Bug 54489](https://bugs.winehq.org/show_bug.cgi?id=54489)~~ — vbscript Abs on BSTR returns invalid value
- ~~[Bug 54490](https://bugs.winehq.org/show_bug.cgi?id=54490)~~ — vbscript fails to compile when statement follows ElseIf
- ~~[Bug 54493](https://bugs.winehq.org/show_bug.cgi?id=54493)~~ — vbscript fails to compile concat when used without space and expression begins with H
- ~~[Bug 54731](https://bugs.winehq.org/show_bug.cgi?id=54731)~~ — vbscript: stack_pop_bool doesn't support floats or ole color
- ~~[Bug 54978](https://bugs.winehq.org/show_bug.cgi?id=54978)~~ — vbscript fails to compile Sub when End Sub on same line
- ~~[Bug 55042](https://bugs.winehq.org/show_bug.cgi?id=55042)~~ — IDictionary::Add() fails to add entries with numerical keys that have the same hashes
- ~~[Bug 55052](https://bugs.winehq.org/show_bug.cgi?id=55052)~~ — For loop where right bound is string coercion issue
- ~~[Bug 55185](https://bugs.winehq.org/show_bug.cgi?id=55185)~~ — vbscript round does not handle numdecimalplaces argument
- ~~[Bug 55931](https://bugs.winehq.org/show_bug.cgi?id=55931)~~ — vbscript: empty MOD 100000 returns garbage instead of 0
- ~~[Bug 55969](https://bugs.winehq.org/show_bug.cgi?id=55969)~~ — vbscript fails to return TypeName for Nothing
- ~~[Bug 56139](https://bugs.winehq.org/show_bug.cgi?id=56139)~~ — scrrun: Dictionary does not allow storing at key Undefined
- ~~[Bug 56464](https://bugs.winehq.org/show_bug.cgi?id=56464)~~ — vbscript: Join on array with "empty" items fails
- ~~[Bug 56781](https://bugs.winehq.org/show_bug.cgi?id=56781)~~ — scrrun: Dictionary setting item to object fails
- ~~[Bug 57563](https://bugs.winehq.org/show_bug.cgi?id=57563)~~ — vbscript: mid() throws when passed VT_EMPTY instead of returning empty string

Additionally, support was added for commands that Wine's VBScript engine had marked as `E_NOTIMPL`:
- `Execute`
- `ExecuteGlobal`
- `Eval`
- `GetRef`

## Known Open Issues

The following Wine bugs are still open and may require workarounds:

- [Bug 53844](https://bugs.winehq.org/show_bug.cgi?id=53844) — invoke_vbdisp not handling let property correctly for VT_DISPATCH arguments
- [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877) — compile_assignment assertion when assigning multidimensional array by indices
- [Bug 53889](https://bugs.winehq.org/show_bug.cgi?id=53889) — does not support Get_Item call on IDispatch objects
- [Bug 54177](https://bugs.winehq.org/show_bug.cgi?id=54177) — fails to compile sub call when argument expression contains multiplication
- [Bug 54221](https://bugs.winehq.org/show_bug.cgi?id=54221) — missing support for GetRef
- [Bug 54291](https://bugs.winehq.org/show_bug.cgi?id=54291) — stuck in endless for loop when UBound on Empty and On Error Resume Next
- [Bug 55006](https://bugs.winehq.org/show_bug.cgi?id=55006) — single line if else without else body fails compilation
- [Bug 55037](https://bugs.winehq.org/show_bug.cgi?id=55037) — colon on new line after Then fails
- [Bug 55093](https://bugs.winehq.org/show_bug.cgi?id=55093) — if boolean condition should work without braces
- [Bug 56280](https://bugs.winehq.org/show_bug.cgi?id=56280) — string coerced to Integer instead of Long
- [Bug 56281](https://bugs.winehq.org/show_bug.cgi?id=56281) — string number converted to ascii value instead of parsed value
- [Bug 56480](https://bugs.winehq.org/show_bug.cgi?id=56480) — underscore line continue issues
- [Bug 56931](https://bugs.winehq.org/show_bug.cgi?id=56931) — Const used before declaration fails (explicit)
- [Bug 57511](https://bugs.winehq.org/show_bug.cgi?id=57511) — For loop where loop var is not defined throws error without context
- [Bug 58051](https://bugs.winehq.org/show_bug.cgi?id=58051) — Dictionary direct Keys/Items access causes parse error
- [Bug 58056](https://bugs.winehq.org/show_bug.cgi?id=58056) — directly indexing a Split returns Empty
- [Bug 58248](https://bugs.winehq.org/show_bug.cgi?id=58248) — Me(Idx) fails to compile

## VBScript Quirks and Workarounds

These issues stem from differences in how VBScript is interpreted in the Wine engine compared to native Windows.

### Using `Not` in an `If` statement needs parentheses

```vbscript
' does not work
If isGIOn <> Not IsOff Then

' workaround
If isGIOn <> (Not IsOff) Then
```

See: [Bug 55093](https://bugs.winehq.org/show_bug.cgi?id=55093)

### `Else..End If` on the same line without a colon

```vbscript
' does not work
else keygrad1 = 0 end if

' workaround
else
   keygrad1 = 0
end if
```

### Colon on a new line after `Then`

```vbscript
' does not work
If Keycode = StartGameKey Then
  :pupevent 800
End If

' workaround — remove the colon
If Keycode = StartGameKey Then
  pupevent 800
End If
```

See: [Bug 55037](https://bugs.winehq.org/show_bug.cgi?id=55037)

### Constants must be defined before use

```vbscript
' does not work
Dim x
x = data
Const data = 1

' workaround
Const data = 1
Dim x
x = data
```

### Setting values in a 2D array

This is one of the most common issues. It can be automatically patched by [vpxtool](https://github.com/francisdb/vpxtool).

```vbscript
' does not work
DTArray(i)(4) = DTCheckBrick(Activeball, DTArray(i)(2))

' workaround — use a class with named properties instead
DTArray(i).animate = DTCheckBrick(Activeball, DTArray(i).prim)
```

See: [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877)

### Evals fail when setting a 2D array

```vbscript
' does not work
dy = -1*(EVAL("roachxy" & xx)(1)(roachstep) - EVAL("roachxy" & xx)(1)(roachstep-1))

' workaround
dim roachxy : roachxy = EVAL("roachxy" & xx)
dy = -1*(roachxy(1)(roachstep) - roachxy(1)(roachstep-1))
```

### For loop coercion issue when right bound is a string

```vbscript
' loops incorrectly when num is a string
Dim i, num
num = "16"
For i = 0 To num

' workaround
For i = 0 To CInt(num)
```

See: [Bug 55052](https://bugs.winehq.org/show_bug.cgi?id=55052)

### Execute fails when object does not exist

```vbscript
' does not work
For i = 0 To 127: Execute "Set Lights(" & i & ") = L" & i: Next

' workaround
For i = 0 To 127
    If IsObject(Eval("L" & i)) Then
        Execute "Set Lights(" & i & ") = L" & i
    End If
Next
```

### Trailing `Else` without `End If`

```vbscript
' does not work
If FlasherOnG = False then FlasherTimer3.Enabled = 1: Else
FlasherTimer4.Enabled = 0

' workaround
If FlasherOnG = False then FlasherTimer3.Enabled = 1
FlasherTimer4.Enabled = 0
```

See: [Bug 55006](https://bugs.winehq.org/show_bug.cgi?id=55006)

### `WScript.Shell` is not supported

There is no workaround — `WScript.Shell` and registry access are not available. In most cases the offending lines can simply be commented out.

## Acknowledgments

Wine and the people who support the VBScript engine:
- Robert Wilhelm
- Nikolay Sivov
- Jacek Caban

## License

This library contains code from the [Wine project](https://www.winehq.org/), licensed under the GNU Lesser General Public License (LGPL).
