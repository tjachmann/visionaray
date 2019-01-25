// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/filesystem.hpp>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <visionaray/math/constants.h>
#include <visionaray/math/forward.h>
#include <visionaray/math/unorm.h>
#include <visionaray/math/vector.h>
#include <visionaray/texture/texture.h>

#include "cfile.h"
#include "model.h"
#include "sg.h"
#include "vsnray_loader.h"

using namespace visionaray;


namespace data_file
{

//-------------------------------------------------------------------------------------------------
// (included) data file meta data
//

struct meta_data
{
    enum encoding_t
    {
        Ascii,
        Binary
    };

    // VecN are binary compatible w/ visionaray::vecN
    enum data_type_t
    {
        U8,
        Float,
        Vec2u8,
        Vec2f,
        Vec2,
        Vec3u8,
        Vec3f,
        Vec3,
        Vec4u8,
        Vec4f,
        Vec4
    };

    enum compression_t
    {
        Raw
    };

    std::string   path;
    encoding_t    encoding    = Binary;
    data_type_t   data_type   = U8;
    int           num_items   = 0;
    compression_t compression = Raw;
    char          separator   = ' ';
};

} // data_file


//-------------------------------------------------------------------------------------------------
// Floating point number parser
//

template <typename It, typename Vector>
bool parse_floats(It first, It last, Vector& vec, char separator = ' ')
{
    namespace qi = boost::spirit::qi;

    return qi::phrase_parse(
            first,
            last,
            qi::float_ % *qi::char_(separator),
            qi::ascii::space,
            vec
            );
}

template <size_t N, typename Container>
bool parse_as_vecNf(data_file::meta_data md, Container& vecNfs)
{
    if (md.data_type == data_file::meta_data::Float)
    {
        if (md.num_items % N != 0)
        {
            return false;
        }

        boost::iostreams::mapped_file_source file(md.path);

        std::vector<float> floats;

        if (md.encoding == data_file::meta_data::Ascii)
        {
            boost::string_ref text(file.data(), file.size());

            parse_floats(text.cbegin(), text.cend(), floats, md.separator);

            if (floats.size() != md.num_items)
            {
                return false;
            }
        }
        else // Binary
        {
            floats.resize(md.num_items);
            std::copy(
                file.data(),
                file.data() + file.size(),
                reinterpret_cast<char*>(floats.data())
                );
        }

        vecNfs.resize(md.num_items / N);
        for (size_t i = 0; i < vecNfs.size(); ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                vecNfs[i][j] = floats[i * N + j];
            }
        }
    }

    return true;
}

template <typename Container>
bool parse_as_vec2f(data_file::meta_data md, Container& vec2fs)
{
    return parse_as_vecNf<2>(md, vec2fs);
}

template <typename Container>
bool parse_as_vec3f(data_file::meta_data md, Container& vec3fs)
{
    return parse_as_vecNf<3>(md, vec3fs);
}


//-------------------------------------------------------------------------------------------------
// .vsnray parser
//

class vsnray_parser
{
public:

    vsnray_parser(std::string filename)
        : filename_(filename)
    {
    }

    void parse_children(std::shared_ptr<sg::node> parent, rapidjson::Value const& entries);

