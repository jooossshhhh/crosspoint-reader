#include "DictionaryLookup.h"

#include <Logging.h>
#include <Utf8.h>

#include <cstring>

namespace {

bool readOneLine(FsFile& f, size_t fileSize, char* buf, size_t cap) {
  size_t i = 0;
  while (i + 1 < cap && f.position() < fileSize) {
    char b;
    if (f.read(&b, 1) != 1) {
      break;
    }
    if (b == '\n') {
      break;
    }
    if (b != '\r') {
      buf[i++] = b;
    }
  }
  buf[i] = '\0';
  return true;
}

void skipToNextLine(FsFile& f, size_t fileSize) {
  char b;
  while (f.position() < fileSize && static_cast<size_t>(f.read(&b, 1)) == 1 && b != '\n') {}
}

}  // namespace

size_t utf8CountCodepoints(const char* s) {
  if (!s) {
    return 0;
  }
  size_t n = 0;
  const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
  uint32_t cp = 0;
  while ((cp = utf8NextCodepoint(&p)) != 0) {
    (void)cp;
    n++;
  }
  return n;
}

bool utf8NthCodepointToBuffer(const char* flat, int cpIndex, char* out, size_t outCap) {
  if (!flat || !out || outCap == 0 || cpIndex < 0) {
    return false;
  }
  const unsigned char* p = reinterpret_cast<const unsigned char*>(flat);
  for (int i = 0; i < cpIndex; i++) {
    const uint32_t cp = utf8NextCodepoint(&p);
    if (cp == 0) {
      return false;
    }
  }
  const unsigned char* const segStart = p;
  const uint32_t cp = utf8NextCodepoint(&p);
  if (cp == 0) {
    return false;
  }
  const size_t len = static_cast<size_t>(reinterpret_cast<const char*>(p) - reinterpret_cast<const char*>(segStart));
  if (len + 1 > outCap) {
    return false;
  }
  memcpy(out, segStart, len);
  out[len] = '\0';
  return true;
}

bool dictionaryLookupSortedTsv(FsFile& file, size_t fileSize, const char* key, char* out, size_t outLen) {
  if (!key || !*key || !out || outLen == 0) {
    return false;
  }
  out[0] = '\0';
  if (fileSize == 0) {
    return false;
  }

  size_t lo = 0;
  size_t hi = fileSize;
  int guard = 0;

  while (lo < hi && guard++ < 65536) {
    const size_t mid = lo + (hi - lo) / 2;
    if (!file.seek(mid)) {
      LOG_ERR("DICT", "seek failed");
      return false;
    }
    if (mid > 0) {
      skipToNextLine(file, fileSize);
    }
    const size_t lineStart = file.position();
    if (lineStart >= fileSize) {
      hi = mid;
      continue;
    }

    char line[320];
    readOneLine(file, fileSize, line, sizeof(line));
    const size_t afterLine = file.position();

    const char* tab = strchr(line, '\t');
    if (!tab) {
      hi = lineStart > 0 ? lineStart : mid;
      continue;
    }

    const size_t keyLen = static_cast<size_t>(tab - line);
    char lineKey[128];
    if (keyLen >= sizeof(lineKey)) {
      hi = lineStart;
      continue;
    }
    memcpy(lineKey, line, keyLen);
    lineKey[keyLen] = '\0';

    const int cmp = strcmp(lineKey, key);
    if (cmp == 0) {
      // Found the row during bisection — do not only set hi = lineStart (that leaves lo < hi with the same
      // mid forever: strcmp stays 0 on the same line and the loop never converges).
      const char* gloss = tab + 1;
      strncpy(out, gloss, outLen - 1);
      out[outLen - 1] = '\0';
      const int safeLen = utf8SafeTruncateBuffer(out, static_cast<int>(strlen(out)));
      if (safeLen >= 0 && static_cast<size_t>(safeLen) < outLen) {
        out[safeLen] = '\0';
      }
      return true;
    }
    if (cmp < 0) {
      lo = afterLine > lineStart ? afterLine : lineStart + 1;
      if (lo > fileSize) {
        break;
      }
    } else {
      hi = lineStart;
    }
  }

  if (!file.seek(lo)) {
    return false;
  }
  if (lo > 0) {
    skipToNextLine(file, fileSize);
  }

  char line[320];
  readOneLine(file, fileSize, line, sizeof(line));
  const char* tab = strchr(line, '\t');
  if (!tab) {
    return false;
  }
  const size_t keyLen = static_cast<size_t>(tab - line);
  char lineKey[128];
  if (keyLen >= sizeof(lineKey)) {
    return false;
  }
  memcpy(lineKey, line, keyLen);
  lineKey[keyLen] = '\0';
  if (strcmp(lineKey, key) != 0) {
    return false;
  }

  const char* gloss = tab + 1;
  strncpy(out, gloss, outLen - 1);
  out[outLen - 1] = '\0';
  const int safeLen = utf8SafeTruncateBuffer(out, static_cast<int>(strlen(out)));
  if (safeLen >= 0 && static_cast<size_t>(safeLen) < outLen) {
    out[safeLen] = '\0';
  }
  return true;
}
