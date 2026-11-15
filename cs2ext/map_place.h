#pragma once
#include <cstring>
#include <cmath>

inline void normalize_place_key(const char* raw, char* key, size_t max) {
    key[0] = 0;
    if (!raw) return;
    size_t j = 0;
    for (size_t i = 0; raw[i] && j + 1 < max; i++) {
        unsigned char c = (unsigned char)raw[i];
        if (c == ' ' || c == '_' || c == '-' || c == '.') continue;
        if (c >= 'A' && c <= 'Z') c = (unsigned char)(c + 32);
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
            key[j++] = (char)c;
    }
    key[j] = 0;
}

inline bool map_has(const char* map, const char* id) {
    return map && id && std::strstr(map, id);
}

// Finer Chinese callouts by world position. Smaller radius wins when overlapping.
struct PlaceSpot {
    const char* map;
    const char* zh;
    float x, y;
    float r;
};

static constexpr PlaceSpot kPlaceSpots[] = {
    // ---- de_dust2 (community callouts, 蓝车 / A小道 / A包点 split) ----
    {"de_dust2", "蓝车",       1788.f, 1812.f, 280.f},
    {"de_dust2", "A包点",      1098.f, 2554.f, 330.f},
    {"de_dust2", "A平台下",     838.f, 2254.f, 230.f},
    {"de_dust2", "A斜坡",      1460.f, 2696.f, 260.f},
    {"de_dust2", "A延伸",       462.f, 2386.f, 280.f},
    {"de_dust2", "A小道楼梯",    384.f, 1644.f, 200.f},
    {"de_dust2", "A小道",       -82.f, 1252.f, 340.f},
    {"de_dust2", "盒子",       -250.f, 1320.f, 160.f},
    {"de_dust2", "A大道",      1428.f, 1544.f, 360.f},
    {"de_dust2", "A大门",       986.f,  476.f, 300.f},
    {"de_dust2", "A大门外",     344.f,   88.f, 360.f},
    {"de_dust2", "深坑",       1360.f,  456.f, 260.f},
    {"de_dust2", "A大侧",      1684.f,  888.f, 230.f},
    {"de_dust2", "中路上",     -348.f,   76.f, 360.f},
    {"de_dust2", "中路",       -384.f, 1136.f, 360.f},
    {"de_dust2", "中门",       -506.f, 2082.f, 260.f},
    {"de_dust2", "CT出生",      140.f, 2276.f, 360.f},
    {"de_dust2", "B门",       -1090.f, 2486.f, 260.f},
    {"de_dust2", "B洞口",     -1316.f, 2660.f, 190.f},
    {"de_dust2", "B包点",     -1757.f, 2593.f, 420.f},
    {"de_dust2", "上洞",      -1503.f, 1210.f, 420.f},
    {"de_dust2", "洞楼梯",    -1144.f, 1117.f, 200.f},
    {"de_dust2", "下洞",       -872.f, 1408.f, 280.f},
    {"de_dust2", "洞外",      -1632.f,  288.f, 300.f},
    {"de_dust2", "T斜坡",     -2036.f, -584.f, 300.f},
    {"de_dust2", "T出生",      -976.f, -664.f, 520.f},

    // ---- de_mirage ----
    {"de_mirage", "A包点",     -662.f, -1612.f, 320.f},
    {"de_mirage", "B包点",    -2550.f,   162.f, 360.f},
    {"de_mirage", "皇宫内",    1325.f, -1362.f, 280.f},
    {"de_mirage", "皇宫巷",    1187.f,  -762.f, 280.f},
    {"de_mirage", "中路",      -162.f,  -887.f, 280.f},
    {"de_mirage", "中路上",     475.f,  -900.f, 280.f},
    {"de_mirage", "猫道",      -125.f,  -425.f, 240.f},
    {"de_mirage", "连接道",    -675.f, -1225.f, 240.f},
    {"de_mirage", "丛林",      -850.f, -1537.f, 260.f},
    {"de_mirage", "狙击位",   -1250.f, -1087.f, 240.f},
    {"de_mirage", "公寓",     -2012.f,   762.f, 320.f},
    {"de_mirage", "卡车",     -2300.f,   687.f, 220.f},
    {"de_mirage", "商店",     -2325.f,  -400.f, 240.f},
    {"de_mirage", "T出生",     1262.f,   462.f, 400.f},
    {"de_mirage", "CT出生",   -1512.f,  -787.f, 360.f},
    {"de_mirage", "地下室",    -475.f,   412.f, 240.f},
    {"de_mirage", "T斜坡",      512.f, -1625.f, 280.f},

    // ---- de_inferno ----
    {"de_inferno", "A包点",    2000.f,  544.f, 360.f},
    {"de_inferno", "B包点",     586.f, 3057.f, 400.f},
    {"de_inferno", "香蕉道",    208.f, 1568.f, 360.f},
    {"de_inferno", "二中",     834.f,   20.f, 400.f},
    {"de_inferno", "中路",     698.f,  662.f, 280.f},
    {"de_inferno", "CT出生",   1666.f, 1671.f, 360.f},
    {"de_inferno", "T出生",   -1252.f,  104.f, 500.f},
    {"de_inferno", "公寓",     850.f, -273.f, 360.f},
    {"de_inferno", "拱门",    1880.f, 1228.f, 260.f},
    {"de_inferno", "图书馆",  2480.f, 1396.f, 240.f},
    {"de_inferno", "深坑",    2360.f, -276.f, 220.f},

    // ---- de_nuke ----
    {"de_nuke", "A包点",       399.f, -1037.f, 400.f},
    {"de_nuke", "B包点",       807.f,  -347.f, 320.f},
    {"de_nuke", "大门",       650.f,  -172.f, 240.f},
    {"de_nuke", "木屋",       437.f,  -825.f, 200.f},
    {"de_nuke", "天堂",      1237.f,  -400.f, 220.f},
    {"de_nuke", "地狱",      1350.f,  -425.f, 200.f},
    {"de_nuke", "外场",       400.f, -1200.f, 600.f},
    {"de_nuke", "T出生",     -962.f, -1075.f, 400.f},
    {"de_nuke", "CT出生",    2087.f,  -575.f, 360.f},

    // ---- de_ancient ----
    {"de_ancient", "A包点",   -975.f, 1087.f, 320.f},
    {"de_ancient", "B包点",   1300.f,  725.f, 320.f},
    {"de_ancient", "中路",    -625.f, -775.f, 360.f},
    {"de_ancient", "T出生",   -662.f,-2412.f, 480.f},
    {"de_ancient", "CT出生",  -425.f, 1412.f, 320.f},
};