    template <typename Object>
    std::shared_ptr<sg::node> parse_node(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_camera(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_include(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_point_light(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_spot_light(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_reference(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_transform(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_surface_properties(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_triangle_mesh(Object const& obj);

    template <typename Object>
    std::shared_ptr<sg::node> parse_indexed_triangle_mesh(Object const& obj);


    template <typename Object>
    data_file::meta_data parse_file_meta_data(Object const& obj);

private:

    std::string filename_;

};


//-------------------------------------------------------------------------------------------------
// Parse nodes
//

void vsnray_parser::parse_children(std::shared_ptr<sg::node> parent, rapidjson::Value const& entries)
{
    if (!entries.IsArray())
    {
        throw std::runtime_error("");
    }

    parent->children().resize(entries.Capacity());

    size_t i = 0;
    for (auto const& c : entries.GetArray())
    {
        auto const& obj = c.GetObject();

        parent->children().at(i++) = parse_node(obj);
    }

    if (i != entries.Capacity())
    {
        throw std::runtime_error("");
    }
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_node(Object const& obj)
{
    std::shared_ptr<sg::node> result = nullptr;

    if (obj.HasMember("type"))
    {
        // Parse individual node types
        auto const& type_string = obj["type"];
        if (strncmp(type_string.GetString(), "node", 4) == 0)
        {
            // Empty node, (may still contain children, e.g. root)
            result = std::make_shared<sg::node>();
        }
        else if (strncmp(type_string.GetString(), "camera", 6) == 0)
        {
            result = parse_camera(obj);
        }
        else if (strncmp(type_string.GetString(), "include", 6) == 0)
        {
            result = parse_include(obj);
        }
        else if (strncmp(type_string.GetString(), "point_light", 11) == 0)
        {
            result = parse_point_light(obj);
        }
        else if (strncmp(type_string.GetString(), "spot_light", 10) == 0)
        {
            result = parse_spot_light(obj);
        }
        else if (strncmp(type_string.GetString(), "reference", 9) == 0)
        {
            result = parse_reference(obj);
        }
        else if (strncmp(type_string.GetString(), "transform", 9) == 0)
        {
            result = parse_transform(obj);
        }
        else if (strncmp(type_string.GetString(), "surface_properties", 18) == 0)
        {
            result = parse_surface_properties(obj);
        }
        else if (strncmp(type_string.GetString(), "triangle_mesh", 13) == 0)
        {
            result = parse_triangle_mesh(obj);
        }
        else if (strncmp(type_string.GetString(), "indexed_triangle_mesh", 21) == 0)
        {
            result = parse_indexed_triangle_mesh(obj);
        }
        else
        {
            throw std::runtime_error("");
        }

        // Parse common node properties
        if (obj.HasMember("name"))
        {
            assert(result != nullptr);

            rapidjson::Value const& name = obj["name"];
            result->name() = name.GetString();
        }

        if (obj.HasMember("children"))
        {
            assert(result != nullptr);

            rapidjson::Value const& children = obj["children"];
            parse_children(result, children);
        }
    }
    else
    {
        throw std::runtime_error("");
    }

    return result;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_camera(Object const& obj)
{
    auto cam = std::make_shared<sg::camera>();

    vec3 eye(0.0f);
    if (obj.HasMember("eye"))
    {
        auto const& cam_eye = obj["eye"];

        int i = 0;
        for (auto const& item : cam_eye.GetArray())
        {
            eye[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    vec3 center(0.0f);
    if (obj.HasMember("center"))
    {
        auto const& cam_center = obj["center"];

        int i = 0;
        for (auto const& item : cam_center.GetArray())
        {
            center[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    vec3 up(0.0f);
    if (obj.HasMember("up"))
    {
        auto const& cam_up = obj["up"];

        int i = 0;
        for (auto const& item : cam_up.GetArray())
        {
            up[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    float fovy = 45.0f;
    if (obj.HasMember("fovy"))
    {
        fovy = obj["fovy"].GetFloat();
    }

    float znear = 0.001f;
    if (obj.HasMember("znear"))
    {
        znear = obj["znear"].GetFloat();
    }

    float zfar = 1000.0f;
    if (obj.HasMember("zfar"))
    {
        zfar = obj["zfar"].GetFloat();
    }

    recti viewport(0, 0, 0, 0);
    if (obj.HasMember("viewport"))
    {
        auto const& cam_viewport = obj["viewport"];

        int i = 0;
        for (auto const& item : cam_viewport.GetArray())
        {
            viewport.data()[i++] = item.GetInt();
        }

        if (i != 4)
        {
            throw std::runtime_error("");
        }
    }

    float lens_radius = 0.1f;
    if (obj.HasMember("lens_radius"))
    {
        lens_radius = obj["lens_radius"].GetFloat();
    }

    float focal_distance = 10.0f;
    if (obj.HasMember("focal_distance"))
    {
        focal_distance = obj["focal_distance"].GetFloat();
    }

    float aspect = viewport.w > 0 && viewport.h > 0
                 ? viewport.w / static_cast<float>(viewport.h)
                 : 1;

    cam->perspective(fovy * constants::degrees_to_radians<float>(), aspect, znear, zfar);
    if (viewport.w > 0 && viewport.h > 0)
    {
        cam->set_viewport(viewport);
    }
    cam->set_lens_radius(lens_radius);
    cam->set_focal_distance(focal_distance);
    cam->look_at(eye, center, up);

    return cam;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_include(Object const& obj)
{
    auto inc = std::make_shared<sg::node>();

    if (obj.HasMember("path"))
    {
        std::string path_string(obj["path"].GetString());

        boost::filesystem::path p(path_string);

        if (!p.is_absolute())
        {
            // Extract base path
            boost::filesystem::path bp(filename_);
            bp = bp.parent_path();

            // Append path to base path
            p = bp / p;

            path_string = p.string();
        }

        model mod;
        if (mod.load(path_string))
        {
            if (mod.scene_graph == nullptr)
            {
                std::unordered_map<std::string, std::shared_ptr<sg::texture2d<vector<4, unorm<8>>>>> texture_map;

                for (auto it = mod.texture_map.begin(); it != mod.texture_map.end(); ++it)
                {
                    auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
                    tex->name() = it->first;
                    tex->resize(it->second.width(), it->second.height());
                    tex->reset(it->second.data());
                    tex->set_filter_mode(it->second.get_filter_mode());
                    tex->set_address_mode(it->second.get_address_mode());

                    texture_map.insert(std::make_pair(it->first, tex));
                }

                if (mod.primitives.size() > 0)
                {
                    // Vertices (disassemble triangles..)
                    for (auto tri : mod.primitives)
                    {
                        if (tri.geom_id >= inc->children().size())
                        {
                            auto props = std::make_shared<sg::surface_properties>();

                            // Add material
                            auto obj = std::make_shared<sg::obj_material>();
                            obj->ca = mod.materials[tri.geom_id].ca;
                            obj->cd = mod.materials[tri.geom_id].cd;
                            obj->cs = mod.materials[tri.geom_id].cs;
                            obj->ce = mod.materials[tri.geom_id].ce;
                            obj->cr = mod.materials[tri.geom_id].cr;
                            obj->ior = mod.materials[tri.geom_id].ior;
                            obj->absorption = mod.materials[tri.geom_id].absorption;
                            obj->transmission = mod.materials[tri.geom_id].transmission;
                            obj->specular_exp = mod.materials[tri.geom_id].specular_exp;
                            obj->illum = mod.materials[tri.geom_id].illum;
                            props->material() = obj;

                            bool insert_dummy = false;

                            if (tri.geom_id < mod.textures.size())
                            {
                                // Find texture in texture_map
                                bool found = false;
                                for (auto it = mod.texture_map.begin(); it != mod.texture_map.end(); ++it)
                                {
                                    auto ref = texture_ref<vector<4, unorm<8>>, 2>(it->second);

                                    if (ref.data() == mod.textures[tri.geom_id].data())
                                    {
                                        std::string name = it->first;
                                        // Find in local texture map
                                        auto res = texture_map.find(name);
                                        if (res != texture_map.end())
                                        {
                                            props->add_texture(res->second);
                                            found = true;
                                            break;
                                        }
                                    }
                                }

                                if (!found)
                                {
                                    insert_dummy = true;
                                }
                            }
                            else
                            {
                                insert_dummy = true;
                            }

                            if (insert_dummy)
                            {
                                // Add a dummy texture
                                vector<4, unorm<8>> dummy_texel(1.0f, 1.0f, 1.0f, 1.0f);
                                auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
                                tex->resize(1, 1);
                                tex->set_address_mode(Wrap);
                                tex->set_filter_mode(Nearest);
                                tex->reset(&dummy_texel);
                                props->add_texture(tex);
                            }

                            // Add to scene graph
                            props->add_child(std::make_shared<sg::triangle_mesh>());
                            inc->add_child(props);
                        }

                        auto mesh = std::dynamic_pointer_cast<sg::triangle_mesh>(
                                inc->children()[tri.geom_id]->children()[0]
                                );

                        vec3 verts[3] = {
                            tri.v1,
                            tri.v1 + tri.e1,
                            tri.v1 + tri.e2
                            };
                        mesh->vertices.insert(mesh->vertices.end(), verts, verts + 3);

                        if (mod.shading_normals.size() > tri.prim_id * 3 + 3)
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->normals.push_back(mod.shading_normals[tri.prim_id * 3 + i]);
                            }
                        }
                        else
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->normals.push_back(normalize(cross(tri.e1, tri.e2)));
                            }
                        }

                        if (mod.tex_coords.size() >= tri.prim_id * 3 + 3)
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->tex_coords.push_back(mod.tex_coords[tri.prim_id * 3 + i]);
                            }
                        }
                        else
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->tex_coords.push_back(vec2(0.0f, 0.0f));
                            }
                        }

                        if (mod.colors.size() >= tri.prim_id * 3 + 3)
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->colors.push_back(vector<3, unorm<8>>(mod.colors[tri.prim_id * 3 + i]));
                            }
                        }
                        else
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                mesh->colors.push_back(vector<3, unorm<8>>(1.0f, 1.0f, 1.0f));
                            }
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("");
                }
            }
            else
            {
                // TODO: don't allow circular references..
                inc = mod.scene_graph;
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        throw std::runtime_error("");
    }

    return inc;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_point_light(Object const& obj)
{
    auto light = std::make_shared<sg::point_light>();

    vec3 cl(1.0f);
    if (obj.HasMember("cl"))
    {
        auto const& color = obj["cl"];

        if (color.Capacity() != 3)
        {
            throw std::runtime_error("");
        }

        cl.x = color[0].GetFloat();
        cl.y = color[1].GetFloat();
        cl.z = color[2].GetFloat();
    }

    float kl = 1.0f;
    if (obj.HasMember("kl"))
    {
        kl = obj["kl"].GetFloat();
    }

    vec3 position(0.0f);
    if (obj.HasMember("position"))
    {
        auto const& pos = obj["position"];

        if (pos.Capacity() != 3)
        {
            throw std::runtime_error("");
        }

        position.x = pos[0].GetFloat();
        position.y = pos[1].GetFloat();
        position.z = pos[2].GetFloat();
    }

    float constant_attenuation = 1.0f;
    if (obj.HasMember("constant_attenuation"))
    {
        constant_attenuation = obj["constant_attenuation"].GetFloat();
    }

    float linear_attenuation = 0.0f;
    if (obj.HasMember("linear_attenuation"))
    {
        linear_attenuation = obj["linear_attenuation"].GetFloat();
    }

    float quadratic_attenuation = 0.0f;
    if (obj.HasMember("quadratic_attenuation"))
    {
        quadratic_attenuation = obj["quadratic_attenuation"].GetFloat();
    }

    light->set_cl(cl);
    light->set_kl(kl);
    light->set_position(position);
    light->set_constant_attenuation(constant_attenuation);
    light->set_linear_attenuation(linear_attenuation);
    light->set_quadratic_attenuation(quadratic_attenuation);

    return light;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_spot_light(Object const& obj)
{
    auto light = std::make_shared<sg::spot_light>();

    vec3 cl(1.0f);
    if (obj.HasMember("cl"))
    {
        auto const& color = obj["cl"];

        if (color.Capacity() != 3)
        {
            throw std::runtime_error("");
        }

        cl.x = color[0].GetFloat();
        cl.y = color[1].GetFloat();
        cl.z = color[2].GetFloat();
    }

    float kl = 1.0f;
    if (obj.HasMember("kl"))
    {
        kl = obj["kl"].GetFloat();
    }

    vec3 position(0.0f);
    if (obj.HasMember("position"))
    {
        auto const& pos = obj["position"];

        if (pos.Capacity() != 3)
        {
            throw std::runtime_error("");
        }

        position.x = pos[0].GetFloat();
        position.y = pos[1].GetFloat();
        position.z = pos[2].GetFloat();
    }

    vec3 spot_direction(0.0f, 0.0f, -1.0f);
    if (obj.HasMember("spot_direction"))
    {
        auto const& sdir = obj["spot_direction"];

        if (sdir.Capacity() != 3)
        {
            throw std::runtime_error("");
        }

        spot_direction.x = sdir[0].GetFloat();
        spot_direction.y = sdir[1].GetFloat();
        spot_direction.z = sdir[2].GetFloat();
    }

    assert(length(spot_direction) == 1.0f);

    float spot_cutoff = 180.0f * constants::degrees_to_radians<float>();
    if (obj.HasMember("spot_cutoff"))
    {
        spot_cutoff = obj["spot_cutoff"].GetFloat();
    }

    float spot_exponent = 0.0f;
    if (obj.HasMember("spot_exponent"))
    {
        spot_exponent = obj["spot_exponent"].GetFloat();
    }

    float constant_attenuation = 1.0f;
    if (obj.HasMember("constant_attenuation"))
    {
        constant_attenuation = obj["constant_attenuation"].GetFloat();
    }

    float linear_attenuation = 0.0f;
    if (obj.HasMember("linear_attenuation"))
    {
        linear_attenuation = obj["linear_attenuation"].GetFloat();
    }

    float quadratic_attenuation = 0.0f;
    if (obj.HasMember("quadratic_attenuation"))
    {
        quadratic_attenuation = obj["quadratic_attenuation"].GetFloat();
    }

    light->set_cl(cl);
    light->set_kl(kl);
    light->set_position(position);
    light->set_spot_direction(spot_direction);
    light->set_spot_cutoff(spot_cutoff);
    light->set_spot_exponent(spot_exponent);
    light->set_constant_attenuation(constant_attenuation);
    light->set_linear_attenuation(linear_attenuation);
    light->set_quadratic_attenuation(quadratic_attenuation);

    return light;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_reference(Object const& obj)
{
    return std::make_shared<sg::node>();
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_transform(Object const& obj)
{
    auto transform = std::make_shared<sg::transform>();

    if (obj.HasMember("matrix"))
    {
        auto const& mat = obj["matrix"];

        if (mat.Capacity() != 16)
        {
            throw std::runtime_error("");
        }

        for (rapidjson::SizeType i = 0; i < mat.Capacity(); ++i)
        {
            transform->matrix().data()[i] = mat[i].GetFloat();
        }
    }

    return transform;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_surface_properties(Object const& obj)
{
    auto props = std::make_shared<sg::surface_properties>();

    if (obj.HasMember("material"))
    {
        auto const& mat = obj["material"];

        if (mat.HasMember("type"))
        {
            auto const& type_string = mat["type"];
            if (strncmp(type_string.GetString(), "obj", 3) == 0)
            {
                auto obj = std::make_shared<sg::obj_material>();

                if (mat.HasMember("ca"))
                {
                    auto const& ca = mat["ca"];

                    if (ca.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->ca.x = ca[0].GetFloat();
                    obj->ca.y = ca[1].GetFloat();
                    obj->ca.z = ca[2].GetFloat();
                }

                if (mat.HasMember("cd"))
                {
                    auto const& cd = mat["cd"];

                    if (cd.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->cd.x = cd[0].GetFloat();
                    obj->cd.y = cd[1].GetFloat();
                    obj->cd.z = cd[2].GetFloat();
                }

                if (mat.HasMember("cs"))
                {
                    auto const& cs = mat["cs"];

                    if (cs.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->cs.x = cs[0].GetFloat();
                    obj->cs.y = cs[1].GetFloat();
                    obj->cs.z = cs[2].GetFloat();
                }

                if (mat.HasMember("ce"))
                {
                    auto const& ce = mat["ce"];

                    if (ce.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->ce.x = ce[0].GetFloat();
                    obj->ce.y = ce[1].GetFloat();
                    obj->ce.z = ce[2].GetFloat();
                }

                props->material() = obj;
            }
            else if (strncmp(type_string.GetString(), "glass", 5) == 0)
            {
                auto glass = std::make_shared<sg::glass_material>();

                if (mat.HasMember("ct"))
                {
                    auto const& ct = mat["ct"];

                    if (ct.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    glass->ct.x = ct[0].GetFloat();
                    glass->ct.y = ct[1].GetFloat();
                    glass->ct.z = ct[2].GetFloat();
                }

                if (mat.HasMember("cr"))
                {
                    auto const& cr = mat["cr"];

                    if (cr.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    glass->cr.x = cr[0].GetFloat();
                    glass->cr.y = cr[1].GetFloat();
                    glass->cr.z = cr[2].GetFloat();
                }

                if (mat.HasMember("ior"))
                {
                    auto const& ior = mat["ior"];

                    if (ior.Capacity() != 3)
                    {
                        throw std::runtime_error("");
                    }

                    glass->ior.x = ior[0].GetFloat();
                    glass->ior.y = ior[1].GetFloat();
                    glass->ior.z = ior[2].GetFloat();
                }

                props->material() = glass;
            }
            else
            {
                throw std::runtime_error("");
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        // Set default material (wavefront obj)
        auto obj = std::make_shared<sg::obj_material>();
        props->material() = obj;
    }

    if (obj.HasMember("diffuse"))
    {
        // TODO: load from file
#if 1
        vector<4, unorm<8>> dummy_texel(1.0f, 1.0f, 1.0f, 1.0f);
        auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
        tex->resize(1, 1);
        tex->set_address_mode(Wrap);
        tex->set_filter_mode(Nearest);
        tex->reset(&dummy_texel);

        props->add_texture(tex);
#endif
    }
    else
    {
        // Set a dummy texture
        vector<4, unorm<8>> dummy_texel(1.0f, 1.0f, 1.0f, 1.0f);
        auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
        tex->resize(1, 1);
        tex->set_address_mode(Wrap);
        tex->set_filter_mode(Nearest);
        tex->reset(&dummy_texel);

        props->add_texture(tex);
    }

    return props;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_triangle_mesh(Object const& obj)
{
    auto mesh = std::make_shared<sg::triangle_mesh>();

    if (obj.HasMember("vertices"))
    {
        auto const& verts = obj["vertices"];

        if (verts.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < verts.Capacity(); i += 3)
            {
                mesh->vertices.emplace_back(
                    verts[i].GetFloat(),
                    verts[i + 1].GetFloat(),
                    verts[i + 2].GetFloat()
                    );
            }
        }
        else if (verts.IsObject())
        {
            auto const& type_string = verts["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(verts);

                if (!parse_as_vec3f(md, mesh->vertices))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (obj.HasMember("normals"))
    {
        auto const& normals = obj["normals"];

        if (normals.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < normals.Capacity(); i += 3)
            {
                mesh->normals.emplace_back(
                    normals[i].GetFloat(),
                    normals[i + 1].GetFloat(),
                    normals[i + 2].GetFloat()
                    );
            }
        }
        else if (normals.IsObject())
        {
            auto const& type_string = normals["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(normals);

                if (!parse_as_vec3f(md, mesh->normals))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); i += 3)
        {
            vec3 v1 = mesh->vertices[i];
            vec3 v2 = mesh->vertices[i + 1];
            vec3 v3 = mesh->vertices[i + 2];

            vec3 e1 = v2 - v1;
            vec3 e2 = v3 - v1;

            vec3 gn = normalize(cross(e1, e2));

            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
        }
    }

    if (obj.HasMember("tex_coords"))
    {
        auto const& tex_coords = obj["tex_coords"];

        if (tex_coords.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < tex_coords.Capacity(); i += 2)
            {
                mesh->tex_coords.emplace_back(
                    tex_coords[i].GetFloat(),
                    tex_coords[i + 1].GetFloat()
                    );
            }
        }
        else if (tex_coords.IsObject())
        {
            auto const& type_string = tex_coords["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(tex_coords);

                if (!parse_as_vec2f(md, mesh->tex_coords))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); i += 3)
        {
            mesh->tex_coords.emplace_back(0.0f, 0.0f);
            mesh->tex_coords.emplace_back(0.0f, 0.0f);
            mesh->tex_coords.emplace_back(0.0f, 0.0f);
        }
    }

    if (obj.HasMember("colors"))
    {
        auto const& colors = obj["colors"];

        if (colors.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < colors.Capacity(); i += 3)
            {
                mesh->colors.emplace_back(
                    colors[i].GetFloat(),
                    colors[i + 1].GetFloat(),
                    colors[i + 2].GetFloat()
                    );
            }
        }
        else if (colors.IsObject())
        {
            auto const& type_string = colors["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(colors);

                if (!parse_as_vec3f(md, mesh->colors))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); i += 3)
        {
            mesh->colors.emplace_back(1.0f, 1.0f, 1.0f);
            mesh->colors.emplace_back(1.0f, 1.0f, 1.0f);
            mesh->colors.emplace_back(1.0f, 1.0f, 1.0f);
        }
    }

    return mesh;
}

template <typename Object>
std::shared_ptr<sg::node> vsnray_parser::parse_indexed_triangle_mesh(Object const& obj)
{
    auto mesh = std::make_shared<sg::indexed_triangle_mesh>();

    if (obj.HasMember("indices"))
    {
        auto const& indices = obj["indices"];

        for (auto const& item : indices.GetArray())
        {
            mesh->indices.push_back(item.GetInt());
        }
    }

    if (obj.HasMember("vertices"))
    {
        auto const& verts = obj["vertices"];

        if (verts.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < verts.Capacity(); i += 3)
            {
                mesh->vertices.emplace_back(
                    verts[i].GetFloat(),
                    verts[i + 1].GetFloat(),
                    verts[i + 2].GetFloat()
                    );
            }
        }
        else if (verts.IsObject())
        {
            auto const& type_string = verts["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(verts);

                if (!parse_as_vec3f(md, mesh->vertices))
                {
                    throw std::runtime_error("");
                }
            }
        }
    }

    if (obj.HasMember("normals"))
    {
        auto const& normals = obj["normals"];

        if (normals.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < normals.Capacity(); i += 3)
            {
                mesh->normals.emplace_back(
                    normals[i].GetFloat(),
                    normals[i + 1].GetFloat(),
                    normals[i + 2].GetFloat()
                    );
            }
        }
        else if (normals.IsObject())
        {
            auto const& type_string = normals["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(normals);

                if (!parse_as_vec3f(md, mesh->normals))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (obj.HasMember("tex_coords"))
    {
        auto const& tex_coords = obj["tex_coords"];

        if (tex_coords.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < tex_coords.Capacity(); i += 2)
            {
                mesh->tex_coords.emplace_back(
                    tex_coords[i].GetFloat(),
                    tex_coords[i + 1].GetFloat()
                    );
            }
        }
        else if (tex_coords.IsObject())
        {
            auto const& type_string = tex_coords["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(tex_coords);

                if (!parse_as_vec2f(md, mesh->tex_coords))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (obj.HasMember("colors"))
    {
        auto const& colors = obj["colors"];

        if (colors.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < colors.Capacity(); i += 3)
            {
                mesh->colors.emplace_back(
                    colors[i].GetFloat(),
                    colors[i + 1].GetFloat(),
                    colors[i + 2].GetFloat()
                    );
            }
        }
        else if (colors.IsObject())
        {
            auto const& type_string = colors["type"];
            if (strncmp(type_string.GetString(), "file", 4) == 0)
            {
                auto md = parse_file_meta_data(colors);

                if (!parse_as_vec3f(md, mesh->colors))
                {
                    throw std::runtime_error("");
                }
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    return mesh;
}

template <typename Object>
data_file::meta_data vsnray_parser::parse_file_meta_data(Object const& obj)
{
    data_file::meta_data result;

    if (obj.HasMember("path"))
    {
        result.path = obj["path"].GetString();
    }
    else
    {
        throw std::runtime_error("");
    }

    if (obj.HasMember("encoding"))
    {
        std::string encoding = obj["encoding"].GetString();
        if (encoding == "ascii")
        {
            result.encoding = data_file::meta_data::Ascii;
        }
        else if (encoding == "binary")
        {
            result.encoding = data_file::meta_data::Binary;
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        throw std::runtime_error("");
    }

    if (obj.HasMember("data_type"))
    {
        std::string data_type = obj["data_type"].GetString();
        if (data_type == "float")
        {
            result.data_type = data_file::meta_data::Float;
        }
        else if (data_type == "vec2u8")
        {
            result.data_type = data_file::meta_data::Vec2u8;
        }
        else if (data_type == "vec2f")
        {
            result.data_type = data_file::meta_data::Vec2u8;
        }
        else if (data_type == "vec2")
        {
            result.data_type = data_file::meta_data::Vec2;
        }
        else if (data_type == "vec3u8")
        {
            result.data_type = data_file::meta_data::Vec3u8;
        }
        else if (data_type == "vec3f")
        {
            result.data_type = data_file::meta_data::Vec3u8;
        }
        else if (data_type == "vec3")
        {
            result.data_type = data_file::meta_data::Vec3;
        }
        else if (data_type == "vec4u8")
        {
            result.data_type = data_file::meta_data::Vec4u8;
        }
        else if (data_type == "vec4f")
        {
            result.data_type = data_file::meta_data::Vec4u8;
        }
        else if (data_type == "vec4")
        {
            result.data_type = data_file::meta_data::Vec4;
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        throw std::runtime_error("");
    }

    if (obj.HasMember("num_items"))
    {
        result.num_items = obj["num_items"].GetInt();
    }
    else
    {
        throw std::runtime_error("");
    }

    if (obj.HasMember("compression"))
    {
        std::string compression = obj["compression"].GetString();
        if (compression == "none" || compression == "raw")
        {
            result.compression = data_file::meta_data::Raw;
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (obj.HasMember("separator"))
    {
        std::string separator = obj["separator"].GetString();
        result.separator = separator[0];
    }

    return result;
}


namespace visionaray
{

//-------------------------------------------------------------------------------------------------
// Interface
//

void load_vsnray(std::string const& filename, model& mod)
{
    std::vector<std::string> filenames(1);

    filenames[0] = filename;

    load_vsnray(filenames, mod);
}

void load_vsnray(std::vector<std::string> const& filenames, model& mod)
{
    auto root = std::make_shared<sg::node>();

    for (auto filename : filenames)
    {
        cfile file(filename, "r");
        if (!file.good())
        {
            std::cerr << "Cannot open " << filename << '\n';
            return;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(file.get(), buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream(frs);

        if (doc.IsObject())
        {
            vsnray_parser parser(filename);
            root = parser.parse_node(doc.GetObject());
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (mod.scene_graph == nullptr)
    {
        mod.scene_graph = std::make_shared<sg::node>();
    }

    mod.scene_graph->add_child(root);
}

} // visionaray
