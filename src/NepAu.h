//
// Created by arjun on 7/25/25.
//
#ifndef NEPAU_H
#define NEPAU_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <dlfcn.h>

namespace NepAU {
    void* get_il2cpp();
    bool initialize();
    void* get_class(const char* name_space, const char* type_name);
}

class LoadClass {
    void* thisclass = nullptr;
    void* GetClass(const char* ns, const char* name);
public:
    LoadClass(const char* namespaze, const char* clazz);
    uintptr_t GetMethodOffsetByName(const char* name, int paramcount);
    uintptr_t GetFieldOffset(const char* name);
    void GetStaticFieldValue(const char* name, void *out);
    void SetStaticFieldValue(const char* name, void *out);
};
struct Module {
    uintptr_t start_address;
    uintptr_t end_address;
    intptr_t size;
    std::string name;
};

Module GetModule(std::string module_name);

class Address {
    uintptr_t g_base;
    uintptr_t g_addr;

public:
    Address(uintptr_t base = 0, uintptr_t address = 0);

    uintptr_t offset();
    uintptr_t addr();
    operator uintptr_t() const;
    operator void*() const;
};

class NField {
    std::string name;
    LoadClass thisclass;

public:
    NField(LoadClass thisclass, std::string name);
    uintptr_t offset();
    void getStatic(void *out);
    void setStatic(void *value);
};

class NClass {
    std::string libName;
    LoadClass thisclass;

public:
    NClass(std::string libName, const char *name);
    NClass(std::string libName, const char *nameSpace, const char *name);

    Address method(std::string name, int param = 0);
    NField field(std::string name);
};

class iNepAu {
public:
    virtual ~iNepAu() = default;

    virtual Address get_symbol(const std::string& symbol) = 0;
    virtual NClass get_class(const std::string& clasz) = 0;
    virtual NClass get_class(const std::string& namespaze, const std::string& clasz) = 0;
    virtual Address get_pattern(const std::string& pattern) = 0;

};

enum LibType {
    Unity,
    Native
};
class NativeLibrary : public iNepAu {
    void* handle;
    std::string name;

public:
    explicit NativeLibrary(void* h, std::string name);

    Address get_symbol(const std::string& symbol) override;
    NClass get_class(const std::string& clasz) override;
    NClass get_class(const std::string& namespaze, const std::string& clasz) override;
    Address get_pattern(const std::string& pattern) override;
};

class LibManager : public iNepAu {
    bool success = false;
    std::string name;
    std::unique_ptr<iNepAu> lib;
    LibType libType;

public:
    explicit LibManager(const std::string& libName, LibType libType = Unity);

    Address get_symbol(const std::string& symbol) override;
    NClass get_class(const std::string& clasz) override;
    NClass get_class(const std::string& namespaze, const std::string& clasz) override;
    Address get_pattern(const std::string& pattern) override;

    bool loaded() const;
    bool is_unity() const;
    const std::string& get_name() const;
};

#endif // NEPAU_H
