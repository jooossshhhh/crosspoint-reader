#include "TextFontPick.h"

#include <Utf8.h>

#include "fontIds.h"

namespace TextFontPick {

namespace {

bool codepointNeedsJpUiFont(const uint32_t cp) {
  if (cp >= 0x3000 && cp <= 0x303F) return true;
  if (cp >= 0x3040 && cp <= 0x309F) return true;
  if (cp >= 0x30A0 && cp <= 0x30FF) return true;
  if (cp >= 0x31F0 && cp <= 0x31FF) return true;
  if (cp >= 0x3400 && cp <= 0x4DBF) return true;
  if (cp >= 0x4E00 && cp <= 0x9FFF) return true;
  if (cp >= 0xF900 && cp <= 0xFAFF) return true;
  if (cp >= 0xFF00 && cp <= 0xFFEF) return true;
  if (cp >= 0x20000 && cp <= 0x323AF) return true;
  return false;
}

}  // namespace

bool utf8NeedsCjkFont(const char* utf8) {
  if (!utf8) {
    return false;
  }
  const unsigned char* p = reinterpret_cast<const unsigned char*>(utf8);
  uint32_t cp = 0;
  while ((cp = utf8NextCodepoint(&p)) != 0) {
    if (codepointNeedsJpUiFont(cp)) {
      return true;
    }
  }
  return false;
}

BookLineFont titleChromeFont(const char* utf8) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {UI_12_FONT_ID, EpdFontFamily::BOLD};
}

BookLineFont subtitleChromeFont(const char* utf8) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {UI_10_FONT_ID, EpdFontFamily::REGULAR};
}

BookLineFont listPrimaryLineFont(const char* utf8) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {UI_10_FONT_ID, EpdFontFamily::REGULAR};
}

BookLineFont listSecondaryLineFont(const char* utf8) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {SMALL_FONT_ID, EpdFontFamily::REGULAR};
}

BookLineFont coversTileTitleFont(const char* utf8) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {SMALL_FONT_ID, EpdFontFamily::REGULAR};
}

BookLineFont listRowTitleFont(const char* utf8, const bool rowHasSubtitle) {
  if (utf8NeedsCjkFont(utf8)) {
    return {NOTOSANSJP_14_FONT_ID, EpdFontFamily::REGULAR};
  }
  if (rowHasSubtitle) {
    return {UI_12_FONT_ID, EpdFontFamily::REGULAR};
  }
  return {UI_10_FONT_ID, EpdFontFamily::REGULAR};
}

}  // namespace TextFontPick
