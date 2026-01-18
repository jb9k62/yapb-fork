// Solution for Lesson 11: Entity System
//
// This exercise creates a player listing function.

// ============================================================
// IMPLEMENTATION in src/botlib.cpp (or utility file)
// ============================================================

void listPlayers() {
    ctrl.msg("=== Player List ===");

    int playerIndex = 0;
    int botCount = 0;
    int humanCount = 0;
    int aliveCount = 0;

    for (const auto &client : util.getClients()) {
        // Skip unused slots
        if (!(client.flags & ClientFlags::Used)) {
            continue;
        }

        edict_t* ent = client.ent;
        if (game.isNullEntity(ent)) {
            continue;
        }

        // Gather information
        const char* name = ent->v.netname.chars();
        bool isAlive = (client.flags & ClientFlags::Alive) != 0;
        bool isBot = bots[ent] != nullptr;

        // Team name
        const char* team = "?";
        if (client.team == Team::Terrorist) {
            team = "T";
        } else if (client.team == Team::CT) {
            team = "CT";
        } else if (client.team == Team::Spectator) {
            team = "SPEC";
        }

        // Status
        const char* status = isAlive ? "ALIVE" : "DEAD";
        const char* type = isBot ? "BOT" : "HUMAN";

        // Print
        ctrl.msg("[%2d] %-4s %-5s %-5s | HP: %3.0f | %s",
                 playerIndex + 1,
                 team,
                 type,
                 status,
                 ent->v.health,
                 name);

        // Count stats
        playerIndex++;
        if (isBot) botCount++; else humanCount++;
        if (isAlive) aliveCount++;
    }

    ctrl.msg("---");
    ctrl.msg("Total: %d players (%d bots, %d humans), %d alive",
             playerIndex, botCount, humanCount, aliveCount);
}

// ============================================================
// EXTENDED VERSION: More detailed player info
// ============================================================

void listPlayersDetailed() {
    ctrl.msg("=== Detailed Player List ===");

    for (const auto &client : util.getClients()) {
        if (!(client.flags & ClientFlags::Used)) continue;

        edict_t* ent = client.ent;
        if (game.isNullEntity(ent)) continue;

        const entvars_t& ev = ent->v;

        ctrl.msg("Player: %s", ev.netname.chars());
        ctrl.msg("  Index: %d", game.indexOfEntity(ent));
        ctrl.msg("  Team: %d", client.team);
        ctrl.msg("  Health: %.0f / %.0f", ev.health, ev.max_health);
        ctrl.msg("  Armor: %.0f", ev.armorvalue);
        ctrl.msg("  Position: (%.0f, %.0f, %.0f)",
                 ev.origin.x, ev.origin.y, ev.origin.z);
        ctrl.msg("  Velocity: (%.0f, %.0f, %.0f)",
                 ev.velocity.x, ev.velocity.y, ev.velocity.z);

        // Check flags
        bool onGround = (ev.flags & FL_ONGROUND) != 0;
        bool ducking = (ev.flags & FL_DUCKING) != 0;
        bool isBot = (ev.flags & FL_FAKECLIENT) != 0;

        ctrl.msg("  OnGround: %s, Ducking: %s, IsBot: %s",
                 onGround ? "Yes" : "No",
                 ducking ? "Yes" : "No",
                 isBot ? "Yes" : "No");

        // Weapon info
        ctrl.msg("  Weapons bitmask: 0x%X", ev.weapons);

        ctrl.msg("");  // Blank line between players
    }
}

// ============================================================
// FIND ENTITIES BY CLASS
// ============================================================

void listEntitiesByClass(const char* classname) {
    ctrl.msg("=== Entities: %s ===", classname);

    int count = 0;
    game.searchEntities("classname", classname, [&](edict_t* ent) {
        ctrl.msg("[%d] at (%.0f, %.0f, %.0f)",
                 count++,
                 ent->v.origin.x,
                 ent->v.origin.y,
                 ent->v.origin.z);
        return EntitySearchResult::Continue;
    });

    ctrl.msg("Found %d %s entities", count, classname);
}

// Usage:
// listEntitiesByClass("hostage_entity");
// listEntitiesByClass("func_door");
// listEntitiesByClass("weaponbox");

// ============================================================
// CONSOLE COMMAND in src/control.cpp
// ============================================================

if (isCommand("players") || isCommand("list")) {
    listPlayers();
    return true;
}

if (isCommand("playersdetail")) {
    listPlayersDetailed();
    return true;
}

if (isCommand("entities")) {
    const char* classname = arg1.empty() ? "player" : arg1.chars();
    listEntitiesByClass(classname);
    return true;
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
interface Client {
    ent: Entity | null;
    flags: number;
    team: Team;
    origin: Vector;
}

function listPlayers(): void {
    console.log("=== Player List ===");

    let index = 0;
    for (const client of util.getClients()) {
        if (!(client.flags & ClientFlags.Used)) continue;
        if (!client.ent) continue;

        const name = client.ent.netname;
        const isAlive = (client.flags & ClientFlags.Alive) !== 0;
        const isBot = bots.has(client.ent);
        const team = client.team === Team.Terrorist ? "T" : "CT";
        const status = isAlive ? "ALIVE" : "DEAD";
        const type = isBot ? "BOT" : "HUMAN";

        console.log(
            `[${index + 1}] ${team} ${type} ${status} | ` +
            `HP: ${client.ent.health} | ${name}`
        );

        index++;
    }

    console.log(`Total: ${index} players`);
}
*/

// ============================================================
// KEY CONCEPTS
// ============================================================

// 1. Entity iteration uses Client array, not raw edict_t array
//    - util.getClients() is pre-filtered and cached
//    - More efficient than scanning all entities

// 2. Check ClientFlags before accessing entity
//    - ClientFlags::Used = slot is occupied
//    - ClientFlags::Alive = player is alive

// 3. Access entity data via ent->v
//    - ent->v.netname = player name
//    - ent->v.health = current health
//    - ent->v.origin = position

// 4. Bot check: bots[ent] returns Bot* or nullptr
//    - If non-null, entity is a YaPB bot
//    - If null, entity is human or other bot
