// enginefuncs.zig
//
// A hand transcription of the GoldSrc `enginefuncs_t` function-pointer table
// (ext/linkage/linkage/goldsrc.h, struct at line 1047) into a Zig `extern struct`.
//
// WHY THIS EXISTS
//   YaPB's headers are C++, so `@cImport` cannot parse them. In Zig the correct,
//   reliable way to talk to a C function table is to reproduce its layout as an
//   `extern struct`. Since every member is a pointer, the whole table is
//   "an array of pointers," so layout is trivial and offsets are exact.
//
// THE PREFIX TRICK
//   An `extern struct` need not list every member. We only need the *leading*
//   members up to the highest-offset function we actually call. Trailing members
//   are irrelevant to the offsets of the leading ones.
//
//   Functions we don't call are typed as a generic no-arg pointer:
//       *const fn () callconv(.c) void
//   All non-`void` pointers are still one machine word, so this changes nothing
//   about the layout — it just keeps the transcription readable. (See the note at
//   the bottom of the file about verifying offsets with `@offsetOf`.)
//
// TARGET
//   CS 1.6 on Linux, 32-bit (`-Dtarget=x86-linux-gnu`). On a 64-bit build the
//   offsets still match because every member is `*const fn` (pointer-sized).

const c_int = @import("std").c.c_int;

/// GoldSrc entity handle. We never dereference it for Steps 1-7, so opaque is fine.
/// (For `!whereami` in Step 8 we still don't need to read it — `pfnSetOrigin`
///  takes the entity plus a `*const f32` origin, so it stays opaque.)
pub const edict_t = opaque {};
const cvar_t = opaque {};
const TraceResult = extern struct {}; // unused here

pub const PRINT_TYPE = enum(c_int) {
    console = 0,
    center = 1,
    chat = 2,
};

