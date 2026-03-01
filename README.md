# <img src="https://raw.githubusercontent.com/kravich/notepadng/master/images/notepadng.svg" alt="Notepadng" width="32" height="32" /> Notepadng

Notepadng is a Notepad++-like editor for programmers and power users.

Notepadng is a fork of a [Notepadqq](https://github.com/notepadqq/notepadqq) editor with Web-based editor component replaced by Scintilla for better user experience, stability and maintainability.
It also incorporates original color themes from [Notepad++](https://github.com/notepad-plus-plus/notepad-plus-plus) itself to replicate brand Notepad++ look and feel.

For better experience, it's suggested to install Courier New font, which is proprietary but free for personal use. Notepadng will make use of it automatically with default settings.

On Debian/Ubuntu this could be achieved by:

    sudo apt install ttf-mscorefonts-installer

## Manual build

Notepadng could be built in Qt6 and Qt5 variants. Please find below a list of required dependencies.

| Dependency       | Deb package (Qt6 build) | Deb package (Qt5 build) |
|------------------|-------------------------|-------------------------|
| CMake            | cmake                   | cmake                   |
| pkg-config       | pkgconf                 | pkgconf                 |
| Qt Core          | qt6-base-dev            | qtbase5-dev             |
| Qt Gui           |                         |                         |
| Qt Widgets       |                         |                         |
| Qt PrintSupport  |                         |                         |
| Qt DBus          |                         |                         |
| Qt Network       |                         |                         |
| Qt Xml           |                         |                         |
| Qt Svg           | qt6-svg-dev             | libqt5svg5-dev          |
| Qt LinguistTools | qt6-tools-dev           | qttools5-dev            |
| Qt6 Core5Compat  | qt6-5compat-dev         | N/A                     |
| QScintilla       | libqscintilla2-qt6-dev  | libqscintilla2-qt5-dev  |
| uchardet         | libuchardet-dev         | libuchardet-dev         |

### Get the source

    $ git clone https://github.com/kravich/notepadng.git
    $ cd notepadng

### Build (Qt6)

    notepadng$ cmake -DCMAKE_INSTALL_PREFIX=/usr .
    notepadng$ make

### Build (Qt5)

    notepadng$ cmake -DWITH_QT=QT5 -DCMAKE_INSTALL_PREFIX=/usr .
    notepadng$ make

### Install

You can run notepadng from its build output folder. If however you want to install it, first build it
by following the above steps, then run:

    notepadng$ sudo make install

## Distribution Packages

> Not yet available. The plans are to provide native Ubuntu, Debian and Arch packages as well as Snap and Flatpack distributions
