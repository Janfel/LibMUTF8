#pragma once

/// @file
///
/// This library implements the encoding and decoding of Java’s Modified UTF-8
/// Encoding, hereafter MUTF-8. There are two major differences between MUTF-8
/// and regular UTF-8.
///
/// 1. The null character (U+0000) is encoded as a two byte sequence.
/// 2. Instead of using four byte sequences, MUTF-8 encodes codepoints outside
///    of the BMP using UTF-8 encoded UTF-16 surrogate pairs.
/// 
/// As such, MUTF-8 is really just UTF-16 in disguise, which makes the
/// conversion rather more painless than expected. As a tradeoff, this
/// implementation only validates the input on the code unit level, but not on
/// the codepoint level. Validating against unpaired surrogate codepoints is
/// left to the UTF-16 implementation, we just pass along what is given to us.
///
/// <https://docs.oracle.com/javase/8/docs/api/java/io/DataInput.html>

#include <stddef.h>
#include <stdint.h>

/// The maximum number of MUTF-8 code units per UTF-16 code unit.
#define MUTF8_MAX 3

/// Generates the next UTF-16 code unit from a narrow MUTF-8 string.
///
/// On success, sets `pc16` to the decoded UTF-16 code unit and returns the
/// number of MUTF-8 code units read from `string`. If `string` is not valid
/// MUTF-8, sets `errno` to `EILSEQ` and returns `(size_t) -1`. If `string`
/// contains an incomplete but valid sequence of MUTF-8 code units, returns
/// `(size_t) -2`.
///
/// If `pc16` is null, the next UTF-16 code unit is decoded but not written.
/// If `string` is null, nothing is written to `pc16` and the return value is 0.
size_t mutf8toc16(uint16_t *restrict pc16, uint8_t const *restrict string, size_t string_size);

/// Converts a UTF-16 code unit into a narrow MUTF-8 string.
///
/// Writes up to `MUTF8_MAX` MUTF-8 code units to `pc8` and returns the number
/// of decoded MUTF-8 code units. If `pc8` is null, nothing is written and the
/// return value is 0.
size_t c16tomutf8(uint8_t *restrict pc8, uint16_t c16);
