local fs, names, copied, failCopy, failDelete, partial, copyError, listError, markerFailure
local directory = "C:\\Test path\\中文\\HostVM\\hostvm.d"
local pending = directory .. "\\.hostvm-fragments.pending"
installx = {
    FilePathExists = function(path) return fs[path] ~= nil end,
    FilePathListFiles = function()
        if listError then return nil, 5 end
        local result = {}
        for path, kind in pairs(fs) do
            if kind == "file" and path:sub(1, #directory + 1) == directory .. "\\" then
                table.insert(result, path:sub(#directory + 2))
            end
        end
        return result
    end,
    HostVMSubvmNames = function(path, fragment)
        if not fragment then return names.site end
        return names[path:sub(#directory + 2)]
    end,
    FilePathCreateDirectory = function(path)
        if fs[path] or markerFailure then return false end
        fs[path] = "dir"
        return true
    end,
    FilePathCopy = function(source, destination, failIfExists)
        assert(failIfExists == true)
        if fs[destination] then return false, 80 end
        if destination:find(failCopy or "NEVER_MATCH", 1, true) then
            if partial then fs[destination] = "file" end
            return false, copyError or 5
        end
        fs[destination] = "file"
        table.insert(copied, destination)
        return true, 0
    end,
    FilePathDelete = function(path)
        if failDelete and path:find(failDelete, 1, true) then return false end
        fs[path] = nil
        return true
    end,
    LogPrint = function() end,
}
ErrorHint = function() end
assert(loadfile("projects/TaijiMPC6/Install/Install.lua"))()
ResetInstallPath("C:\\Test path\\中文")
local function reset()
    fs, copied = {}, {}
    names = {site = {}, ["100-ident.xml.template"] = {"ident"}, ["120-hostpy.xml.template"] = {"hostpy"}}
    for _, name in ipairs({"100-ident.xml.template", "120-hostpy.xml.template"}) do
        fs[directory .. "\\" .. name] = "file"
    end
    failCopy, failDelete, partial, copyError, listError, markerFailure = nil, nil, nil, nil, nil, nil
end
local count = 0
local function check(expected, copies)
    assert(InstallHostVMFragments() == expected, "case " .. count)
    assert(#copied == copies, "copy count " .. count)
    count = count + 1
end
reset(); check(true, 2); assert(not fs[pending])
for _, xml in ipairs({"site.XML", "200-other.xml", "100-ident.xml"}) do
    reset(); fs[directory .. "\\" .. xml] = "file"; check(true, 0)
end
reset(); names.site = {"hostpy"}; check(true, 1); assert(not fs[directory .. "\\120-hostpy.xml"])
reset(); names.site = {"hostpy", "ident"}; check(true, 0)
reset(); names["120-hostpy.xml.template"] = {"hostpy", "other"}; names.site = {"hostpy"}; check(false, 0)
reset(); names.site = nil; check(false, 0)
reset(); names["100-ident.xml.template"] = nil; check(false, 0)
reset(); names["120-hostpy.xml.template"] = {"ident"}; check(false, 0)
reset(); fs = {}; check(false, 0)
reset(); listError = true; check(false, 0)
reset(); markerFailure = true; check(false, 0)
reset(); failCopy = "120-hostpy.xml"; partial = true; check(false, 1)
assert(not fs[pending] and not fs[directory .. "\\100-ident.xml"] and not fs[directory .. "\\120-hostpy.xml"])
failCopy = nil; copied = {}; check(true, 2)
reset(); failCopy = "120-hostpy.xml"; partial = true; failDelete = "100-ident.xml"; check(false, 1)
assert(fs[pending]); copied = {}; check(false, 0)
reset(); failCopy = "120-hostpy.xml"; partial = true; failDelete = "120-hostpy.xml"; check(false, 1)
assert(fs[pending]); copied = {}; check(false, 0)
reset(); failDelete = ".hostvm-fragments.pending"; check(false, 2)
copied = {}; check(false, 0)
reset(); fs[pending] = "dir"; check(false, 0)
reset(); failCopy = "120-hostpy.xml"; partial = true; copyError = 80; check(false, 1)
assert(fs[directory .. "\\120-hostpy.xml"])
print("HostVM template installer: " .. count .. " cases passed")
