#ifndef UNITYAPI_H
#define UNITYAPI_H

#include <cstddef>
#include <dlfcn.h>
#include <string>
#include <optional>
#include <functional>
#include <iostream>
#include "Server/Sources/NepAU/includeOnly/macros.h"

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
    inline void* get_il2cpp() {
        void *IL2Cpp_Handle = nullptr;

        while (!IL2Cpp_Handle) {
            IL2Cpp_Handle = custom_dlopen("libil2cpp.so", 4);
            sleep(1);
        }
        return IL2Cpp_Handle;
    }

    // Initialize all function pointers
    inline bool initialize() {
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

    inline void* get_class(const char* name_space, const char* type_name) {
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

class LoadClass {
    void* thisclass = nullptr;

    void* GetClass(const char* ns, const char* name) {
        return NepAU::get_class(ns, name);
    }

public:
    LoadClass(const char* namespaze, const char* clazz) {
        do { thisclass = GetClass(namespaze, clazz); } while (!thisclass);
    }
    uintptr_t GetMethodOffsetByName(const char* name, int paramcount) {
        return (uintptr_t) *NepAUApi::class_get_method_from_name(thisclass, name, paramcount);
    }
    uintptr_t GetFieldOffset(const char* name) {
        if (!thisclass) return 0x0;
        void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
        if (!fieldInfo) return 0x0;
        size_t offset = NepAUApi::field_get_offset(fieldInfo);
        return reinterpret_cast<uintptr_t>(offset);
    }
    void GetStaticFieldValue(const char* name, void *out) {
        if (!thisclass) return;
        void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
        if (!fieldInfo) return;
        NepAUApi::field_get_static(fieldInfo, out);
    }
    void SetStaticFieldValue(const char* name, void *out) {
        if (!thisclass) return;
        void* fieldInfo = NepAUApi::class_get_field_from_name(thisclass, name);
        if (!fieldInfo) return;
        NepAUApi::field_set_static(fieldInfo, out);
    }
};

#endif // UNITYAPI_H
