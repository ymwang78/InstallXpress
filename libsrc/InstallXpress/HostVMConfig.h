#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <xmllite.h>
#include <atlbase.h>
#include <string>
#include <vector>
#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

// Read only direct subvm_info children. XML comments never count as active declarations.
inline bool readHostVMSubvmNames(const std::wstring& path, const wchar_t* expectedRoot,
                                std::vector<std::wstring>& names)
{
    names.clear();
    CComPtr<IStream> stream;
    if (FAILED(SHCreateStreamOnFileEx(path.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE,
                                    FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &stream))) return false;
    CComPtr<IXmlReader> reader;
    if (FAILED(CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&reader), nullptr)) ||
        FAILED(reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)) ||
        FAILED(reader->SetInput(stream))) return false;
    bool foundRoot = false;
    XmlNodeType type;
    HRESULT result;
    while ((result = reader->Read(&type)) == S_OK) {
        if (type != XmlNodeType_Element) continue;
        UINT depth = 0;
        const wchar_t* tag = nullptr;
        if (FAILED(reader->GetDepth(&depth)) || FAILED(reader->GetQualifiedName(&tag, nullptr))) return false;
        if (depth == 0) {
            if (wcscmp(tag, expectedRoot) != 0) return false;
            foundRoot = true;
        } else if (depth == 1 && wcscmp(tag, L"subvm_info") == 0) {
            if (reader->MoveToAttributeByName(L"vmname", nullptr) != S_OK) return false;
            const wchar_t* value = nullptr;
            if (FAILED(reader->GetValue(&value, nullptr)) || !value || !*value) return false;
            names.emplace_back(value);
            if (FAILED(reader->MoveToElement())) return false;
        }
    }
    return result == S_FALSE && foundRoot;
}
