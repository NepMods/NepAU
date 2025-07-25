//
// Created by arjun on 7/25/25.
//

#include "NepAu.h"
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>
#include "macros.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <algorithm>

uintptr_t  getBase(const char *library) {
        char filename[0xFF] = {0},
                buffer[1024] = {0};
        FILE *fp = NULL;
        uintptr_t address = 0;

        sprintf(filename, "/proc/self/maps");
        fp = fopen(filename, "rt");
        if (fp == NULL) {
            perror("fopen");
            goto done;
        }
        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strstr(buffer, library)) {
                address = (uintptr_t) strtoul(buffer, NULL, 16);
                goto done;
            }
        }
        done:
        if (fp) {
            fclose(fp);
        }
        return address;
}

namespace NepAUApi {
    typedef void* (*class_from_name_t)(const void* assembly, const char* name_space, const char* name);
    typedef void** (*class_get_method_from_name_t)(void* klass, const char* name, int paramcount);
    typedef void* (*domain_get_t)();
    typedef const void** (*domain_get_assemblies_t)(const void* domain, size_t* size);
    typedef const void* (*assembly_get_image_t)(const void* assembly);
    typedef const void* (*domain_assembly_open_t)(void* domain, const char* name);
    typedef void* (*class_get_field_from_name_t)(void* klass, const char* name);
    typedef size_t (*field_get_offset_t)(void* fieldInfo);
    typedef void (*il2cpp_field_static_value)(void *field, void *value);

    inline class_from_name_t             class_from_name             = nullptr;
    inline class_get_method_from_name_t  class_get_method_from_name  = nullptr;
    inline domain_get_t                  domain_get                  = nullptr;
    inline domain_get_assemblies_t       domain_get_assemblies       = nullptr;
    inline assembly_get_image_t          assembly_get_image          = nullptr;
    inline domain_assembly_open_t        domain_assembly_open        = nullptr;
    inline class_get_field_from_name_t   class_get_field_from_name   = nullptr;
    inline field_get_offset_t            field_get_offset            = nullptr;
    inline il2cpp_field_static_value     field_get_static            = nullptr;
    inline il2cpp_field_static_value     field_set_static            = nullptr;
}



namespace NepAU {
    void* get_il2cpp() {
        void *IL2Cpp_Handle = nullptr;
        while (!IL2Cpp_Handle) {
            IL2Cpp_Handle = custom_dlopen("libil2cpp.so", 4);
            sleep(1);
        }
        return IL2Cpp_Handle;
    }

    bool initialize() {
        void* il2cpp = get_il2cpp();
        if (!il2cpp) return false;

        using namespace NepAUApi;

        class_from_name             = reinterpret_cast<class_from_name_t>(custom_dlsym(il2cpp, "il2cpp_class_from_name"));
        class_get_method_from_name  = reinterpret_cast<class_get_method_from_name_t>(custom_dlsym(il2cpp, "il2cpp_class_get_method_from_name"));
        domain_get                  = reinterpret_cast<domain_get_t>(custom_dlsym(il2cpp, "il2cpp_domain_get"));
        domain_get_assemblies       = reinterpret_cast<domain_get_assemblies_t>(custom_dlsym(il2cpp, "il2cpp_domain_get_assemblies"));
        assembly_get_image          = reinterpret_cast<assembly_get_image_t>(custom_dlsym(il2cpp, "il2cpp_assembly_get_image"));
        domain_assembly_open        = reinterpret_cast<domain_assembly_open_t>(custom_dlsym(il2cpp, "il2cpp_domain_assembly_open"));
        class_get_field_from_name   = reinterpret_cast<class_get_field_from_name_t>(custom_dlsym(il2cpp, "il2cpp_class_get_field_from_name"));
        field_get_offset            = reinterpret_cast<field_get_offset_t>(custom_dlsym(il2cpp, "il2cpp_field_get_offset"));
        field_get_static            = reinterpret_cast<il2cpp_field_static_value>(custom_dlsym(il2cpp, "il2cpp_field_static_get_value"));
        field_set_static            = reinterpret_cast<il2cpp_field_static_value>(custom_dlsym(il2cpp, "il2cpp_field_static_set_value"));

        return class_from_name && class_get_method_from_name && domain_get &&
               domain_get_assemblies && assembly_get_image &&
               domain_assembly_open && class_get_field_from_name &&
               field_get_offset;
    }

    void* get_class(const char* name_space, const char* type_name) {
        using namespace NepAUApi;
        void* domain = domain_get();
        if (!domain) return nullptr;

        size_t count = 0;
        const void** assemblies = domain_get_assemblies(domain, &count);
        for (size_t i = 0; i < count; ++i) {
            const void* image = assembly_get_image(assemblies[i]);
            if (!image) continue;
            void* klass = class_from_name(image, name_space, type_name);
            if (klass) return klass;
        }
        return nullptr;
    }
}

void* LoadClass::GetClass(const char* ns, const char* name) {
    return NepAU::get_class(ns, name);
}

LoadClass::LoadClass(const char* namespaze, const char* clazz) {
    do {
        thisclass = GetClass(namespaze, clazz);
    } while (!thisclass);
}

uintptr_t LoadClass::GetMethodOffsetByName(const char* name, int paramcount) {
    return (uintptr_t) *NepAUApi::class_get_method_from_name(thisclass, name, paramcount);
}

