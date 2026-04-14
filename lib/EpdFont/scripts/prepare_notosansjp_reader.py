#!/usr/bin/env python3
"""Subset Noto Sans JP for CrossPoint reader: kana + punctuation + kanji sample.

The full Noto Sans JP OTF covers ~12k+ CJK codepoints; embedding all at 4 sizes does not
fit typical ESP32 app partitions. We keep every kana / CJK-punctuation / fullwidth codepoint
present in the source font, add ASCII, then pick kanji (CJK Unified) as follows:

  1) Prefer codepoints listed in data/kanji_priority_hex.txt (Jōyō table order via cjkvi).
     This matches *school / official common kanji*, not newspaper corpus frequency — but it
     tracks real-world literacy much better than uniform Unicode stepping for fiction.
  2) If NOTOSANSJP_KANJI_TARGET is larger than that list's overlap with the font, fill the
     remainder by stratified sampling over remaining CJK in the font (legacy behaviour).

Override the kanji budget with env NOTOSANSJP_KANJI_TARGET (default 800).
Override the priority file with env NOTOSANSJP_KANJI_PRIORITY_FILE (path to hex lines).

Inputs (first existing path wins):
  - argv[1]: path to NotoSansJP-Regular.otf or .ttf (same glyph set; both work)
  - or env NOTOSANSJP_REGULAR

Output:
  - argv[2] or env NOTOSANSJP_SUBSET_TTF: path to write subset TTF (default: next to input,
    named NotoSansJP-ReaderSubset.ttf)
"""
from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import Dict, List, Set

from fontTools.ttLib import TTFont
from fontTools.subset import Subsetter


def load_kanji_priority(path: Path) -> List[int]:
  if not path.is_file():
    return []
  out: List[int] = []
  for line in path.read_text(encoding="utf-8").splitlines():
    line = line.strip()
    if not line or line.startswith("#"):
      continue
    out.append(int(line, 16))
  return out


def fill_stratified(cjk_in_font: Set[int], sampled: Set[int], kanji_target: int) -> None:
  if len(sampled) >= kanji_target:
    return
  remainder = sorted(cjk_in_font - sampled)
  if not remainder:
    return
  need = kanji_target - len(sampled)
  step = max(1, len(remainder) // need)
  for i in range(0, len(remainder), step):
    if len(sampled) >= kanji_target:
      break
    sampled.add(remainder[i])
  # Rounding can leave us a few short — top up sequentially.
  if len(sampled) < kanji_target:
    for cp in remainder:
      if len(sampled) >= kanji_target:
        break
      sampled.add(cp)


def build_unicodes(cm: Dict[int, int], kanji_target: int, priority_path: Path) -> Set[int]:
  cjk_in_font = {cp for cp in cm if 0x4E00 <= cp <= 0x9FFF}
  sampled: Set[int] = set()
  if kanji_target > 0 and cjk_in_font:
    for cp in load_kanji_priority(priority_path):
      if cp in cjk_in_font and len(sampled) < kanji_target:
        sampled.add(cp)
    fill_stratified(cjk_in_font, sampled, kanji_target)

  unicodes: Set[int] = set(sampled)
  for cp in cm:
    if (
        (0x3040 <= cp <= 0x309F)
        or (0x30A0 <= cp <= 0x30FF)
        or (0x31F0 <= cp <= 0x31FF)
        or (0x3000 <= cp <= 0x303F)
        or (0xFF60 <= cp <= 0xFF9F)
        or (0xFF01 <= cp <= 0xFFEE)
    ):
      unicodes.add(cp)
  unicodes.update(range(0x20, 0x7F))
  return unicodes


def main() -> int:
  inp = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("NOTOSANSJP_REGULAR", "")
  if not inp or not os.path.isfile(inp):
    print(
        "Usage: prepare_notosansjp_reader.py /path/to/NotoSansJP-Regular.{otf,ttf} [out.ttf]\n"
        "Env: NOTOSANSJP_KANJI_TARGET (default 800), NOTOSANSJP_SUBSET_TTF,\n"
        "     NOTOSANSJP_KANJI_PRIORITY_FILE (hex list; default scripts/data/kanji_priority_hex.txt)",
        file=sys.stderr,
    )
    return 1

  out = (
      sys.argv[2]
      if len(sys.argv) > 2
      else os.environ.get("NOTOSANSJP_SUBSET_TTF", os.path.join(os.path.dirname(inp), "NotoSansJP-ReaderSubset.ttf"))
  )

  kanji_target = int(os.environ.get("NOTOSANSJP_KANJI_TARGET", "800"))
  priority_file = Path(
      os.environ.get(
          "NOTOSANSJP_KANJI_PRIORITY_FILE",
          str(Path(__file__).resolve().parent / "data" / "kanji_priority_hex.txt"),
      )
  )

  font = TTFont(inp)
  cm = font["cmap"].getBestCmap()
  unicodes = build_unicodes(cm, kanji_target, priority_file)
  subsetter = Subsetter()
  subsetter.populate(unicodes=unicodes)
  subsetter.subset(font)
  font.save(out)
  print(f"Wrote {out} ({os.path.getsize(out)} bytes, {len(unicodes)} Unicode scalars, kanji sample cap={kanji_target})")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
