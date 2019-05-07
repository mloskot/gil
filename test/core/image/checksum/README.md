# Boost.GIL test for image checksum

Tests based image data, image and view manipulations from the
[test/image/legacy/image.cpp](https://github.com/boostorg/gil/blob/develop/test/legacy/image.cpp)

* `test_fixture.hpp` - implementation of image and view processing test cases.
* `calculate.cpp` - test suite to verify checksums calculated at run-time against checksums from the reference file.
* `generate.cpp` - program to generate checksums for views produced by each test case.
