// Lesson 05 scaffold — main.zig
//
// The complete MetaMod bootstrap for our plugin, transcribed from YaPB's
// WORKING C++ (src/linkage.cpp). Re-type it, comment it in your own words,
// and get it to load. Everything after this lesson reuses `engfuncs`.
//
// Files:
//   main.zig        <- this file (re-type it)
//   enginefuncs.zig <- copy from ../reference/enginefuncs.zig
//   build.zig       <- provided in this folder

const std = @import("std");
const eng = @import("enginefuncs.zig");

const c_int = std.c.c_int;
const edict_t = eng.edict_t;
const enginefuncs_t = eng.enginefuncs_t;

// ---------------------------------------------------------------------------
// MMOD ENUMS  (ext/linkage/linkage/metamod.h)
// ---------------------------------------------------------------------------
const PLUG_LOADTIME = enum(c_int) { never = 0, startup = 1, changelevel = 2, anytime = 3, anypause = 4 };
const PL_UNLOAD_REASON = enum(c_int) { _null = 0, ini_deleted = 1, file_newer = 2, command = 3, cmd_forced = 4, delayed = 5, plugin = 6, plg_forced = 7, reload = 8 };
const META_RES = enum(c_int) { unset = 0, ignored = 1, handled = 2, override = 3, supercede = 4 };

const META_INTERFACE_VERSION = "5:13";

// ---------------------------------------------------------------------------
// PLUGIN INFO  (static description handed back by Meta_Query)
// ---------------------------------------------------------------------------
var plugin_info: plugin_info_t = .{
    .ifvers = META_INTERFACE_VERSION,
    .name = "csguess",
    .version = "0.1.0",
    .date = "2026",
    .author = "you",
    .url = "",
    .logtag = "CSGUESS",
    .loadable = .anytime,
    .unloadable = .anytime,
};

// ---------------------------------------------------------------------------
// TABLE STRUCTS  (all `extern` = exact C layout; every member pointer-sized)
// ---------------------------------------------------------------------------
const plugin_info_t = extern struct {
    ifvers: [*:0]const u8,
    name: [*:0]const u8,
    version: [*:0]const u8,
    date: [*:0]const u8,
    author: [*:0]const u8,
    url: [*:0]const u8,
    logtag: [*:0]const u8,
    loadable: PLUG_LOADTIME,
    unloadable: PLUG_LOADTIME,
};

const meta_globals_t = extern struct {
    mres: META_RES,
    prev_mres: META_RES,
    status: META_RES,
    orig_ret: ?*anyopaque,
    override_ret: ?*anyopaque,
};

// The game DLL callback table. We only need the prefix up to pfnClientCommand
// (the hook we install). Unused members keep a generic pointer type — the
// layout still lines up because every member is one pointer.
const gamefuncs_t = extern struct {
    pfnGameInit: *const fn () callconv(.c) void,
    pfnSpawn: *const fn (?*edict_t) callconv(.c) c_int,
    pfnThink: *const fn (?*edict_t) callconv(.c) void,
    pfnUse: *const fn (?*edict_t, ?*edict_t) callconv(.c) void,
    pfnTouch: *const fn (?*edict_t, ?*edict_t) callconv(.c) void,
    pfnBlocked: *const fn (?*edict_t, ?*edict_t) callconv(.c) void,
    pfnKeyValue: *const fn (?*edict_t, ?*anyopaque) callconv(.c) void,
    pfnSave: *const fn (?*edict_t, ?*anyopaque) callconv(.c) void,
    pfnRestore: *const fn (?*edict_t, ?*anyopaque, c_int) callconv(.c) c_int,
    pfnSetAbsBox: *const fn (?*edict_t) callconv(.c) void,
    pfnSaveWriteFields: *const fn (?*anyopaque, [*:0]const u8, ?*anyopaque, ?*anyopaque, c_int) callconv(.c) void,
    pfnSaveReadFields: *const fn (?*anyopaque, [*:0]const u8, ?*anyopaque, ?*anyopaque, c_int) callconv(.c) void,
    pfnSaveGlobalState: *const fn (?*anyopaque) callconv(.c) void,
    pfnRestoreGlobalState: *const fn (?*anyopaque) callconv(.c) void,
    pfnResetGlobalState: *const fn () callconv(.c) void,
    pfnClientConnect: *const fn (?*edict_t, [*:0]const u8, [*:0]const u8, [*]u8) callconv(.c) c_int,
    pfnClientDisconnect: *const fn (?*edict_t) callconv(.c) void,
    pfnClientKill: *const fn (?*edict_t) callconv(.c) void,
    pfnClientPutInServer: *const fn (?*edict_t) callconv(.c) void,
    // *** OUR CHAT HOOK TARGET ***
    pfnClientCommand: *const fn (?*edict_t) callconv(.c) void,
};