struct PlaceTok {
    const char* map; // null = all maps
    const char* k;
    const char* zh;
};

static constexpr PlaceTok kPlaceToks[] = {
    // Dust2 tokens — match CN callouts, not literal English
    {"de_dust2", "undera", "A平台下"},
    {"de_dust2", "extendeda", "A延伸"},
    {"de_dust2", "shortstairs", "A小道楼梯"},
    {"de_dust2", "catwalk", "A小道"},
    {"de_dust2", "cat", "A小道"},
    {"de_dust2", "short", "A小道"},
    {"de_dust2", "ashort", "A小道"},
    {"de_dust2", "longa", "A大道"},
    {"de_dust2", "along", "A大道"},
    {"de_dust2", "long", "A大道"},
    {"de_dust2", "longdoors", "A大门"},
    {"de_dust2", "longdoor", "A大门"},
    {"de_dust2", "outsidelong", "A大门外"},
    {"de_dust2", "outsidelonga", "A大门外"},
    {"de_dust2", "bombsitea", "A包点"},
    {"de_dust2", "asite", "A包点"},
    {"de_dust2", "bombsiteb", "B包点"},
    {"de_dust2", "bsite", "B包点"},
    {"de_dust2", "aramp", "A斜坡"},
    {"de_dust2", "pit", "深坑"},
    {"de_dust2", "side", "A大侧"},
    {"de_dust2", "xbox", "盒子"},
    {"de_dust2", "goose", "鹅"},
    {"de_dust2", "middoors", "中门"},
    {"de_dust2", "bdoors", "B门"},
    {"de_dust2", "bdoor", "B门"},
    {"de_dust2", "hole", "B洞口"},
    {"de_dust2", "uppertunnel", "上洞"},
    {"de_dust2", "uppertunnels", "上洞"},
    {"de_dust2", "lowertunnel", "下洞"},
    {"de_dust2", "lowertunnels", "下洞"},
    {"de_dust2", "outsidetunnel", "洞外"},
    {"de_dust2", "outsidetunnels", "洞外"},
    {"de_dust2", "tunnelstairs", "洞楼梯"},
    {"de_dust2", "topofmid", "中路上"},
    {"de_dust2", "middle", "中路"},
    {"de_dust2", "mid", "中路"},
    {"de_dust2", "ctspawn", "CT出生"},
    {"de_dust2", "tspawn", "T出生"},
    {"de_dust2", "tramp", "T斜坡"},
    {"de_dust2", "car", "蓝车"},
    {"de_dust2", "acar", "蓝车"},

    {"de_mirage", "catwalk", "中路猫道"},
    {"de_mirage", "palaceinterior", "皇宫内"},
    {"de_mirage", "palacealley", "皇宫巷"},
    {"de_mirage", "palace", "皇宫"},
    {"de_mirage", "apartments", "公寓"},
    {"de_mirage", "snipersnest", "狙击位"},
    {"de_mirage", "connector", "连接道"},
    {"de_mirage", "jungle", "丛林"},
    {"de_mirage", "underpass", "地下室"},
    {"de_mirage", "bombsitea", "A包点"},
    {"de_mirage", "bombsiteb", "B包点"},

    {"de_inferno", "banana", "香蕉道"},
    {"de_inferno", "secondmid", "二中"},
    {"de_inferno", "apartments", "公寓"},
    {"de_inferno", "bombsitea", "A包点"},
    {"de_inferno", "bombsiteb", "B包点"},
    {"de_inferno", "arch", "拱门"},
    {"de_inferno", "library", "图书馆"},

    {"de_nuke", "heaven", "天堂"},
    {"de_nuke", "hell", "地狱"},
    {"de_nuke", "hut", "木屋"},
    {"de_nuke", "ramp", "大门"},
    {"de_nuke", "bombsitea", "A包点"},
    {"de_nuke", "bombsiteb", "B包点"},
    {"de_nuke", "outside", "外场"},

    {nullptr, "undera", "A平台下"},
    {nullptr, "extendeda", "A延伸"},
    {nullptr, "shortstairs", "A小道楼梯"},
    {nullptr, "bombsitea", "A包点"},
    {nullptr, "asite", "A包点"},
    {nullptr, "sitea", "A包点"},
    {nullptr, "bombsiteb", "B包点"},
    {nullptr, "bsite", "B包点"},
    {nullptr, "siteb", "B包点"},
    {nullptr, "tspawn", "T出生"},
    {nullptr, "ctspawn", "CT出生"},
    {nullptr, "mid", "中路"},
    {nullptr, "middle", "中路"},
    {nullptr, "topmid", "中路上"},
    {nullptr, "topofmid", "中路上"},
    {nullptr, "bottommid", "中路下"},
    {nullptr, "middoors", "中门"},
    {nullptr, "middoor", "中门"},
    {nullptr, "catwalk", "猫道"},
    {nullptr, "longdoors", "A大门"},
    {nullptr, "longdoor", "A大门"},
    {nullptr, "longa", "A大道"},
    {nullptr, "along", "A大道"},
    {nullptr, "outsidelong", "A大门外"},
    {nullptr, "aramp", "A斜坡"},
    {nullptr, "bramp", "B斜坡"},
    {nullptr, "tramp", "T斜坡"},
    {nullptr, "ashort", "A小道"},
    {nullptr, "short", "小道"},
    {nullptr, "tunnels", "通道"},
    {nullptr, "uppertunnels", "上通道"},
    {nullptr, "lowertunnels", "下通道"},
    {nullptr, "outsidetunnels", "通道外"},
    {nullptr, "outsidetunnel", "通道外"},
    {nullptr, "tunnelstairs", "洞楼梯"},
    {nullptr, "palms", "棕榈树"},
    {nullptr, "pit", "深坑"},
    {nullptr, "goose", "鹅"},
    {nullptr, "xbox", "盒子"},
    {nullptr, "stairs", "楼梯"},
    {nullptr, "connector", "连接道"},
    {nullptr, "jungle", "丛林"},
    {nullptr, "window", "窗"},
    {nullptr, "palace", "皇宫"},
    {nullptr, "apartments", "公寓"},
    {nullptr, "apps", "公寓"},
    {nullptr, "market", "市场"},
    {nullptr, "ticketbooth", "售票口"},
    {nullptr, "ticket", "售票口"},
    {nullptr, "truck", "卡车"},
    {nullptr, "van", "面包车"},
    {nullptr, "snipersnest", "狙击位"},
    {nullptr, "scaffold", "脚手架"},
    {nullptr, "bridge", "桥"},
    {nullptr, "heaven", "天堂"},
    {nullptr, "hell", "地狱"},
    {nullptr, "lobby", "大厅"},
    {nullptr, "library", "图书馆"},
    {nullptr, "kitchen", "厨房"},
    {nullptr, "garage", "车库"},
    {nullptr, "backalley", "后巷"},
    {nullptr, "frontyard", "前院"},
    {nullptr, "backyard", "后院"},
    {nullptr, "banana", "香蕉道"},
    {nullptr, "underpass", "地下道"},
    {nullptr, "overpass", "天桥"},
    {nullptr, "fountain", "喷泉"},
    {nullptr, "sidealley", "侧巷"},
    {nullptr, "elbow", "肘部"},
    {nullptr, "sandwich", "三明治"},
    {nullptr, "tetris", "俄罗斯方块"},
    {nullptr, "roof", "屋顶"},
    {nullptr, "hut", "木屋"},
    {nullptr, "plat", "平台"},
    {nullptr, "platform", "平台"},
    {nullptr, "default", "默认位"},
    {nullptr, "ninja", "忍者位"},
    {nullptr, "graveyard", "墓地"},
    {nullptr, "ruins", "废墟"},
    {nullptr, "bdoor", "B门"},
    {nullptr, "bdoors", "B门"},
    {nullptr, "adoor", "A门"},
    {nullptr, "adoors", "A门"},
    {nullptr, "tdoor", "T门"},
    {nullptr, "tdoors", "T门"},
    {nullptr, "ctdoor", "CT门"},
    {nullptr, "ctdoors", "CT门"},
    {nullptr, "firebox", "火箱"},
    {nullptr, "aplat", "A平台"},
    {nullptr, "bplat", "B平台"},
    {nullptr, "hole", "洞口"},
    {nullptr, "side", "侧边"},
    {nullptr, "car", "车"},
    {nullptr, "secondmid", "二中"},
    {nullptr, "bombsitec", "C点"},
};

