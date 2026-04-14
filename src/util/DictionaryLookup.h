#pragma once

#include <HalStorage.h>

#include <cstddef>
#include <cstdint>

/// Sorted UTF-8 TSV on the SD card: each line is `key<TAB>definition` (definition may contain tabs if
/// escaped — not supported; keep glosses tab-free). Lines sorted by byte-wise strcmp of key.
constexpr const char* JP_DICTIONARY_SDCARD_PATH = "/dict/jp-dictionary.tsv";

/// Returns the number of Unicode scalar values (codepoints) in valid UTF-8 \p s.
size_t utf8CountCodepoints(const char* s);

/// Writes one UTF-8 codepoint from \p flat at codepoint index \p cpIndex into \p out (null-terminated).
/// Returns false if index out of range or invalid UTF-8.
bool utf8NthCodepointToBuffer(const char* flat, int cpIndex, char* out, size_t outCap);

/// Looks up \p key in a sorted TSV opened as \p file with size \p fileSize. Writes gloss into \p out.
/// Returns true if an exact key match was found.
bool dictionaryLookupSortedTsv(FsFile& file, size_t fileSize, const char* key, char* out, size_t outLen);
