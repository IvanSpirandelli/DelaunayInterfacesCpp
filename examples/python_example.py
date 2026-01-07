#!/usr/bin/env python3
"""
DelaunayInterfaces Python Example

This example demonstrates how to use the Python bindings for the
DelaunayInterfaces C++ library.
"""

import numpy as np
try:
    from delaunay_interfaces import (
        InterfaceGenerator,
        ComplexConfig,
        get_barycentric_subdivision_and_filtration
    )
except ImportError:
    print("Error: delaunay_interfaces module not found.")
    print("Please build and install the Python bindings first:")
    print("  cd python && pip install .")
    exit(1)

def main():
    print("DelaunayInterfaces Python Example")
    print("=" * 50)
    print()

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
    radii = [0.3] * len(points)

    print(f"Input:")
    print(f"  Points: {len(points)}")
    print(f"  Colors: 2 groups")
    print()

    # Example 1: Using InterfaceGenerator
    print("Example 1: Using InterfaceGenerator")
    print("-" * 50)

    generator = InterfaceGenerator()
    config = ComplexConfig(weighted=True, alpha=True)
    surface = generator.compute_interface_surface(points, colors, radii, config)

    print(f"  Barycenters: {len(surface.vertices)}")
    print(f"  Filtration simplices: {len(surface.filtration)}")
    print(f"  Weighted: {surface.weighted}")
    print(f"  Alpha: {surface.alpha}")
    print()

    # Example 2: Using convenience function
    print("Example 2: Using convenience function")
    print("-" * 50)

    vertices, filtration = get_barycentric_subdivision_and_filtration(
        points, colors, radii, weighted=True, alpha=True
    )

    print(f"  Barycenters: {len(vertices)}")
    print(f"  Filtration simplices: {len(filtration)}")
    print()

    # Show some filtration details
    print("First 10 filtration simplices:")
    for i, (simplex, value) in enumerate(filtration[:10]):
        print(f"  {i}: dim={len(simplex)-1}, value={value:.6f}")
    print()

    # Example 3: Compare different complex types
    print("Example 3: Comparing different complex types")
    print("-" * 50)

    configs = [
        ("Regular Delaunay", ComplexConfig(weighted=False, alpha=False)),
        ("Weighted Delaunay", ComplexConfig(weighted=True, alpha=False)),
        ("Weighted Alpha", ComplexConfig(weighted=True, alpha=True)),
    ]

    for name, config in configs:
        if config.weighted:
            surface = generator.compute_interface_surface(points, colors, radii, config)
        else:
            surface = generator.compute_interface_surface(points, colors, [], config)
        print(f"  {name:20} -> {len(surface.vertices):3} barycenters, "
              f"{len(surface.filtration):3} simplices")

    print()
    print("Done!")

if __name__ == "__main__":
    main()
