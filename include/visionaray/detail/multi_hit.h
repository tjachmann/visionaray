// This file is distributed under the MIT license.
// See the LICENSE file for details.

#pragma once

#ifndef VSNRAY_DETAIL_MULTI_HIT_H
#define VSNRAY_DETAIL_MULTI_HIT_H 1

#include <cstddef>
#include <type_traits>

#include "../math/simd/type_traits.h"
#include "../array.h"

#include "tags.h"

namespace visionaray
{
namespace detail
{

//-------------------------------------------------------------------------------------------------
// is_multi_hit_record
//
// Trait class to check if a hit record template parameter is a multi-hit record.
// Assumes that the given parameter is any type of hit record!
//

template <typename HR>
struct is_multi_hit_record
{
    enum { value = false };
};

template <typename HR, size_t N>
struct is_multi_hit_record<array<HR, N>>
{
    enum { value = true };
};

} // detail

namespace algo
{

//-------------------------------------------------------------------------------------------------
// SIMD version of insert_sorted()
//
// Checks either for is_simd_vector<T>::value, or T contains a type scalar_type so that
// is_simd_vector<scalar_type>::value holds true
//
// TODO: consolidate w/ scalar version of insert_sorted() / move to a better place
// TODO: fix/consolidate overloading with SFINAE
//

template <
    template <typename...> class HR, // TODO: what if HR isn't a template ?!
    typename ...Args,
    typename RandIt,
    typename std::enable_if<simd::is_simd_vector<typename HR<Args...>::scalar_type>::value>::type* = nullptr,
    typename Cond
    >
VSNRAY_FUNC
void insert_sorted(HR<Args...> const& item, RandIt first, RandIt last, Cond cond)
{
    using S = typename HR<Args...>::scalar_type;
    using I = simd::int_type_t<S>;
    using M = simd::mask_type_t<S>;
    using int_array = simd::aligned_array_t<I>;

    int i = 0;
    int length = last - first;

    I pos = length;
    M active = true;

    while (i < length)
    {
        auto insert = cond(item, first[i]) && active;
        pos = select(insert, I(i), pos);
        active = select(insert, M(false), active);

        if (!any(active))
        {
            break;
        }

        ++i;
    }

    i = any(!active) ? length - 1 : 0;

    while (i >= 0)
    {
        M must_shift = I(i) > pos;
        M must_insert = I(i) == pos;

        auto update = [&](M const& condition, HR<Args...> const& item)
        {
            if (all(condition))
            {
                first[i] = item;
            }
            else if (any(condition))
            {
                int_array mask;
                store(mask, convert_to_int(condition));

                auto dst = unpack(first[i]);
                auto src = unpack(item);

                decltype(dst) arr;
                for (size_t j = 0; j < arr.size(); ++j)
                {
                    arr[j] = mask[j] ? src[j] : dst[j];
                }
                first[i] = simd::pack(arr);
            }
        };

        update(must_shift, first[i - 1]);
        update(must_insert, item);

        if (!any(must_shift) && !any(must_insert))
        {
            break;
        }

        --i;
    }
}

} // algo


//-------------------------------------------------------------------------------------------------
// update_if() for multi-hit traversal
//

template <
    typename HR1,
    typename HR2,
    typename std::enable_if< detail::is_multi_hit_record<HR1>::value>::type* = nullptr,
    typename std::enable_if<!detail::is_multi_hit_record<HR2>::value>::type* = nullptr,
    typename Cond
    >
VSNRAY_FUNC
inline void update_if(HR1& dst, HR2 const& src, Cond const& cond)
{
    VSNRAY_UNUSED(cond);

    algo::insert_sorted(src, dst.begin(), dst.end(), is_closer_t());
}

// TODO: find a way to enable_if this function w/o having to
// disable_if<MultiHit> the more general update_if overload
template <typename HR, size_t N, typename Cond>
VSNRAY_FUNC
inline void update_if(array<HR, N>& dst, array<HR, N> const& src, Cond const& cond)
{
    VSNRAY_UNUSED(cond);

    if (!any(dst[0].hit))
    {
        // Optimize for the case that no valid hit was found before
        dst = src;
    }
    else
    {
        for (auto const& hr : src)
        {
            if (!any(hr.hit))
            {
                break;
            }

            algo::insert_sorted(hr, dst.begin(), dst.end(), is_closer_t());
        }
    }
}


//-------------------------------------------------------------------------------------------------
// is_closer() for multi-hit traversal
//
// Test if a single-hit record is closer than any result in the multi-hit reference
//

template <
    typename HR1,
    typename HR2,
    typename std::enable_if<!detail::is_multi_hit_record<HR1>::value>::type* = nullptr,
    typename std::enable_if< detail::is_multi_hit_record<HR2>::value>::type* = nullptr,
    typename T
    >
VSNRAY_FUNC
inline simd::mask_type_t<T> is_closer(HR1 const& query, HR2 const& reference, T const& tmin, T const& tmax)
{
    using RT = simd::mask_type_t<T>;

    RT result(false);

    for (size_t i = 0; i < reference.size(); ++i)
    {
        result |= is_closer(query, reference[i], tmin, tmax);

        if (all(result))
        {
            break;
        }
    }

    return result;
}

// TODO: rename, this is not is_closer!!
template <
    typename HR1,
    typename HR2,
    typename std::enable_if<detail::is_multi_hit_record<HR1>::value>::type* = nullptr,
    typename std::enable_if<detail::is_multi_hit_record<HR2>::value>::type* = nullptr,
    typename T
    >
VSNRAY_FUNC
inline simd::mask_type_t<T> is_closer(HR1 const& query, HR2 const& reference, T const& tmin, T const& tmax)
{
    VSNRAY_UNUSED(query);
    VSNRAY_UNUSED(reference);
    VSNRAY_UNUSED(tmin);
    VSNRAY_UNUSED(tmax);

    return simd::mask_type_t<T>(true);
}

} // visionaray

#endif // VSNRAY_DETAIL_MULTI_HIT_H
