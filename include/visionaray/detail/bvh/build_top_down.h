// This file is distributed under the MIT license.
// See the LICENSE file for details.

#pragma once

#ifndef VSNRAY_DETAIL_BVH_BUILD_TOP_DOWN_H
#define VSNRAY_DETAIL_BVH_BUILD_TOP_DOWN_H 1

#include <algorithm>
#include <cassert>
#include <type_traits>

#include "../../aligned_vector.h"

#include "../algorithm.h"

namespace visionaray
{
namespace detail
{

//--------------------------------------------------------------------------------------------------
// build_top_down_impl
//

template <typename Nodes, typename Indices, typename Builder, typename LeafInfo, typename Data>
inline void build_top_down_impl(
        int             index,
        Nodes&          nodes,
        Indices&        indices,
        Builder&        builder,
        LeafInfo const& leaf,
        Data const&     data,
        int             max_leaf_size
        )
{
    typename Builder::leaf_infos childs;

    auto split = builder.split(childs, leaf, data, max_leaf_size);

    if (split)
    {
        auto first_child_index = static_cast<int>(nodes.size());

        nodes[index].set_inner(leaf.prim_bounds, first_child_index);

        nodes.emplace_back();
        nodes.emplace_back();

        // Construct right subtree
        build_top_down_impl(
                first_child_index + 1,
                nodes,
                indices,
                builder,
                childs[1],
                data,
                max_leaf_size
                );

        // Construct left subtree
        build_top_down_impl(
                first_child_index + 0,
                nodes,
                indices,
                builder,
                childs[0],
                data,
                max_leaf_size
                );
    }
    else
    {
        auto first = static_cast<int>(indices.size());
        auto count = builder.insert_indices(indices, leaf);

        nodes[index].set_leaf(leaf.prim_bounds, first, count);
    }
}


//--------------------------------------------------------------------------------------------------
// build_top_down
//

template <typename Tree, typename Builder, typename Root, typename I>
inline void build_top_down_work(
        Tree&          tree,
        Builder&       builder,
        Root           root,
        I              first,
        I              /*last*/,
        int            max_leaf_size,
        std::true_type /*is_index_bvh*/
        )
{
    build_top_down_impl(
            0, // root node index
            tree.nodes(),
            tree.indices(),
            builder,
            root,
            first, // primitive data
            max_leaf_size
            );
}

template <typename Tree, typename Builder, typename Root, typename I>
inline void build_top_down_work(
        Tree&           tree,
        Builder&        builder,
        Root            root,
        I               first,
        I               /*last*/,
        int             max_leaf_size,
        std::false_type /*is_index_bvh*/
        )
{
    // TODO:
    // Maybe rewrite the builder to directly shuffle the primitives?!?!

    aligned_vector<unsigned> indices;

    //assert(builder.use_spatial_splits == false);

    auto uss = builder.use_spatial_splits;

    builder.use_spatial_splits = false;

    build_top_down_impl(
            0, // root node index
            tree.nodes(),
            indices,
            builder,
            root,
            first, // primitive data
            max_leaf_size
            );

    builder.use_spatial_splits = uss;

    assert(indices.size() == tree.primitives().size());

    // Reorder the primitives according to the indices.
    algo::reorder_n(indices.begin(), tree.primitives().begin(), indices.size());
}

template <typename Tree, typename Builder, typename I>
inline void build_top_down(Tree& tree, Builder& builder, I first, I last, int max_leaf_size = -1)
{
    if (max_leaf_size <= 0)
    {
        max_leaf_size = 4;
    }

    // Precompute primitive data needed by the builder

    auto root = builder.init(first, last);

    // Preallocate memory
    // Guess number of nodes...

    auto count = std::distance(first, last);

    tree.clear(2 * (count / max_leaf_size));

    // Build the tree

    // Create root node
    tree.nodes().emplace_back();

    build_top_down_work(tree, builder, root, first, last, max_leaf_size, is_index_bvh<Tree>());
}

} // detail
} // visionaray

#endif // VSNRAY_DETAIL_BVH_BUILD_TOP_DOWN_H
