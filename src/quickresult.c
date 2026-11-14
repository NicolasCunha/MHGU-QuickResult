// Quick Result: skips the "Quest Complete" screen at the end of a quest and goes straight to the rewards.
//
// How the game ends a quest (MHGU 1.4.0, found with a logging build):
//   quest state (byte +0x50 of the object behind GOT 0x1838D28)
//     3 = quest cleared, "return to village" countdown running (SkipEndQuestTime zeroes this one on L+R+Right)
//     4 = the "Quest Complete" screen: the game restarts the same countdown at 899 and moves on when it reaches 0
//   the countdown: u16 at +0xBC of the object at +0x3830 of the object at main+0x2126498
//   then loading, then the result screen (uUIResult), whose first state already saves the clear and shows the rewards.
// This plugin only zeroes the countdown in state 4 (the game's own way out of it), so nothing is skipped that the game
// does in other states. Optional: also zero it in state 3 (no time to carve or gather after the clear).
#include "forge.h"

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION "dev"
#endif

#define QUEST_GLOBAL 0x2126498  // object pointer (+0x3830 -> +0xBC: the end-of-quest countdown)
#define GOT_SQUEST   0x1838D28  // the quest object (byte +0x50: its state)
#define Q_STATE      0x50
#define ST_RETURN    3
#define ST_COMPLETE  4

static u32 g_main;
static bool g_skip_complete = true;   // skip the Quest Complete screen
static bool g_skip_return = false;    // also skip the return countdown right after the clear
static u32 g_skipped_complete, g_skipped_return;

static bool ptr_ok(u32 p) { return p - 0x10000u <= 0xEFFEFFFFu && !(p & 3); }

static u32 quest_state(void) {
    u32 slot = *(u32 *)(g_main + GOT_SQUEST);
    if (!ptr_ok(slot)) return 0xFF;
    u32 q = *(u32 *)slot;
    return ptr_ok(q) ? *(u8 *)(q + Q_STATE) : 0xFF;
}

static u16 *countdown(void) {
    u32 g = *(u32 *)(g_main + QUEST_GLOBAL);
    if (!ptr_ok(g)) return NULL;
    u32 o = *(u32 *)(g + 0x3830);
    return ptr_ok(o) ? (u16 *)(o + 0xBC) : NULL;
}

EXPORT void forge_onLoad(ForgePluginInfo *info) {
    static const char name[] = "Quick Result " PLUGIN_VERSION;
    if (!info) return;
    info->major = 1, info->minor = 5, info->patch = 1;
    for (u32 i = 0; i < sizeof info->name; i++) info->name[i] = i < sizeof name ? name[i] : 0;
}

EXPORT void forge_onInit(void) {
    g_main = forge_mem_getMainTextAddr();
    forge_log(LOG_INFO, "[QuickResult] " PLUGIN_VERSION " ready: the Quest Complete screen is skipped (toggle in the Forge menu, Insert)");
}

EXPORT void forge_onUpdate(float dt) {
    (void)dt;
    if (!g_main) return;
    u32 st = quest_state();
    if (st != ST_COMPLETE && st != ST_RETURN) return;
    if ((st == ST_COMPLETE && !g_skip_complete) || (st == ST_RETURN && !g_skip_return)) return;
    u16 *cd = countdown();
    // a real countdown is at most 1200 (return) / 899 (Quest Complete); 0xFFxx are the game's own loading markers
    if (!cd || *cd == 0 || *cd > 1200) return;
    forge_log(LOG_INFO, "[QuickResult] %s skipped (%u ticks left)",
              st == ST_COMPLETE ? "Quest Complete screen" : "return countdown", *cd);
    *cd = 0;
    if (st == ST_COMPLETE) g_skipped_complete++; else g_skipped_return++;
}

// ---------------------------------------------------------------- Forge menu

EXPORT void forge_onImGuiInit(void *ctx, void *alloc, void *freefn) {
    if (!ctx || !alloc || !freefn) return;
    igSetCurrentContext(ctx);
    igSetAllocatorFunctions(alloc, freefn, NULL);
}

static char *put(char *q, const char *s) { while (*s) *q++ = *s++; return q; }
static char *put_u(char *q, u32 v) {
    static const u32 tens[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
    bool started = false;
    for (u32 i = 0; i < sizeof tens / sizeof *tens; i++) {
        char d = '0';
        while (v >= tens[i]) { v -= tens[i]; d++; }
        if (d != '0' || started || tens[i] == 1) { *q++ = d; started = true; }
    }
    return q;
}

EXPORT void forge_onImGuiRender(void) {
    if (!igTreeNode_Str("Quick Result###quickresult")) return;
    igCheckbox("Skip the Quest Complete screen", &g_skip_complete);
    igCheckbox("Also skip the return countdown after the clear", &g_skip_return);
    igTextUnformatted("(no time to carve or gather after the clear with this one)", NULL);
    static char line[96];
    char *q = put(line, "This session: ");
    q = put_u(q, g_skipped_complete);
    q = put(q, " Quest Complete screens, ");
    q = put_u(q, g_skipped_return);
    q = put(q, " countdowns skipped");
    *q = 0;
    igTextUnformatted(line, NULL);
    igTreePop();
}
