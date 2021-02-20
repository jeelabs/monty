Compact vector for variable-sized entries

* a "VaryVec" is a simple indexed collection

* the entries are byte chunks

* most efficient for reading, writing will move data

* an insert, delete, and replace any entry

## design

* the start of the data is a index of 16-bit offsets into the file

* after that comes each entry, one after the other in the same order

* no gaps, i.e. the size of entry N is "index[N+1]-index[N]"

* there is one more index slot that than that there are entries

* "index[0]" has value "2*(N+1)", where N is total # of entries

* the maximum VaryVec size (index + all data) is 65535 bytes (2^16-1)