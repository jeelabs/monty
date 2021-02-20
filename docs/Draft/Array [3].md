# Array storage for scalars

* used for signed and unsigned 8-, 16-, 32, and 64-bit ints
* also used for 1-, 2-, and 4- bit values, i.e. 0..1, 0..3, and 0..15
* the "VaryVec" is also available as Array (of `bytes` objects)
* can be converted to a lossless text representation and back

## design
* a single `Array` class wraps the entire implementation
* a 1-letter code identifies the type, upper case is for unsigned values
* all data is mapped onto a `ByteVec`, and then cast behind the scenes
* `len()`, `getAt()`, and `setAt()` methods take and return a `Value`
* 64-bit ints can only be signed, Monty cannot handle unsigned 64b ints
* the `Array` datatype could/should be made optional in Monty
* floats and doubles are not yet implemented
* 1/2/4 bit get/set work, but insert/delete only handles 8-bit multiples
* see <https://github.com/jeelabs/monty/blob/main/lib/monty/array.cpp>