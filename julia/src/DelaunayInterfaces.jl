module DelaunayInterfaces

using CxxWrap

@wrapmodule(() -> joinpath(dirname(@__FILE__), "..", "..", "build", "julia", "libdelaunay_interfaces_jl"))

function __init__()
    @initcxx
end

"""
    InterfaceSurface(points, color_labels[, radii]; weighted=true, alpha=true)

Compute the interface surface from a colored point cloud.

# Arguments
- `points::Vector{Vector{Float64}}`: Vector of 3D points
- `color_labels::Vector{Int}`: Color label for each point
- `radii::Vector{Float64}`: Radius for each point (optional, required if weighted=true)
- `weighted::Bool`: Use weighted Delaunay/alpha complex (default: true)
- `alpha::Bool`: Use alpha complex vs Delaunay complex (default: true)

# Returns
- `InterfaceSurface`: Object containing vertices and filtration

# Examples
```julia
points = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
colors = [1, 1, 2, 2]
radii = [0.5, 0.5, 0.5, 0.5]

surface = InterfaceSurface(points, colors, radii)
```
"""
function InterfaceSurface(
    points::Vector{Vector{Float64}},
    color_labels::Vector{Int},
    radii::Vector{Float64} = Float64[];
    weighted::Bool = true,
    alpha::Bool = true
)
    gen = InterfaceGenerator()
    return compute_interface_surface(gen, points, color_labels, radii, weighted, alpha)
end

"""
    get_multicolored_tetrahedra(points, color_labels[, radii]; weighted=true, alpha=true)

Get all multicolored tetrahedra from the Delaunay/alpha complex.

# Returns
- Matrix where each row is a tetrahedron with 4 vertex indices [v0, v1, v2, v3]
"""
function get_multicolored_tetrahedra_wrapper(
    points::Vector{Vector{Float64}},
    color_labels::Vector{Int},
    radii::Vector{Float64} = Float64[];
    weighted::Bool = true,
    alpha::Bool = true
)
    gen = InterfaceGenerator()
    flat_result = get_multicolored_tetrahedra(gen, points, color_labels, radii, weighted, alpha)

    # Reshape flat array into matrix where each row is a tetrahedron
    if isempty(flat_result)
        return Matrix{Int}(undef, 0, 4)
    end
    return reshape(flat_result, 4, :)'
end

export InterfaceSurface, InterfaceGenerator
export get_multicolored_tetrahedra_wrapper

end # module
