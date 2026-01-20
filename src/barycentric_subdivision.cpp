#include "delaunay_interfaces/barycentric_subdivision.hpp"
#include "delaunay_interfaces/chromatic_partitioning.hpp"
#include "delaunay_interfaces/interface_generation.hpp"
#include <algorithm>
#include <stdexcept>

namespace delaunay_interfaces {

BarycentricSubdivision::BarycentricSubdivision(
    const Points& points,
    const ColorLabels& color_labels
) : points_(points), color_labels_(color_labels) {}

Partition BarycentricSubdivision::get_chromatic_partitioning(const Tetrahedron& tet) const {
    return delaunay_interfaces::get_chromatic_partitioning(tet, color_labels_);
}

Point3D BarycentricSubdivision::get_barycenter(const std::vector<int>& vertices) const {
    return compute_barycenter(points_, vertices);
}

Point3D BarycentricSubdivision::get_barycenter_from_points(const std::vector<Point3D>& points) const {
    Point3D center = Point3D::Zero();
    for (const auto& p : points) {
        center += p;
    }
    return center / static_cast<double>(points.size());
}

double BarycentricSubdivision::compute_filtration_value(const Partition& partitioning) const {
    if (partitioning.size() == 2) {
        auto bc1 = get_barycenter(partitioning[0]);
        auto bc2 = get_barycenter(partitioning[1]);
        return euclidean_distance(bc1, bc2);
    } else if (partitioning.size() == 3) {
        auto bc1 = get_barycenter(partitioning[0]);
        auto bc2 = get_barycenter(partitioning[1]);
        auto bc3 = get_barycenter(partitioning[2]);
        double a = euclidean_distance(bc1, bc2);
        double b = euclidean_distance(bc1, bc3);
        double c = euclidean_distance(bc2, bc3);
        return (a + b + c) / 3.0;
    } else if (partitioning.size() == 4) {
        auto bc1 = get_barycenter(partitioning[0]);
        auto bc2 = get_barycenter(partitioning[1]);
        auto bc3 = get_barycenter(partitioning[2]);
        auto bc4 = get_barycenter(partitioning[3]);
        double a = euclidean_distance(bc1, bc2);
        double b = euclidean_distance(bc1, bc3);
        double c = euclidean_distance(bc1, bc4);
        double d = euclidean_distance(bc2, bc3);
        double e = euclidean_distance(bc2, bc4);
        double f = euclidean_distance(bc3, bc4);
        return (a + b + c + d + e + f) / 6.0;
    }
    return 0.0;
}

BarycentricSubdivision::SimplexInfo BarycentricSubdivision::get_or_create_simplex(
    const std::vector<std::vector<int>>& partitioning
) {
    // Create sorted key from all vertices
    std::vector<int> key;
    for (const auto& part : partitioning) {
        key.insert(key.end(), part.begin(), part.end());
    }
    std::sort(key.begin(), key.end());

    auto it = simplex_map_.find(key);
    if (it != simplex_map_.end()) {
        return SimplexInfo{it->second.first, it->second.second, false};
    } else {
        int32_t id = next_simplex_id_++;
        double value = compute_filtration_value(partitioning);
        simplex_map_[key] = {id, value};
        return SimplexInfo{id, value, true};
    }
}

void BarycentricSubdivision::extend_scaffold_2_2(
    const std::vector<int>& part1,
    const std::vector<int>& part2
) {
    // 2-2 partitioning: [u,v] vs [x,y]
    int u = part1[0], v = part1[1];
    int x = part2[0], y = part2[1];

    // Define the 9 multicolored combinations
    std::vector<std::vector<std::vector<int>>> mc_combinations = {
        {{u},{x}}, {{v},{x}}, {{v},{y}}, {{u},{y}},
        {{u,v},{x}}, {{v},{x,y}}, {{u,v},{y}}, {{u},{x,y}},
        {{u,v},{x,y}}
    };

    std::vector<std::pair<int32_t, double>> vertices;
    std::vector<bool> created;

    // Create or get simplex IDs
    for (const auto& comb : mc_combinations) {
        auto info = get_or_create_simplex(comb);
        vertices.push_back({info.id, info.value});
        created.push_back(info.newly_created);
    }

    // Compute new barycenters
    std::vector<Point3D> new_barycenters(9);

    // Edge barycenters (indices 0-4, 5, 6, 7)
    for (size_t i = 0; i < mc_combinations.size(); ++i) {
        std::vector<int> all_vertices;
        for (const auto& part : mc_combinations[i]) {
            all_vertices.insert(all_vertices.end(), part.begin(), part.end());
        }
        new_barycenters[i] = get_barycenter(all_vertices);
    }

    // Add newly created barycenters
    for (size_t i = 0; i < created.size(); ++i) {
        if (created[i]) {
            barycenters_.push_back(new_barycenters[i]);
        }
    }

    // Add edges
    std::vector<std::pair<int, int>> edge_indices = {
        {8, 0}, {8, 1}, {8, 2}, {8, 3},
        {8, 4}, {8, 5}, {8, 6}, {8, 7},
        {0, 4}, {1, 4}, {1, 5}, {2, 5},
        {2, 6}, {3, 6}, {3, 7}, {0, 7}
    };

    for (const auto& [i, j] : edge_indices) {
        Simplex edge = {vertices[i].first, vertices[j].first};
        std::sort(edge.begin(), edge.end());
        double val = std::min(vertices[i].second, vertices[j].second);
        filtration_set_.insert({edge, val});
    }

    // Add triangles (8 triangles as in Julia code)
    std::vector<std::tuple<int, int, int>> triangle_indices = {
        {8, 0, 4}, {8, 4, 1}, {8, 1, 5}, {8, 5, 2},
        {8, 2, 6}, {8, 6, 3}, {8, 3, 7}, {8, 7, 0}
    };

    for (const auto& [i, j, k] : triangle_indices) {
        Simplex tri = {vertices[i].first, vertices[j].first, vertices[k].first};
        std::sort(tri.begin(), tri.end());
        double val = std::min({vertices[i].second, vertices[j].second, vertices[k].second});
        filtration_set_.insert({tri, val});
    }

    // Add vertices to filtration
    for (const auto& [id, val] : vertices) {
        filtration_set_.insert({{id}, val});
    }
}

void BarycentricSubdivision::extend_scaffold_3_1(
    const std::vector<int>& part1,
    const std::vector<int>& part2
) {
    // 3-1 partitioning: [u,v,w] vs [x]
    int u = part1[0], v = part1[1], w = part1[2];
    int x = part2[0];

    std::vector<std::vector<std::vector<int>>> mc_combinations = {
        {{u},{x}}, {{v},{x}}, {{w},{x}},
        {{u,v},{x}}, {{v,w},{x}}, {{u,w},{x}},
        {{u,v,w},{x}}
    };

    std::vector<std::pair<int32_t, double>> vertices;
    std::vector<bool> created;

    for (const auto& comb : mc_combinations) {
        auto info = get_or_create_simplex(comb);
        vertices.push_back({info.id, info.value});
        created.push_back(info.newly_created);
    }

    // Compute barycenters
    std::vector<Point3D> new_barycenters(7);
    for (size_t i = 0; i < mc_combinations.size(); ++i) {
        std::vector<int> all_vertices;
        for (const auto& part : mc_combinations[i]) {
            all_vertices.insert(all_vertices.end(), part.begin(), part.end());
        }
        new_barycenters[i] = get_barycenter(all_vertices);
    }

    for (size_t i = 0; i < created.size(); ++i) {
        if (created[i]) {
            barycenters_.push_back(new_barycenters[i]);
        }
    }

    // Add edges
    std::vector<std::pair<int, int>> edge_indices = {
        {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5},
        {0, 3}, {1, 3}, {1, 4}, {2, 4}, {2, 5}, {0, 5}
    };

    for (const auto& [i, j] : edge_indices) {
        Simplex edge = {vertices[i].first, vertices[j].first};
        std::sort(edge.begin(), edge.end());
        double val = std::min(vertices[i].second, vertices[j].second);
        filtration_set_.insert({edge, val});
    }

    // Add triangles (6 triangles)
    std::vector<std::tuple<int, int, int>> triangle_indices = {
        {6, 0, 3}, {6, 3, 1}, {6, 1, 4},
        {6, 4, 2}, {6, 2, 5}, {6, 5, 0}
    };

    for (const auto& [i, j, k] : triangle_indices) {
        Simplex tri = {vertices[i].first, vertices[j].first, vertices[k].first};
        std::sort(tri.begin(), tri.end());
        double val = std::min({vertices[i].second, vertices[j].second, vertices[k].second});
        filtration_set_.insert({tri, val});
    }

    for (const auto& [id, val] : vertices) {
        filtration_set_.insert({{id}, val});
    }
}

void BarycentricSubdivision::extend_scaffold_2_1_1(
    const std::vector<int>& part1,
    const std::vector<int>& part2,
    const std::vector<int>& part3
) {
    // 2-1-1 partitioning: [a,b] vs [u] vs [x]
    int a = part1[0], b = part1[1];
    int u = part2[0];
    int x = part3[0];

    std::vector<std::vector<std::vector<int>>> mc_combinations = {
        {{a},{u}}, {{a},{x}}, {{b},{x}}, {{b},{u}},
        {{u},{x}},
        {{a},{u},{x}}, {{a,b},{x}}, {{b},{u},{x}}, {{a,b},{u}},
        {{a,b},{u},{x}}
    };

    std::vector<std::pair<int32_t, double>> vertices;
    std::vector<bool> created;

    for (const auto& comb : mc_combinations) {
        auto info = get_or_create_simplex(comb);
        vertices.push_back({info.id, info.value});
        created.push_back(info.newly_created);
    }

    std::vector<Point3D> new_barycenters(10);
    for (size_t i = 0; i < mc_combinations.size(); ++i) {
        std::vector<int> all_vertices;
        for (const auto& part : mc_combinations[i]) {
            all_vertices.insert(all_vertices.end(), part.begin(), part.end());
        }
        new_barycenters[i] = get_barycenter(all_vertices);
    }

    for (size_t i = 0; i < created.size(); ++i) {
        if (created[i]) {
            barycenters_.push_back(new_barycenters[i]);
        }
    }

    // Add edges (connections between simplices)
    std::vector<std::pair<int, int>> edge_indices = {
        {9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4},
        {9, 5}, {9, 6}, {9, 7}, {9, 8},
        {0, 5}, {1, 5}, {2, 6}, {3, 8}, {4, 5}, {4, 7}
    };

    for (const auto& [i, j] : edge_indices) {
        Simplex edge = {vertices[i].first, vertices[j].first};
        std::sort(edge.begin(), edge.end());
        double val = std::min(vertices[i].second, vertices[j].second);
        filtration_set_.insert({edge, val});
    }

    // Add triangles (10 triangles)
    std::vector<std::tuple<int, int, int>> triangle_indices = {
        {9, 0, 5}, {9, 5, 4}, {9, 4, 7}, {9, 7, 3},
        {9, 3, 8}, {9, 8, 0}, {9, 2, 7}, {9, 5, 1},
        {9, 1, 6}, {9, 6, 2}
    };

    for (const auto& [i, j, k] : triangle_indices) {
        Simplex tri = {vertices[i].first, vertices[j].first, vertices[k].first};
        std::sort(tri.begin(), tri.end());
        double val = std::min({vertices[i].second, vertices[j].second, vertices[k].second});
        filtration_set_.insert({tri, val});
    }

    for (const auto& [id, val] : vertices) {
        filtration_set_.insert({{id}, val});
    }
}

void BarycentricSubdivision::extend_scaffold_1_1_1_1(
    const std::vector<int>& part1,
    const std::vector<int>& part2,
    const std::vector<int>& part3,
    const std::vector<int>& part4
) {
    // 1-1-1-1 partitioning: [a] vs [i] vs [u] vs [x]
    int a = part1[0];
    int i = part2[0];
    int u = part3[0];
    int x = part4[0];

    std::vector<std::vector<std::vector<int>>> mc_combinations = {
        {{a},{i}}, {{a},{u}}, {{a},{x}},
        {{i},{u}}, {{i},{x}}, {{u},{x}},
        {{a},{i},{u}}, {{a},{i},{x}}, {{i},{u},{x}}, {{a},{u},{x}},
        {{a},{i},{u},{x}}
    };

    std::vector<std::pair<int32_t, double>> vertices;
    std::vector<bool> created;

    for (const auto& comb : mc_combinations) {
        auto info = get_or_create_simplex(comb);
        vertices.push_back({info.id, info.value});
        created.push_back(info.newly_created);
    }

    std::vector<Point3D> new_barycenters(11);
    for (size_t i = 0; i < mc_combinations.size(); ++i) {
        std::vector<int> all_vertices;
        for (const auto& part : mc_combinations[i]) {
            all_vertices.insert(all_vertices.end(), part.begin(), part.end());
        }
        new_barycenters[i] = get_barycenter(all_vertices);
    }

    for (size_t i = 0; i < created.size(); ++i) {
        if (created[i]) {
            barycenters_.push_back(new_barycenters[i]);
        }
    }

    // Add edges
    std::vector<std::pair<int, int>> edge_indices = {
        {10, 0}, {10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5},
        {10, 6}, {10, 7}, {10, 8}, {10, 9},
        {0, 6}, {1, 6}, {3, 8}, {4, 7}, {5, 9}, {0, 7}, {1, 9}, {3, 6}, {4, 8}, {5, 8}
    };

    for (const auto& [i, j] : edge_indices) {
        Simplex edge = {vertices[i].first, vertices[j].first};
        std::sort(edge.begin(), edge.end());
        double val = std::min(vertices[i].second, vertices[j].second);
        filtration_set_.insert({edge, val});
    }

    // Add triangles (12 triangles)
    std::vector<std::tuple<int, int, int>> triangle_indices = {
        {10, 3, 8}, {10, 8, 4}, {10, 4, 7}, {10, 7, 0},
        {10, 0, 6}, {10, 6, 3}, {10, 9, 1}, {10, 5, 9},
        {10, 8, 5}, {10, 1, 6}, {10, 9, 2}, {10, 2, 7}
    };

    for (const auto& [i, j, k] : triangle_indices) {
        Simplex tri = {vertices[i].first, vertices[j].first, vertices[k].first};
        std::sort(tri.begin(), tri.end());
        double val = std::min({vertices[i].second, vertices[j].second, vertices[k].second});
        filtration_set_.insert({tri, val});
    }

    for (const auto& [id, val] : vertices) {
        filtration_set_.insert({{id}, val});
    }
}

void BarycentricSubdivision::process_tetrahedron(const Tetrahedron& tet) {
    auto parts = get_chromatic_partitioning(tet);

    if (parts.size() == 2) {
        if (parts[0].size() == 2 && parts[1].size() == 2) {
            extend_scaffold_2_2(parts[0], parts[1]);
        } else if (parts[0].size() == 3 && parts[1].size() == 1) {
            extend_scaffold_3_1(parts[0], parts[1]);
        } else if (parts[0].size() == 1 && parts[1].size() == 3) {
            extend_scaffold_3_1(parts[1], parts[0]);
        } else {
            throw std::runtime_error("Invalid 2-part partitioning");
        }
    } else if (parts.size() == 3) {
        extend_scaffold_2_1_1(parts[0], parts[1], parts[2]);
    } else if (parts.size() == 4) {
        extend_scaffold_1_1_1_1(parts[0], parts[1], parts[2], parts[3]);
    }
}

Filtration BarycentricSubdivision::get_filtration() const {
    Filtration result(filtration_set_.begin(), filtration_set_.end());

    // Sort by simplex size, then by filtration value
    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) {
            if (std::get<0>(a).size() != std::get<0>(b).size()) {
                return std::get<0>(a).size() < std::get<0>(b).size();
            }
            return std::get<1>(a) < std::get<1>(b);
        }
    );

    return result;
}

std::pair<Points, Filtration> get_barycentric_subdivision_and_filtration(
    const Points& points,
    const ColorLabels& color_labels,
    const Radii& radii,
    bool weighted,
    bool alpha
) {
    if (points.size() != color_labels.size()) {
        throw std::invalid_argument("Each point must have a corresponding color_label");
    }

    if (weighted && radii.size() != points.size()) {
        throw std::invalid_argument("Each point must have an assigned radius for weighted complexes");
    }

    InterfaceGenerator generator;
    auto tetrahedra = generator.get_multicolored_tetrahedra(points, color_labels, radii, weighted, alpha);

    BarycentricSubdivision subdivision(points, color_labels);

    for (const auto& tet : tetrahedra) {
        subdivision.process_tetrahedron(tet);
    }

    return {subdivision.get_barycenters(), subdivision.get_filtration()};
}

} // namespace delaunay_interfaces
