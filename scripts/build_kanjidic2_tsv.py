#!/usr/bin/env python3
"""
Convert EDRDG Kanjidic2 (kanjidic2.xml.gz) to CrossPoint dictionary TSV:
  key<TAB>gloss

Keys are single kanji (literal). Glosses use English <meaning> text only (no m_lang, or m_lang=en);
readings (ja_on / ja_kun) are prefixed when present. Tabs and newlines are stripped from gloss.

Output is sorted by UTF-8 bytes (memcmp order) for binary search on device.

Prereq: place kanjidic2.xml.gz from http://ftp.edrdg.org/pub/Nihongo/ (or mirror).

Example:
  python3 scripts/build_kanjidic2_tsv.py -i kanjidic2.xml.gz -o extras/dictionary/jp-dictionary.tsv

Copy the TSV to the SD card as: /dict/jp-dictionary.tsv
"""

from __future__ import annotations

import argparse
import gzip
import sys
import xml.etree.ElementTree as ET


def cap_utf8_bytes(s: str, max_bytes: int) -> str:
    """Truncate s to at most max_bytes UTF-8 bytes without splitting a codepoint."""
    b = s.encode("utf-8")
    if len(b) <= max_bytes:
        return s
    b = b[:max_bytes]
    while b and (b[-1] & 0xC0) == 0x80:
        b = b[:-1]
    return b.decode("utf-8", errors="ignore")


def english_meanings(elem: ET.Element) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for m in elem.findall(".//meaning"):
        lang = m.get("m_lang")
        if lang and lang.lower() not in ("", "en"):
            continue
        t = (m.text or "").strip()
        if not t:
            continue
        key = t.lower()
        if key in seen:
            continue
        seen.add(key)
        out.append(t)
    return out


def readings(elem: ET.Element) -> tuple[list[str], list[str]]:
    on: list[str] = []
    kun: list[str] = []
    for r in elem.findall(".//reading"):
        rt = r.get("r_type")
        t = (r.text or "").strip()
        if not t:
            continue
        if rt == "ja_on":
            on.append(t)
        elif rt == "ja_kun":
            kun.append(t)
    return on, kun


def gloss_for_character(elem: ET.Element, max_gloss_bytes: int) -> tuple[str, str] | None:
    literal = (elem.findtext("literal") or "").strip()
    if not literal:
        return None
    # One codepoint keys only (Kanjidic2 literals are normally one kanji).
    meanings = english_meanings(elem)
    if not meanings:
        return None

    on, kun = readings(elem)
    parts: list[str] = []
    if on or kun:
        ro = ", ".join(on) if on else "—"
        rk = ", ".join(kun) if kun else "—"
        parts.append(f"On {ro}; Kun {rk}")

    parts.append("; ".join(meanings))
    gloss = " — ".join(parts)
    gloss = gloss.replace("\t", " ").replace("\r", " ").replace("\n", " ")
    gloss = " ".join(gloss.split())
    gloss = cap_utf8_bytes(gloss, max_gloss_bytes)
    return literal, gloss


def build_tsv(inp: str, out: str, max_gloss_bytes: int) -> int:
    lines: list[str] = []
    with gzip.open(inp, "rb") as f:
        for event, elem in ET.iterparse(f, events=("end",)):
            if elem.tag != "character":
                continue
            pair = gloss_for_character(elem, max_gloss_bytes)
            elem.clear()
            if not pair:
                continue
            key, gloss = pair
            lines.append(key + "\t" + gloss)

    # Sort by UTF-8 bytes (same order as strcmp on device).
    lines.sort(key=lambda ln: ln.split("\t", 1)[0].encode("utf-8"))

    with open(out, "w", encoding="utf-8", newline="\n") as fo:
        fo.write("\n".join(lines))
        if lines:
            fo.write("\n")

    return len(lines)


def main() -> int:
    ap = argparse.ArgumentParser(description="Kanjidic2 → CrossPoint jp-dictionary.tsv")
    ap.add_argument(
        "-i",
        "--input",
        default="kanjidic2.xml.gz",
        help="Path to kanjidic2.xml.gz (default: ./kanjidic2.xml.gz)",
    )
    ap.add_argument(
        "-o",
        "--output",
        default="extras/dictionary/jp-dictionary.tsv",
        help="Output TSV path (default: extras/dictionary/jp-dictionary.tsv)",
    )
    ap.add_argument(
        "--max-gloss-bytes",
        type=int,
        default=700,
        help="Max UTF-8 bytes per gloss (device buffer is ~767; default 700)",
    )
    args = ap.parse_args()

    try:
        n = build_tsv(args.input, args.output, args.max_gloss_bytes)
    except OSError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except ET.ParseError as e:
        print(f"XML parse error: {e}", file=sys.stderr)
        return 1

    print(f"Wrote {n} lines to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
