#pragma once
#include <windows.h>

class ResourceHandler {
public:
    ResourceHandler(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
        : m_hModule(hModule), m_hResource(nullptr), m_hLoadedResource(nullptr) {
        m_hResource = FindResource(hModule, lpName, lpType);
        if (m_hResource) {
            m_hLoadedResource = LoadResource(hModule, m_hResource);
            // 错误处理
        }
    }

    ~ResourceHandler() {
        // 不需要调用FreeResource，因为LoadResource返回的是只读指针
    }

    const void* GetData() const {
        return m_hLoadedResource ? LockResource(m_hLoadedResource) : nullptr;
    }

    DWORD GetSize() const {
        return m_hResource ? SizeofResource(m_hModule, m_hResource) : 0;
    }

private:
    HMODULE m_hModule;
    HRSRC m_hResource;
    HGLOBAL m_hLoadedResource;
};