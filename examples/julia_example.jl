#!/usr/bin/env julia
"""
DelaunayInterfaces Julia Example

This example demonstrates how to use the Julia bindings for the
DelaunayInterfaces C++ library.
"""

using DelaunayInterfacesCpp

function main()
    println("DelaunayInterfaces Julia Example")
    println("=" ^ 50)
    println()

    # Create a simple point cloud with two color groups
    points = [
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.5, 1.0, 0.0],
        [0.5, 0.5, 1.0],
        [2.0, 0.0, 0.0],
        [2.5, 1.0, 0.0],
        [2.5, 0.5, 1.0],
        [1.5, 0.5, 0.5]
    ]

    colors = [1, 1, 1, 1, 2, 2, 2, 2]
    radii = fill(0.3, length(points))

    println("Input:")
    println("  Points: ", length(points))
    println("  Colors: 2 groups")
    println()

    # Example 1: Using InterfaceSurface constructor
    println("Example 1: Using InterfaceSurface")
    println("-" ^ 50)

    surface = InterfaceSurface(points, colors, radii; weighted=true, alpha=true)

    verts = vertices(surface)
    filt = filtration(surface)

    println("  Barycenters: ", length(verts))
    println("  Filtration simplices: ", length(filt))
    println("  Weighted: ", weighted(surface))
    println("  Alpha: ", alpha(surface))
    println()

    # Example 2: Using convenience function
    println("Example 2: Using convenience function")
    println("-" ^ 50)

    vertices_result, filtration_result = get_barycentric_subdivision_and_filtration(
        points, colors, radii, true, true
    )

    println("  Barycenters: ", length(vertices_result))
    println("  Filtration simplices: ", length(filtration_result))
    println()

    # Show some filtration details
    println("First 10 filtration simplices:")
    for (i, (simplex, value)) in enumerate(filtration_result[1:min(10, end)])
        println("  ", i, ": dim=", length(simplex)-1, ", value=", value)
    end
    println()

    # Example 3: Get multicolored tetrahedra
    println("Example 3: Get multicolored tetrahedra")
    println("-" ^ 50)

    mc_tets = get_multicolored_tetrahedra_wrapper(
        points, colors, radii; weighted=true, alpha=true
    )

    println("  Number of multicolored tetrahedra: ", length(mc_tets))
    if length(mc_tets) > 0
        println("  First tetrahedron: ", mc_tets[1])
    end
    println()

    println("Done!")
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
