# Platform support

The platform support in Monty comes from PlatformIO's ability to compile (and
upload) code for a wide range of platforms (hence its name, no doubt). This is
all determined by the `platformio.ini` file, which is extensively documented on
PIO's [documentation site][INI].

[INI]: https://docs.platformio.org/en/latest/projectconf/index.html

To easily customise builds for Monty, it's a good idea to keep all these changes
separate from Monty's defaults by placing all settings in a file called
`pio-local.ini` (which is not part of Monty and excluded from `git`).  This
optional "local" file will be loaded after `platformio.ini` to extend and
override any settings.

E.g. to use a "Blue Pill" board connected via a Black Magic Probe, create
`pio-local.ini` with the following content:

```
[platformio]
default_envs = bluepill

[env:bluepill]
extends = stm32
board = bluepill_f103c8
build_flags = ${stm32.build_flags} -DSTM32F1
upload_protocol = blackmagic
```

Since the default is now set to `bluepill`, the following commands will now
target the Blue Pill: `inv flash`, `inv mrfs`, `inv runner`, and `inv upload`.
Keep in mind that more adjustments are often needed, because each target can
(and will!) differ from the `nucleo-l432` in many ways.

Platforms are not limited to STM32 or ARM, but in this case some more changes
will be required (such as creating a new `lib/arch-<name>/` area and and using a
new `<NAME>` for the code generator).

### Multiple configurations

The `pio-local.ini` file can define multiple configurations, each with their own
`[env:<somename>]` section.  This makes it easy to keep _all_ possible build
variants in a single file, and then pick one to work with.  For brevity,
sections can also extend others and override just a few settings (see the
`[env:noassert]` and `[env:nopyvm]` sections in `platformio.ini` for an
example).

With multiple configurations, it may be more convenient to _disable_ the
`default_envs` setting:

```
[platformio]
default_envs =

[...]
```

Then, simply choose the target in the shell using an environment variable:

```
$ export PLATFORMIO_DEFAULT_ENVS=bluepill
$ inv flash runner
...
```

As you can see, PlatformIO's offers substantial flexibility on how to set things
up. The reference for all the available configuration settings is [here][ENV].

[ENV]: https://docs.platformio.org/en/latest/projectconf/section_env.html
