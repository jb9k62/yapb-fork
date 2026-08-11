// Lesson 02 solution — main.zig (the !guess game in pure Zig)
//
// Reference. Type it yourself first — you'll reuse this logic in Lesson 07.
const std = @import("std");

/// Decide a reaction to a guess. This exact function is the heart of the
/// in-game plugin in Lesson 07.
fn react(secret: u8, guess: u8) []const u8 {
    if (guess == secret) return "Correct!";
    const delta = if (guess > secret) guess - secret else secret - guess;
    if (delta <= 5) return "Hot!"; // within 5: warm
    return "Cold."; // far away: cold
}

fn play(io: std.Io) !void {
    // secret number 1..=100
    var prng = std.Random.DefaultPrng.init(@intCast(std.time.nanoTimestamp()));
    const rand = prng.random();
    const secret = rand.intRangeAtMost(u8, 1, 100);

    var wbuf: [4096]u8 = undefined;
    var writer = std.Io.File.stdout().writerStreaming(io, &wbuf);
    var out = &writer.interface;

    var rbuf: [512]u8 = undefined;
    var reader = std.Io.File.stdin().readerStreaming(io, &rbuf);
    var input = &reader.interface;

    var msg: [64]u8 = undefined;

    var guesses: u32 = 0;
    while (true) {
        try out.writeAll("Guess 1..100 (or 'q' to quit): ");
        try writer.flush();

        const line = (try input.takeDelimiter('\n')) orelse break;
        if (std.mem.eql(u8, line, "q")) {
            try out.writeAll("Bye\n");
            break;
        }

        const guess = std.fmt.parseInt(u8, line, 10) catch {
            try out.writeAll("Not a number!\n");
            continue;
        };
        guesses += 1;

        const reaction = react(secret, guess);
        const side = if (secret > guess) "low" else "high";

        if (guess == secret) {
            const done = std.fmt.bufPrint(&msg, "You got it in {d} guesses! Secret was {d}.\n", .{ guesses, secret }) catch "?\n";
            try out.writeAll(done);
            break;
        } else {
            const line2 = std.fmt.bufPrint(&msg, "[{d}] guess {d}: {s} (too {s})\n", .{ guesses, guess, reaction, side }) catch "?\n";
            try out.writeAll(line2);
        }
    }
    try writer.flush();
}

pub fn main(init: std.process.Init) !void {
    try play(init.io);
}
