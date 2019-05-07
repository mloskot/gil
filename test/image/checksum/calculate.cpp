//
// Copyright 2019 Mateusz Loskot <mateusz at loskot dot net>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
#ifdef _MSC_VER
#pragma warning(disable:4244) // conversion from 'gil::image<V,Alloc>::coord_t' to 'int', possible loss of data (visual studio compiler doesn't realize that the two types are the same)
#pragma warning(disable:4701) // potentially uninitialized local variable 'result' used in boost/crc.hpp
#endif

#include <boost/gil.hpp>
#include <boost/crc.hpp>

#include "unit_test.hpp"
#include "test_fixture.hpp"

#include<map>
#include<memory>
#include<fstream>
#include<ios>
#include<string>

struct test_image_checksum_calculate : boost::gil::test::fixture::checksum_base
{
    void initialize(std::string const& filename)
    {
        boost::crc_32_type::value_type crc_result;
        std::ifstream ifs(filename, std::ios::in);
        while (true)
        {
            std::string crc_name;
            ifs >> crc_name >> std::hex >> crc_result;
            if (ifs.fail()) break;
            if (!crc_name.empty() && crc_name[0] == '#')
            {
                crc_result = 0; // skip test case
                crc_name = crc_name.substr(1);
            }
            crc_map_[crc_name] = crc_result;
        }
        ifs.close();
    }

    void test_case_impl(boost::gil::rgb8c_view_t const& view, std::string const& name)
    {
        BOOST_TEST_INFO(name);
        if (name.find_first_of("TODO") != std::string::npos)
        {
            BOOST_TEST_MESSAGE(name);
        }
        else
        {
            BOOST_TEST(crc_map_.count(name) != std::size_t{0}, "No reference checksum for test case \'" << name << "\'");
            boost::crc_32_type crc32_accumulator;
            crc32_accumulator.process_bytes(view.row_begin(0), view.size() * 3);
            auto const checksum_calculated = crc32_accumulator.checksum();
            auto const checksum_expect = crc_map_[name];
            BOOST_TEST(checksum_calculated  == checksum_expect,
                "Checksum test failed for \'" << name << "\': "
                << std::hex << checksum_calculated << " !="
                << std::hex << checksum_expect);
        }
    }

private:
    std::map<std::string, boost::crc_32_type::value_type> crc_map_;
};

// The default test runner compiled into the static library uses the obsolete
// initialization function signature.
// We link Boost.UTF statically, do not define BOOST_TEST_ALTERNATIVE_INIT_API
but::test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[])
{
    auto& master_test_suite = but::framework::master_test_suite();
    master_test_suite.p_name.value = "Checksum Calculation Master Test Suite";

    if (master_test_suite.argc < 2)
        throw but::framework::setup_error("No file with reference checksums specified");
    std::string checksums_filename(master_test_suite.argv[1]);

    auto tester = std::make_shared<test_image_checksum_calculate>();
    tester->initialize(checksums_filename);

    auto ts_basic = BOOST_TEST_SUITE("test_basic_image");
    ts_basic->add(BOOST_TEST_CASE(std::bind(&test_image_checksum_calculate::run_suite_basic_image, tester)));
    master_test_suite.add(ts_basic);

    auto ts_packed = BOOST_TEST_SUITE("test_packed_image");
    ts_packed->add(BOOST_TEST_CASE(std::bind(&test_image_checksum_calculate::run_suite_packed_image, tester)));
    master_test_suite.add(ts_packed);

    auto ts_virtual = BOOST_TEST_SUITE("test_virtual_view");
    ts_virtual->add(BOOST_TEST_CASE(std::bind(&test_image_checksum_calculate::run_suite_virtual_view, tester)));
    master_test_suite.add(ts_virtual);

    auto ts_dynamic = BOOST_TEST_SUITE("test_dynamic_image");
    ts_dynamic->add(BOOST_TEST_CASE(std::bind(&test_image_checksum_calculate::run_suite_dynamic_image, tester)));
    master_test_suite.add(ts_dynamic);

    return nullptr;
}

//int main(int argc, char* argv[])
//{
//    return but::unit_test_main(&init_unit_test, argc, argv);
//}
