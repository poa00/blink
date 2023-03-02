![Blink logo](https://raw.githubusercontent.com/ychclone/blink/master/Resources/Images/graphics3.png)

# Blink code search
GUI of live indexed grep for source code. Fuzzy suggestion in autocomplete.
Files locator, search and replace. Switch different projects and start searching.
Drag and drop of filenames to your favourite editor.

[![Build status](https://ci.appveyor.com/api/projects/status/afn8q3ai3e7wphrf?svg=true)](https://ci.appveyor.com/project/ychclone/blink)

# Screenshots
![Screencast](https://raw.githubusercontent.com/ychclone/blink/master/Screencast/Usage.gif)

# Features
* Fast search using prebuilt index
* Fuzzy suggestion in autocomplete
* Live grep
* Drag and drop for filename to your favourite editor 
* Switch between multiple projects
* Very small index size compared to trigram
* Replaces in files for searched symbol
* Cross platform and requires no installation

# Download, Install

Windows and Linux binaries are available at 
* https://sourceforge.net/projects/blink-code-search/files/

# Code search
![Code search](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_codesearch.png)

# File listing
![File listing](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_filelisting.png)

# Replace in files
<img src="https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_replaceinfiles.png" width="680">

# Usage

1. Drop the folder (e.g. from file explorer, nautilus) into the window below
project tab.
![Drop folder](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/usage_drop_folder.png)
2. A new project dialog will appears. Type the file extensions that you want to index.
<img src="https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/usage_new_project.png" width="512">
3. Right click on the project name
4. Click "Rebuild Symbol"
5. Double click the project name to make it the active project.
Or right click and select "Load"
6. Start file filtering and code search on the file and symbol tab
7. After symbol search in symbol tab or select files in file tab, 
it can show replace in files tool in menu->tool

# Advanced Usage

For symbol queries, advanced options can be entered:
There is no spacing in between e.g. /a10, /n3, /xinclude

* /a NumberOfLinesAfter
* /b NumberOfLinesBefore
* /n NumberOfLinesBeforeAndAFter
* /f FileNameToMatch
* /x PatternToExclude

In addition, regular expression can be entered for the queries:
e.g. .*mainWindows, (_clicked|_Pressed)
It will match all ("and" condition) if multiple symbols are entered for
queries.

# Configuration

The text editor when double clicking the filename in file tab
could be set in configuration. The filename could be dropped to
other editor.
Option->Setting->Main->Default Editor

# Compilation (QMake Windows) 

1. Download Qt Open Source installer:
https://www.qt.io/download-open-source

2. Install Qt 6.4.2 (Mingw 64 bit and its tool)

3. Set Qt and mingw 64 binaries path

```
set QTDIR=C:\Qt6\6.4.2\mingw_64
set PATH=%PATH%;%QTDIR%\bin;C:\Qt6\Tools\mingw1120_64\bin
```

4. Download QScintilla 2.13.3 and install
https://riverbankcomputing.com/software/qscintilla/download

```
cd QScintilla_src-2.13.3/src
qmake qscintilla.pro
make
make install
```

5. cd blink
6. qmake blink.pro
7. mingw32-make

# Compilation (CMake Linux) 

1. Download Qt Open Source installer:
https://www.qt.io/download-open-source

2. Install Qt 6.4.2 and its shipped CMake

3. Link qmake to /usr/bin/qmake
```
sudo ln -s /home/ychclone/Qt6/6.4.2/gcc_64/bin/qmake /usr/bin/qmake
```

3. Download QScintilla 2.13.0 and install
https://riverbankcomputing.com/software/qscintilla/download

Set to static library for qscintilla
```
qscintilla.pro:
  CONFIG += staticlib

```

Compile and install QScintilla 
```
cd QScintilla_src-2.13.0/src
qmake qscintilla.pro
make
make install
```

4. Build in CMake

```
/home/ychclone/Qt6/Tools/CMake/bin/cmake .
```

or

```
cmake -DCMAKE_PREFIX_PATH="/home/ychclone/Qt6/6.4.2/gcc_64/lib/cmake" . 
```

# AppImage

1. Download linuxdeployqt-7-x86_64.AppImage:
https://github.com/probonopd/linuxdeployqt/releases

2. chmod u+x linuxdeployqt-7-x86_64.AppImage
3. ./linuxdeployqt-7-x86_64.AppImage blinkAppImage/usr/share/applications/blink.desktop -verbose=2 -appimage -extra-plugins=iconengines,platformthemes/libqgtk3.so

# Tips

## Display result lines before and after
In the text field for symbol, "/n3" could be used to display 3 lines before and after
![Multiple lines](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_multiple_lines.png)

## Match for multiple symbol
Multiple symbols (and condition) could be input for symbol queries.
e.g. Multiple match are input "CMainWindow", "_on" as queries.
![Multiple symbols](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_multiple_symbols.png)

## Show multiple project
Regular expression could be used to filter the project name and source filename name.
e.g. "Or" condition for project name using the pipe "|" regular expression.
![Regular expression](https://raw.githubusercontent.com/ychclone/blink/master/Screenshot/blink_regular_expression.png)

# Troubleshooting

## /usr/bin/ld: cannot find -lGL; collect2: error: ld returned 1 exit status
Sol: sudo apt install libgl1-mesa-dev

## Qt version not match
```
sudo rm /usr/bin/qmake
sudo ln -s /home/ychclone/Qt5.14.2/5.14.2/gcc_64/bin/qmake /usr/bin/qmake
```

## Segmentation fault (core dumped) when start
Please cp ./blink to build directory.
Please make sure the following config files existed in build directory:
- blink.ini
- qtag.conf
- qt.conf
- record.xml 

## qt.qpa.plugin: Could not find the Qt platform plugin "xcb" in ""
Error message: This application failed to start because no Qt platform plugin
could be initialized. Reinstalling the application may fix this problem.

Sol: 
```
export QT_QPA_PLATFORM_PLUGIN_PATH=/home/ychclone/Qt5.14.2/5.14.2/gcc_64/plugins/platforms
```


