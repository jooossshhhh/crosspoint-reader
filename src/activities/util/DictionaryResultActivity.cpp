#include "DictionaryResultActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "../reader/ReaderUtils.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "util/TextFontPick.h"

namespace {
constexpr int kMarginX = 16;
constexpr int kMaxBodyLines = 18;
}  // namespace

DictionaryResultActivity::DictionaryResultActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                                   std::string headingIn, std::string bodyIn)
    : Activity("DictionaryResult", renderer, mappedInput),
      heading(std::move(headingIn)),
      body(std::move(bodyIn)) {}

void DictionaryResultActivity::onEnter() {
  Activity::onEnter();
  ReaderUtils::releaseReaderFontDecompressionCache(renderer);
  requestUpdate();
}

void DictionaryResultActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) ||
      mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    finish();
  }
}

void DictionaryResultActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const int w = renderer.getScreenWidth() - 2 * kMarginX;
  const int titleY = 12;
  // Use same JP-capable chrome fonts as Home / file browser (UI fonts alone can mis-handle CJK).
  const auto titlePick = TextFontPick::titleChromeFont(heading.c_str());
  const std::string h = renderer.truncatedText(titlePick.fontId, heading.c_str(), w, titlePick.style);
  renderer.drawText(titlePick.fontId, kMarginX, titleY, h.c_str(), true, titlePick.style);

  const auto bodyPick = TextFontPick::subtitleChromeFont(body.c_str());
  auto lines = renderer.wrappedText(bodyPick.fontId, body.c_str(), w, kMaxBodyLines);
  const int bodyLineH = renderer.getLineHeight(bodyPick.fontId);
  int y = titleY + renderer.getLineHeight(titlePick.fontId) + 8;
  for (const auto& ln : lines) {
    renderer.drawText(bodyPick.fontId, kMarginX, y, ln.c_str(), true, bodyPick.style);
    y += bodyLineH;
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_OK_BUTTON), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