const newgamefuncs_t = extern struct {};

const plid_t = *plugin_info_t;

// MetaMod utility functions. Only pfnGetHookTables is used here (to get the
// real engine table); the rest keep generic pointer types for layout only.
const mutil_funcs_t = extern struct {
    pfnLogConsole: *const fn () callconv(.c) c_int,
    pfnLogMessage: *const fn () callconv(.c) c_int,
    pfnLogError: *const fn () callconv(.c) c_int,
    pfnLogDeveloper: *const fn () callconv(.c) c_int,
    pfnCenterSay: *const fn () callconv(.c) c_int,
    pfnCenterSayParms: *const fn () callconv(.c) c_int,
    pfnCenterSayVarargs: *const fn () callconv(.c) c_int,
    pfnCallGameEntity: *const fn () callconv(.c) c_int,
    pfnGetUserMsgID: *const fn () callconv(.c) c_int,
    pfnGetUserMsgName: *const fn () callconv(.c) c_int,
    pfnGetPluginPath: *const fn () callconv(.c) c_int,
    pfnGetGameInfo: *const fn () callconv(.c) c_int,
    pfnLoadPlugin: *const fn () callconv(.c) c_int,
    pfnUnloadPlugin: *const fn () callconv(.c) c_int,
    pfnUnloadPluginByHandle: *const fn () callconv(.c) c_int,
    pfnIsQueryingClienCVar: *const fn () callconv(.c) c_int,
    pfnMakeRequestID: *const fn () callconv(.c) c_int,
    // the one we actually call:
    pfnGetHookTables: *const fn (plid_t, ?*?*enginefuncs_t, ?*?*gamefuncs_t, ?*?*newgamefuncs_t) callconv(.c) c_int,
};

const gamedll_funcs_t = extern struct {
    dllapi_table: ?*gamefuncs_t,
    newapi_table: ?*newgamefuncs_t,
};

// The metamod function table: what *we* hand MetaMod so it can obtain our hooks.
const GETENTITYAPI_FN = *const fn (?*gamefuncs_t, c_int) callconv(.c) c_int;
const GETENTITYAPI2_FN = *const fn (?*gamefuncs_t, *c_int) callconv(.c) c_int;
const GETNEWDLLFUNCTIONS_FN = *const fn (?*newgamefuncs_t, *c_int) callconv(.c) c_int;
const GET_ENGINE_FUNCTIONS_FN = *const fn (?*enginefuncs_t, *c_int) callconv(.c) c_int;

const metamod_funcs_t = extern struct {
    pfnGetEntityAPI: ?GETENTITYAPI_FN,
    pfnGetEntityAPI_Post: ?GETENTITYAPI_FN,
    pfnGetEntityAPI2: ?GETENTITYAPI2_FN,
    pfnGetEntityAPI2_Post: ?GETENTITYAPI2_FN,
    pfnGetNewDLLFunctions: ?GETNEWDLLFUNCTIONS_FN,
    pfnGetNewDLLFunctions_Post: ?GETNEWDLLFUNCTIONS_FN,
    pfnGetEngineFunctions: ?GET_ENGINE_FUNCTIONS_FN,
    pfnGetEngineFunctions_Post: ?GET_ENGINE_FUNCTIONS_FN,
};

