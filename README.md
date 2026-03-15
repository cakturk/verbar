# verbar

`verbar` is a modular status generator. The original `dwm` behavior is still
available through the X11 backend, and there is also a plain-text stdout
backend which can be used with `swaybar` via `status_command`.

It currently reports:

- Dropbox status
- Wi-Fi status
- Wired network status
- CPU usage
- Memory usage
- Battery charge
- Volume
- Time

## Backends

`verbar` currently supports these output backends:

- `x11`: write the status string to the X11 root window name for `dwm`
- `stdout-text`: write one plain-text status line to standard output on each
  update

Examples:

```sh
verbar
verbar --backend=stdout-text
```

For `sway`/`swaybar`, the intended usage is:

```ini
bar {
    status_command verbar --backend=stdout-text
}
```

## Build options

The installation path and feature flags can be tweaked in `config.mk` or on
the `make` command line.

Current build flags:

- `WITH_X11=1`: build the X11 backend
- `WITH_DROPBOX=1`: build the Dropbox section
- `WITH_NET=1`: build the network section
- `WITH_VOLUME=1`: build the volume section

Examples:

```sh
make
make install
```

```sh
make WITH_X11=0
```

```sh
make WITH_X11=0 WITH_DROPBOX=0 WITH_NET=0 WITH_VOLUME=0
```

## Dependencies

Dependencies are feature-dependent:

- X11 development files, only when `WITH_X11=1`
- `libmnl`, only when `WITH_NET=1`
- PulseAudio client development files, only when `WITH_VOLUME=1`

The core scheduler and several sections do not require X11.

## PulseAudio and PipeWire

The current volume implementation is a `libpulse` client. On PipeWire systems,
it is expected to work through the PulseAudio compatibility server
(`pipewire-pulse`). That means a dedicated PipeWire rewrite is not required for
the current backend work.

If the PulseAudio compatibility service is not available, the volume section
will not work in either X11 or stdout mode unless `WITH_VOLUME=0` is used.
