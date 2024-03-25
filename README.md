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
- View archive content (aforementioned formats, plus more, e.g. `.apk`) 
- Extraxt archives (aforementioned formats, `.apk` etc.)
- Install packages (`.rpm`, `.apk`)
- Share files (to Android programs, Bluetooth etc.)
- Supports Box, Dropbox, Google Drive, OneDrive and WebDAV cloud storage services

<sup>Note that the ability to access cloud storage providers [may be broken](https://github.com/sailfishos-applications/filecase/issues/48) (this is [known for Dropbox](https://github.com/sailfishos-applications/filecase/issues/4)) due to API changes on their side; the same may apply to file sharing via Bluetooth on recent SailfishOS releases.</sup><br />
Pull requests with fixes, improvements and enhancements are welcome!

## Translating FileCase (l10n / i18n)

If you want to translate FileCase to a language it does not support yet or improve an extant translation, please [read the translations-README](./translations#readme).

## Screenshots of FileCase

|       |       |       |       |       |
| :---: | :---: | :---: | :---: | :---: |
|       |       |       |       |       |
| ![View files and directories](./.xdata/screenshots/screenshot-001.jpg?raw=true) | ![Select files and directories](./.xdata/screenshots/screenshot-002.jpg?raw=true) | ![View file properties with media preview](./.xdata/screenshots/screenshot-003.jpg?raw=true) | ![Storage locations](./.xdata/screenshots/screenshot-004.jpg?raw=true) | ![Cloud storage locations](./.xdata/screenshots/screenshot-005.jpg?raw=true) |
| <sub>View files and directories</sub> | <sub>Select files and directories</sub>  | <sub>View file properties,<br />with media preview</sub> | <sub>Storage locations</sub> | <sub>Cloud storage locations</sub> |
|       |       |       |       |       |
|       |       |       |       |       |
| ![Settings page](./.xdata/screenshots/screenshot-006.jpg?raw=true) | ![Share page](./.xdata/screenshots/screenshot-007.jpg?raw=true) | ![Creating archive files](./.xdata/screenshots/screenshot-008.jpg?raw=true) | ![Extracting archive files](./.xdata/screenshots/screenshot-009.jpg?raw=true) | ![Transfers page](./.xdata/screenshots/screenshot-010.jpg?raw=true) |
| <sub>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Settings&nbsp;page&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</sub> | <sub>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Share&nbsp;page&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</sub>  | <sub>&nbsp;Creating&nbsp;archive&nbsp;files&nbsp;</sub> | <sub>Extracting&nbsp;archive&nbsp;files</sub> | <sub>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Transfers&nbsp;page&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</sub> |
|       |       |       |       |       |

## History of FileCase

The original [FileCase for SailfishOS (2015 - 2016)](https://openrepos.net/content/cepiperez/filecase-0#content) started as a port of [FileCase for MeeGo-Harmattan (2014)](https://openrepos.net/content/cepiperez/filecase#content) (on the Nokia N9), which in turn is an overhauled [FileBox (2013)](https://openrepos.net/content/cepiperez/filebox#content).  All three applications were written by [Matias Perez (CepiPerez)](https://github.com/CepiPerez).  In 2021 Matias [released the source code of FileCase](https://github.com/sailfishos-applications/filecase/commit/d4294bbc5e854bb1a32481201f556ba453fbaef6) by creating this git repository at GitHub.

In 2021 [David Llewellyn-Jones (flypig)](https://github.com/llewelld) cleaned up [the source code repository and adapted FileCase](https://github.com/sailfishos-applications/filecase/compare/0.1-9...0.2.1) to run on recent releases of SailfishOS.

In 2022 and 2023 [olf (Olf0)](https://github.com/Olf0) overhauled infrastructure aspects, such as this README, a [new OpenRepos page](https://openrepos.net/content/olf/filecase#content), the [Transifex integration](https://github.com/sailfishos-applications/filecase/pull/34), making the spec file suitable for [the SailfishOS-OBS](https://build.sailfishos.org/) and [the SailfishOS:Chum community repository](https://github.com/sailfishos-chum/main#readme) etc.

## Credits
#### Original author
[Matias Perez (CepiPerez)](https://github.com/CepiPerez)
#### Contributors
- [David Llewellyn-Jones (llewelld / flypig)](https://github.com/llewelld)
- [olf (Olf0)](https://github.com/Olf0)
- [Simon Schmeisser (simonschmeisser)](https://github.com/simonschmeisser)
<br />

### License: [MPL 2.0](https://spdx.org/licenses/MPL-2.0-no-copyleft-exception.html)

