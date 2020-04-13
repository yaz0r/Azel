# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased][unreleased]
### Added
- On Windows, it is now possible to specify a custom playback device on the
  command line using the new `-d/--device` option.  Additionally, all valid
  playback devices can be listed by passing `--list-devices`.
- MIDI file dumping has been added.  See the Readme file for information about
  the engines and the MIDI events currently implemented.
- Instrument samples are now dumped to .wav files for the following engines:
  * Dreamcast: low-level
- The program now terminates once the song has ended.  Useful for batch
  processing.

#### Changes to the Makefile:
- The Makefile should now detect 64-bit Linux systems automatically; however,
  specifying `OSTYPE=linux` is still necessary.
- The Makefile now comes with two build targets: a `release` target which
  compiles the code with optimizations, and a `debug` target that generates
  debugging information and disables optimizations.
- Audio Overload's own `CC`, `CPP`, `CFLAGS`, `LDFLAGS` and `LIBS` are now
  added to the environment variables of the same names during the build.
- Binaries are now suffixed with GCC's build machine identifier.

### Changed
- The README file now is formatted using Markdown syntax.
- Moved the Ikaruga sample into the samples/ subdirectory.
- Corlett tags now are stored using an unlimited key→value hash table and are
  no longer limited to 255 bytes.  In return, though, all `corlett_t`
  structures now need to be freed using `corlett_free()` in order to not leak
  the memory used for the tags.
- The `ao_display_info` structure now uses char pointers instead of fixed-size
  char buffers.

### Fixed
- PSF2 DMA4 and DMA7 implementations have been fixed; both the Final Fantasy IV
  rip by CaitSith2 and the Final Fantasy X rip by nenolod now play correctly.
  Thanks to kode54 for the tip.
- On Windows, Unicode filenames passed on the command line are now supported
  correctly.
- Corlett length and fade tags with more than three fractional digits are now
  supported correctly.

### Removed
- Due to the inclusion of [win32_utf8](https://github.com/thpatch/win32_utf8)
  for transparent handling of Unicode file names on Windows, compilation on
  Cygwin's GCC is no longer supported.

## [1.4.8] - 2009-02-15
### Fixed
- Guard against invalid data sometimes created by `makessf.py` (fixes crashing
  Pebble Beach ST-V rips).

## [1.4.7] - 2008-11-03
### Fixed
- Fixed slightly too slow tempo for DSFs (shown by Virtua Fighter 4 DSF rip).

## [1.4.6] - 2008-11-02
### Fixed
- More AICA/DSF fixes for games using the DTPK driver.

## [1.4.5] - 2008-11-02
### Fixed
- AICA/DSF fixes for AEG readback, invalid loop parameters, and AICA Timer B
  operation.
- On newer Linux distros with PulseAudio `/dev/dsp` is missing but `/dev/dsp1`
  exists.  Try both.

## [1.4.4] - 208-10-19
### Fixed
- AICA/DSF fixes for Naomi rips using the full 8 MB of RAM.  (kingshriek)

## [1.4.3] - 2008-07-28
### Fixed
- More AICA fixes and cleanups.  (ajax16384)

## [1.4.2] - 2008-07-28
### Fixed
- Many AICA fixes.  Sound quality is improved and DC offset is fixed.  (Deunan
  Knute, ajax16384)

## [1.4.1] - 2008-04-23
### Fixed
- SCSP fix for crash cymbal issue in Shining the Holy Ark.  Use with
  kingshriek's new re-rip for best results.

## [1.4.0] - 2008-03-07
### Added
- DSF format engine, after some AICA fixes/improvements and some ARM7 cleanup.
  This is considered good enough to call a non-alpha.

## [1.3.1] - 2008-01-19
### Fixed
- Further FM and pitch improvements to SCSP. (kingshriek)

## [1.3] - 2008-01-13
### Added
- Added length and fade tag support to SSF. (R. Belmont)

### Fixed
- Fixed FM bug in SCSP for SSF files. (kingshriek)

## [1.2.1] - 2008-01-05
### Fixed
- Working FM synthesis in the SCSP, so more SSF files sound good.

## [1.2.0] - 2007-12-20
### Fixed
- Major overhaul of the PSF2 support.  More games work, including many
  Squaresoft titles.

## [1.1.7] - 2007-12-16
### Fixed
- kingshriek fixed an SCSP pitch calculation overflow and added FM support.

## [1.1.6] - 2007-12-14
### Fixed
- kingshriek strikes again: SCSP playback quality is much improved!

## [1.1.5] - 2007-12-12
### Fixed
- kingshriek strikes again: SCSP panning and other fixes, plus interpolation
  merged from MAME.

## [1.1.4] - 2007-12-11
### Fixed
- Even more SSF & SCSP fixes from kingshriek.

## [1.1.3] - 2007-12-09
### Fixed
- Even more SSF & SCSP fixes from kingshriek.

## [1.1.2] - 2007-12-02
### Fixed
- More SSF & SCSP fixes from kingshriek.

## [1.1.1] - 2007-12-01
### Fixed
- More SSF & SCSP fixes from kingshriek.

## [1.1] - 2007-11-06
### Fixed
- Two fixes for the SSF are included from kingshriek.

### Added
- PSF, PSF2, and SPU format engines are included.
- Known working example songs are included.


[unreleased]: https://github.com/nmlgc/aosdk/compare/v1.4.8...HEAD
[1.4.8]: https://github.com/nmlgc/aosdk/compare/v1.4.7...v1.4.8
[1.4.7]: https://github.com/nmlgc/aosdk/compare/v1.4.6...v1.4.7
[1.4.6]: https://github.com/nmlgc/aosdk/compare/v1.4.5...v1.4.6
[1.4.5]: https://github.com/nmlgc/aosdk/compare/v1.4.4...v1.4.5
[1.4.4]: https://github.com/nmlgc/aosdk/compare/v1.4.3...v1.4.4
[1.4.3]: https://github.com/nmlgc/aosdk/compare/v1.4.2...v1.4.3
[1.4.2]: https://github.com/nmlgc/aosdk/compare/v1.4.1...v1.4.2
[1.4.1]: https://github.com/nmlgc/aosdk/compare/v1.4.0...v1.4.1
[1.4.0]: https://github.com/nmlgc/aosdk/compare/v1.3.1...v1.4.0
[1.3.1]: https://github.com/nmlgc/aosdk/compare/v1.3...v1.3.1
[1.3]: https://github.com/nmlgc/aosdk/compare/v1.2.1...v1.3
[1.2.1]: https://github.com/nmlgc/aosdk/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/nmlgc/aosdk/compare/v1.1.7...v1.2.0
[1.1.7]: https://github.com/nmlgc/aosdk/compare/v1.1.6...v1.1.7
[1.1.6]: https://github.com/nmlgc/aosdk/compare/v1.1.5...v1.1.6
[1.1.5]: https://github.com/nmlgc/aosdk/compare/v1.1.4...v1.1.5
[1.1.4]: https://github.com/nmlgc/aosdk/compare/v1.1.3...v1.1.4
[1.1.3]: https://github.com/nmlgc/aosdk/compare/v1.1.2...v1.1.3
[1.1.2]: https://github.com/nmlgc/aosdk/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/nmlgc/aosdk/compare/v1.1...v1.1.1
[1.1]: https://github.com/nmlgc/aosdk/compare/v1.0...v1.1
