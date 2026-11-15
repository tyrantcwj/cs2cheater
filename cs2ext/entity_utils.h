#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include "offsets.h"

namespace EntityList {
    constexpr size_t ENTRY_STRIDE = 112;     // 0x70
    constexpr size_t PAGE_HEADER = 16;       // 0x10
    constexpr size_t PAGE_PTR_SIZE = 8;
    constexpr uint32_t HANDLE_MASK = 0x7FFF;
    constexpr uint32_t PAGE_SHIFT = 9;
    constexpr uint32_t INDEX_MASK = 0x1FF;
    constexpr int MAX_PLAYERS = 64;

    inline uintptr_t get_page(uintptr_t list, uint32_t handle) {
        return g_memory->read<uintptr_t>(
            list + PAGE_PTR_SIZE * ((handle & HANDLE_MASK) >> PAGE_SHIFT) + PAGE_HEADER);
    }

    inline uintptr_t get_entry(uintptr_t page, uint32_t handle) {
        return g_memory->read<uintptr_t>(page + ENTRY_STRIDE * (handle & INDEX_MASK));
    }

    inline uintptr_t resolve_handle(uintptr_t list, uint32_t handle) {
        if (!handle) return 0;
        uintptr_t page = get_page(list, handle);
        if (!page) return 0;
        return get_entry(page, handle);
    }
}

inline bool entity_count_sane(int n) {
    return n > 0 && n < 16384;
}

// dwEntityList is sometimes the pointer slot, sometimes one extra pointer,
// and sometimes the object itself. Keep the candidate whose entity count is sane.
inline uintptr_t resolve_entity_system(int* out_highest) {
    const uint32_t hi_off = g_offsets.client.highestEntityIndex
        ? g_offsets.client.highestEntityIndex : 0x2120;
    const uintptr_t slot = g_memory->get_client_base() + g_offsets.client.dwEntityList;

    auto accept = [&](uintptr_t obj) -> bool {
        if (obj < 0x10000) return false;
        const int hi = g_memory->read<int>(obj + hi_off);
        if (!entity_count_sane(hi)) return false;
        const uintptr_t page = g_memory->read<uintptr_t>(obj + EntityList::PAGE_HEADER);
        if (!page) return false;
        if (out_highest) *out_highest = hi;
        return true;
    };

    const uintptr_t pointed = g_memory->read<uintptr_t>(slot);
    if (accept(pointed)) return pointed;
    if (pointed) {
        const uintptr_t inner = g_memory->read<uintptr_t>(pointed);
        if (accept(inner)) return inner;
    }
    if (accept(slot)) return slot;

    if (out_highest)
        *out_highest = pointed ? g_memory->read<int>(pointed + hi_off) : -1;
    return pointed;
}

inline bool pawn_handle_ok(uint32_t h) {
    return h != 0 && h != 0xFFFFFFFF;
}

inline uint32_t get_pawn_handle(uintptr_t controller) {
    uint32_t player = g_memory->read<uint32_t>(
        controller + g_offsets.CCSPlayerController.m_hPlayerPawn);
    if (pawn_handle_ok(player))
        return player;
    uint32_t pawn = g_memory->read<uint32_t>(
        controller + g_offsets.CCSPlayerController.m_hPawn);
    return pawn_handle_ok(pawn) ? pawn : 0;
}

inline int utf8_next(const char*& p) {
    auto b0 = (unsigned char)*p;
    if (!b0) return 0;
    if (b0 < 0x80) { p++; return b0; }

    // Truncated names may end mid-sequence; never read past the terminator.
    auto b1 = (unsigned char)p[1];
    if (!b1) { p++; return 0xFFFD; }
    if ((b0 & 0xE0) == 0xC0) {
        p += 2;
        return ((b0 & 0x1F) << 6) | (b1 & 0x3F);
    }

    auto b2 = (unsigned char)p[2];
    if (!b2) { p++; return 0xFFFD; }
    if ((b0 & 0xF0) == 0xE0) {
        p += 3;
        return ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    }

    auto b3 = (unsigned char)p[3];
    if (!b3) { p++; return 0xFFFD; }
    if ((b0 & 0xF8) == 0xF0) {
        p += 4;
        return ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
    }

    p++;
    return 0xFFFD;
}