// ---------------------------------------------------------------------------
// GLOBAL STATE  — explicit in Zig, as it should be
// ---------------------------------------------------------------------------
var engfuncs: ?*enginefuncs_t = null; // the engine's function table (set at attach)
var gp_globals: ?*meta_globals_t = null;
var gp_mutil: ?*mutil_funcs_t = null;

// The MetaMod "RETURN_META" macro, written out: tell metamod whether to keep
// chaining this call to the real game (ignored) or stop (supercede).
fn return_meta(res: META_RES) void {
    if (gp_globals) |g| g.mres = res;
}

fn version_ok(theirs: [*:0]const u8) bool {
    const needed = META_INTERFACE_VERSION;
    const tlen = std.mem.len(theirs);
    if (tlen < needed.len) return false;
    return std.mem.eql(u8, needed, theirs[0..needed.len]);
}

// ---------------------------------------------------------------------------
// OUR CHAT HOOK  (Lesson 06 grows this into the real game)
// ---------------------------------------------------------------------------
fn our_client_command(ent: ?*edict_t) callconv(.c) void {
    _ = ent;
    if (engfuncs) |e| e.pfnServerPrint("csguess: [DEBUG] client command fired\n");
    return_meta(.ignored); // keep chaining to the real game
}

// Called by metamod so we can register which game-DLL callbacks to intercept.
fn get_entity_api(table: ?*gamefuncs_t, _: c_int) callconv(.c) c_int {
    if (table) |t| {
        t.* = std.mem.zeroes(gamefuncs_t);
        t.pfnClientCommand = our_client_command; // grab chat/console commands
    }
    return 1; // true
}

// Called by metamod so we can intercept engine calls. We don't need any.
fn get_engine_functions(_: ?*enginefuncs_t, _: *c_int) callconv(.c) c_int {
    return 1;
}

// ---------------------------------------------------------------------------
// THE FOUR META_* EXPORTS
// ---------------------------------------------------------------------------
export fn Meta_Init() callconv(.c) void {}

export fn Meta_Query(ifvers: [*:0]u8, info: *?*plugin_info_t, util: ?*mutil_funcs_t) callconv(.c) c_int {
    gp_mutil = util;
    info.* = &plugin_info;
    if (!version_ok(ifvers)) return 0;
    return 1;
}

export fn Meta_Attach(now: PLUG_LOADTIME, fTable: ?*metamod_funcs_t, pGlobals: ?*meta_globals_t, _: ?*gamedll_funcs_t) callconv(.c) c_int {
    if (@intFromEnum(now) > @intFromEnum(plugin_info.loadable)) return 0;
    gp_globals = pGlobals;

    // hand metamod our function table so it can obtain our hooks
    var mine = metamod_funcs_t{
        .pfnGetEntityAPI = get_entity_api,
        .pfnGetEntityAPI_Post = null,
        .pfnGetEntityAPI2 = null,
        .pfnGetEntityAPI2_Post = null,
        .pfnGetNewDLLFunctions = null,
        .pfnGetNewDLLFunctions_Post = null,
        .pfnGetEngineFunctions = get_engine_functions,
        .pfnGetEngineFunctions_Post = null,
    };
    if (fTable) |ft| ft.* = mine;

    // THE key line: metamod hands plugins the real engine table via
    // pfnGetHookTables — this is where `engfuncs` comes to life in Zig.
    var eng_ptr: ?*enginefuncs_t = null;
    var dll_ptr: ?*gamefuncs_t = null;
    var newdll_ptr: ?*newgamefuncs_t = null;
    if (gp_mutil) |m| {
        _ = m.pfnGetHookTables(&plugin_info, &eng_ptr, &dll_ptr, &newdll_ptr);
    }
    engfuncs = eng_ptr;

    if (engfuncs) |e| e.pfnServerPrint("csguess attached!\n");
    return 1;
}

export fn Meta_Detach(_: PLUG_LOADTIME, _: PL_UNLOAD_REASON) callconv(.c) c_int {
    return 1;
}
