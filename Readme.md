# 🧠 NepAu


[![License](https://img.shields.io/github/license/nepmods/NepAu)](https://github.com/nepmods/NepAu/blob/master/LICENSE)
[![Last Commit](https://img.shields.io/github/last-commit/nepmods/NepAu)](https://github.com/nepmods/NepAu/commits/main)
[![Release](https://img.shields.io/github/v/release/nepmods/NepAu)](https://github.com/nepmods/NepAu/releases)
[![Issues](https://img.shields.io/github/issues/nepmods/NepAu)](https://github.com/nepmods/NepAu/issues)
[![Stars](https://img.shields.io/github/stars/nepmods/NepAu)](https://github.com/nepmods/NepAu/stargazers)

**NepAu** (Native Lib for Auto-Update) is a runtime dynamic analysis library designed to interface with Unity IL2CPP binaries and native libraries (`.so`). Ideal for reverse engineering, modding, and automation, it provides an elegant and consistent interface to access classes, fields, methods, symbols, and patterns in memory.

---

## 📦 Repository

- 📁 **Repo Name**: `NepAu`
- 🌐 **URL**: [https://github.com/nepmods/NepAu](https://github.com/nepmods/NepAu)
- 👤 **Author**: [@nepmods / arjun](https://github.com/nepmods)

---

## ✨ Features

- 🧠 IL2CPP class/method/field accessor
- 🔍 Pattern scanning via memory range
- 📜 Symbol resolver with offset calculation
- 📚 Module metadata handling (`start`, `end`, `size`)
- 🔄 Dynamic loader with retry and fallback
- 🧪 Support for Unity & custom libraries

---

## 🏗️ Instructions

## Include Only
This Project can be used by just including one file

```c++
#include "NepAU/includeOnly/NepAU.h
```
## Source
This Project can also be used by building source file
put this on your cmakelist/android.mk
```
    NepAU/src/NepAu.cpp
```

```c++
#include "NepAU/src/NepAu.h
```
---

## 🔧 Dependencies

| Component     | Version     |
|---------------|-------------|
| C++ Compiler  | C++17+      |
| Android NDK   | r25+        |
| Kernel (Linux)| ≥ 4.15      |
| Unity IL2CPP  | Supported   |
| libc / POSIX  | Required    |

---

## 🧰 Example Usage

```cpp

LibManager manager("libil2cpp.so");

if (manager.loaded()) {
    NClass playerClass = manager.get_class("PlayerControl");

    Address killMethod = playerClass.method("MurderPlayer", 1);
    NField nameField = playerClass.field("playerName");

    uintptr_t offset = nameField.offset();

    std::string playerName;
    nameField.getStatic(&playerName);

    LOGI("Kill method at: 0x%lx", (uintptr_t)killMethod);
    LOGI("Name offset: %lx", offset);
}
```

---

## 📘 API Overview

### 🔹 `LibManager`
| Method | Description |
|--------|-------------|
| `get_symbol(name)` | Resolve dlsym symbol to absolute address |
| `get_class(className)` | Get class in global namespace |
| `get_class(namespace, className)` | Get class in specific namespace |
| `get_pattern(pattern)` | Search memory for hex pattern |
| `loaded()` | Returns `true` if library is loaded |
| `is_unity()` | Returns if library type is `Unity` |

### 🔹 `NClass`
| Method | Description |
|--------|-------------|
| `method(name, paramCount)` | Get method address |
| `field(name)` | Get field accessor |

### 🔹 `NField`
| Method | Description |
|--------|-------------|
| `offset()` | Get offset of field |
| `getStatic(out)` | Get static field value |
| `setStatic(value)` | Set static field value |

---

## 📦 Releases

| Version | Date       | Description        |
|---------|------------|--------------------|
| `v1.0.0` | 2025-07-25 | Initial public release |


---

## 🧪 Testing

Run inside your injected project or loader environment:
-  Dobby-based mods
- Android shared object loaders
- Unity IL2CPP hooks (e.g. ModMenu)

---

## 🙋 Contributing

We welcome contributions!

1. 🍴 Fork the repo
2. 🔧 Create a feature branch (`feature/something`)
3. ✅ Commit your changes
4. 🚀 Open a Pull Request

📜 Please ensure changes are clean and well-documented.

---

## 📄 License

This project is licensed under the **MIT License**.  
See the [LICENSE](https://github.com/nepmods/NepAu/blob/main/LICENSE) file for details.

---

## 🧠 Credits

- Developed by [@nepmods / arjun](https://github.com/nepmods)
- Inspired by IL2CPP reverse engineering community
- With appreciation to:
    - [Frida](https://frida.re/)
    - [Dobby](https://github.com/jmpews/Dobby)
    - [xDL](https://github.com/ViRb3/xDL)

---

## 📫 Contact

For issues or feature requests:  
👉 [Open an issue](https://github.com/nepmods/NepAu/issues)  
Or email: `nepmods@proton.me` *(example email, customize as needed)*

---
