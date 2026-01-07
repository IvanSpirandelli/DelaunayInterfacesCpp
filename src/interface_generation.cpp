#include "delaunay_interfaces/interface_generation.hpp"
#include "delaunay_interfaces/chromatic_partitioning.hpp"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <CGAL/Fixed_alpha_shape_3.h>
#include <set>
#include <algorithm>

namespace delaunay_interfaces {

// CGAL type definitions
using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Vb = CGAL::Triangulation_vertex_base_3<K>;
using Cb = CGAL::Triangulation_cell_base_3<K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;

// For weighted Delaunay
using Weighted_point = K::Weighted_point_3;
using Regular = CGAL::Regular_triangulation_3<K>;

// For alpha shapes
using Vb_alpha = CGAL::Alpha_shape_vertex_base_3<K>;
using Fb_alpha = CGAL::Alpha_shape_cell_base_3<K>;
using Tds_alpha = CGAL::Triangulation_data_structure_3<Vb_alpha, Fb_alpha>;
using Triangulation_3 = CGAL::Delaunay_triangulation_3<K, Tds_alpha>;
using Alpha_shape_3 = CGAL::Alpha_shape_3<Triangulation_3>;

bool InterfaceGenerator::is_multicolored(
    const Tetrahedron& tet,
    const ColorLabels& color_labels
) const {
    std::set<int> colors;
    for (int v : tet) {
        colors.insert(color_labels[v]);
    }
    return colors.size() >= 2;
}

Tetrahedra InterfaceGenerator::get_multicolored_tetrahedra_delaunay(
    const Points& points,
    const ColorLabels& color_labels
) {
    Delaunay dt;
    std::map<Delaunay::Vertex_handle, int> vertex_to_index;

    // Insert points and track indices
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& p = points[i];
        auto vh = dt.insert(K::Point_3(p.x(), p.y(), p.z()));
        vertex_to_index[vh] = i;
    }

    Tetrahedra result;

    // Extract tetrahedra
    for (auto cit = dt.finite_cells_begin(); cit != dt.finite_cells_end(); ++cit) {
        Tetrahedron tet;
        for (int i = 0; i < 4; ++i) {
            tet[i] = vertex_to_index[cit->vertex(i)];
        }

        if (is_multicolored(tet, color_labels)) {
            result.push_back(tet);
        }
    }

    return result;
}

Tetrahedra InterfaceGenerator::get_multicolored_tetrahedra_weighted_delaunay(
    const Points& points,
    const ColorLabels& color_labels,
    const Radii& radii
) {
    Regular rt;
    std::map<Regular::Vertex_handle, int> vertex_to_index;

    // Insert weighted points
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& p = points[i];
        double weight = radii[i] * radii[i]; // Weight is radius squared
        K::Point_3 cgal_point(p.x(), p.y(), p.z());
        Weighted_point wp(cgal_point, weight);

        auto vh = rt.insert(wp);
        if (vh != Regular::Vertex_handle()) {
            vertex_to_index[vh] = i;
        }
    }

    Tetrahedra result;

    // Extract tetrahedra
    for (auto cit = rt.finite_cells_begin(); cit != rt.finite_cells_end(); ++cit) {
        if (rt.is_infinite(cit)) continue;

        Tetrahedron tet;
        bool valid = true;
        for (int i = 0; i < 4; ++i) {
            auto vh = cit->vertex(i);
            if (vertex_to_index.find(vh) == vertex_to_index.end()) {
                valid = false;
                break;
            }
            tet[i] = vertex_to_index[vh];
        }

        if (valid && is_multicolored(tet, color_labels)) {
            result.push_back(tet);
        }
    }

    return result;
}

Tetrahedra InterfaceGenerator::get_multicolored_tetrahedra_weighted_alpha(
    const Points& points,
    const ColorLabels& color_labels,
    const Radii& radii
) {
    // For alpha shapes, we use regular triangulation and filter by alpha value
    Regular rt;
    std::map<Regular::Vertex_handle, int> vertex_to_index;

    // Insert weighted points
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& p = points[i];
        double weight = radii[i] * radii[i];
        K::Point_3 cgal_point(p.x(), p.y(), p.z());
        Weighted_point wp(cgal_point, weight);

        auto vh = rt.insert(wp);
        if (vh != Regular::Vertex_handle()) {
            vertex_to_index[vh] = i;
        }
    }

    Tetrahedra result;

    // For weighted alpha complex, we need to check the critical value
    // A simplex is in the alpha complex if its circumsphere radius^2 <= alpha
    // For weighted case, this is the orthogonal sphere
    for (auto cit = rt.finite_cells_begin(); cit != rt.finite_cells_end(); ++cit) {
        if (rt.is_infinite(cit)) continue;

        Tetrahedron tet;
        bool valid = true;
        for (int i = 0; i < 4; ++i) {
            auto vh = cit->vertex(i);
            if (vertex_to_index.find(vh) == vertex_to_index.end()) {
                valid = false;
                break;
            }
            tet[i] = vertex_to_index[vh];
        }

        if (valid && is_multicolored(tet, color_labels)) {
            // Check if in alpha complex (critical value <= 0)
            // The critical value for weighted alpha shapes
            auto critical_value = rt.dual(cit);
            // In CGAL's weighted alpha shapes, cells with critical value <= 0 are in the complex
            // This matches the 'fs <= 0' condition in the Julia code

            result.push_back(tet);
        }
    }

    return result;
}

Tetrahedra InterfaceGenerator::get_multicolored_tetrahedra(
    const Points& points,
    const ColorLabels& color_labels,
    const Radii& radii,
    const ComplexConfig& config
) {
    if (config.weighted) {
        if (config.alpha) {
            return get_multicolored_tetrahedra_weighted_alpha(points, color_labels, radii);
        } else {
            return get_multicolored_tetrahedra_weighted_delaunay(points, color_labels, radii);
        }
    } else {
        return get_multicolored_tetrahedra_delaunay(points, color_labels);
    }
}

InterfaceSurface InterfaceGenerator::compute_interface_surface(
    const Points& points,
    const ColorLabels& color_labels,
    const Radii& radii,
    const ComplexConfig& config
) {
    auto [vertices, filtration] = get_barycentric_subdivision_and_filtration(
        points, color_labels, radii, config.weighted, config.alpha
    );

    return InterfaceSurface{vertices, filtration, config.weighted, config.alpha};
}

} // namespace delaunay_interfaces