uintptr_t LoadClass::GetFieldOffset(const char* name) {
    if (!thisclass) return 0;
    void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
    if (!fieldInfo) return 0;
    return (uintptr_t) NepAUApi::field_get_offset(fieldInfo);
}

void LoadClass::GetStaticFieldValue(const char* name, void *out) {
    if (!thisclass) return;
    void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
    if (!fieldInfo) return;
    NepAUApi::field_get_static(fieldInfo, out);
}

void LoadClass::SetStaticFieldValue(const char* name, void *out) {
    if (!thisclass) return;
    void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
    if (!fieldInfo) return;
    NepAUApi::field_set_static(fieldInfo, out);
}


std::vector<std::string> moduleNames;
std::vector<Module> modules;

Module GetModule(std::string module_name) {
    auto itr = std::find(moduleNames.begin(), moduleNames.end(), module_name);
    if (itr != moduleNames.end()) {
        int pos = itr - moduleNames.begin();
        return modules[pos];
    }

    std::unique_ptr<FILE, decltype(&fclose)> maps_handle(fopen("/proc/self/maps", "r"), &fclose);
    char line[512], mod_name[64];
    uintptr_t startAddr, endAddr;

    while (fgets(line, sizeof(line), maps_handle.get())) {
        if (std::sscanf(line, "%x-%x %*s %*ld %*s %*d %s", &startAddr, &endAddr, mod_name)) {
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
    return {};
}

// Address class
Address::Address(uintptr_t base, uintptr_t address)
        : g_base(base), g_addr(address) {}

uintptr_t Address::offset() { return g_addr - g_base; }
uintptr_t Address::addr() { return g_addr; }
Address::operator uintptr_t() const { return g_addr; }
Address::operator void*() const { return reinterpret_cast<void*>(g_addr); }

// NField class
NField::NField(LoadClass thisclass, std::string name) : thisclass(thisclass), name(std::move(name)) {}

uintptr_t NField::offset() { return thisclass.GetFieldOffset(name.c_str()); }
void NField::getStatic(void *out) { thisclass.GetStaticFieldValue(name.c_str(), out); }
void NField::setStatic(void *value) { thisclass.SetStaticFieldValue(name.c_str(), value); }

// NClass class
NClass::NClass(std::string libName, const char *name)
        : libName(std::move(libName)), thisclass(LoadClass("", name)) {}

NClass::NClass(std::string libName, const char *nameSpace, const char *name)
        : libName(std::move(libName)), thisclass(LoadClass(nameSpace, name)) {}

Address NClass::method(std::string name, int param) {
    return {getBase(libName.c_str()), thisclass.GetMethodOffsetByName(name.c_str(), param)};
}

NField NClass::field(std::string name) {
    return {thisclass, std::move(name)};
}

// NativeLibrary
NativeLibrary::NativeLibrary(void* h, std::string name)
        : handle(h), name(std::move(name)) {}

Address NativeLibrary::get_symbol(const std::string& symbol) {
    void* addr = custom_dlsym(handle, symbol.c_str());
    return {getBase(name.c_str()), reinterpret_cast<uintptr_t>(addr)};
}

NClass NativeLibrary::get_class(const std::string& clasz) {
    return {name, clasz.c_str()};
}

NClass NativeLibrary::get_class(const std::string& namespaze, const std::string& clasz) {
    return {name, namespaze.c_str(), clasz.c_str()};
}

Address NativeLibrary::get_pattern(const std::string& pattern) {
    Address ret;
    Module module = GetModule(name);
    if (!module.name.empty()) {
        const char* pat = pattern.c_str();
        uint8_t* start = reinterpret_cast<uint8_t*>(module.start_address);
        uint8_t* first_match = nullptr;

        for (uint8_t* current_byte = start; current_byte < (start + module.size); ++current_byte) {
            if (*pat == '?' || *current_byte == strtoul(pat, nullptr, 16)) {
                if (!first_match) first_match = current_byte;

                if (!pat[2]) {
                    return Address(module.start_address, reinterpret_cast<uintptr_t>(first_match));
                }

                pat += *(uint16_t*)pat == 16191 || *pat != '?' ? 3 : 2;
            } else if (first_match) {
                current_byte = first_match;
                pat = pattern.c_str();
                first_match = nullptr;
            }
        }
    }
    return ret;
}

// LibManager
LibManager::LibManager(const std::string& libName, LibType libType)
        : name(libName), libType(libType) {
    void* IL2Cpp_Handle = nullptr;

    while (!IL2Cpp_Handle) {
        IL2Cpp_Handle = custom_dlopen(libName.c_str(), 4);
        sleep(1);
    }

    success = (IL2Cpp_Handle != nullptr);

    if (libType == Unity) {
        while (!NepAU::initialize()) {
            sleep(1);
        }
    }

    lib = std::make_unique<NativeLibrary>(IL2Cpp_Handle, libName);
}

Address LibManager::get_symbol(const std::string& symbol) {
    return lib->get_symbol(symbol);
}

NClass LibManager::get_class(const std::string& clasz) {
    return lib->get_class(clasz);
}

NClass LibManager::get_class(const std::string& namespaze, const std::string& clasz) {
    return lib->get_class(namespaze, clasz);
}

Address LibManager::get_pattern(const std::string& pattern) {
    return lib->get_pattern(pattern);
}

bool LibManager::loaded() const { return success; }
bool LibManager::is_unity() const { return libType == Unity; }
const std::string& LibManager::get_name() const { return name; }
