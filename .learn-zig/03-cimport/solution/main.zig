// Lesson 03 solution — main.zig (call our C helper from Zig)
const std = @import("std");

const c = @cImport({
    @cInclude("helper.h");
});

pub fn main(init: std.process.Init) !void {
    const v = c.Vec2{ .x = 3, .y = 4 };
    const len = c.vec_len(v);
    std.debug.print("vec_len({{3,4}}) = {d}\n", .{len});

    const v2 = c.Vec2{ .x = -1.5, .y = 2.0 };
    std.debug.print("vec_len({{-1.5,2}}) = {d:.2}\n", .{c.vec_len(v2)});

    std.debug.print("Vec2 size = {d} bytes\n", .{@sizeOf(c.Vec2)});
}
