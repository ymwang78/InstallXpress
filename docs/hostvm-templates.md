# TaijiMPC6 HostVM templates

`InstallHostVMFragments()` runs before service registration in `PostSetup()`.
It enumerates immediate files in `HostVM/hostvm.d`. Any `.xml` file (case insensitive)
preserves the entire existing configuration. Otherwise it copies `1xx-*.xml.template`
to `1xx-*.xml`, retaining templates and passing `true` as `FilePathCopy`'s
`bFailIfExists` argument. A failed copy rolls back files created in this attempt and
aborts setup. Missing templates or enumeration errors also abort setup.

`installx.FilePathListFiles(directory)` returns an array of immediate file names,
excluding directories. Failure returns `nil, win32Error`; an empty directory returns
an empty table. Names are UTF-8, like the other file APIs.

Run the Lua regression script from the repository root:
`lua tests/test_hostvm_templates.lua`.
