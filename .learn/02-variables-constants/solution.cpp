// Solution for Lesson 02: Variables & Constants
//
// This lesson was primarily about understanding existing code,
// not writing new code. Here's a summary of what you found:

// ============================================================
// DECLARATIONS (inc/yapb.h:953-954)
// ============================================================

// These tell the compiler "these variables exist somewhere":
extern ConVar cv_camping_time_min;
extern ConVar cv_camping_time_max;

// 'extern' means:
// - The variable is DECLARED here (we know its name and type)
// - The variable is DEFINED elsewhere (actual storage)
// - Similar to TypeScript: declare const x: ConVar;

// ============================================================
// DEFINITIONS (src/tasks.cpp, near top of file)
// ============================================================

// These create the actual variables with default values:
ConVar cv_camping_time_min ("camping_time_min", "15.0",
    "Lower bound of camping time.");

ConVar cv_camping_time_max ("camping_time_max", "45.0",
    "Upper bound of camping time.");

// ConVar constructor parameters:
// 1. "camping_time_min"  - Name used in console (yb camping_time_min)
// 2. "15.0"              - Default value as string
// 3. "Lower bound..."    - Description for help text

// ============================================================
// USAGE (src/tasks.cpp, in camp_ function)
// ============================================================

// Getting the value:
float minTime = cv_camping_time_min.as<float>();
float maxTime = cv_camping_time_max.as<float>();

// Using in camping logic:
float campTime = rg(minTime, maxTime);  // Random between min and max
m_timeCamping = game.time() + campTime;

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
TypeScript version might look like:

// declaration (in types.d.ts)
declare const cv_camping_time_min: ConVar;
declare const cv_camping_time_max: ConVar;

// definition (in tasks.ts)
const cv_camping_time_min = new ConVar(
  "camping_time_min",
  "15.0",
  "Lower bound of camping time."
);

// usage
const campTime = randomFloat(
  cv_camping_time_min.asFloat(),
  cv_camping_time_max.asFloat()
);
*/

// ============================================================
// KEY LEARNINGS
// ============================================================

// 1. constexpr vs const:
constexpr int COMPILE_TIME = 100;  // Evaluated at compile time
const int RUNTIME = getConfig();   // Set once at runtime

// 2. Sized integers for explicit sizes:
int32_t  explicitSize = 42;        // Always 32 bits
int      implicitSize = 42;        // "Usually" 32 bits (platform dependent)

// 3. Float literals need 'f' suffix:
float good = 3.14f;   // Correct: float literal
float warn = 3.14;    // Warning: double literal, implicit conversion

// 4. ConVar pattern separates declaration and runtime configuration:
// - Declare with extern in header
// - Define with defaults in source
// - User can change via console at runtime
