Name:       filecase

# These macros should already be defined in the RPMbuild environment, see: rpm --showrc
%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{!?qmake5_install:%define qmake5_install make install INSTALL_ROOT=%{buildroot}}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    An advanced file-manager for SailfishOS
# The <version> tag must adhere to semantic versioning: Among multiple other
# reasons due to its use for `qmake5` in line 107.  See https://semver.org/
Version:    0.4.3.1
# The <release> tag comprises one of {alpha,beta,rc,release} postfixed with a
# natural number greater or equal to 1 (e.g., "beta3") and may additionally be
# postfixed with a plus character ("+"), the name of the packager and a release
# number chosen by her (e.g., "rc2+jane4").  `{alpha|beta|rc|release}`
# indicates the expected status of the software.  No other identifiers shall be
# used for any published version, but for the purpose of testing infrastructure
# other nonsensual identifiers as `adud` may be used, which do *not* trigger a
# build at GitHub and OBS, when configured accordingly; mind the sorting
# (`adud` < `alpha`).  For details and reasons, see
# https://github.com/storeman-developers/harbour-storeman/wiki/Git-tag-format
Release:    rc2
# The Group tag should comprise one of the groups listed here:
# https://github.com/mer-tools/spectacle/blob/master/data/GROUPS
Group:      Applications/File
License:    MPL-2.0-no-copyleft-exception
URL:        https://github.com/sailfishos-applications/%{name}
# Altering the `Vendor:` field breaks the update path on SailfishOS, see
# https://en.opensuse.org/SDB:Vendor_change_update#Disabling_Vendor_stickiness
#Vendor:     meego
# The "Source0:" line below requires that the value of %%{name} is also the
# project name at GitHub and the value of `%%{release}/%%{version}` is also
# the name of a correspondingly set Git tag.  For details and reasons, see
# https://github.com/storeman-developers/harbour-storeman/wiki/Git-tag-format
Source0:    %{url}/archive/%{release}/%{version}/%{name}-%{version}.tar.gz
# Note that the rpmlintrc file MUST be named exactly so according to
# https://en.opensuse.org/openSUSE:Packaging_checks#Building_Packages_in_spite_of_errors
Source99:   %{name}.rpmlintrc
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils

# This description section includes metadata for SailfishOS:Chum, see
# https://github.com/sailfishos-chum/main/blob/main/Metadata.md
%description
FileCase is an advanced, feature-rich file-manager for SailfishOS.

%if 0%{?_chum}
Title: FileCase for SailfishOS
Type: desktop-application
Categories:
 - System
 - Utility
 - Network
 - Compression
 - FileManager
 - Filesystem
 - FileTools
 - FileTransfer
 - Qt
 - TextEditor
 - Viewer
DeveloperName: Matias Perez (CepiPerez), olf (Olf0), David L.-J. (llewelld / flypig)
Custom:
  Repo: %{url}
PackageIcon: %{url}/raw/master/icons/filecase3.png
Screenshots:
 - %{url}/raw/master/.xdata/screenshots/screenshot-001.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-002.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-003.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-004.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-005.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-006.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-007.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-008.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-009.jpg
 - %{url}/raw/master/.xdata/screenshots/screenshot-010.jpg
Links:
  Homepage: https://openrepos.net/content/olf/filecase
  Help: %{url}/issues
  Bugtracker: %{url}/issues
  Donation: https://openrepos.net/donate
%endif

# Define (S)RPM compression sensibly, taking compatibility into account, see
# https://github.com/sailfishos-patches/patchmanager/pull/417#issuecomment-1429068156
%define _source_payload w6.gzdio
%define _binary_payload w2.xzdio

%prep
%setup -q

%build
%qtc_qmake5 VERSION=%{version}
%qtc_make %{?_smp_mflags}

%install
%qmake5_install
desktop-file-install --delete-original --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/%{name}.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/86x86/apps/%{name}.png

