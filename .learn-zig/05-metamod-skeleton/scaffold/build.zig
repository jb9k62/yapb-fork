// Lesson 05 scaffold — build.zig (shared library, cross-compiled to 32-bit)
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addLibrary(.{
        .linkage = .dynamic, // -> libcsguess.so
        .name = "csguess",
        .root_module = b.createModule(.{
            .root_source_file = b.path("main.zig"),
            // enginefuncs.zig must sit next to main.zig (copy from ../reference/)
            .target = target,
            .optimize = optimize,
        }),
    });

    b.installArtifact(lib);
}
