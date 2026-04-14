# Noto Sans JP (reader subset source)

CrossPoint builds **compressed reader fonts** (`notosansjp_*_regular.h`) from:

1. **NotoSansJP-Regular** — place **`NotoSansJP-Regular.otf`** or **`NotoSansJP-Regular.ttf`** in this directory (same face; `convert-notosansjp-fonts.sh` picks whichever exists).  
   Example: [noto-cjk 16_NotoSansJP.zip](https://github.com/notofonts/noto-cjk/releases/download/Sans2.004/16_NotoSansJP.zip) (`.otf`), or a Google Fonts bundle (often `.ttf` under `static/`).

2. **NotoSans-Regular.ttf** — already required under `../NotoSans/` for Latin/Cyrillic stacking in the converter.

Then run from `lib/EpdFont/scripts/`:

```bash
pip install -r requirements.txt   # freetype-py, fonttools
./convert-notosansjp-fonts.sh
bash build-font-ids.sh > ../../../src/fontIds.h   # Python 3 (stdlib)
```

`prepare_notosansjp_reader.py` writes `NotoSansJP-ReaderSubset.ttf` here (gitignored). The subset keeps **all kana** and **CJK punctuation/fullwidth** glyphs from the OTF plus a **stratified sample of kanji** (default **800**; set `NOTOSANSJP_KANJI_TARGET` higher for better coverage if your firmware partition allows it).

**Firmware note:** The default app partition is ~6.4 MiB. Only **14 px** and **18 px** JP bitmap fonts are embedded; **Settings → Small/Medium** map to 14 px and **Large/Extra large** to 18 px. To ship 12/16 as well you need more flash (e.g. larger `app0` in `partitions.csv` and an OTA-safe layout).

License: see `LICENSE` (SIL Open Font License).