inline size_t utf8_put(char* dst, size_t cap, size_t i, unsigned cp) {
    if (cp < 0x80) {
        if (i + 1 >= cap) return i;
        dst[i++] = (char)cp;
    } else if (cp < 0x800) {
        if (i + 2 >= cap) return i;
        dst[i++] = (char)(0xC0 | (cp >> 6));
        dst[i++] = (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        if (i + 3 >= cap) return i;
        dst[i++] = (char)(0xE0 | (cp >> 12));
        dst[i++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        dst[i++] = (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x110000) {
        if (i + 4 >= cap) return i;
        dst[i++] = (char)(0xF0 | (cp >> 18));
        dst[i++] = (char)(0x80 | ((cp >> 12) & 0x3F));
        dst[i++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        dst[i++] = (char)(0x80 | (cp & 0x3F));
    }
    return i;
}

inline bool name_cp_keep(unsigned c) {
    if (c >= 0x20 && c <= 0x7E) return true;
    if (c >= 0xA0 && c <= 0x024F) return true;      // Latin-1 / Extended-A
    if (c >= 0x0370 && c <= 0x03FF) return true;    // Greek
    if (c >= 0x0400 && c <= 0x052F) return true;    // Cyrillic
    if (c >= 0x0600 && c <= 0x06FF) return true;    // Arabic
    if (c >= 0x0E00 && c <= 0x0E7F) return true;    // Thai
    if (c >= 0x1100 && c <= 0x11FF) return true;    // Hangul Jamo
    if (c >= 0x3040 && c <= 0x30FF) return true;    // Kana
    if (c >= 0x3130 && c <= 0x318F) return true;    // Hangul compat
    if (c >= 0x4E00 && c <= 0x9FFF) return true;    // CJK
    if (c >= 0xAC00 && c <= 0xD7A3) return true;    // Hangul
    if (c >= 0x3000 && c <= 0x303F) return true;    // CJK punctuation
    return false;
}

inline void emit_normalized_cp(char* dst, size_t cap, size_t& i, unsigned c) {
    auto emit_ch = [&](char ch) {
        if (i + 1 < cap) dst[i++] = ch;
    };
    auto emit_str = [&](const char* s) {
        while (*s && i + 1 < cap) dst[i++] = *s++;
    };

    if (c < 0x20) { emit_ch(' '); return; }
    if (c == 0x7F || c == 0xFFFD) return;
    if (c >= 0xFE00 && c <= 0xFE0F) return;         // variation selectors
    if (c >= 0x200B && c <= 0x200F) return;         // zwsp / bidi
    if (c >= 0xFEFF && c <= 0xFEFF) return;

    if (c >= 0xFF01 && c <= 0xFF5E) { emit_ch((char)(c - 0xFEE0)); return; }
    if (c == 0x3000) { emit_ch(' '); return; }

    if (c >= 0x1D400 && c <= 0x1D7FF) {
        static constexpr unsigned kLetter[] = {
            0x1D400, 0x1D434, 0x1D468, 0x1D49C, 0x1D4D0,
            0x1D504, 0x1D538, 0x1D56C, 0x1D5A0, 0x1D5D4,
            0x1D608, 0x1D63C, 0x1D670
        };
        for (unsigned s : kLetter) {
            if (c >= s && c < s + 52) {
                unsigned idx = c - s;
                emit_ch((char)((idx < 26) ? ('A' + idx) : ('a' + (idx - 26))));
                return;
            }
        }
        if (c >= 0x1D7CE && c <= 0x1D7D7) { emit_ch((char)('0' + (c - 0x1D7CE))); return; }
        if (c >= 0x1D7D8 && c <= 0x1D7E1) { emit_ch((char)('0' + (c - 0x1D7D8))); return; }
        if (c >= 0x1D7E2 && c <= 0x1D7EB) { emit_ch((char)('0' + (c - 0x1D7E2))); return; }
        if (c >= 0x1D7EC && c <= 0x1D7F5) { emit_ch((char)('0' + (c - 0x1D7EC))); return; }
        if (c >= 0x1D7F6 && c <= 0x1D7FF) { emit_ch((char)('0' + (c - 0x1D7F6))); return; }
    }

    if (c >= 0x24B6 && c <= 0x24CF) { emit_ch((char)('A' + (c - 0x24B6))); return; }
    if (c >= 0x24D0 && c <= 0x24E9) { emit_ch((char)('a' + (c - 0x24D0))); return; }
    if (c >= 0x249C && c <= 0x24B5) { emit_ch((char)('a' + (c - 0x249C))); return; }
    if (c >= 0x1F130 && c <= 0x1F149) { emit_ch((char)('A' + (c - 0x1F130))); return; }
    if (c >= 0x1F150 && c <= 0x1F169) { emit_ch((char)('A' + (c - 0x1F150))); return; }
    if (c >= 0x1F170 && c <= 0x1F189) { emit_ch((char)('A' + (c - 0x1F170))); return; }
    if (c >= 0x1F1E6 && c <= 0x1F1FF) { emit_ch((char)('A' + (c - 0x1F1E6))); return; }

    switch (c) {
    case 0x2122: emit_str("TM"); return;
    case 0x00AE: emit_str("(R)"); return;
    case 0x00A9: emit_str("(C)"); return;
    case 0x00AB: case 0x2039: case 0x2329: case 0x27E8: case 0x27EA:
    case 0x276E: case 0x3008: case 0x300A: case 0xA9C1: case 0x25C0:
    case 0x25C4: case 0x2B05: case 0x2190: case 0x21D0: case 0x1438:
    case 0x2282: case 0x27E6: case 0xFF1C:
        emit_ch('<'); return;
    case 0x00BB: case 0x203A: case 0x232A: case 0x27E9: case 0x27EB:
    case 0x276F: case 0x3009: case 0x300B: case 0xA9C2: case 0x25B6:
    case 0x25BA: case 0x27A4: case 0x2192: case 0x21D2: case 0x1433:
    case 0x2283: case 0x27E7: case 0xFF1E:
        emit_ch('>'); return;
    default:
        break;
    }

    if (name_cp_keep(c)) {
        i = utf8_put(dst, cap, i, c);
        return;
    }
    // Decorative / emoji leftovers: drop so they don't become tofu.
}

inline void decode_html_entities(char* s) {
    if (!s || !s[0]) return;
    char buf[256];
    size_t w = 0;
    const size_t cap = sizeof(buf);
    for (const char* r = s; *r && w + 4 < cap; ) {
        if (*r != '&') {
            int cp = utf8_next(r);
            if (cp <= 0) break;
            w = utf8_put(buf, cap, w, (unsigned)cp);
            continue;
        }
        auto named = [&](const char* ent, unsigned repl) -> bool {
            size_t n = std::strlen(ent);
            if (std::strncmp(r, ent, n) != 0) return false;
            w = utf8_put(buf, cap, w, repl);
            r += n;
            return true;
        };
        if (named("&amp;", '&') || named("&lt;", '<') || named("&gt;", '>') ||
            named("&quot;", '"') || named("&apos;", '\'') || named("&#39;", '\'') ||
            named("&#34;", '"') || named("&nbsp;", ' '))
            continue;

        if (r[1] == '#') {
            const char* p = r + 2;
            int base = 10;
            if (*p == 'x' || *p == 'X') { base = 16; ++p; }
            char* end = nullptr;
            unsigned long cp = std::strtoul(p, &end, base);
            if (end != p && *end == ';' && cp > 0 && cp < 0x110000) {
                w = utf8_put(buf, cap, w, (unsigned)cp);
                r = end + 1;
                continue;
            }
        }
        w = utf8_put(buf, cap, w, (unsigned char)*r++);
    }
    buf[w] = 0;
    std::strncpy(s, buf, 127);
    s[127] = 0;
}

inline void normalize_player_name(char* s) {
    if (!s || !s[0]) return;
    decode_html_entities(s);
    char buf[256];
    size_t w = 0;
    const char* p = s;
    while (*p && w + 4 < sizeof(buf)) {
        int cp = utf8_next(p);
        if (cp <= 0) break;
        emit_normalized_cp(buf, sizeof(buf), w, (unsigned)cp);
    }
    buf[w] = 0;
    std::strncpy(s, buf, 127);
    s[127] = 0;
}

inline bool name_is_placeholder(const char* s) {
    if (!s || !s[0]) return true;
    int vis = 0, q = 0;
    for (const char* p = s; *p; ) {
        int cp = utf8_next(p);
        if (cp <= 0) break;
        if (cp == ' ') continue;
        vis++;
        if (cp == '?') q++;
    }
    return vis == 0 || q == vis;
}

inline void read_player_name(uintptr_t controller, char* out, size_t max_len) {
    out[0] = 0;
    char raw[128]{};
    if (g_offsets.CCSPlayerController.m_iszPlayerName) {
        g_memory->read_raw(controller + g_offsets.CCSPlayerController.m_iszPlayerName,
                           raw, sizeof(raw) - 1);
        raw[sizeof(raw) - 1] = 0;
        normalize_player_name(raw);
    }

    char sanitized[128]{};
    uintptr_t ptr = g_memory->read<uintptr_t>(
        controller + g_offsets.CCSPlayerController.m_sSanitizedPlayerName);
    if (ptr) {
        g_memory->read_raw(ptr, sanitized, sizeof(sanitized) - 1);
        sanitized[sizeof(sanitized) - 1] = 0;
        normalize_player_name(sanitized);
    }

    const char* src = raw;
    if (name_is_placeholder(raw) && !name_is_placeholder(sanitized))
        src = sanitized;
    std::strncpy(out, src, max_len - 1);
    out[max_len - 1] = 0;
}