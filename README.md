# Mossball
A patch generator for RPG Maker 2000/3 projects, complete with detailed changelogs. While it is primarily intended for use in the Collective Unconscious project, you may use it for your own projects as well.

This is a replacement for the legacy changelog generator, compliant with the latest version of the changelog and including additional features such as automatic map transfer detection and built-in file selection.

## Building
Requires `Qt6` and uses CMake for building. `bit7z` and `liblcf` will be built as part of the process.

```
git clone https://github.com/lumiscosity/mossball
cd mossball
git submodule update --init
cmake -B builddir
cmake --build builddir
```

On Linux, you can build a Flatpak:

```
flathub-builder linux/flatpak-build linux/com.github.lumiscosity.Mossball.yml --repo linux/flatpak-repo --install-deps-from=flathub
flatpak build-bundle linux/flatpak-repo linux/mossball.flatpak com.github.lumiscosity.Mossball --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
flatpak install linux/mossball.flatpak
```

## License
Mossball is free software licensed under GPLv3. The icon is licensed under CC-BY-SA.
