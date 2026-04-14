#!/bin/bash
# Build reader fonts: Noto Sans (Latin/Cyrillic stack) + subset Noto Sans JP (kana + sampled kanji).
# Prereqs: Python 3, pip packages in scripts/requirements.txt (freetype-py, fonttools).
#
# 1) Place the regular Noto Sans JP face under:
#    ../builtinFonts/source/NotoSansJP/
#    Name it either NotoSansJP-Regular.otf (noto-cjk zip) or NotoSansJP-Regular.ttf
#    (Google Fonts / other bundles): the script accepts either.
# 2) NotoSans-Regular.ttf must exist (same as other convert scripts).
#
# Env:
#   NOTOSANSJP_KANJI_TARGET  — kanji cap (default 800). Kanji are chosen Jōyō-first from
#                              scripts/data/kanji_priority_hex.txt, then stratified fill.
#   NOTOSANSJP_KANJI_PRIORITY_FILE — alternate hex list (one codepoint per line).

set -euo pipefail

cd "$(dirname "$0")"

JP_DIR="../builtinFonts/source/NotoSansJP"
LATIN="../builtinFonts/source/NotoSans/NotoSans-Regular.ttf"
SUBSET_TTF="${JP_DIR}/NotoSansJP-ReaderSubset.ttf"

if [[ -f "${JP_DIR}/NotoSansJP-Regular.otf" ]]; then
  JP_IN="${JP_DIR}/NotoSansJP-Regular.otf"
elif [[ -f "${JP_DIR}/NotoSansJP-Regular.ttf" ]]; then
  JP_IN="${JP_DIR}/NotoSansJP-Regular.ttf"
else
  echo "Missing: ${JP_DIR}/NotoSansJP-Regular.otf or ${JP_DIR}/NotoSansJP-Regular.ttf"
  echo "Copy the regular (400) Noto Sans JP file from your download into that folder and use one of those names."
  exit 1
fi
if [[ ! -f "$LATIN" ]]; then
  echo "Missing ${LATIN}"
  exit 1
fi

python3 prepare_notosansjp_reader.py "$JP_IN" "$SUBSET_TTF"

# Japanese blocks + full CJK range (fontconvert skips codepoints with no glyph in the stack).
JP_INTERVALS=(
  "--additional-intervals" "0x3040,0x309F"
  "--additional-intervals" "0x30A0,0x30FF"
  "--additional-intervals" "0x31F0,0x31FF"
  "--additional-intervals" "0x3000,0x303F"
  "--additional-intervals" "0xFF60,0xFF9F"
  "--additional-intervals" "0xFF01,0xFFEE"
  "--additional-intervals" "0x4E00,0x9FFF"
)

# Only two pixel sizes ship in firmware (~6.5 MB app partition). Regenerate 12/16 only if you enlarge the partition.
SIZES=(14 18)
for size in "${SIZES[@]}"; do
  name="notosansjp_${size}_regular"
  out="../builtinFonts/${name}.h"
  echo "Generating $out ..."
  python3 fontconvert.py "$name" "$size" "$SUBSET_TTF" "$LATIN" --2bit --compress "${JP_INTERVALS[@]}" > "$out"
done

echo ""
echo "Running compression verification..."
python3 verify_compression.py ../builtinFonts/

echo ""
echo "Regenerate src/fontIds.h:"
echo "  (cd lib/EpdFont/scripts && bash build-font-ids.sh > ../../../src/fontIds.h)"
