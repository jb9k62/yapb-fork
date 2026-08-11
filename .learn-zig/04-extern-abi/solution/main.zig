// Lesson 04 solution — main.zig (prove the engine table transcription)
const std = @import("std");

const eng = @import("enginefuncs.zig");
const enginefuncs_t = eng.enginefuncs_t;

pub fn main() void {
    std.debug.print("offsetof pfnSetOrigin     = {d}\n", .{@offsetOf(enginefuncs_t, "pfnSetOrigin")});
    std.debug.print("offsetof pfnClientPrintf  = {d}\n", .{@offsetOf(enginefuncs_t, "pfnClientPrintf")});
    std.debug.print("size of enginefuncs_t    = {d} bytes\n", .{@sizeOf(enginefuncs_t)});
}

// A C-callable symbol: the shape the plugin's Meta_* functions will take.
export fn game_started() c_int {
    return 1234;
}
