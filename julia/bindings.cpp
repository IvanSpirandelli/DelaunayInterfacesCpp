#include "jlcxx/jlcxx.hpp"
#include "delaunay_interfaces/interface_generation.hpp"
#include "delaunay_interfaces/barycentric_subdivision.hpp"
#include <vector>

using namespace delaunay_interfaces;

// Helper to convert Julia arrays to C++ vectors
template<typename T>
std::vector<T> julia_array_to_vector(jlcxx::ArrayRef<T> arr) {
    return std::vector<T>(arr.begin(), arr.end());
}

// Helper to convert C++ Point3D to Julia array
jlcxx::Array<double> point_to_julia(const Point3D& p) {
    jlcxx::Array<double> result;
    result.push_back(p.x());
    result.push_back(p.y());
    result.push_back(p.z());
    return result;
}

JLCXX_MODULE define_julia_module(jlcxx::Module& mod) {
    mod.set_const("VERSION", "0.1.0");

    // ComplexConfig
    mod.add_type<ComplexConfig>("ComplexConfig")
        .constructor<>()
        .constructor<bool, bool>()
        .method("weighted", [](const ComplexConfig& c) { return c.weighted; })
        .method("alpha", [](const ComplexConfig& c) { return c.alpha; })
        .method("set_weighted!", [](ComplexConfig& c, bool w) { c.weighted = w; })
        .method("set_alpha!", [](ComplexConfig& c, bool a) { c.alpha = a; });

    // InterfaceSurface
    mod.add_type<InterfaceSurface>("InterfaceSurface")
        .method("vertices", [](const InterfaceSurface& is) {
            // Convert Points to Julia array of arrays
            jlcxx::Array<jlcxx::Array<double>> result;
            for (const auto& p : is.vertices) {
                jlcxx::Array<double> point;
                point.push_back(p.x());
                point.push_back(p.y());
                point.push_back(p.z());
                result.push_back(point);
            }
            return result;
        })
        .method("filtration", [](const InterfaceSurface& is) {
            return is.filtration;
        })
        .method("weighted", [](const InterfaceSurface& is) { return is.weighted; })
        .method("alpha", [](const InterfaceSurface& is) { return is.alpha; });

    // InterfaceGenerator
    mod.add_type<InterfaceGenerator>("InterfaceGenerator")
        .constructor<>()
        .method("compute_interface_surface", [](
            InterfaceGenerator& gen,
            jlcxx::ArrayRef<jlcxx::ArrayRef<double>> points_arr,
            jlcxx::ArrayRef<int> color_labels_arr,
            jlcxx::ArrayRef<double> radii_arr,
            const ComplexConfig& config
        ) {
            // Convert Julia arrays to C++ vectors
            Points points;
            for (const auto& p : points_arr) {
                if (p.size() != 3) {
                    throw std::runtime_error("Each point must be 3D");
                }
                points.emplace_back(p[0], p[1], p[2]);
            }

            ColorLabels color_labels = julia_array_to_vector(color_labels_arr);
            Radii radii = julia_array_to_vector(radii_arr);

            return gen.compute_interface_surface(points, color_labels, radii, config);
        })
        .method("get_multicolored_tetrahedra", [](
            InterfaceGenerator& gen,
            jlcxx::ArrayRef<jlcxx::ArrayRef<double>> points_arr,
            jlcxx::ArrayRef<int> color_labels_arr,
            jlcxx::ArrayRef<double> radii_arr,
            const ComplexConfig& config
        ) {
            Points points;
            for (const auto& p : points_arr) {
                if (p.size() != 3) {
                    throw std::runtime_error("Each point must be 3D");
                }
                points.emplace_back(p[0], p[1], p[2]);
            }

            ColorLabels color_labels = julia_array_to_vector(color_labels_arr);
            Radii radii = julia_array_to_vector(radii_arr);

            return gen.get_multicolored_tetrahedra(points, color_labels, radii, config);
        });

    // Convenience function
    mod.method("get_barycentric_subdivision_and_filtration", [](
        jlcxx::ArrayRef<jlcxx::ArrayRef<double>> points_arr,
        jlcxx::ArrayRef<int> color_labels_arr,
        jlcxx::ArrayRef<double> radii_arr,
        bool weighted,
        bool alpha
    ) {
        Points points;
        for (const auto& p : points_arr) {
            if (p.size() != 3) {
                throw std::runtime_error("Each point must be 3D");
            }
            points.emplace_back(p[0], p[1], p[2]);
        }

        ColorLabels color_labels = julia_array_to_vector(color_labels_arr);
        Radii radii = julia_array_to_vector(radii_arr);

        return get_barycentric_subdivision_and_filtration(
            points, color_labels, radii, weighted, alpha
        );
    });
}
