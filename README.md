# FileCase for SailfishOS

**FileCase is an advanced, feature-rich file-manager.**

Features:
- Copy, move and rename files
- Custom clipboard (for copy, move and create symlinks actions)
- Search files by filename with wildcards
- Open files via `xdg-open` action (uses [MIME-handlers](https://github.com/llelectronics/mimer))
- Show hidden files and folders
- Show thumbnails of images
- View text files
- Edit text files with a built-in, simple text editor
- Create archives (`.zip`, `.tar`, `.tar.gz`, `.tar.bz2`)
- View archive content (aforementioned formats, plus more, e.g., `.apk`) 
- Extraxt archives (aforementioned formats, `.apk` etc.)
- Install packages (`.rpm`, `.apk`)
- Share files (to Android programs, Bluetooth etc.)
- Supports Box, Dropbox, Google Drive and OneDrive and WebDAV cloud storage services

Note that the ability to access cloud storage providers may be broken (this is [known for Dropbox](https://github.com/sailfishos-applications/filecase/issues/4)) due to API changes on their side; the same may apply to sharing via Bluetooth on recent SailfishOS releases.<br />
Pull requests with fixes, improvements and enhancements are welcome!

## Translating FileCase (l10n / i18n)

If you want to translate FileCase to a language it does not support yet or improve an extant translation, please [read the translations-README](./translations#readme).

## Screenshots of FileCase

|       |       |       |       |       |
| :---: | :---: | :---: | :---: | :---: |
|       |       |       |       |       |
| ![View files and directories](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-001.jpg) | ![Select files and directories](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-002.jpg) | ![View file properties with media preview](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-003.jpg) | ![Storage locations](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-004.jpg) | ![Cloud storage locations](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-005.jpg) |
| <sub>View files and directories</sub> | <sub>Select files and directories</sub>  | <sub>View file properties with media preview</sub> | <sub>Storage locations</sub> | <sub>Cloud storage locations</sub> |
|       |       |       |       |       |
|       |       |       |       |       |
| ![Settings page](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-006.jpg) | ![Share page](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-007.jpg) | ![Creating archive files](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-008.jpg) | ![Extracting archive files](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-009.jpg) | ![Transfers page](https://github.com/sailfishos-applications/filecase/raw/master/.xdata/screenshots/screenshot-010.jpg) |
| <sub>Settings page</sub> | <sub>Share page</sub>  | <sub>Creating archive files</sub> | <sub>Extracting archive files</sub> | <sub>Transfers page</sub> |
|       |       |       |       |       |

## History of FileCase

The original [FileCase for SailfishOS (2015 - 2016)](https://openrepos.net/content/cepiperez/filecase-0#content) started as a port of [FileCase for MeeGo-Harmattan (2014)](https://openrepos.net/content/cepiperez/filecase#content) (on the Nokia N9), which in turn is an overhauled [FileBox (2013)](https://openrepos.net/content/cepiperez/filebox#content).  All three appllications were written by [Matias Perez (CepiPerez)](https://github.com/CepiPerez).  In 2021 Matias [released the source code of FileCase](https://github.com/sailfishos-applications/filecase/commit/d4294bbc5e854bb1a32481201f556ba453fbaef6) by creating this git repository at GitHub.

In 2022 [David Llewellyn-Jones (flypig)](https://github.com/llewelld) cleaned up [the source code repository and adapted FileCase](https://github.com/sailfishos-applications/filecase/compare/0.1-9...0.2.1) to run on recent releases of SailfishOS.

In 2022 and 2023 [olf (Olf0)](https://github.com/Olf0) overhauled infrastructure aspects, such as this README, a [new OpenRepos page](https://openrepos.net/content/olf/filecase#content), the [Transifex integration](https://github.com/sailfishos-applications/filecase/pull/34), making the spec file suitable for [the Sailfish-OBS](https://build.sailfishos.org/) and [the SailfishOS:Chum community repository](https://build.sailfishos.org/project/show/sailfishos:chum) etc.

## Credits
#### Original author
[Matias Perez (CepiPerez)](https://github.com/CepiPerez)
#### Contributors
- [David Llewellyn-Jones (flypig)](https://github.com/llewelld)
- [olf (Olf0)](https://github.com/Olf0)

