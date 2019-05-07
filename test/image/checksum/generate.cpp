//
// Copyright 2005-2007 Adobe Systems Incorporated
// Copyright (c) 2019 Mateusz Loskot <mateusz at loskot dot net>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// Program to generate reference checksums for image and view manipulation
// implemented by the `checksum_base` in `test_fixture.hpp`.
//
#ifdef _MSC_VER
#pragma warning(disable:4244) // conversion from 'gil::image<V,Alloc>::coord_t' to 'int', possible loss of data (visual studio compiler doesn't realize that the two types are the same)
#pragma warning(disable:4701) // potentially uninitialized local variable 'result' used in boost/crc.hpp
#endif
#include <boost/gil.hpp>
#include <boost/crc.hpp>
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include "test_fixture.hpp"

struct image_checksum_generator : boost::gil::test::fixture::checksum_base
{
    image_checksum_generator(std::ostream& os) : os_(os) {}

    void test_case_impl(boost::gil::rgb8c_view_t const& view, std::string const& name)
    {
        if (name.find_first_of("TODO") != std::string::npos)
            return; // skip

        boost::crc_32_type crc32_accumulator;
        crc32_accumulator.process_bytes(view.row_begin(0), view.size() * 3);
        os_ << name << " " << std::hex << crc32_accumulator.checksum() << std::endl;
    }

private:
    std::ostream& os_;
};

int main(int argc, char* argv[])
{
    try
    {
        std::ostream* os{nullptr};
        std::ofstream ofs;
        if (argc == 2)
        {
            std::string filename(argv[1]);
            ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            ofs.open(filename, std::ios::out);
            os = std::addressof(ofs);
        }
        else
        {
            os = std::addressof(std::cout);
        }
        assert(os);

        image_checksum_generator icg(*os);
        icg.run_suite_basic_image();
        icg.run_suite_dynamic_image();
        icg.run_suite_packed_image();
        icg.run_suite_virtual_view();

        return EXIT_SUCCESS;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown error" << std::endl;
    }
    return EXIT_FAILURE;
}
