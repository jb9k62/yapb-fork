// Lesson 03 solution — build.zig (link a .c source into the module)
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "bridge",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    // compile our C source into the same module so vec_len is callable
    exe.root_module.addCSourceFile(.{ .file = b.path("src/helper.c"), .flags = &.{ "-std=c11" } });

    const run = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the bridge");
    run_step.dependOn(&run.step);

    b.installArtifact(exe);
}
