#include <mutf8.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#define MUTF8_ACCEPT 0
#define MUTF8_REJECT 15

/// Decodes one MUTF-8 code unit into `utf16cu`.
/// 
/// @param[inout] utf16cu The UTF-16 code unit being constructed.
/// @param[in]    mutf8cu The MUTF-8 code unit to be processed.
/// @param[inout] state   The current conversion state.
/// @return               The value of `*state`.
/// 
/// The values of `*utf16cu` and `*state` shall be initialized to zero on first
/// call. If the returned value of `*state` is `MUTF8_ACCEPT`, the value of
/// `*utf16cu` is a complete UTF-16 code unit. If the state is `MUTF8_REJECT`,
/// then the input sequence of MUTF-8 code units is invalid. If the state has
/// any other value, the value of `*utf16cu` is incomplete and the caller must
/// call this function again with the next MUTF-8 code unit to complete it.
/// 
/// The input sequence is not validated against invalid codepoints or unpaired
/// surrogate codepoints.
/// 
/// This function is based on a UTF-8 decoder by Björn Höhrmann, which I have
/// modified to support no-validation MUTF-8.
///
/// > Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
/// >
/// > Permission is hereby granted, free of charge, to any person obtaining a copy
/// > of this software and associated documentation files (the "Software"), to deal
/// > in the Software without restriction, including without limitation the rights
/// > to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// > copies of the Software, and to permit persons to whom the Software is
/// > furnished to do so, subject to the following conditions:
/// >
/// > The above copyright notice and this permission notice shall be included in
/// > all copies or substantial portions of the Software.
/// >
/// > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// > IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// > FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// > AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// > LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// > OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// > SOFTWARE.
///
/// <https://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
static inline unsigned int
decode_mutf8(uint16_t *restrict utf16cu, uint8_t mutf8cu, unsigned int *restrict state)
{
	// clang-format off
	static uint8_t const mutf8class[] = {
		// The first part of the table maps bytes to character classes that
		// to reduce the size of the transition table and create bitmasks.
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 2, 2, 3, 12,
		// The second part is a transition table that maps a combination
		// of a state of the automaton and a character class to a state.
		 0, 15,  5, 10, 15,
		15,  0, 15, 15, 15,
		15,  5, 15, 15, 15,
		15, 15, 15, 15, 15,
	};  // clang-format on

	// NOLINTBEGIN(readability-magic-numbers)
	uint8_t type = mutf8class[mutf8cu >> 4U];

	*utf16cu = (*state != MUTF8_ACCEPT)
		? (*utf16cu << 6U) | (mutf8cu & 0x3FU)  // Append continuation bits
		: mutf8cu & (0xFFU >> type);            // Set to payload bits

	return *state = mutf8class[16U + *state + (type & 0x07U)];
	// NOLINTEND(readability-magic-numbers)
}

size_t mutf8toc16(uint16_t *restrict pc16, uint8_t const *restrict string, size_t string_size)
{
	if (!string) return 0;

	uint16_t c16;        // NOLINT(cppcoreguidelines-init-variables)
	unsigned int state;  // NOLINT(cppcoreguidelines-init-variables)

	for (size_t i = 0; i < string_size; ++i) {
		switch (decode_mutf8(&c16, string[i], &state)) {
		case MUTF8_ACCEPT:
			if (pc16) *pc16 = c16;
			return i + 1;
		case MUTF8_REJECT: errno = EILSEQ; return (size_t) -1;
		}
	}

	return (size_t) -2;
}

size_t c16tomutf8(uint8_t *restrict pc8, uint16_t c16)
{
	// NOLINTBEGIN(readability-magic-numbers)
	if (!pc8) return 0;

	if (c16 == 0) {
		pc8[0] = 0b11000000;
		pc8[1] = 0b10000000;
		return 2;
	}

	if (c16 < (1 << 7)) {
		pc8[0] = c16;
		return 1;
	}

	if (c16 < (1 << 11)) {
		pc8[0] = 0b11000000 | ((c16 >> 6) & 0b00111111);
		pc8[1] = 0b10000000 | ((c16 >> 0) & 0b00111111);
		return 2;
	}

	pc8[0] = 0b11100000 | ((c16 >> 12) & 0b00111111);
	pc8[1] = 0b10000000 | ((c16 >> 6) & 0b00111111);
	pc8[2] = 0b10000000 | ((c16 >> 0) & 0b00111111);
	return 3;
	// NOLINTEND(readability-magic-numbers)
}
