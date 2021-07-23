// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include "../../../detail/math.h"

namespace MATH_NAMESPACE
{
namespace simd
{

//-------------------------------------------------------------------------------------------------
// int16 members
//

MATH_FUNC
VSNRAY_FORCE_INLINE int16::basic_int(
        int  x1, int  x2, int  x3, int  x4,
        int  x5, int  x6, int  x7, int  x8,
        int  x9, int x10, int x11, int x12,
        int x13, int x14, int x15, int x16
        )
    : value{
         x1,  x2,  x3,  x4,
         x5,  x6,  x7,  x8,
         x9, x10, x11, x12,
        x13, x14, x15, x16
        }
{
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16::basic_int(int const v[16])
    : value{
        v[ 0], v[ 1], v[ 2], v[ 3],
        v[ 4], v[ 5], v[ 6], v[ 7],
        v[ 8], v[ 9], v[10], v[11],
        v[12], v[13], v[14], v[15]
        }
{
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16::basic_int(int s)
    : value{s, s, s, s, s, s, s, s, s, s, s, s, s, s, s, s}
{
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16::basic_int(unsigned s)
    : value{
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s),
        static_cast<int>(s)
        }
{
}


//-------------------------------------------------------------------------------------------------
// Bitwise cast
//

MATH_FUNC
VSNRAY_FORCE_INLINE float16 reinterpret_as_float(int16 const& a)
{
    return *reinterpret_cast<float16 const*>(&a);
}


//-------------------------------------------------------------------------------------------------
// Static cast
//

MATH_FUNC
VSNRAY_FORCE_INLINE float16 convert_to_float(int16 const& a)
{
    return float16(
            static_cast<float>(a.value[ 0]),
            static_cast<float>(a.value[ 1]),
            static_cast<float>(a.value[ 2]),
            static_cast<float>(a.value[ 3]),
            static_cast<float>(a.value[ 4]),
            static_cast<float>(a.value[ 5]),
            static_cast<float>(a.value[ 6]),
            static_cast<float>(a.value[ 7]),
            static_cast<float>(a.value[ 8]),
            static_cast<float>(a.value[ 9]),
            static_cast<float>(a.value[10]),
            static_cast<float>(a.value[11]),
            static_cast<float>(a.value[12]),
            static_cast<float>(a.value[13]),
            static_cast<float>(a.value[14]),
            static_cast<float>(a.value[15])
            );
}


//-------------------------------------------------------------------------------------------------
// select intrinsic
//

MATH_FUNC
VSNRAY_FORCE_INLINE int16 select(mask16 const& m, int16 const& a, int16 const& b)
{
    return int16(
            m.value[ 0] ? a.value[ 0] : b.value[ 0],
            m.value[ 1] ? a.value[ 1] : b.value[ 1],
            m.value[ 2] ? a.value[ 2] : b.value[ 2],
            m.value[ 3] ? a.value[ 3] : b.value[ 3],
            m.value[ 4] ? a.value[ 4] : b.value[ 4],
            m.value[ 5] ? a.value[ 5] : b.value[ 5],
            m.value[ 6] ? a.value[ 6] : b.value[ 6],
            m.value[ 7] ? a.value[ 7] : b.value[ 7],
            m.value[ 8] ? a.value[ 8] : b.value[ 8],
            m.value[ 9] ? a.value[ 9] : b.value[ 9],
            m.value[10] ? a.value[10] : b.value[10],
            m.value[11] ? a.value[11] : b.value[11],
            m.value[12] ? a.value[12] : b.value[12],
            m.value[13] ? a.value[13] : b.value[13],
            m.value[14] ? a.value[14] : b.value[14],
            m.value[15] ? a.value[15] : b.value[15]
            );
}


//-------------------------------------------------------------------------------------------------
// Load / store / get
//

MATH_FUNC
VSNRAY_FORCE_INLINE void store(int dst[16], int16 const& v)
{
    dst[ 0] = v.value[ 0];
    dst[ 1] = v.value[ 1];
    dst[ 2] = v.value[ 2];
    dst[ 3] = v.value[ 3];
    dst[ 4] = v.value[ 4];
    dst[ 5] = v.value[ 5];
    dst[ 6] = v.value[ 6];
    dst[ 7] = v.value[ 7];
    dst[ 8] = v.value[ 8];
    dst[ 9] = v.value[ 9];
    dst[10] = v.value[10];
    dst[11] = v.value[11];
    dst[12] = v.value[12];
    dst[13] = v.value[13];
    dst[14] = v.value[14];
    dst[15] = v.value[15];
}

MATH_FUNC
VSNRAY_FORCE_INLINE void store(unsigned dst[16], int16 const& v)
{
    dst[ 0] = static_cast<unsigned>(v.value[ 0]);
    dst[ 1] = static_cast<unsigned>(v.value[ 1]);
    dst[ 2] = static_cast<unsigned>(v.value[ 2]);
    dst[ 3] = static_cast<unsigned>(v.value[ 3]);
    dst[ 4] = static_cast<unsigned>(v.value[ 4]);
    dst[ 5] = static_cast<unsigned>(v.value[ 5]);
    dst[ 6] = static_cast<unsigned>(v.value[ 6]);
    dst[ 7] = static_cast<unsigned>(v.value[ 7]);
    dst[ 8] = static_cast<unsigned>(v.value[ 8]);
    dst[ 9] = static_cast<unsigned>(v.value[ 9]);
    dst[10] = static_cast<unsigned>(v.value[10]);
    dst[11] = static_cast<unsigned>(v.value[11]);
    dst[12] = static_cast<unsigned>(v.value[12]);
    dst[13] = static_cast<unsigned>(v.value[13]);
    dst[14] = static_cast<unsigned>(v.value[14]);
    dst[15] = static_cast<unsigned>(v.value[15]);
}

template <size_t I>
MATH_FUNC
VSNRAY_FORCE_INLINE int& get(int16& v)
{
    static_assert(I >= 0 && I < 16, "Index out of range for SIMD vector access");

    return v.value[I];
}

template <size_t I>
MATH_FUNC
VSNRAY_FORCE_INLINE int const& get(int16 const& v)
{
    static_assert(I >= 0 && I < 16, "Index out of range for SIMD vector access");

    return v.value[I];
}


//-------------------------------------------------------------------------------------------------
// Basic arithmethic
//

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator+(int16 const& v)
{
    return int16(
            +v.value[ 0],
            +v.value[ 1],
            +v.value[ 2],
            +v.value[ 3],
            +v.value[ 4],
            +v.value[ 5],
            +v.value[ 6],
            +v.value[ 7],
            +v.value[ 8],
            +v.value[ 9],
            +v.value[10],
            +v.value[11],
            +v.value[12],
            +v.value[13],
            +v.value[14],
            +v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator-(int16 const& v)
{
    return int16(
            -v.value[ 0],
            -v.value[ 1],
            -v.value[ 2],
            -v.value[ 3],
            -v.value[ 4],
            -v.value[ 5],
            -v.value[ 6],
            -v.value[ 7],
            -v.value[ 8],
            -v.value[ 9],
            -v.value[10],
            -v.value[11],
            -v.value[12],
            -v.value[13],
            -v.value[14],
            -v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator+(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] + v.value[ 0],
            u.value[ 1] + v.value[ 1],
            u.value[ 2] + v.value[ 2],
            u.value[ 3] + v.value[ 3],
            u.value[ 4] + v.value[ 4],
            u.value[ 5] + v.value[ 5],
            u.value[ 6] + v.value[ 6],
            u.value[ 7] + v.value[ 7],
            u.value[ 8] + v.value[ 8],
            u.value[ 9] + v.value[ 9],
            u.value[10] + v.value[10],
            u.value[11] + v.value[11],
            u.value[12] + v.value[12],
            u.value[13] + v.value[13],
            u.value[14] + v.value[14],
            u.value[15] + v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator-(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] - v.value[ 0],
            u.value[ 1] - v.value[ 1],
            u.value[ 2] - v.value[ 2],
            u.value[ 3] - v.value[ 3],
            u.value[ 4] - v.value[ 4],
            u.value[ 5] - v.value[ 5],
            u.value[ 6] - v.value[ 6],
            u.value[ 7] - v.value[ 7],
            u.value[ 8] - v.value[ 8],
            u.value[ 9] - v.value[ 9],
            u.value[10] - v.value[10],
            u.value[11] - v.value[11],
            u.value[12] - v.value[12],
            u.value[13] - v.value[13],
            u.value[14] - v.value[14],
            u.value[15] - v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator*(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] * v.value[ 0],
            u.value[ 1] * v.value[ 1],
            u.value[ 2] * v.value[ 2],
            u.value[ 3] * v.value[ 3],
            u.value[ 4] * v.value[ 4],
            u.value[ 5] * v.value[ 5],
            u.value[ 6] * v.value[ 6],
            u.value[ 7] * v.value[ 7],
            u.value[ 8] * v.value[ 8],
            u.value[ 9] * v.value[ 9],
            u.value[10] * v.value[10],
            u.value[11] * v.value[11],
            u.value[12] * v.value[12],
            u.value[13] * v.value[13],
            u.value[14] * v.value[14],
            u.value[15] * v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator/(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] / v.value[ 0],
            u.value[ 1] / v.value[ 1],
            u.value[ 2] / v.value[ 2],
            u.value[ 3] / v.value[ 3],
            u.value[ 4] / v.value[ 4],
            u.value[ 5] / v.value[ 5],
            u.value[ 6] / v.value[ 6],
            u.value[ 7] / v.value[ 7],
            u.value[ 8] / v.value[ 8],
            u.value[ 9] / v.value[ 9],
            u.value[10] / v.value[10],
            u.value[11] / v.value[11],
            u.value[12] / v.value[12],
            u.value[13] / v.value[13],
            u.value[14] / v.value[14],
            u.value[15] / v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator%(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] % v.value[ 0],
            u.value[ 1] % v.value[ 1],
            u.value[ 2] % v.value[ 2],
            u.value[ 3] % v.value[ 3],
            u.value[ 4] % v.value[ 4],
            u.value[ 5] % v.value[ 5],
            u.value[ 6] % v.value[ 6],
            u.value[ 7] % v.value[ 7],
            u.value[ 8] % v.value[ 8],
            u.value[ 9] % v.value[ 9],
            u.value[10] % v.value[10],
            u.value[11] % v.value[11],
            u.value[12] % v.value[12],
            u.value[13] % v.value[13],
            u.value[14] % v.value[14],
            u.value[15] % v.value[15]
            );
}


//-------------------------------------------------------------------------------------------------
// Bitwise operations
//

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator&(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] & v.value[ 0],
            u.value[ 1] & v.value[ 1],
            u.value[ 2] & v.value[ 2],
            u.value[ 3] & v.value[ 3],
            u.value[ 4] & v.value[ 4],
            u.value[ 5] & v.value[ 5],
            u.value[ 6] & v.value[ 6],
            u.value[ 7] & v.value[ 7],
            u.value[ 8] & v.value[ 8],
            u.value[ 9] & v.value[ 9],
            u.value[10] & v.value[10],
            u.value[11] & v.value[11],
            u.value[12] & v.value[12],
            u.value[13] & v.value[13],
            u.value[14] & v.value[14],
            u.value[15] & v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator|(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] | v.value[ 0],
            u.value[ 1] | v.value[ 1],
            u.value[ 2] | v.value[ 2],
            u.value[ 3] | v.value[ 3],
            u.value[ 4] | v.value[ 4],
            u.value[ 5] | v.value[ 5],
            u.value[ 6] | v.value[ 6],
            u.value[ 7] | v.value[ 7],
            u.value[ 8] | v.value[ 8],
            u.value[ 9] | v.value[ 9],
            u.value[10] | v.value[10],
            u.value[11] | v.value[11],
            u.value[12] | v.value[12],
            u.value[13] | v.value[13],
            u.value[14] | v.value[14],
            u.value[15] | v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator^(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] ^ v.value[ 0],
            u.value[ 1] ^ v.value[ 1],
            u.value[ 2] ^ v.value[ 2],
            u.value[ 3] ^ v.value[ 3],
            u.value[ 4] ^ v.value[ 4],
            u.value[ 5] ^ v.value[ 5],
            u.value[ 6] ^ v.value[ 6],
            u.value[ 7] ^ v.value[ 7],
            u.value[ 8] ^ v.value[ 8],
            u.value[ 9] ^ v.value[ 9],
            u.value[10] ^ v.value[10],
            u.value[11] ^ v.value[11],
            u.value[12] ^ v.value[12],
            u.value[13] ^ v.value[13],
            u.value[14] ^ v.value[14],
            u.value[15] ^ v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator<<(int16 const& a, int count)
{
    return int16(
            a.value[ 0] << count,
            a.value[ 1] << count,
            a.value[ 2] << count,
            a.value[ 3] << count,
            a.value[ 4] << count,
            a.value[ 5] << count,
            a.value[ 6] << count,
            a.value[ 7] << count,
            a.value[ 8] << count,
            a.value[ 9] << count,
            a.value[10] << count,
            a.value[11] << count,
            a.value[12] << count,
            a.value[13] << count,
            a.value[14] << count,
            a.value[15] << count
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 operator>>(int16 const& a, int count)
{
    return int16(
            a.value[ 0] >> count,
            a.value[ 1] >> count,
            a.value[ 2] >> count,
            a.value[ 3] >> count,
            a.value[ 4] >> count,
            a.value[ 5] >> count,
            a.value[ 6] >> count,
            a.value[ 7] >> count,
            a.value[ 8] >> count,
            a.value[ 9] >> count,
            a.value[10] >> count,
            a.value[11] >> count,
            a.value[12] >> count,
            a.value[13] >> count,
            a.value[14] >> count,
            a.value[15] >> count
            );
}


//-------------------------------------------------------------------------------------------------
// Logical operations
//

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator&&(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] && v.value[ 0],
            u.value[ 1] && v.value[ 1],
            u.value[ 2] && v.value[ 2],
            u.value[ 3] && v.value[ 3],
            u.value[ 4] && v.value[ 4],
            u.value[ 5] && v.value[ 5],
            u.value[ 6] && v.value[ 6],
            u.value[ 7] && v.value[ 7],
            u.value[ 8] && v.value[ 8],
            u.value[ 9] && v.value[ 9],
            u.value[10] && v.value[10],
            u.value[11] && v.value[11],
            u.value[12] && v.value[12],
            u.value[13] && v.value[13],
            u.value[14] && v.value[14],
            u.value[15] && v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator||(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] || v.value[ 0],
            u.value[ 1] || v.value[ 1],
            u.value[ 2] || v.value[ 2],
            u.value[ 3] || v.value[ 3],
            u.value[ 4] || v.value[ 4],
            u.value[ 5] || v.value[ 5],
            u.value[ 6] || v.value[ 6],
            u.value[ 7] || v.value[ 7],
            u.value[ 8] || v.value[ 8],
            u.value[ 9] || v.value[ 9],
            u.value[10] || v.value[10],
            u.value[11] || v.value[11],
            u.value[12] || v.value[12],
            u.value[13] || v.value[13],
            u.value[14] || v.value[14],
            u.value[15] || v.value[15]
            );
}


//-------------------------------------------------------------------------------------------------
// Comparisons
//

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator<(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] < v.value[ 0],
            u.value[ 1] < v.value[ 1],
            u.value[ 2] < v.value[ 2],
            u.value[ 3] < v.value[ 3],
            u.value[ 4] < v.value[ 4],
            u.value[ 5] < v.value[ 5],
            u.value[ 6] < v.value[ 6],
            u.value[ 7] < v.value[ 7],
            u.value[ 8] < v.value[ 8],
            u.value[ 9] < v.value[ 9],
            u.value[10] < v.value[10],
            u.value[11] < v.value[11],
            u.value[12] < v.value[12],
            u.value[13] < v.value[13],
            u.value[14] < v.value[14],
            u.value[15] < v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator>(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] > v.value[ 0],
            u.value[ 1] > v.value[ 1],
            u.value[ 2] > v.value[ 2],
            u.value[ 3] > v.value[ 3],
            u.value[ 4] > v.value[ 4],
            u.value[ 5] > v.value[ 5],
            u.value[ 6] > v.value[ 6],
            u.value[ 7] > v.value[ 7],
            u.value[ 8] > v.value[ 8],
            u.value[ 9] > v.value[ 9],
            u.value[10] > v.value[10],
            u.value[11] > v.value[11],
            u.value[12] > v.value[12],
            u.value[13] > v.value[13],
            u.value[14] > v.value[14],
            u.value[15] > v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator==(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] == v.value[ 0],
            u.value[ 1] == v.value[ 1],
            u.value[ 2] == v.value[ 2],
            u.value[ 3] == v.value[ 3],
            u.value[ 4] == v.value[ 4],
            u.value[ 5] == v.value[ 5],
            u.value[ 6] == v.value[ 6],
            u.value[ 7] == v.value[ 7],
            u.value[ 8] == v.value[ 8],
            u.value[ 9] == v.value[ 9],
            u.value[10] == v.value[10],
            u.value[11] == v.value[11],
            u.value[12] == v.value[12],
            u.value[13] == v.value[13],
            u.value[14] == v.value[14],
            u.value[15] == v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator<=(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] <= v.value[ 0],
            u.value[ 1] <= v.value[ 1],
            u.value[ 2] <= v.value[ 2],
            u.value[ 3] <= v.value[ 3],
            u.value[ 4] <= v.value[ 4],
            u.value[ 5] <= v.value[ 5],
            u.value[ 6] <= v.value[ 6],
            u.value[ 7] <= v.value[ 7],
            u.value[ 8] <= v.value[ 8],
            u.value[ 9] <= v.value[ 9],
            u.value[10] <= v.value[10],
            u.value[11] <= v.value[11],
            u.value[12] <= v.value[12],
            u.value[13] <= v.value[13],
            u.value[14] <= v.value[14],
            u.value[15] <= v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator>=(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] >= v.value[ 0],
            u.value[ 1] >= v.value[ 1],
            u.value[ 2] >= v.value[ 2],
            u.value[ 3] >= v.value[ 3],
            u.value[ 4] >= v.value[ 4],
            u.value[ 5] >= v.value[ 5],
            u.value[ 6] >= v.value[ 6],
            u.value[ 7] >= v.value[ 7],
            u.value[ 8] >= v.value[ 8],
            u.value[ 9] >= v.value[ 9],
            u.value[10] >= v.value[10],
            u.value[11] >= v.value[11],
            u.value[12] >= v.value[12],
            u.value[13] >= v.value[13],
            u.value[14] >= v.value[14],
            u.value[15] >= v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE mask16 operator!=(int16 const& u, int16 const& v)
{
    return mask16(
            u.value[ 0] != v.value[ 0],
            u.value[ 1] != v.value[ 1],
            u.value[ 2] != v.value[ 2],
            u.value[ 3] != v.value[ 3],
            u.value[ 4] != v.value[ 4],
            u.value[ 5] != v.value[ 5],
            u.value[ 6] != v.value[ 6],
            u.value[ 7] != v.value[ 7],
            u.value[ 8] != v.value[ 8],
            u.value[ 9] != v.value[ 9],
            u.value[10] != v.value[10],
            u.value[11] != v.value[11],
            u.value[12] != v.value[12],
            u.value[13] != v.value[13],
            u.value[14] != v.value[14],
            u.value[15] != v.value[15]
            );
}


//-------------------------------------------------------------------------------------------------
// Math functions
//

MATH_FUNC
VSNRAY_FORCE_INLINE int16 min(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] < v.value[ 0] ? u.value[ 0] : v.value[ 0],
            u.value[ 1] < v.value[ 1] ? u.value[ 1] : v.value[ 1],
            u.value[ 2] < v.value[ 2] ? u.value[ 2] : v.value[ 2],
            u.value[ 3] < v.value[ 3] ? u.value[ 3] : v.value[ 3],
            u.value[ 4] < v.value[ 4] ? u.value[ 4] : v.value[ 4],
            u.value[ 5] < v.value[ 5] ? u.value[ 5] : v.value[ 5],
            u.value[ 6] < v.value[ 6] ? u.value[ 6] : v.value[ 6],
            u.value[ 7] < v.value[ 7] ? u.value[ 7] : v.value[ 7],
            u.value[ 8] < v.value[ 8] ? u.value[ 8] : v.value[ 8],
            u.value[ 9] < v.value[ 9] ? u.value[ 9] : v.value[ 9],
            u.value[10] < v.value[10] ? u.value[10] : v.value[10],
            u.value[11] < v.value[11] ? u.value[11] : v.value[11],
            u.value[12] < v.value[12] ? u.value[12] : v.value[12],
            u.value[13] < v.value[13] ? u.value[13] : v.value[13],
            u.value[14] < v.value[14] ? u.value[14] : v.value[14],
            u.value[15] < v.value[15] ? u.value[15] : v.value[15]
            );
}

MATH_FUNC
VSNRAY_FORCE_INLINE int16 max(int16 const& u, int16 const& v)
{
    return int16(
            u.value[ 0] < v.value[ 0] ? v.value[ 0] : u.value[ 0],
            u.value[ 1] < v.value[ 1] ? v.value[ 1] : u.value[ 1],
            u.value[ 2] < v.value[ 2] ? v.value[ 2] : u.value[ 2],
            u.value[ 3] < v.value[ 3] ? v.value[ 3] : u.value[ 3],
            u.value[ 4] < v.value[ 4] ? v.value[ 4] : u.value[ 4],
            u.value[ 5] < v.value[ 5] ? v.value[ 5] : u.value[ 5],
            u.value[ 6] < v.value[ 6] ? v.value[ 6] : u.value[ 6],
            u.value[ 7] < v.value[ 7] ? v.value[ 7] : u.value[ 7],
            u.value[ 8] < v.value[ 8] ? v.value[ 8] : u.value[ 8],
            u.value[ 9] < v.value[ 9] ? v.value[ 9] : u.value[ 9],
            u.value[10] < v.value[10] ? v.value[10] : u.value[10],
            u.value[11] < v.value[11] ? v.value[11] : u.value[11],
            u.value[12] < v.value[12] ? v.value[12] : u.value[12],
            u.value[13] < v.value[13] ? v.value[13] : u.value[13],
            u.value[14] < v.value[14] ? v.value[14] : u.value[14],
            u.value[15] < v.value[15] ? v.value[15] : u.value[15]
            );
}

} // simd
} // MATH_NAMESPACE
