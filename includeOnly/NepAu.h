#ifndef NEPAU_H
#define NEPAU_H

#include <cstdint>
#include <memory>
#include <string>
#include <dlfcn.h>
#include <unistd.h>
#include "macros.h"
#include "UnityApi.h"
#include "vector"

struct Module {
    uintptr_t start_address;
    uintptr_t end_address;
    intptr_t size;
    std::string name;
};
std::vector<std::string> moduleNames;
std::vector<Module> modules;

Module GetModule(std::string module_name) {
    std::vector<std::string>::iterator itr = std::find(moduleNames.begin() , moduleNames.end() , module_name);
    if (itr != moduleNames.end()) {
        int pos = itr - moduleNames.begin();
        return modules[pos];
    }

    std::unique_ptr<FILE, decltype(&fclose)> maps_handle(fopen(("/proc/self/maps"), ("r")), &fclose);
    char line[512], mod_name[64];
    uintptr_t startAddr , endAddr;

    while (fgets(line, sizeof(line), maps_handle.get())) {
        if (std::sscanf(line, ("%x-%x %*s %*ld %*s %*d %s"), &startAddr, &endAddr, &mod_name)) {
            if (std::strstr(mod_name, module_name.c_str())) {
                Module module;
                module.start_address = startAddr;
                module.end_address = endAddr;
                module.size = endAddr - startAddr;
                module.name = module_name;

                moduleNames.push_back(module.name);
                modules.push_back(module);
                return module;
            }
        }
    }
    Module module;
    return module;
}
class Address {
    uintptr_t g_base;
    uintptr_t g_addr;

public:
    Address(uintptr_t base = 0, uintptr_t address = 0)
        : g_base(base), g_addr(address) {}

        uintptr_t offset() {
            return g_addr - g_base;
    }
    uintptr_t addr() {
        return g_addr;
    }
    operator uintptr_t() const {
        return g_addr;
    }

    // Conversion to void*
    operator void*() const {
        return reinterpret_cast<void*>(g_addr);
    }

};

class NField {
    std::string name;
    LoadClass thisclass;
public:
    NField(LoadClass thisclass, std::string name) : thisclass(thisclass), name(name) {

    }
    uintptr_t offset() {
        return thisclass.GetFieldOffset(name.c_str());
    }

    void getStatic(void *out) {
        thisclass.GetStaticFieldValue(name.c_str(), out);
    }
    void setStatic(void *value) {
        thisclass.SetStaticFieldValue(name.c_str(), value);
    }
};

class NClass {
    std::string libName;
    LoadClass thisclass;
public:
    NClass(std::string libName, const char *name) : libName(std::move(libName)), thisclass(LoadClass("", name)) {}
    NClass(std::string libName, const char *nameSpace, const char *name) :libName(std::move(libName)), thisclass(LoadClass(nameSpace, name)) {}

    Address method(std::string name ,int param = 0) {
        return  {getBase(libName.c_str()), thisclass.GetMethodOffsetByName(name.c_str(), param)};
    }

    NField field(std::string name) {
        return {thisclass, std::move(name)};
    }
};

class iNepAu {
public:
    virtual ~iNepAu() = default;

    virtual Address get_symbol(const std::string& symbol) = 0;
    virtual NClass get_class(const std::string& clasz) = 0;
    virtual NClass get_class(const std::string& namespaze, const std::string& clasz) = 0;
    virtual Address get_pattern(const std::string& pattern) = 0;

};

class NativeLibrary : public iNepAu {
    void* handle;
    std::string name;
public:
    explicit NativeLibrary(void* h, std::string name)
        : handle(h), name(std::move(name)) {}

    Address get_symbol(const std::string& symbol) override {
        void* addr = custom_dlsym(handle, symbol.c_str());
        return {getBase(name.c_str()), reinterpret_cast<uintptr_t>(addr)};
    }

    NClass get_class(const std::string& clasz) override {
        return {name, clasz.c_str()};
    }

    NClass get_class(const std::string& namespaze, const std::string& clasz) override {
        return {name, namespaze.c_str(), clasz.c_str()};
    }

    Address get_pattern(const std::string& pattern) override {
        Address ret;
        Module module = GetModule(name);
        if (!module.name.empty()){
            const char* pat = pattern.c_str();
            uint8_t *start = reinterpret_cast<uint8_t *>(module.start_address);
            uint8_t* first_match = 0;

            for (uint8_t *current_byte = start; current_byte < (start + module.size); ++current_byte) {
                if (*pat == '?' || *current_byte == strtoul(pat, NULL, 16)) {
                    if (!first_match) {
                        first_match = current_byte;
                    }

                    if (!pat[2]) {
                        ret = Address(module.start_address, (uintptr_t)first_match);
                        return ret;

                    }
                    pat += *(uint16_t*)pat == 16191 || *pat != '?' ? 3 : 2;

                } else if (first_match) {
                    current_byte = first_match;
                    pat = pattern.c_str();
                    first_match = 0;
                }
            }
            return ret;
        }
        return ret;
    }
};


enum LibType {
    Unity,
    Native
};

class LibManager : public iNepAu {
    bool success = false;
    std::string name;
    std::unique_ptr<iNepAu> lib;
    LibType libType;
public:
    explicit LibManager(const std::string& libName, LibType libType = Unity)
        : name(libName), libType(libType) {
        void *IL2Cpp_Handle = nullptr;

        while (!IL2Cpp_Handle) {
            IL2Cpp_Handle = custom_dlopen(libName.c_str(), 4);
            sleep(1);
        }

        success = (IL2Cpp_Handle != nullptr);

        if(libType == Unity) {
            while(!NepAU::initialize()) {
                sleep(1);
            }
        }
        lib = std::make_unique<NativeLibrary>(IL2Cpp_Handle, libName);
    }

    Address get_symbol(const std::string& symbol) override {
        return lib->get_symbol(symbol);
    }

    NClass get_class(const std::string& clasz) override {
        return lib->get_class(clasz);
    }

    NClass get_class(const std::string& namespaze, const std::string& clasz) override {
        return lib->get_class(namespaze, clasz);
    }

    Address get_pattern(const std::string& pattern) override {
        return lib->get_pattern(pattern);
    }

    bool loaded() const { return success; }
    bool is_unity() const { return libType == Unity; }
    const std::string& get_name() const { return name; }
};

#endif // NEPAU_H
