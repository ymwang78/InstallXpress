#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../libsrc/InstallXpress/HostVMConfig.h"

class HostVMConfigTest : public ::testing::Test {
protected:
    std::filesystem::path path;
    void SetUp() override {
        wchar_t directory[MAX_PATH], filename[MAX_PATH];
        ASSERT_NE(GetTempPathW(MAX_PATH, directory), 0u);
        ASSERT_NE(GetTempFileNameW(directory, L"hvm", 0, filename), 0u);
        path = filename;
    }
    void TearDown() override { std::filesystem::remove(path); }
    bool parse(const char* xml, const wchar_t* root, std::vector<std::wstring>& names) {
        std::ofstream file(path, std::ios::binary);
        file << xml;
        file.close();
        return readHostVMSubvmNames(path.wstring(), root, names);
    }
};
TEST_F(HostVMConfigTest, ReadsActiveNamesAndIgnoresComments) {
    std::vector<std::wstring> names;
    ASSERT_TRUE(parse("<hostvm_server><!-- <subvm_info vmname='ident'/> -->"
                      "<subvm_info vmname='host&#112;y'/></hostvm_server>", L"hostvm_server", names));
    ASSERT_EQ(names, std::vector<std::wstring>{L"hostpy"});
}
TEST_F(HostVMConfigTest, ReadsMultipleFragmentNames) {
    std::vector<std::wstring> names;
    ASSERT_TRUE(parse("<hostvm_fragment><subvm_info vmname='a'/><subvm_info vmname='b'/></hostvm_fragment>",
                      L"hostvm_fragment", names));
    ASSERT_EQ(names.size(), 2u);
}
TEST_F(HostVMConfigTest, RejectsMalformedWrongRootAndMissingNames) {
    for (const char* xml : {"<hostvm_server>", "<wrong/>",
                           "<hostvm_server><subvm_info/></hostvm_server>"}) {
        std::vector<std::wstring> names;
        EXPECT_FALSE(parse(xml, L"hostvm_server", names));
    }
}
TEST_F(HostVMConfigTest, RejectsDTD) {
    std::vector<std::wstring> names;
    EXPECT_FALSE(parse("<!DOCTYPE hostvm_server [<!ENTITY x 'hostpy'>]>"
                       "<hostvm_server><subvm_info vmname='&x;'/></hostvm_server>", L"hostvm_server", names));
}
#ifndef USE_GTEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
