# I) Library setup
Here are the few steps to setup a library created via [lib_template](https://github.com/Scaly-Sphere-Studio/lib_template).

## Run `InstallScripts.ps1`
This will add needed [scripts](https://github.com/Scaly-Sphere-Studio/vcpkg_scripts) as a git submodule to this repository.<br/>
_NOTE :_ Simply run this script again to update the scripts when needed.

## Edit the `CONTROL` file
The [CONTROL](CONTROL) file is used by `vcpkg`. Here is an example of what it should look like :
```
Source: sss-text-rendering
Version: 0.5.1
Description: Text rendering using FreeType and HarfBuzz
Build-Depends: freetype, harfbuzz, sss-commons
```
After installing a package via this file, `vcpkg list` will display :
```
sss-text-rendering:x64-windows        0.5.1        Text rendering using FreeType and HarfBuzz
sss-text-rendering:x86-windows        0.5.1        Text rendering using FreeType and HarfBuzz
```

## Rename the `.vcxproj` and `.vcxproj.filters` files
Rename [untitled.vcxproj](untitled.vcxproj) and [untitled.vcxproj.filters](untitled.vcxproj.filters) to match the repository's name.<br/>
This will allow the installed scripts to compile the project.

## Run the Visual Studio project to edit the target's name
- **en** : Project -> Properties of _[project's name]_ -> Configuration properties -> General -> Target Name
- **fr** : Projet -> Propriétés de _[nom du projet]_ -> Propriétés de la configuration -> Général -> Nom de la cible

The name must match the `Source:` parameter in [CONTROL](CONTROL).<br/>
In the previous example, it should be `sss-text-rendering`.

# II) How to use installed scripts
After you've added sources and headers to your project, you might want to use the installed scripts to export and locally install the library.<br/>
From your Visual Studio project, you should have a `vcpkg_scripts` filter containing `.ps1` scripts.<br/>
Right clicking a `.ps1` file will show the `Execute File` option, which avoids having to run them from the command line.

## export.ps1
_NOTE :_ If you've just edited the project's file structure (added, removed, or renamed files), you might need to close, save, and reopen your Visual Studio project. Else, the scripts won't be able to access the modifications.<br/>
<br/>
This script builds the project in 4 different configs :
- win32 Debug
- win32 Release
- x64 Debug
- x64 Release

Then, it exports the project to a `.zip` file ready to be installed via `vcpkg`.

## local_install.ps1
This script uses the `.zip` file created via `export.ps1` to locally install the package via `vcpkg`.<br/>
You can run it to install, or update a package _(be it installed locally before or not)_.<br />
<br/>
_NOTE :_ Simultaneously open Visual Studio projects won't be updated with the changes until reloading them. The text editor will display errors, but compiling will work fine.

## local_remove.ps1
This script removes any local `vcpkg` installation of this library.<br/>
If the library was installed from another source, it will display a warning and skip the removal.
