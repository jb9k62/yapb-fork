// Lesson 07 solution — repo game state (merge into your bootstrap main.zig)
//
// Everything here is plain Zig; only `pfnClientPrintf` and the hook are engine.

const std = @import("std");

// --- game state (explicit globals, like engfuncs) ---
var secret: u8 = 0;
var have_secret = false;

fn new_secret() void {
    var prng = std.Random.DefaultPrng.init(@intCast(std.time.nanoTimestamp()));
    secret = prng.random().intRangeAtMost(u8, 1, 100);
    have_secret = true;
}

// --- copied VERBATIM from Lesson 02 ---
fn react(s: u8, g: u8) []const u8 {
    if (g == s) return "Correct!";
    const delta = if (g > s) g - s else s - g;
    if (delta <= 5) return "Hot!";
    return "Cold.";
}

fn cstr(p: [*:0]const u8) []const u8 {
    return std.mem.sliceTo(p, 0);
}

// hook: replace on_client_command's body with this
fn on_client_command(ent: ?*edict_t) callconv(.c) void {
    if (engfuncs) |e| {
        const cmd = cstr(e.pfnCmd_Argv(0));
        const args = cstr(e.pfnCmd_Args());
        if (!std.mem.eql(u8, cmd, "say")) {
            return_meta(.ignored);
            return;
        }

        if (!std.mem.startsWith(u8, args, "!guess")) {
            return_meta(.ignored); // normal chat passes to everyone
            return;
        }

        // we own the command from here -> consume it so it doesn't echo raw
        var msg: [128:0]u8 = undefined;

        if (std.mem.eql(u8, args, "!guess new")) {
            new_secret();
            const m = std.fmt.bufPrintZ(&msg, "New secret ready. !guess <1-100>.", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return_meta(.supercede);
            return;
        }

        if (args.len <= "!guess".len) {
            const m = std.fmt.bufPrintZ(&msg, "Usage: !guess <1-100>", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return_meta(.supercede);
            return;
        }

        // parse the number after "!guess" (raw slice includes leading space)
        const num_text = std.mem.trim(u8, args["!guess".len..], " ");
        const guess = std.fmt.parseInt(u8, num_text, 10) catch {
            const m = std.fmt.bufPrintZ(&msg, "That's not a number!", .{}) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
            return_meta(.supercede);
            return;
        };

        if (!have_secret) new_secret();
        const verdict = react(secret, guess);

        if (guess == secret) {
            // big center splash on win
            const big = std.fmt.bufPrintZ(&msg, "!! CORRECT !! Secret was {d}.", .{secret}) catch "?";
            _ = e.pfnClientPrintf(ent, .center, big.ptr);
            new_secret();
        } else {
            const m = std.fmt.bufPrintZ(&msg, "Guess {d}: {s}", .{ guess, verdict }) catch "?";
            _ = e.pfnClientPrintf(ent, .chat, m.ptr);
        }
        return_meta(.supercede);
        return;
    }
}
