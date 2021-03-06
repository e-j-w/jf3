# **jf3**

A program for gamma-ray spectroscopy or similar data analysis.  Uses [GTK](https://www.gtk.org/) to provide smooth rendering and (hopefully) an easy to understand interface.  This is a hobby project, things should mostly work but all the typical disclaimers apply.

## Screenshot

![jf3 user interface screenshot](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/jf3.png "jf3 user interface")

## Features

### Display data

* Import data from one or multiple [files](#filecompat) and plot multiple spectra simultaneously:
    * Sum spectra together.
    * Overlay spectra, with either common or independent scaling on the y-axis.
    * Show spectra in a "stacked" view (tiled vertically with a common x-axis).
* Zoom and pan using the mouse (mouse wheel, click and drag).
* Display in linear or logarithmic scale on the y-axis.
* Calibrate spectra (with constant, linear, quadratic terms) to express bins in desired units.
* Rescale spectra, to perform operations such as background subtraction.
* Rebin spectra, with results displayed in real time.

### Fit data

* Fit multiple Gaussian peak shapes (symmetric or skewed) on quadratic background (iterative least-squares fitter).
* Relative peak widths may be fixed (recommended for gamma-ray spectroscopy) or allowed to freely vary.
* Weight the fit by the data (taking background subtraction into account) or by the fit function.  Or don't weight the fit at all.

### Manage data

* Rename, or delete spectra from the opened session.
* Store custom views of multiple spectra using any of the available plotting modes.
* Double click anywhere on a plot to write/edit a comment there.
* The full session including comments and custom views can be saved to disk using the **.jf3** (compressed) or **.txt** (ACSII) file formats.
* Export image files (.png format) at arbitrary resolution for use in presentations and publications.

### Supported file formats<a name="filecompat"></a>

#### Read/write support

* **.jf3** - Format containing one or more losslessly-compressed spectra with comments and customized views.
* **.txt** - A plaintext file of tab-separated values, can be copy-pasted to/from Excel or similar spreadsheet software.  Each row repesents a bin/channel and each column represents a spectrum (single column needed for 1 spectrum, 2 columns for 2, etc.).
* **.spe** -  Spectrum format used/generated by the [RadWare](https://radware.phy.ornl.gov/) software package, for example when using the 'ws' command in `gf3`.

#### Read-only support

* **.C** (ROOT macro) - [ROOT](https://root.cern.ch/) histogram macro files (.C files generated using the File/Save option in a ROOT TBrowser).  Supports TH1D, TH1F, and TH1I histogram types.
* **.mca** - A 2D array of integers, with the first index denoting a spectrum number (array length up to 100) and the second index denoting a bin number (array length fixed to 32768 ie. 2<sup>15</sup>).
* **.fmca** - The same format as .mca except using floats rather than integers.

#### Format support notes

Some sample files that the program can open are available [here](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/jf3-sample-files.zip) (.zip archive).

Conversion codes for some of the above data formats are available in the [FileConvTools](https://github.com/e-j-w/FileConvTools) repository.

## Getting started

### Compatibility

This program has been tested on CentOS 7, Ubuntu 18.04/20.04, and Arch Linux (as of February 2021) under the GNOME desktop environment.  It should work on any Linux distro that satisfies the listed build dependencies.  It may be possible to build this on other platforms where GTK is [available](https://www.gtk.org/docs/installations/), but that hasn't been tested.

### Build dependencies

* make
* gcc
* pkg-config
* GTK3

In CentOS 7:

```
sudo yum install gcc gtk3-devel
```

In Ubuntu:

```
sudo apt install build-essential libgtk-3-dev
```

In Arch Linux:

```
sudo pacman -S gcc make pkgconf gtk3
```

### Build instructions

A Makefile is provided, build the program using:

```make``` 

The resulting `jf3` executable can be run directly from the command line or your file manager.  Optionally, the program can be installed for all users with:

```sudo make install```

This will place the `jf3` binary in `/usr/bin`, a data-type definition file in `/usr/share/mime/packages` (to allow opening compatible files directly from the file manager), and a desktop entry file in `/usr/share/applications` (for desktop environment integration).  These changes can be undone with:

```sudo make uninstall```

## Usage tips

* Preferences are stored in a plaintext configuration file on a per-user basis at `$HOME/.config/jf3/jf3.conf`.
* When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `jf3 /path/to/file1 /path/to/file2`).
* After fitting a spectrum, the onscreen fit can be cleared using the right mouse button.
