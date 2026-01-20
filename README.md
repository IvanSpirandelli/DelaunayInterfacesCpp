# DelaunayInterfaces

A C++ library for computing interface surfaces from multicolored point clouds using Delaunay/alpha complexes and barycentric subdivision.

## Features

- Compute interface surfaces from colored 3D point clouds
- Python bindings via pybind11
- Julia bindings via CxxWrap.jl

## Dependencies

- C++17 compiler
- CMake >= 3.15
- [CGAL](https://www.cgal.org/) - Computational Geometry Algorithms Library
- [Eigen3](https://eigen.tuxfamily.org/) - Linear algebra library
- [pybind11](https://github.com/pybind/pybind11) - For Python bindings (auto-downloaded if not found)
- [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl) - For Julia bindings

### Installing Dependencies

#### macOS (using Homebrew)
```bash
brew install cmake cgal eigen
```

#### Ubuntu/Debian
```bash
sudo apt-get install cmake libcgal-dev libeigen3-dev
```

#### For Julia bindings
```julia
using Pkg
Pkg.add("CxxWrap")
```

## Building from Source

### Basic Build

```bash
git clone <repository-url>
cd DelaunayInterfaces
mkdir build && cd build
cmake ..
sudo make install  # Note: sudo required on some systems for file permissions
```

**Note**: On macOS, you may need to use `sudo make install` due to file permission requirements during the build process.

### Build Options

- `BUILD_PYTHON_BINDINGS` (default: ON) - Build Python bindings
- `BUILD_JULIA_BINDINGS` (default: ON) - Build Julia bindings
- `BUILD_TESTS` (default: ON) - Build tests
- `BUILD_EXAMPLES` (default: ON) - Build examples

Example:
```bash
cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_JULIA_BINDINGS=OFF ..
```

### Building with Julia Bindings (CxxWrap.jl)

When building with Julia bindings enabled, CMake needs to locate the `libcxxwrap-julia` library. The recommended approach is to use CxxWrap.jl's CMake prefix path:

1. First, get the CxxWrap prefix path from Julia:
```julia
using CxxWrap
CxxWrap.prefix_path()
```

2. Then build with the prefix path:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/path/to/libcxxwrap-julia-prefix \
      ..
cmake --build . --config Release
```

#### Requirements for Julia Bindings

- C++17 compatible compiler (GCC 7+, clang 5+, or Xcode 9.3+ for macOS)
- CxxWrap.jl installed in Julia (uses `libcxxwrap_julia_jll` for binary management)

#### Platform-Specific Notes

- **Windows/MSVC**: Default CxxWrap binaries use GCC cross-compilation and are incompatible with Visual Studio C++. You must build `libcxxwrap-julia` and wrapper modules from source. See the [libcxxwrap-julia README](https://github.com/JuliaInterop/libcxxwrap-julia) Windows section for details.

## Installation

### C++ Library
```bash
cd build
sudo make install
```

### Python Package
```bash
cd python
pip install .
```

### Julia Package

The Julia package requires the C++ library to be built first with `BUILD_JULIA_BINDINGS=ON`.

```julia
# After building the C++ library with Julia bindings
using Pkg
Pkg.develop(path="path/to/DelaunayInterfaces/julia")
using DelaunayInterfaces
```

**Julia Package Structure**:
- `julia/Project.toml` - Package metadata and dependencies
- `julia/src/DelaunayInterfaces.jl` - Main module
- Build artifacts are in `build/julia/`

**Compatibility**: Julia 1.9-1.10, CxxWrap 0.14-0.15

## Usage

### C++

```cpp
#include <delaunay_interfaces/interface_generation.hpp>

using namespace delaunay_interfaces;

int main() {
    // Define points
    Points points = {
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };

    // Color labels (at least 2 different colors)
    ColorLabels colors = {1, 1, 2, 2};

    // Radii for weighted complex
    Radii radii = {0.5, 0.5, 0.5, 0.5};

    // Compute interface surface
    InterfaceGenerator generator;
    ComplexConfig config{true, true}; // weighted=true, alpha=true
    auto surface = generator.compute_interface_surface(points, colors, radii, config);

    std::cout << "Number of barycenters: " << surface.vertices.size() << std::endl;
    std::cout << "Filtration size: " << surface.filtration.size() << std::endl;

    return 0;
}
```

### Python

```python
import numpy as np
from delaunay_interfaces import InterfaceGenerator, ComplexConfig

# Define points
points = [
    [0.0, 0.0, 0.0],
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]
]

# Color labels
colors = [1, 1, 2, 2]

# Radii
radii = [0.5, 0.5, 0.5, 0.5]

# Compute interface surface
generator = InterfaceGenerator()
config = ComplexConfig(weighted=True, alpha=True)
surface = generator.compute_interface_surface(points, colors, radii, config)

print(f"Number of barycenters: {len(surface.vertices)}")
print(f"Filtration size: {len(surface.filtration)}")

# Or use the convenience function
from delaunay_interfaces import get_barycentric_subdivision_and_filtration

vertices, filtration = get_barycentric_subdivision_and_filtration(
    points, colors, radii, weighted=True, alpha=True
)
```

### Julia

```julia
using DelaunayInterfaces

# Define points
points = [
    [0.0, 0.0, 0.0],
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]
]

# Color labels (can be any integers, at least 2 different colors needed)
colors = [1, 1, 2, 2]

# Radii (required for weighted complexes)
radii = [0.5, 0.5, 0.5, 0.5]

# Create configuration
config = ComplexConfig(true, true)  # weighted=true, alpha=true

# Create generator and compute interface surface
gen = InterfaceGenerator()
surface = compute_interface_surface(gen, points, colors, radii, config)

# Access vertices (note: 0-based indexing from C++)
println("Number of vertices: ", num_vertices(surface))
for i in 0:num_vertices(surface)-1
    vertex = get_vertex(surface, i)
    println("Vertex $i: ", vertex)
end

# Access filtration
println("Number of simplices: ", num_simplices(surface))
for i in 0:num_simplices(surface)-1
    simplex_vertices = get_simplex_vertices(surface, i)
    simplex_value = get_simplex_value(surface, i)
    println("Simplex $i: vertices=$simplex_vertices, value=$simplex_value")
end

# Get multicolored tetrahedra
tets = get_multicolored_tetrahedra(gen, points, colors, radii, config)
println("Number of multicolored tetrahedra: ", length(tets))
```

**Note**: Julia bindings use 0-based indexing for accessing vertices and simplices (inherited from C++), but the vertex indices in simplices are also 0-based.

## Understanding the Output

The main computation returns two components:

### Vertices (Barycenters)
A list of 3D points representing the barycenters computed from the barycentric subdivision of multicolored tetrahedra.

### Filtration
In topological data analysis (TDA), a **filtration** is a nested sequence of simplicial complexes indexed by a parameter. Here, the filtration is represented as a list of pairs `(simplex, filtration_value)` where:
- **simplex**: A list of vertex indices (into the vertices array)
  - Size 1: vertices (0-simplices)
  - Size 2: edges (1-simplices)
  - Size 3: triangles (2-simplices) - **these form the interface surface**
- **filtration_value**: The parameter value at which the simplex appears in the filtered complex, computed from distances between color partition barycenters

To extract the interface surface triangles, filter the filtration for simplices of size 3.

## Algorithm Overview

The library implements the following algorithm:

1. **Delaunay/Alpha Complex Construction**: Computes the Delaunay triangulation or weighted alpha complex of the input points using CGAL
2. **Multicolored Tetrahedra Extraction**: Filters tetrahedra that have vertices with at least 2 different colors
3. **Chromatic Partitioning**: Groups vertices of each tetrahedron by color
4. **Barycentric Subdivision**: Subdivides each multicolored tetrahedron based on its chromatic partitioning
5. **Filtration Computation**: Computes filtration values based on Euclidean distances between partition barycenters