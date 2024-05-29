#include <mutf8.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t data_size)
{
	if (data_size % 2 != 0) return -1;

	uint16_t c16 = 0;
	uint8_t c8s[MUTF8_MAX] = {};

	for (size_t i = 0; i < data_size / 2; ++i) {
		memcpy(&c16, &data[2 * i], sizeof(c16));
		c16tomutf8(c8s, c16);
	}

	return 0;
}
