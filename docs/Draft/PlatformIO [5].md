# Development with PlatformIO

* super tool for taking care of all the toolchain details
* see <https://docs.platformio.org/en/latest/>
* I'm not using the IDE plugin myself, only the command-line
* basic command is `pio run ...`
* but in the Monty projects, PIO is mostly used via `inv ...`
* to update PIO and all tools & libs: `pio upgrade && pio update`

## design
* the trick is to fit into PIO's natural flow and not fight it
* PIO finds libraries, and also fetches/updates them as needed
* the optional `platformio-local.ini` file is never stored in git
	* the idea is to use it for local (re-) configuration
* see `examples/minimal/` for a second use of `platformio.ini`
* library search mode is "strict" to allow filtering on platform
	* see `lib/arch-stm32/library.json` for an example
* all platform-specific main dirs have an `arch.h` include file
	* but only one of them will match the current build
* ... this way, PIO automatically picks the proper set of sources
* the `[codegen]` section is not used by PIO, but by `inv generate`