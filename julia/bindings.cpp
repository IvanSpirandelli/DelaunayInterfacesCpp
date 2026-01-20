#include "jlcxx/jlcxx.hpp"
#include "delaunay_interfaces/interface_generation.hpp"
#include <vector>
#include <string>

using namespace delaunay_interfaces;

// Mark types as non-mirrored
namespace jlcxx {
    template<> struct IsMirroredType<InterfaceGenerator> : std::false_type { };
}

// Helper to convert Julia arrays to C++ vectors
template<typename T>
std::vector<T> julia_array_to_vector(jlcxx::ArrayRef<T> arr) {
    return std::vector<T>(arr.begin(), arr.end());
}

JLCXX_MODULE define_julia_module(jlcxx::Module& mod) {
    mod.method("version", []() { return std::string("0.1.0"); });

    // InterfaceGenerator
    mod.add_type<InterfaceGenerator>("InterfaceGenerator")
        .constructor<>()
        .method("get_multicolored_tetrahedra", [](
            InterfaceGenerator& gen,
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

            auto tets = gen.get_multicolored_tetrahedra(points, color_labels, radii, weighted, alpha);

            // Convert to a flat array where each 4 consecutive integers represent one tetrahedron
            std::vector<int> result;
            result.reserve(tets.size() * 4);
            for (const auto& tet : tets) {
                result.push_back(tet[0]);
                result.push_back(tet[1]);
                result.push_back(tet[2]);
                result.push_back(tet[3]);
            }
            return result;
        });
}
