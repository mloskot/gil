//
// Copyright 2005-2007 Adobe Systems Incorporated
// Copyright 2019 Mateusz Loskot <mateusz at loskot dot net>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// The set of test cases and manipulations based on test/image/legacy/image.cpp
//
#ifndef BOOST_GIL_TEST_IMAGE_CHECKSUM_TEST_FIXTURE_HPP
#define BOOST_GIL_TEST_IMAGE_CHECKSUM_TEST_FIXTURE_HPP

#include <boost/gil.hpp>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/mp11.hpp>

#include <array>
#include <cmath>
#include <string>
#include <type_traits>

// Sample data form sample_view.cpp
extern boost::gil::rgb8c_planar_view_t sample_view;

namespace boost { namespace gil { namespace test { namespace fixture {

// Utility traits (NOTE: similar defined in Toolbox extension)
template <typename Pixel>
struct pixel_is_homogeneous : std::true_type {};

template <typename P, typename C, typename L>
struct pixel_is_homogeneous<packed_pixel<P, C, L> > : std::false_type {};

template <typename View>
struct view_is_homogeneous : pixel_is_homogeneous<typename View::value_type> {};

// Testing custom color conversion
template <typename C1, typename C2>
struct my_color_converter_impl : default_color_converter_impl<C1, C2> {};

template <typename C1>
struct my_color_converter_impl<C1, gray_t>
{
    template <typename P1, typename P2>
    void operator()(P1 const& src, P2& dst) const
    {
        default_color_converter_impl<C1, gray_t>()(src, dst);
        get_color(dst, gray_color_t{}) = channel_invert(get_color(dst, gray_color_t{}));
    }
};

struct my_color_converter
{
    template <typename SrcP, typename DstP>
    void operator()(SrcP const& src, DstP& dst) const
    {
        using cs_src_t = typename color_space_type<SrcP>::type;
        using cs_dst_t = typename color_space_type<DstP>::type;
        my_color_converter_impl<cs_src_t, cs_dst_t>()(src, dst);
    }
};

/// Testing virtual views
template <typename Pixel>
struct mandelbrot_fn
{
    using point_t = boost::gil::point_t;
    using const_t = mandelbrot_fn<Pixel>;
    using value_type = Pixel;
    using reference = value_type;
    using const_reference = value_type;
    using argument_type = point_t;
    using result_type = reference;
    static constexpr bool is_mutable = false;

    value_type _in_color, _out_color;
    point_t _img_size;
    static constexpr int MAX_ITERATIONS = 100;

    mandelbrot_fn() = default;
    mandelbrot_fn(point_t const& sz, value_type const& in_color, value_type const & out_color)
        : _in_color(in_color), _out_color(out_color), _img_size(sz)
    {}

    result_type operator()(point_t const& p) const
    {
        // normalize the coords to (-2..1, -1.5..1.5)
        // (actually make y -1.0..2 so it is asymmetric, so we can verify some view factory methods)
        //                                                                                      1.5f
        point<double> p_norm{p.x / (double)_img_size.x * 3 - 2, p.y / (double)_img_size.y * 3 - 1.0f};
        double t = get_num_iter(p_norm);
        t = std::pow(t, 0.2);

        value_type ret;
        for (int k = 0; k < num_channels<Pixel>::value; ++k)
            ret[k] = (typename channel_type<value_type>::type)(_in_color[k] * t + _out_color[k] * (1 - t));
        return ret;
    }

private:
    double get_num_iter(point<double> const& p) const
    {
        point<double> pz(0, 0);
        for (int i = 0; i < MAX_ITERATIONS; ++i)
        {
            pz = point<double>(pz.x * pz.x - pz.y * pz.y + p.x, 2 * pz.x * pz.y + p.y);
            if (pz.x * pz.x + pz.y * pz.y > 4)
                return i / (double)MAX_ITERATIONS;
        }
        return 0;
    }
};

template <typename View>
void x_gradient(View const& src, gray8s_view_t const& dst)
{
    for (typename View::x_coord_t y = 0; y < src.height(); ++y)
    {
        typename View::x_iterator src_it = src.row_begin(y);
        gray8s_view_t::x_iterator dst_it = dst.row_begin(y);

        for (typename View::y_coord_t x = 1; x < src.width() - 1; ++x)
            dst_it[x] = (src_it[x + 1] - src_it[x - 1]) / 2;
    }
}

// Base fixture defines image and view processing
class checksum_base
{
public:
    virtual ~checksum_base() {}

