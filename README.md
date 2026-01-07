# DelaunayInterfaces C++

A high-performance C++ library for computing interface surfaces from multicolored point clouds using Delaunay/alpha complexes and barycentric subdivision.

This is a C++ implementation of the original [DelaunayInterfaces.jl](https://github.com/ivanspirandelli/DelaunayInterfaces.jl) Julia package, with bindings for both Julia and Python.

## Features

- Compute interface surfaces from colored 3D point clouds
- Support for:
  - Regular Delaunay triangulation
  - Weighted Delaunay triangulation
  - Weighted alpha complexes
- Barycentric subdivision with filtration values
- High-performance C++ implementation using CGAL
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

```bash
git clone <repository-url>
cd DelaunayInterfacesCpp
mkdir build && cd build
cmake ..
make -j4
```

### Build Options

- `BUILD_PYTHON_BINDINGS` (default: ON) - Build Python bindings
- `BUILD_JULIA_BINDINGS` (default: ON) - Build Julia bindings
- `BUILD_TESTS` (default: ON) - Build tests
- `BUILD_EXAMPLES` (default: ON) - Build examples

Example:
```bash
cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_JULIA_BINDINGS=OFF ..
```

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
```julia
# After building with BUILD_JULIA_BINDINGS=ON
using Pkg
Pkg.develop(path="path/to/DelaunayInterfacesCpp/julia")
```

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
using DelaunayInterfacesCpp

# Define points
points = [
    [0.0, 0.0, 0.0],
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]
]

# Color labels (Julia uses 1-based indexing, but colors can be any integers)
colors = [1, 1, 2, 2]

# Radii
radii = [0.5, 0.5, 0.5, 0.5]

# Compute interface surface
surface = InterfaceSurface(points, colors, radii; weighted=true, alpha=true)

println("Number of barycenters: ", length(vertices(surface)))
println("Filtration size: ", length(filtration(surface)))

# Or use the convenience function
vertices, filtration = get_barycentric_subdivision_and_filtration(
    points, colors, radii, true, true
)
```

## Algorithm Overview

The library implements the following algorithm:

1. **Delaunay/Alpha Complex Construction**: Computes the Delaunay triangulation or weighted alpha complex of the input points using CGAL
2. **Multicolored Tetrahedra Extraction**: Filters tetrahedra that have vertices with at least 2 different colors
3. **Chromatic Partitioning**: Groups vertices of each tetrahedron by color
4. **Barycentric Subdivision**: Subdivides each multicolored tetrahedron based on its chromatic partitioning
5. **Filtration Computation**: Computes filtration values based on Euclidean distances between partition barycenters

## Performance

The C++ implementation offers significant performance improvements over the original Julia implementation:
- Direct CGAL integration (no Python interop overhead)
- Optimized memory management
- Parallel processing capabilities (future enhancement)

## Citation

If you use this library in your research, please cite:

```
[Citation information to be added]
```

## License

[License to be determined]

## Authors

- Ivan Spirandelli (original Julia implementation)
- C++ port: [To be added]

## Acknowledgments

This C++ implementation is based on the original [DelaunayInterfaces.jl](https://github.com/ivanspirandelli/DelaunayInterfaces.jl) package.
