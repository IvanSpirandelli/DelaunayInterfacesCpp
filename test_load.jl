println("Starting test...")

using CxxWrap
println("CxxWrap loaded successfully")

lib_path = joinpath(@__DIR__, "build", "julia", "libdelaunay_interfaces_jl")
println("Library path: $lib_path")

println("Attempting to wrap module...")
try
    @wrapmodule(() -> lib_path)
    println("Module wrapped successfully!")
catch e
    println("Error wrapping module:")
    println(e)
    rethrow(e)
end