    void run_suite_basic_image()
    {
        test_basic<gray8_image_t>("gray8_");
        test_basic<bgr8_image_t>("bgr8_");
        test_basic<rgb8_image_t>("rgb8_");
        test_basic<rgb8_planar_image_t>("rgb8_planar_");

        // Test cases based on sample image from sample_view.cpp
        {
            gray8_image_t img(sample_view.dimensions());
            copy_and_convert_pixels(sample_view, view(img));
            test_view_transformations(view(img), "gray8_views_");
            test_histogram(const_view(img), "gray8_histogram_");
        }
        {
            bgr8_image_t img(sample_view.dimensions());
            copy_and_convert_pixels(sample_view, view(img));
            test_view_transformations(view(img), "bgr8_views_");
            test_histogram(const_view(img), "bgr8_histogram_");
        }
        {
            rgb8_image_t img(sample_view.dimensions());
            copy_and_convert_pixels(sample_view, view(img));
            test_view_transformations(view(img), "rgb8_views_");
            test_histogram(const_view(img), "rgb8_histogram_");
        }
        {
            rgb8_planar_image_t img(sample_view.dimensions());
            copy_and_convert_pixels(sample_view, view(img));
            test_view_transformations(view(img), "rgb8_planar_views_");
            test_histogram(const_view(img), "rgb8_planar_histogram_");
        }
    }

    void run_suite_packed_image()
    {
        using bgr121_ref_t = bit_aligned_pixel_reference
            <
                boost::uint8_t,
                mp11::mp_list_c<int, 1, 2, 1>,
                bgr_layout_t,
                true
            > const;
        using bgr121_image_t = image<bgr121_ref_t, false>;
        std::string name = "packed_bgr121_";
        test_basic<bgr121_image_t>(name);

        bgr121_image_t img(sample_view.dimensions());
        copy_and_convert_pixels(sample_view, view(img));
        test_view_transformations(view(img), name + "views_");
        test_histogram(const_view(img), name + "histogram_");
    }

    void run_suite_virtual_view()
    {
        using deref_t = mandelbrot_fn<rgb8_pixel_t>;
        using point_t = deref_t::point_t;
        using locator_t = virtual_2d_locator<deref_t, false>;
        using my_virt_view_t = image_view<locator_t>;

        point_t dims{200, 200};
        my_virt_view_t mandel(dims, locator_t({0, 0}, {1, 1},
            deref_t(dims, rgb8_pixel_t(255, 0, 255), rgb8_pixel_t(0, 255, 0))));

        gray8s_image_t img(dims);
        fill_pixels(view(img), 0);   // our x_gradient algorithm doesn't change the first & last column, so make sure they are 0
        x_gradient(color_converted_view<gray8_pixel_t>(mandel), view(img));
        test_case(color_converted_view<gray8_pixel_t>(const_view(img)), "virtual_color_converted_view_luminosity_gradient");

        test_view_transformations(mandel, "virtual_");
        test_histogram(mandel, "virtual_");
    }

    void run_suite_dynamic_image()
    {
        using any_image_t = any_image
            <
            mp11::mp_list
            <
            gray8_image_t,
            bgr8_image_t,
            argb8_image_t,
            rgb8_image_t,
            rgb8_planar_image_t
            >
            >;
        rgb8_planar_image_t img(sample_view.dimensions());
        copy_pixels(sample_view, view(img));
        any_image_t any_img = any_image_t(img);

        test_case(view(any_img), "dynamic_original");
        test_case(flipped_left_right_view(view(any_img)), "dynamic_flipped_left_right_view");
        test_case(flipped_up_down_view(view(any_img)), "dynamic_flipped_up_down_view");

        auto sub_img_view = subimage_view(view(any_img), 0, 0, 10, 15);
        test_case(sub_img_view, "dynamic_subimage_view");

        auto sub_view_rotated = subsampled_view(rotated180_view(view(any_img)), 2, 1);
        test_case(sub_view_rotated, "dynamic_subsampled_view_rotated180_view");
    }

protected:
    virtual void test_case_impl(rgb8c_view_t const& view, std::string const& name) = 0;

