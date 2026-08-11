// Lesson 06 solution — the hook section of main.zig
// Drop this in where `our_client_command` lived in the Lesson 05 scaffold.
//
// You'll also want two helpers at container scope:
//   fn cstr(s: [*:0]const u8) []const u8  ->  std.mem.sliceTo(s, 0)
//   fn is_say(s: []const u8) bool -> std.mem.startsWith(u8, s, "say")

fn on_client_command(ent: ?*edict_t) callconv(.c) void {
    // You can still scope the whole body to the engine funcs for convenience:
    if (engfuncs) |e| {
        const cmd = cstr(e.pfnCmd_Argv(0));
        const args = cstr(e.pfnCmd_Args());

        // we only care about chat
        if (!std.mem.eql(u8, cmd, "say")) {
            return_meta(.ignored);
            return;
        }

        var msg: [128:0]u8 = undefined; // :0 suffix => NUL-terminated, safe to pass to C

        if (std.mem.startsWith(u8, args, "!greet")) {
            const m = std.fmt.bufPrintZ(&msg, "Hey, you typed a command!", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return_meta(.supercede); // consume our command text
            return;
        }

        if (std.mem.startsWith(u8, args, "!who")) {
            const argc = e.pfnCmd_Argc();
            const m = std.fmt.bufPrintZ(&msg, "cmd: {d} args: {s}", .{ argc, args }) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return_meta(.supercede);
            return;
        }

        // anything else: let the real 'say' broadcast to everyone
        return_meta(.ignored);
        _ = ent;
    }
}
