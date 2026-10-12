# TaijiMPC6 HostVM templates

`InstallHostVMFragments()` runs before service registration in `PostSetup()`.
It enumerates immediate files in `HostVM/hostvm.d`. Any `.xml` file (case insensitive)
preserves the entire existing configuration. Otherwise it copies `1xx-*.xml.template`
to `1xx-*.xml`, retaining templates and passing `true` as `FilePathCopy`'s
`bFailIfExists` argument. A failed copy rolls back files created in this attempt, including any partial failed
copy, and aborts setup. A persistent `.hostvm-fragments.pending` directory is created
exclusively before copying. It is removed only after successful initialization or a
verified clean rollback. If it remains, subsequent attempts abort before the existing
XML check; recover the generated files before removing this marker manually. Missing templates or enumeration errors also abort setup.

`installx.FilePathListFiles(directory)` returns an array of immediate file names,
excluding directories. Failure returns `nil, win32Error`; an empty directory returns
an empty table. Names are UTF-8, like the other file APIs.

Run the Lua regression script from the repository root:
`lua tests/test_hostvm_templates.lua`.

Before copying, the installer parses `hostvm.xml` and template XML using Windows
XmlLite (DTD prohibited). Templates whose subvm names are all declared in the main
config are skipped, protecting legacy hostpy installations. Partial overlaps and
cross-template duplicate names fail before creating any files. XML comments do not
count as declarations. `HostVMSubvmNames(path, isFragment)` returns names or nil on
parse/read failure. `FilePathCopy` additionally returns the Win32 error code so
rollback never deletes a concurrent file on ERROR_FILE_EXISTS/ALREADY_EXISTS.
