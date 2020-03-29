# **jf3**

**A simple spectrum viewer in GTK3**

## Description

A (1D) spectrum viewer app, with functionality inspired by the `gf3` program for gamma-ray spectroscopy in the [RadWare](https://radware.phy.ornl.gov/) software package.

**NOTE**: This is still in the early stages - spectra are drawing correctly, rudimentary fitting support is in but it is not science ready yet.

## Features

### Support for various file formats

* **.txt** - A plaintext file of tab-separated values, where each row repesents a bin/channel and each column represents a spectrum (single column needed for 1 spectrum, 2 columns for 2, etc.).
* **.mca** - An .mca file is simply a 2D array of integers, with the first index denoting a spectrum number (array length up to 100) and the second index denoting a bin number (array length fixed to 32768 ie. 2<sup>15</sup>).
* **.fmca** - An .fmca file has the same format as .mca except it uses floats rather than integers.
* **.spe** -  Spectrum format used/generated by the [RadWare](https://radware.phy.ornl.gov/) software package, for example when using the 'ws' command in `gf3`.

Some sample files that the program can open are available [here](https://mega.nz/#!yUtRBAYR!ATst0ngazksR-g-P-Qdsw2rd4lpHJXBpd6nJq6pW77I) (.zip archive).

Conversion codes for some of these data formats are available in the [FileConvTools](https://github.com/e-j-w/FileConvTools) repository.

### Various spectrum display options

* Import spectra from multiple files and plot multiple spectra simultaneously:
    * Sum spectra together.
    * Overlay spectra, with either common or independent scaling on the y-axis.
    * Show spectra in a "stacked" view (tiled vertically with a common x-axis).
* Zoom and pan using the mouse (mouse wheel, click and drag).
* Display in linear or logarithmic scale on the y-axis.
* Rebin spectra, with results displayed in real time.

### Peak fitting

* Fit multiple Gaussian peak shapes on quadratic background (WIP).

### Cool user interface

* Designed to be as simple and intuitive as possible given the supported featureset.

## How to build

So far the program has only been tested with Linux-based systems.  A Makefile is provided, build the program using:

```make``` 

The program can be installed for all users with:

```sudo make install```

This will place the `jf3` binary in `/usr/bin` and a desktop entry file in `/usr/share/applications` (for desktop environment integration).  These changes can be undone with:

```sudo make uninstall```

This build process has been tested on CentOS 7 and Arch Linux (as of March 2020) under the GNOME desktop environment.

### Build dependencies

* GNU make
* gcc
* GTK 3 (`gtk3-devel` in CentOS)


## Usage notes

* Preferences are stored in a plaintext configuration file on a per-user basis at `$HOME/.config/jf3/jf3.conf`.
* When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `jf3 /path/to/file1 /path/to/file2`).
* After fitting a spectrum, the onscreen fit can be cleared using the right mouse button.

## Screenshot

![jf3 user interface screenshot](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/jf3.png "jf3 user interface")


## Future plans

* Peak labels?
* Import of spectra exported from ROOT?

## Credits

Developer/Maintainer: Jonathan Williams
