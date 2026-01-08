# Build Instructions

This guide provides detailed instructions for building DelaunayInterfaces on different platforms.

## Prerequisites

### All Platforms

- CMake >= 3.15
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CGAL >= 5.0
- Eigen3 >= 3.3

### Platform-Specific

#### macOS

Install dependencies using Homebrew:
```bash
brew install cmake cgal eigen boost gmp mpfr
```

For Python bindings:
```bash
pip install numpy
```

For Julia bindings:
```julia
using Pkg
Pkg.add("CxxWrap")
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libcgal-dev libeigen3-dev
sudo apt-get install libboost-all-dev libgmp-dev libmpfr-dev
```

For Python bindings:
```bash
pip install numpy
```

For Julia bindings:
```julia
using Pkg
Pkg.add("CxxWrap")
```

#### Windows

We recommend using vcpkg for dependency management:

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install cgal eigen3
```

## Building the Library

### Basic Build

```bash
# Clone the repository
git clone <repository-url>
cd DelaunayInterfaces

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . -j4

# Optional: Install
sudo cmake --install .
```

### Build Options

You can customize the build with the following options:

```bash
cmake -DBUILD_PYTHON_BINDINGS=ON \
      -DBUILD_JULIA_BINDINGS=ON \
      -DBUILD_TESTS=ON \
      -DBUILD_EXAMPLES=ON \
      -DCMAKE_BUILD_TYPE=Release \
      ..
```

#### Available Options

- `BUILD_PYTHON_BINDINGS` (default: ON) - Build Python bindings using pybind11
- `BUILD_JULIA_BINDINGS` (default: ON) - Build Julia bindings using CxxWrap
- `BUILD_TESTS` (default: ON) - Build test suite
- `BUILD_EXAMPLES` (default: ON) - Build example programs
- `CMAKE_BUILD_TYPE` - Build type (Debug, Release, RelWithDebInfo, MinSizeRel)

### Python-Only Build

If you only need Python bindings:

```bash
cd python
pip install .
```

This will:
1. Download pybind11 if not available
2. Build the C++ library
3. Build the Python bindings
4. Install the Python package

### Julia-Only Build

For Julia bindings, you need to build the C++ library first:

```bash
mkdir build && cd build
cmake -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_JULIA_BINDINGS=ON ..
make -j4
```

Then in Julia:
```julia
using Pkg
Pkg.develop(path="/path/to/DelaunayInterfaces/julia")
```

## Troubleshooting

### CGAL Not Found

If CMake cannot find CGAL:

```bash
# macOS
export CGAL_DIR=$(brew --prefix cgal)/lib/cmake/CGAL

# Linux
export CGAL_DIR=/usr/lib/cmake/CGAL

# Then run cmake again
cmake -DCGAL_DIR=$CGAL_DIR ..
```

### Eigen3 Not Found

If CMake cannot find Eigen3:

```bash
# macOS
export Eigen3_DIR=$(brew --prefix eigen)/share/eigen3/cmake

# Linux
export Eigen3_DIR=/usr/lib/cmake/eigen3

cmake -DEigen3_DIR=$Eigen3_DIR ..
```

### CxxWrap.jl Not Found

For Julia bindings, ensure CxxWrap is installed and findable:

```julia
using CxxWrap
println(CxxWrap.prefix_path())  # Should print a valid path
```

Then set the Julia prefix:
```bash
export Julia_PREFIX=/path/to/julia
cmake -DJulia_PREFIX=$Julia_PREFIX ..
```

### pybind11 Issues

pybind11 is automatically downloaded if not found. If you want to use a system installation:

```bash
# macOS
brew install pybind11

# Linux
sudo apt-get install pybind11-dev

# Then configure with
cmake -DCMAKE_PREFIX_PATH=/usr/local ..
```

## Verify Installation

### C++ Library

```bash
cd build/examples
./simple_example
```

### Python Bindings

```python
import delaunay_interfaces
print(delaunay_interfaces.__version__)
```

### Julia Bindings

```julia
using DelaunayInterfaces
println(DelaunayInterfaces.VERSION)
```

## Running Tests

After building with `BUILD_TESTS=ON`:

```bash
cd build
ctest --output-on-failure
```

## Performance Build

For maximum performance, use:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native" \
      ..
make -j$(nproc)
```

## Development Build

For development with debug symbols:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..
make -j$(nproc)
```

The `compile_commands.json` file will be generated for use with IDEs and language servers.
