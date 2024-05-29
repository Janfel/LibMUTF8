#include <mutf8.h>

#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t data_size)
{
	uint16_t c16 = 0;
	while (1) {
		size_t result = mutf8toc16(&c16, data, data_size);
		if (result == (size_t) -1) break;
		if (result == (size_t) -2) break;
		data += result;
		data_size -= result;
	}
	return 0;
}
