local files, copied, deleted, failCopy, listError
installx = {
    FilePathListFiles = function() return files, listError end,
    FilePathCopy = function(source, destination, failIfExists)
        assert(failIfExists == true)
        if destination:find(failCopy or "NEVER_MATCH", 1, true) then return false end
        table.insert(copied, destination)
        return true
    end,
    FilePathDelete = function(path) table.insert(deleted, path); return true end,
    LogPrint = function() end,
}
ErrorHint = function() end
assert(loadfile("projects/TaijiMPC6/Install/Install.lua"))()
ResetInstallPath("C:\\Test path\\中文")
local function run(input, expected, count, failure)
    files, copied, deleted, failCopy, listError = input, {}, {}, failure, 5
    assert(InstallHostVMFragments() == expected)
    assert(#copied == count)
end
local templates = {"100-ident.xml.template", "120-hostpy.xml.template"}
run(templates, true, 2)
assert(copied[1]:match("100%-ident%.xml$"))
assert(copied[2]:match("120%-hostpy%.xml$"))
run({"site.XML", templates[1]}, true, 0)
run({"200-other.xml", templates[1]}, true, 0)
run({"100-ident.xml", templates[1], templates[2]}, true, 0)
run({"200-other.xml.template", templates[1]}, true, 1)
run({"100-ident.xml.template.bak"}, false, 0)
run({}, false, 0)
run(nil, false, 0)
run(templates, false, 1, "120-hostpy.xml")
assert(#deleted == 1 and deleted[1] == copied[1])
run(templates, true, 2) -- retry after rollback
print("HostVM template installer: 10 cases passed")
