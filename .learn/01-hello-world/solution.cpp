// Solution for Lesson 01: Hello World
//
// This code goes in src/linkage.cpp inside the GiveFnptrsToDll function.
// Insert AFTER the memcpy line that initializes engfuncs.
//
// Location: src/linkage.cpp, around line 1085-1086

// === CONTEXT (existing code) ===
// DLL_GIVEFNPTRSTODLL GiveFnptrsToDll (enginefuncs_t *table, globalvars_t *glob) {
//    memcpy (&engfuncs, table, sizeof (enginefuncs_t));
//    globals = glob;

// === YOUR CODE (insert here) ===
engfuncs.pfnServerPrint("*** HELLO FROM MY YAPB BUILD! ***\n");

// === CONTEXT (existing code continues) ===
//    if (game.postload ()) {
//       return;
//    }
// ...

// ============================================================
// EXPLANATION:
// ============================================================
//
// engfuncs.pfnServerPrint() sends text to the server console.
//
// Breaking down the call:
//   engfuncs           - Global struct holding engine function pointers
//   .pfnServerPrint    - Pointer to the "server print" function
//   ("...\n")          - C string argument with newline
//
// The \n (newline) is important - without it, subsequent output
// may appear on the same line or the text may not flush properly.
//
// ============================================================
// TYPESCRIPT EQUIVALENT:
// ============================================================
//
// In Node.js, this would be similar to:
//
// console.log("*** HELLO FROM MY YAPB BUILD! ***");
//
// But console.log goes to stdout. In the game engine context,
// we must use the engine's print function to output to the
// game console that players see.