inline const char* lookup_place_token(const char* map, const char* key) {
    if (!key || !key[0]) return nullptr;
    const char* generic = nullptr;
    for (const auto& e : kPlaceToks) {
        if (std::strcmp(key, e.k) != 0) continue;
        if (e.map) {
            if (map_has(map, e.map)) return e.zh;
        } else if (!generic) {
            generic = e.zh;
        }
    }
    return generic;
}

inline const char* lookup_place_spot(const char* map, float x, float y) {
    if (!map || !map[0]) return nullptr;
    const char* best = nullptr;
    float best_score = 1e9f;
    for (const auto& s : kPlaceSpots) {
        if (!map_has(map, s.map)) continue;
        float dx = x - s.x, dy = y - s.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d > s.r) continue;
        float score = d / s.r;
        if (score < best_score) {
            best_score = score;
            best = s.zh;
        }
    }
    return best;
}

inline const char* rewrite_place_fragments(const char* key, const char* raw) {
    struct Frag { const char* en; const char* zh; };
    static constexpr Frag kFrags[] = {
        {"bombsite", "包点"}, {"apartments", "公寓"}, {"apartment", "公寓"},
        {"tunnels", "通道"}, {"tunnel", "通道"}, {"connector", "连接道"},
        {"catwalk", "小道"}, {"platform", "平台"}, {"windows", "窗"},
        {"window", "窗"}, {"doors", "门"}, {"door", "门"},
        {"stairs", "楼梯"}, {"stair", "楼梯"}, {"spawn", "出生"},
        {"palace", "皇宫"}, {"jungle", "丛林"}, {"market", "市场"},
        {"banana", "香蕉道"}, {"underpass", "地下道"}, {"overpass", "天桥"},
        {"extended", "延伸"}, {"outside", "外"}, {"inside", "内"},
        {"upper", "上"}, {"lower", "下"}, {"under", "下"},
        {"ramp", "斜坡"}, {"long", "大道"}, {"short", "小道"},
        {"middle", "中路"}, {"mid", "中"}, {"site", "包点"},
        {"house", "房"}, {"apps", "公寓"}, {"plat", "平台"},
        {"ct", "CT"}, {"t", "T"}, {"a", "A"}, {"b", "B"},
    };

    thread_local char generated[48];
    generated[0] = 0;
    size_t oi = 0;
    const char* p = key;
    bool any_zh = false;
    while (*p && oi + 4 < sizeof(generated)) {
        const Frag* best = nullptr;
        size_t best_len = 0;
        for (const auto& f : kFrags) {
            size_t n = std::strlen(f.en);
            if (n > best_len && std::strncmp(p, f.en, n) == 0) {
                best = &f;
                best_len = n;
            }
        }
        if (best) {
            size_t zl = std::strlen(best->zh);
            if (oi + zl >= sizeof(generated)) break;
            std::memcpy(generated + oi, best->zh, zl);
            oi += zl;
            p += best_len;
            if ((unsigned char)best->zh[0] >= 0x80) any_zh = true;
        } else {
            char c = *p++;
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            generated[oi++] = c;
        }
    }
    generated[oi] = 0;
    if (any_zh && generated[0]) return generated;
    return raw;
}

inline const char* resolve_place_name(const char* raw, const char* map, float x, float y) {
    if (const char* spot = lookup_place_spot(map, x, y))
        return spot;

    char key[32]{};
    normalize_place_key(raw, key, sizeof(key));
    if (const char* tok = lookup_place_token(map, key))
        return tok;
    if (!key[0]) return "";
    return rewrite_place_fragments(key, raw);
}

inline const char* translate_place_name(const char* raw) {
    return resolve_place_name(raw, "", 0.f, 0.f);
}
