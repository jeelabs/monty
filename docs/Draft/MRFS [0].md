# Minimal Replaceable File Storage

* top-level use `inv mrfs ?-f <file> files...`
* each entry is 8-byte header + data + 24-byte footer
* entries are always multiples of 32 bytes and can be concatenated
* file names up to 15 chars, `YYMMDD.HHMM` timestamps
* code at <https://github.com/jeelabs/monty/tree/main/lib/mrfs>
* create/upload with <https://github.com/jeelabs/monty/blob/main/src/mrfs.py>

## design
* aimed at memory-mapped use, files must remain contiguous
* no support for incremental file writing, size at start, crc at end
* new file versions are tacked onto the end, last one is valid
* files with zero timestamp mark deleted entries
* intended for flash memory, erased in larger segments
* before free mem runs too low:
	* copy lowest still-valid files up
	* repeat until bottom segment is unused
	* erase bottom segment
	* copy next valid files down
	* if needed, copy more files up, until next segment is unused
	* erase next segment
	* rinse and repeat
* at the end, all valid files are in low mem, and the rest is erased again
* for Python `import` use, files are `.mpy` files, without the extension

## status
* `mrfs.py` is used to upload to attached board over serial, or to write to file
* stm32 build has code to list and find files by name
* `lib/mrfs/` has everything, including test scripts
* includes code to add a new file
* no cleanup/compaction logic yet - must erase all and upload again