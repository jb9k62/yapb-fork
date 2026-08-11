// Lesson 01 solution — hello.zig
//
// Reference for when you're stuck. Type it yourself first!
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const io = init.io;

    // --- writing ---
    var wbuf: [4096]u8 = undefined;
    var writer = std.Io.File.stdout().writerStreaming(io, &wbuf);
    var out = &writer.interface;
    try out.writeAll("What's your name?\n");

    // --- reading one line ---
    var rbuf: [4096]u8 = undefined;
    var reader = std.Io.File.stdin().readerStreaming(io, &rbuf);
    var input = &reader.interface;

    // '|line|' unwraps the optional; the line is non-null inside the block
    const name: []const u8 = (try input.takeDelimiter('\n')) orelse "";
    try out.print("hello {s}, welcome to zig!\n", .{name});

    // iterate the slice with an index; {c} prints a byte as a char
    for (name, 0..) |ch, i| {
        try out.print("{d}:{c} ", .{ i, ch });
    }
    try out.writeAll("\n");

    try writer.flush();
}
