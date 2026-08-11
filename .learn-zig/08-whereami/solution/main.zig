// Lesson 08 solution — the !go and !here branches (for on_client_command)
//
// dest: [3]f32  -- a point in GoldSrc space.
// X = forward, Y = right, Z = up (the third number is height).

const ORIGIN: [3]f32 = .{ 0, 0, 0 };

fn on_command_teleport(e: *eng.enginefuncs_t, ent: ?*eng.edict_t, args: []const u8) void {
    var msg: [160:0]u8 = undefined;

    // "!here" -> snap to world origin
    if (std.mem.eql(u8, args, "!here")) {
        _ = e.pfnSetOrigin(ent, &ORIGIN);
        const m = std.fmt.bufPrintZ(&msg, "Teleported to map origin.", .{}) catch "?";
        _ = e.pfnClientPrintf(ent, .chat, m.ptr);
        return;
    }

    // "!go X Y Z"
    const rest = std.mem.trim(u8, args["!go".len..], " ");
    var dest: [3]f32 = ORIGIN;
    var it = std.mem.tokenizeAny(u8, rest, " ");
    for (0..3) |i| {
        const tok = it.next() orelse {
            const m = std.fmt.bufPrintZ(&msg, "Usage: !go X Y Z", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return;
        };
        const v = std.fmt.parseFloat(f32, tok) catch {
            const m = std.fmt.bufPrintZ(&msg, "Usage: !go X Y Z (numbers)", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return;
        };
        // clamp so a bad input can't throw someone into the void
        dest[i] = @min(@max(v, -4096.0), 4096.0);
    }

    _ = e.pfnSetOrigin(ent, &dest);
    const m = std.fmt.bufPrintZ(&msg, "You moved to {d:.0} {d:.0} {d:.0}", .{ dest[0], dest[1], dest[2] }) catch "?";
    _ = e.pfnClientPrintf(ent, .chat, m.ptr);
}

// wire-in inside on_client_command, in the "!..." branch:
//   if (std.mem.startsWith(u8, args, "!go") or std.mem.eql(u8, args, "!here")) {
//       on_command_teleport(e, ent, args);
//       return_meta(.supercede);
//       return;
//   }
