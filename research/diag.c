// Quick Result - research build (sh build.sh diag): logs the quest-end flow so the "Quest Complete" phase can be located.
// Writes lines starting with "[QRdiag]" to log.txt. Changes nothing in the game.
#include "forge.h"

#define RESULT_UPDATE 0x384848   // uUIResult: per-frame update (ui); state at +0x3C, previous +0x40, substate +0x44
#define RESULT_NEXT   0x381A78   // uUIResult: next state from the one just finished (ui + 0x88, prev) -> state
#define QUEST_GLOBAL  0x2126498  // object pointer SkipEndQuestTime uses (+0x3830 -> +0xBC: end-of-quest countdown)
#define GOT_SQUEST    0x1838D28  // read by uUIResult (byte +0x50 compared with 6)

typedef u32 (*Fn1)(u32);
typedef u32 (*Fn2)(u32, u32);

static u32 g_main;
static u32 g_frame;
static ForgeHook g_update_hook, g_next_hook;
static u32 g_last_ui, g_last_state = 0xFFFFFFFF, g_last_sub = 0xFFFFFFFF, g_last_mode = 0xFFFFFFFF;
static u32 g_last_q50 = 0xFFFFFFFF, g_last_cd = 0xFFFFFFFF, g_last_ui_frame;

static bool ptr_ok(u32 p) { return p - 0x10000u <= 0xEFFEFFFFu && !(p & 3); }

static u32 update_hook(u32 ui) {
    u32 st = *(u32 *)(ui + 0x3C), sub = *(u32 *)(ui + 0x44), mode = *(u32 *)(ui + 0x4C);
    if (ui != g_last_ui) forge_log(LOG_INFO, "[QRdiag] f%u uUIResult new object %08x", g_frame, ui);
    if (ui != g_last_ui || st != g_last_state || sub != g_last_sub || mode != g_last_mode)
        forge_log(LOG_INFO, "[QRdiag] f%u result state %u sub %u mode %u prev %u flag38 %u", g_frame, st, sub, mode,
                  *(u32 *)(ui + 0x40), *(u32 *)(ui + 0x38));
    g_last_ui = ui, g_last_state = st, g_last_sub = sub, g_last_mode = mode, g_last_ui_frame = g_frame;
    return ((Fn1)g_update_hook.original)(ui);
}

static u32 next_hook(u32 seq, u32 prev) {
    u32 r = ((Fn2)g_next_hook.original)(seq, prev);
    u8 *b = (u8 *)seq;
    forge_log(LOG_INFO, "[QRdiag] f%u next state: prev %u -> %u (seq bytes %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x)",
              g_frame, prev, r, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9]);
    return r;
}

EXPORT void forge_onLoad(ForgePluginInfo *info) {
    static const char name[] = "Quick Result diag";
    if (!info) return;
    info->major = 1, info->minor = 5, info->patch = 1;
    for (u32 i = 0; i < sizeof info->name; i++) info->name[i] = i < sizeof name ? name[i] : 0;
}

EXPORT void forge_onInit(void) {
    g_main = forge_mem_getMainTextAddr();
    u32 orig = 0;
    forge_hook_create(&g_update_hook, g_main + RESULT_UPDATE, (u32)update_hook, &orig);
    orig = 0;
    forge_hook_create(&g_next_hook, g_main + RESULT_NEXT, (u32)next_hook, &orig);
    forge_log(LOG_INFO, "[QRdiag] ready: main %08x, hooks update %s next %s", g_main,
              g_update_hook.original ? "ok" : "FAILED", g_next_hook.original ? "ok" : "FAILED");
}

EXPORT void forge_onUpdate(float dt) {
    (void)dt;
    g_frame++;
    if (!g_main) return;
    // the quest object's state byte
    u32 slot = *(u32 *)(g_main + GOT_SQUEST);
    if (ptr_ok(slot)) {
        u32 q = *(u32 *)slot;
        if (ptr_ok(q)) {
            u32 v = *(u8 *)(q + 0x50);
            if (v != g_last_q50) forge_log(LOG_INFO, "[QRdiag] f%u sQuest +0x50: %u -> %u", g_frame, g_last_q50, v);
            g_last_q50 = v;
        }
    }
    // the end-of-quest countdown (logged when it starts, reaches 0, or disappears)
    u32 g = *(u32 *)(g_main + QUEST_GLOBAL), cd = 0xFFFFFFFF;
    if (ptr_ok(g)) {
        u32 o = *(u32 *)(g + 0x3830);
        if (ptr_ok(o)) cd = *(u16 *)(o + 0xBC);
    }
    if ((cd == 0) != (g_last_cd == 0) || (cd == 0xFFFFFFFF) != (g_last_cd == 0xFFFFFFFF) ||
        (g_last_cd != 0xFFFFFFFF && cd > g_last_cd + 1))
        forge_log(LOG_INFO, "[QRdiag] f%u countdown %u -> %u", g_frame, g_last_cd, cd);
    g_last_cd = cd;
    // the result screen going away
    if (g_last_ui && g_frame - g_last_ui_frame == 2) {
        forge_log(LOG_INFO, "[QRdiag] f%u uUIResult stopped updating (last state %u)", g_frame, g_last_state);
        g_last_ui = 0, g_last_state = g_last_sub = g_last_mode = 0xFFFFFFFF;
    }
}
