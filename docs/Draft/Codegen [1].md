# Code generator

* goes through all the `.h` and `.cpp` source files
* triggers on lines containing `//CG`
* the generated code is *inserted* in the source code
	* existing insertions will be replaced on subsequent runs
* uses for the code generator:
	* use a short description to generate boilerplate code
	* collect information, to emit definitions and tables elsewhere
	* generate VM-callable bindings for functions and methods
	* track and auto-generate info needed for modules coded in C++
	* auto-number "quick strings" in the code, i.e. `Q(<num>,"str")`
	* deal with per-platform segmentation of qstrs
	* a single-line change can enable lots of printf's in the VM
	* scan some MPy header files to auto-gen tables for use in Monty
	* generate qstr-table as VaryVec-structured byte vector in flash

## design
* the code generator "owns" the lines it inserts/replaces
* it does this by marking the area it claims:
	* `//CG ...' and `//CG: ...` do not insert anything
	* `//CG1 ...` to `//CG3 ...` own the next 1 to 3 lines of text
	* longer sections are delimited with `//CG< ...` and `//CG>`
* apart from `//CG` scanning, it also scans for `Q(<num>,"str")`
	* these are "quick strings", each unique string has a unique ID
	* repeated use of these strings re-use the same ID
* qstr numbering can affect many sources when a new qstr is added
* source files are only re-written if they actually changed