    template <typename View>
    void test_case(View const& img_view, std::string const& name)
    {
        rgb8_image_t rgb_img(img_view.dimensions());
        copy_and_convert_pixels(img_view, view(rgb_img));
        test_case_impl(const_view(rgb_img), name);
    }

private:
    template <typename Image>
    void test_basic(std::string name)
    {
        using view_t = typename Image::view_t;

        // make a 20x20 image
        Image img(typename view_t::point_t{20, 20});
        view_t const& img_view = view(img);

        // Fill it with red
        gil::rgb8_pixel_t red8{ 255, 0, 0 };
        gil::rgb8_pixel_t green8{ 0, 255, 0 };
        gil::rgb8_pixel_t blue8{ 0, 0, 255 };
        gil::rgb8_pixel_t white8{ 255, 255, 255 };

        using channel_t = typename view_t::value_type;
        channel_t red{0}, green{0}, blue{0}, white{0};
        color_convert(red8, red);
        color_convert(green8, green);
        color_convert(blue8, blue);
        color_convert(white8, white);

        fill(img_view.begin(), img_view.end(), red);
        color_convert(red8, img_view[0]);

        // Draw a blue line along the diagonal
        using xy_locator_t = typename view_t::xy_locator;
        xy_locator_t loc = img_view.xy_at(0, img_view.height() - 1);
        for (typename xy_locator_t::x_coord_t y = 0; y < img_view.height(); ++y)
        {
            *loc = blue;
            ++loc.x();
            loc.y()--;
        }
        // Draw a green dotted line along the main diagonal with step of 3
        loc = img_view.xy_at(img_view.width() - 1, img_view.height() - 1);
        while (loc.x() >= img_view.x_at(0, 0))
        {
            *loc = green;
            loc -= typename view_t::point_t{ 3, 3 };
        }

        // Clone and make every red pixel white
        Image img_white(img);
        for (auto it = view(img_white).end(); (it - 1) != view(img_white).begin(); --it)
        {
            if (*(it - 1) == red)
                *(it - 1) = white;
        }

        name = name + "draw_" ;
        test_case(img_view, name + "red_x");
        test_case(view(img_white), name + "white_x");
    }

    template <typename View>
    void test_view_transformations(View const& img_view, std::string name)
    {
        test_case(img_view, name + "original");
        test_case(subimage_view(img_view, iround(img_view.dimensions() / 4), iround(img_view.dimensions() / 2)), name + "subimage_view_cropped");
        test_case(color_converted_view<gray8_pixel_t>(img_view), name + "color_converted_view_gray8");
        test_case(color_converted_view<gray8_pixel_t>(img_view, my_color_converter()), name + "color_converted_view_my_gray8");
        test_case(transposed_view(img_view), name + "transposed_view");
        test_case(rotated180_view(img_view), name + "rotated180_view");
        test_case(rotated90cw_view(img_view), name + "rotated90cw_view");
        test_case(rotated90ccw_view(img_view), name + "rotated90ccw_view");
        test_case(flipped_up_down_view(img_view), name + "flipped_up_down_view");
        test_case(flipped_left_right_view(img_view), name + "flipped_left_right_view");
        test_case(subsampled_view(img_view, {2, 1}), name + "subsampled_view");
        test_case(kth_channel_view<0>(img_view), name + "kth_channel_view_0th");
        test_homogeneous_view_transformations(img_view, name, view_is_homogeneous<View>());
    }

    template <typename View>
    void test_homogeneous_view_transformations(View const& img_view, std::string name, std::true_type)
    {
        test_case(nth_channel_view(img_view, 0), name + "nth_channel_view_0th");
    }

    template <typename View>
    void test_homogeneous_view_transformations(View const& img_view, std::string name, std::false_type)
    {
        boost::ignore_unused(img_view);
        boost::ignore_unused(name);
    }

    template <typename View>
    void test_histogram(View const& img_view, std::string name)
    {
        std::array<unsigned char, 256> histogram = {};
        {
            auto gray_view = color_converted_view<gray8_pixel_t>(img_view);
            for (auto it = gray_view.begin(); it != gray_view.end(); ++it)
                ++histogram[*it];

            // Alternatively, prefer the algorithm with lambda
            // for_each_pixel(gray_view, [&histogram](gray8_pixel_t const& pixel) {
            //     ++histogram[pixel];
            // });
        }
        auto histogram_pixels = reinterpret_cast<gray8_pixel_t const*>(&histogram[0]);
        gray8c_view_t hist_view = interleaved_view(256, 1, histogram_pixels , 256);
        test_case(hist_view, name + "histogram");
    }

};

}}}} // namespace boost::gil::test::fixture

#endif
