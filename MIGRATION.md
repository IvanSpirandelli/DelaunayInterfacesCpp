# Migration Guide from Julia to C++

This guide helps users of the original DelaunayInterfaces.jl package migrate to the C++ version.

## API Comparison

### Julia Original

```julia
using DelaunayInterfaces

points = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], ...]
colors = [1, 1, 2, 2]
radii = [0.5, 0.5, 0.5, 0.5]

surface = InterfaceSurface(points, colors, radii, true)  # alpha=true
vertices = surface.vertices
filtration = surface.filtration
```

### C++ Version

```cpp
#include <delaunay_interfaces/interface_generation.hpp>

using namespace delaunay_interfaces;

Points points = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, ...};
ColorLabels colors = {1, 1, 2, 2};
Radii radii = {0.5, 0.5, 0.5, 0.5};

InterfaceGenerator generator;
ComplexConfig config{true, true};  // weighted=true, alpha=true
auto surface = generator.compute_interface_surface(points, colors, radii, config);

auto vertices = surface.vertices;
auto filtration = surface.filtration;
```

### Python Bindings

```python
from delaunay_interfaces import InterfaceGenerator, ComplexConfig

points = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], ...]
colors = [1, 1, 2, 2]
radii = [0.5, 0.5, 0.5, 0.5]

generator = InterfaceGenerator()
config = ComplexConfig(weighted=True, alpha=True)
surface = generator.compute_interface_surface(points, colors, radii, config)

vertices = surface.vertices
filtration = surface.filtration
```

### Julia Bindings (to C++)

```julia
using DelaunayInterfacesCpp

points = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], ...]
colors = [1, 1, 2, 2]
radii = [0.5, 0.5, 0.5, 0.5]

surface = InterfaceSurface(points, colors, radii; weighted=true, alpha=true)
vertices = vertices(surface)
filtration = filtration(surface)
```

## Key Differences

### 1. Dependencies

**Julia Original:**
- Uses PyCall to interface with Python's `diode` library
- Requires Python environment with diode installed

**C++ Version:**
- Direct CGAL integration
- No Python dependency for core functionality
- Faster and more reliable

### 2. Type System

**Julia Original:**
```julia
points::Vector{Vector{Float64}}
color_labels::Vector{Int}
```

**C++ Version:**
```cpp
using Points = std::vector<Eigen::Vector3d>;
using ColorLabels = std::vector<int>;
```

### 3. Configuration

**Julia Original:**
```julia
# Parameters passed directly to function
get_barycentric_subdivision_and_filtration(points, colors, radii, true, true)
```

**C++ Version:**
```cpp
// Structured configuration
ComplexConfig config{weighted, alpha};
```

### 4. Return Values

**Julia Original:**
```julia
# Returns tuple
vertices, filtration = get_barycentric_subdivision_and_filtration(...)
```

**C++ Version:**
```cpp
// Returns std::pair
auto [vertices, filtration] = get_barycentric_subdivision_and_filtration(...);
```

## Performance Comparison

Expected performance improvements with C++ version:

| Operation | Julia (PyCall) | C++ | Speedup |
|-----------|---------------|-----|---------|
| Delaunay construction | Baseline | ~2-3x | Direct CGAL |
| Barycentric subdivision | Baseline | ~1.5-2x | Optimized C++ |
| Overall | Baseline | ~2-2.5x | No Python interop |

Actual speedups depend on:
- Point cloud size
- Number of multicolored tetrahedra
- System configuration

## Breaking Changes

1. **Index Convention:**
   - Julia uses 1-based indexing
   - C++ uses 0-based indexing
   - However, for compatibility, the bindings handle this automatically

2. **Configuration Structure:**
   - Old: boolean flags passed as parameters
   - New: `ComplexConfig` struct for better organization

3. **Error Handling:**
   - Julia: Typically throws errors with stack traces
   - C++: Uses exceptions (`std::invalid_argument`, `std::runtime_error`)
   - Python: Converts to Python exceptions
   - Julia bindings: Converts to Julia exceptions

## Migrating Existing Code

### Step 1: Install C++ Library

```bash
cd DelaunayInterfacesCpp
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

### Step 2: Update Julia Code (Option A: Keep Julia)

Replace:
```julia
using DelaunayInterfaces
```

With:
```julia
using DelaunayInterfacesCpp
```

Most code should work with minimal changes. The API is designed to be compatible.

### Step 3: Or Migrate to Python

Python often provides better ecosystem integration:

```python
pip install delaunay-interfaces

# Then use Python API
from delaunay_interfaces import InterfaceGenerator
```

## Gradual Migration Strategy

You can use both versions side-by-side:

```julia
using DelaunayInterfaces as DI_Old
using DelaunayInterfacesCpp as DI_New

# Old code continues to work
surface_old = DI_Old.InterfaceSurface(points, colors, radii)

# New code for performance-critical sections
surface_new = DI_New.InterfaceSurface(points, colors, radii)

# Compare results
@assert isapprox(surface_old.vertices, vertices(surface_new))
```

## Troubleshooting

### Issue: Results differ slightly

**Cause:** Numerical precision differences between diode and CGAL

**Solution:** Use approximate comparisons with tolerance:
```julia
@test isapprox(result1, result2, rtol=1e-10)
```

### Issue: Performance not improved

**Cause:** Debug build or inefficient memory allocation

**Solution:** Use Release build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Issue: Import errors in Python

**Cause:** Library not in Python path

**Solution:**
```bash
cd python
pip install . --force-reinstall
```

## Getting Help

If you encounter issues migrating:

1. Check the [examples](examples/) directory
2. Review the API documentation
3. Open an issue on GitHub
4. Compare with original Julia tests

## Future Compatibility

The C++ version will maintain API compatibility with major versions. We recommend:

1. Pin to specific version in your projects
2. Test thoroughly when upgrading
3. Read release notes for breaking changes
