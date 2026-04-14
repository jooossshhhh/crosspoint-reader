#pragma once

#include <string>

#include "../Activity.h"

/// Full-screen dictionary gloss; Back or Confirm dismisses.
class DictionaryResultActivity final : public Activity {
  std::string heading;
  std::string body;

 public:
  DictionaryResultActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string heading,
                           std::string body);
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
