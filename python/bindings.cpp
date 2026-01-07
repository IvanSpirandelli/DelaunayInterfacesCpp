#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include "delaunay_interfaces/interface_generation.hpp"
#include "delaunay_interfaces/barycentric_subdivision.hpp"

namespace py = pybind11;
using namespace delaunay_interfaces;

PYBIND11_MODULE(delaunay_interfaces, m) {
    m.doc() = "DelaunayInterfaces: Compute interface surfaces from multicolored point clouds";

    // Bind ComplexConfig
    py::class_<ComplexConfig>(m, "ComplexConfig")
        .def(py::init<>())
        .def(py::init<bool, bool>(), py::arg("weighted"), py::arg("alpha"))
        .def_readwrite("weighted", &ComplexConfig::weighted)
        .def_readwrite("alpha", &ComplexConfig::alpha);

    // Bind InterfaceSurface
    py::class_<InterfaceSurface>(m, "InterfaceSurface")
        .def(py::init<>())
        .def_readonly("vertices", &InterfaceSurface::vertices,
            "List of barycenter vertices (3D points)")
        .def_readonly("filtration", &InterfaceSurface::filtration,
            "List of (simplex, filtration_value) tuples")
        .def_readonly("weighted", &InterfaceSurface::weighted,
            "Whether weighted Delaunay/alpha complex was used")
        .def_readonly("alpha", &InterfaceSurface::alpha,
            "Whether alpha complex was used");

    // Bind InterfaceGenerator
    py::class_<InterfaceGenerator>(m, "InterfaceGenerator")
        .def(py::init<>())
        .def("compute_interface_surface", &InterfaceGenerator::compute_interface_surface,
            py::arg("points"),
            py::arg("color_labels"),
            py::arg("radii") = Radii{},
            py::arg("config") = ComplexConfig{},
            "Compute the interface surface from colored points\n\n"
            "Parameters\n"
            "----------\n"
            "points : list of 3D points (numpy arrays or lists)\n"
            "    The input point cloud\n"
            "color_labels : list of int\n"
            "    Color label for each point\n"
            "radii : list of float, optional\n"
            "    Radius for each point (required if weighted=True)\n"
            "config : ComplexConfig, optional\n"
            "    Configuration for complex type\n\n"
            "Returns\n"
            "-------\n"
            "InterfaceSurface\n"
            "    The computed interface surface")
        .def("get_multicolored_tetrahedra", &InterfaceGenerator::get_multicolored_tetrahedra,
            py::arg("points"),
            py::arg("color_labels"),
            py::arg("radii") = Radii{},
            py::arg("config") = ComplexConfig{},
            "Get all multicolored tetrahedra from the complex\n\n"
            "Returns\n"
            "-------\n"
            "list of arrays\n"
            "    List of tetrahedra (each is array of 4 vertex indices)");

    // Convenience function
    m.def("get_barycentric_subdivision_and_filtration",
        &get_barycentric_subdivision_and_filtration,
        py::arg("points"),
        py::arg("color_labels"),
        py::arg("radii") = Radii{},
        py::arg("weighted") = true,
        py::arg("alpha") = true,
        "Compute barycentric subdivision and filtration\n\n"
        "Parameters\n"
        "----------\n"
        "points : list of 3D points\n"
        "    The input point cloud\n"
        "color_labels : list of int\n"
        "    Color label for each point\n"
        "radii : list of float, optional\n"
        "    Radius for each point (required if weighted=True)\n"
        "weighted : bool, default=True\n"
        "    Use weighted Delaunay/alpha complex\n"
        "alpha : bool, default=True\n"
        "    Use alpha complex (vs Delaunay complex)\n\n"
        "Returns\n"
        "-------\n"
        "tuple of (vertices, filtration)\n"
        "    vertices: list of barycenter points\n"
        "    filtration: list of (simplex, filtration_value) tuples");

    // Version info
    m.attr("__version__") = "0.1.0";
}
