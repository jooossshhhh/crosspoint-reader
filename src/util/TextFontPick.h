#pragma once

#include <EpdFontFamily.h>

namespace TextFontPick {

struct BookLineFont {
  int fontId;
  EpdFontFamily::Style style;
};

/// True if \p utf8 contains any codepoint that needs Noto Sans JP (Japanese UI / book metadata).
bool utf8NeedsCjkFont(const char* utf8);

/// Home tiles & reader chrome: ~12 pt bold Latin vs 14 pt JP regular.
BookLineFont titleChromeFont(const char* utf8);

/// Author / secondary line under title: ~10 pt Latin vs 14 pt JP regular.
BookLineFont subtitleChromeFont(const char* utf8);

/// Lyra recent list & similar: primary row (normally UI_10).
BookLineFont listPrimaryLineFont(const char* utf8);

/// Lyra recent list secondary (normally SMALL / 8 pt Latin).
BookLineFont listSecondaryLineFont(const char* utf8);

/// 3-cover tile title (normally SMALL_FONT_ID).
BookLineFont coversTileTitleFont(const char* utf8);

/// Base theme list row title: UI_12 when a subtitle row exists, else UI_10; JP uses 14 pt for both.
BookLineFont listRowTitleFont(const char* utf8, bool rowHasSubtitle);

}  // namespace TextFontPick