pub const enginefuncs_t = extern struct {
    // ---- leading members (transcribed verbatim from goldsrc.h:1047+) ----
    pfnPrecacheModel: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnPrecacheSound: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnSetModel: *const fn (?*edict_t, [*:0]const u8) callconv(.c) void,
    pfnModelIndex: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnModelFrames: *const fn (c_int) callconv(.c) c_int,
    pfnSetSize: *const fn (?*edict_t, [*]const f32, [*]const f32) callconv(.c) void,
    pfnChangeLevel: *const fn ([*:0]u8, [*:0]u8) callconv(.c) void,
    pfnGetSpawnParms: *const fn (?*edict_t) callconv(.c) void,
    pfnSaveSpawnParms: *const fn (?*edict_t) callconv(.c) void,
    pfnVecToYaw: *const fn ([*]const f32) callconv(.c) f32,
    pfnVecToAngles: *const fn ([*]const f32, [*]f32) callconv(.c) void,
    pfnMoveToOrigin: *const fn (?*edict_t, [*]const f32, f32, c_int) callconv(.c) void,
    pfnChangeYaw: *const fn (?*edict_t) callconv(.c) void,
    pfnChangePitch: *const fn (?*edict_t) callconv(.c) void,
    pfnFindEntityByString: *const fn (?*edict_t, [*:0]const u8, [*:0]const u8) callconv(.c) ?*edict_t,
    pfnGetEntityIllum: *const fn (?*edict_t) callconv(.c) c_int,
    pfnFindEntityInSphere: *const fn (?*edict_t, [*]const f32, f32) callconv(.c) ?*edict_t,
    pfnFindClientInPVS: *const fn (?*edict_t) callconv(.c) ?*edict_t,
    pfnEntitiesInPVS: *const fn (?*edict_t) callconv(.c) ?*edict_t,
    pfnMakeVectors: *const fn ([*]const f32) callconv(.c) void,
    pfnAngleVectors: *const fn ([*]const f32, [*]f32, [*]f32, [*]f32) callconv(.c) void,
    pfnCreateEntity: *const fn () callconv(.c) ?*edict_t,
    pfnRemoveEntity: *const fn (?*edict_t) callconv(.c) void,
    pfnCreateNamedEntity: *const fn (usize) callconv(.c) ?*edict_t, // string_t = index
    pfnMakeStatic: *const fn (?*edict_t) callconv(.c) void,
    pfnEntIsOnFloor: *const fn (?*edict_t) callconv(.c) c_int,
    pfnDropToFloor: *const fn (?*edict_t) callconv(.c) c_int,
    pfnWalkMove: *const fn (?*edict_t, f32, f32, c_int) callconv(.c) c_int,
    // *** pfnSetOrigin — THE teleport function (Step 8) ***
    pfnSetOrigin: *const fn (?*edict_t, [*]const f32) callconv(.c) void,
    pfnEmitSound: *const fn (?*edict_t, c_int, [*:0]const u8, f32, f32, c_int, c_int) callconv(.c) void,
    pfnEmitAmbientSound: *const fn (?*edict_t, [*]f32, [*:0]const u8, f32, f32, c_int, c_int) callconv(.c) void,
    pfnTraceLine: *const fn ([*]const f32, [*]const f32, c_int, ?*edict_t, *TraceResult) callconv(.c) void,
    pfnTraceToss: *const fn (?*edict_t, ?*edict_t, *TraceResult) callconv(.c) void,
    pfnTraceMonsterHull: *const fn (?*edict_t, [*]const f32, [*]const f32, c_int, ?*edict_t, *TraceResult) callconv(.c) c_int,
    pfnTraceHull: *const fn ([*]const f32, [*]const f32, c_int, c_int, ?*edict_t, *TraceResult) callconv(.c) void,
    pfnTraceModel: *const fn ([*]const f32, [*]const f32, c_int, ?*edict_t, *TraceResult) callconv(.c) void,
    pfnTraceTexture: *const fn (?*edict_t, [*]const f32, [*]const f32) callconv(.c) [*:0]const u8,
    pfnTraceSphere: *const fn ([*]const f32, [*]const f32, c_int, f32, ?*edict_t, *TraceResult) callconv(.c) void,
    pfnGetAimVector: *const fn (?*edict_t, f32, [*]f32) callconv(.c) void,
    pfnServerCommand: *const fn ([*:0]u8) callconv(.c) void,
    pfnServerExecute: *const fn () callconv(.c) void,
    pfnClientCommand: *const fn (?*edict_t, [*:0]const u8, ...) callconv(.c) void, // varargs
    pfnParticleEffect: *const fn ([*]const f32, [*]const f32, f32, c_int) callconv(.c) void,
    pfnLightStyle: *const fn (c_int, [*:0]u8) callconv(.c) void,
    pfnDecalIndex: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnPointContents: *const fn ([*]const f32) callconv(.c) c_int,
    pfnMessageBegin: *const fn (c_int, c_int, ?[*]const f32, ?*edict_t) callconv(.c) void,
    pfnMessageEnd: *const fn () callconv(.c) void,
    pfnWriteByte: *const fn (c_int) callconv(.c) void,
    pfnWriteChar: *const fn (c_int) callconv(.c) void,
    pfnWriteShort: *const fn (c_int) callconv(.c) void,
    pfnWriteLong: *const fn (c_int) callconv(.c) void,
    pfnWriteAngle: *const fn (f32) callconv(.c) void,
    pfnWriteCoord: *const fn (f32) callconv(.c) void,
    pfnWriteString: *const fn ([*:0]const u8) callconv(.c) void,
    pfnWriteEntity: *const fn (c_int) callconv(.c) void,
    pfnCVarRegister: *const fn (?*cvar_t) callconv(.c) void,
    pfnCVarGetFloat: *const fn ([*:0]const u8) callconv(.c) f32,
    pfnCVarGetString: *const fn ([*:0]const u8) callconv(.c) [*:0]const u8,
    pfnCVarSetFloat: *const fn ([*:0]const u8, f32) callconv(.c) void,
    pfnCVarSetString: *const fn ([*:0]const u8, [*:0]const u8) callconv(.c) void,
    pfnAlertMessage: *const fn (c_int, [*:0]const u8, ...) callconv(.c) void,
    pfnEngineFprintf: *const fn (?*anyopaque, [*:0]const u8, ...) callconv(.c) void,
    pfnPvAllocEntPrivateData: *const fn (?*edict_t, i32) callconv(.c) ?*anyopaque,
    pfnPvEntPrivateData: *const fn (?*edict_t) callconv(.c) ?*anyopaque,
    pfnFreeEntPrivateData: *const fn (?*edict_t) callconv(.c) void,
    pfnSzFromIndex: *const fn (usize) callconv(.c) [*:0]const u8,
    pfnAllocString: *const fn ([*:0]const u8) callconv(.c) usize, // string_t = index
    pfnGetVarsOfEnt: *const fn (?*edict_t) callconv(.c) ?*anyopaque, // -> entvars_s*
    pfnPEntityOfEntOffset: *const fn (c_int) callconv(.c) ?*edict_t,
    pfnEntOffsetOfPEntity: *const fn (const ?*edict_t) callconv(.c) c_int,
    pfnIndexOfEdict: *const fn (const ?*edict_t) callconv(.c) c_int,
    // *** pfnPEntityOfEntIndex — player entity by index (Steps 6-8) ***
    pfnPEntityOfEntIndex: *const fn (c_int) callconv(.c) ?*edict_t,
    pfnFindEntityByVars: *const fn (?*anyopaque) callconv(.c) ?*edict_t,
    pfnGetModelPtr: *const fn (?*edict_t) callconv(.c) ?*anyopaque,
    pfnRegUserMsg: *const fn ([*:0]const u8, c_int) callconv(.c) c_int,
    pfnAnimationAutomove: *const fn (?*edict_t, f32) callconv(.c) void,
    pfnGetBonePosition: *const fn (?*edict_t, c_int, [*]f32, [*]f32) callconv(.c) void,
    pfnFunctionFromName: *const fn ([*:0]const u8) callconv(.c) u32,
    pfnNameForFunction: *const fn (u32) callconv(.c) [*:0]const u8,
    // *** pfnClientPrintf — print to someone's screen/chat (Steps 6-7) ***
    pfnClientPrintf: *const fn (?*edict_t, PRINT_TYPE, [*:0]const u8) callconv(.c) void,
    // *** pfnServerPrint — print to the server console/boilerplate log ***
    pfnServerPrint: *const fn ([*:0]const u8) callconv(.c) void,
    // *** pfnCmd_Args / pfnCmd_Argv / pfnCmd_Argc — chat command args (Steps 6-7) ***
    pfnCmd_Args: *const fn () callconv(.c) [*:0]const u8,
    pfnCmd_Argv: *const fn (c_int) callconv(.c) [*:0]const u8,
    pfnCmd_Argc: *const fn () callconv(.c) c_int,
    pfnGetAttachment: *const fn (const ?*edict_t, c_int, [*]f32, [*]f32) callconv(.c) void,
    pfnCRC32_Init: *const fn (*u32) callconv(.c) void,
    pfnCRC32_ProcessBuffer: *const fn (*u32, ?*anyopaque, c_int) callconv(.c) void,
    pfnCRC32_ProcessByte: *const fn (*u32, u8) callconv(.c) void,
    pfnCRC32_Final: *const fn (u32) callconv(.c) u32,
    pfnRandomLong: *const fn (i32, i32) callconv(.c) i32,
    pfnRandomFloat: *const fn (f32, f32) callconv(.c) f32,
    pfnSetView: *const fn (const ?*edict_t, const ?*edict_t) callconv(.c) void,
    // *** pfnTime — server clock (for turn timing) ***
    pfnTime: *const fn () callconv(.c) f32,
    pfnCrosshairAngle: *const fn (const ?*edict_t, f32, f32) callconv(.c) void,
    pfnLoadFileForMe: *const fn ([*:0]const u8, ?*c_int) callconv(.c) [*]u8,
    pfnFreeFile: *const fn (?*anyopaque) callconv(.c) void,
    pfnEndSection: *const fn ([*:0]const u8) callconv(.c) void,
    pfnCompareFileTime: *const fn ([*:0]u8, [*:0]u8, ?*c_int) callconv(.c) c_int,
    pfnGetGameDir: *const fn ([*:0]u8) callconv(.c) void,
    pfnCvar_RegisterVariable: *const fn (?*cvar_t) callconv(.c) void,
    pfnFadeClientVolume: *const fn (const ?*edict_t, c_int, c_int, c_int, c_int) callconv(.c) void,
    pfnSetClientMaxspeed: *const fn (const ?*edict_t, f32) callconv(.c) void,
    pfnCreateFakeClient: *const fn ([*:0]const u8) callconv(.c) ?*edict_t,
    pfnRunPlayerMove: *const fn (?*edict_t, [*]const f32, f32, f32, f32, u16, u8, u8) callconv(.c) void,
    pfnNumberOfEntities: *const fn () callconv(.c) c_int,
    pfnGetInfoKeyBuffer: *const fn (?*edict_t) callconv(.c) [*:0]u8,
    pfnInfoKeyValue: *const fn ([*:0]u8, [*:0]const u8) callconv(.c) [*:0]const u8,
    pfnSetKeyValue: *const fn ([*:0]u8, [*:0]u8, [*:0]u8) callconv(.c) void,
    pfnSetClientKeyValue: *const fn (c_int, [*:0]u8, [*:0]const u8, [*:0]const u8) callconv(.c) void,
    pfnIsMapValid: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnStaticDecal: *const fn ([*]const f32, c_int, c_int, c_int) callconv(.c) void,
    pfnPrecacheGeneric: *const fn ([*:0]u8) callconv(.c) c_int,
    // *** pfnGetPlayerUserId — stable per-player id (optional State key) ***
    pfnGetPlayerUserId: *const fn (?*edict_t) callconv(.c) c_int,
};

// ---------------------------------------------------------------------------
// Sanity check: prove our offset for a field we rely on matches the C header.
// Do this at comptime — it costs nothing at runtime and catches a transcription
// slip the instant it's made, before you ever load the plugin into CS.
//
//   const Off = @offsetOf(enginefuncs_t, "pfnSetOrigin");
//   _ = Off; // O-1, if it compiles the offset is whatever the struct says
//
// You'll do this intentionally in Steps 4-5.
// ---------------------------------------------------------------------------
