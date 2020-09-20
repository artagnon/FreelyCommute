# The .rM v5 file parser

In under 300 lines of C++, we have a parser for the v5 format of the proprietary format the reMarkable tablet saves its pages in. The format hasn't been fully reverse-engineered yet, and these are indicated by "Skip" values in the parse tables (see [ParseTable.hpp](ParseTable.hpp)). In order to make progress, it is necessary to perform various reduced-case drawings on the tablet, connect via the cloud API to retrieve the corresponding .rM file, and then reverse-engineer it.
