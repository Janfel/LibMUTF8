#include <mutf8.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#define BUFFER_SIZE 4096

int main()
{
	setlocale(LC_ALL, "");

	uint8_t buffer[BUFFER_SIZE];
	size_t prefix = 0;
	FILE *istream = stdin;
	FILE *ostream = stdout;
	mbstate_t ps = {};

	while (1) {
		size_t count = fread(&buffer[prefix], sizeof(buffer[0]), BUFFER_SIZE - prefix, istream);
		if (!count) {
			if (feof(istream)) break;
			if (ferror(istream)) {
				perror("Failed to read from stdin");
				exit(1);
			}
		}

		for (size_t i = 0; i < prefix + count;) {
			uint16_t c16 = 0;
			size_t rc = mutf8toc16(&c16, &buffer[i], (prefix + count) - i);

			if (rc == (size_t) -1) {
				perror("Invalid MUTF-8 sequence");
				exit(1);
			}

			if (rc == (size_t) -2) {
				_Static_assert(BUFFER_SIZE > 2 * MUTF8_MAX, "Can memcpy() without aliasing");
				prefix = (prefix + count) - i;
				memcpy(buffer, &buffer[i], prefix * sizeof(buffer[0]));
				break;
			}

			char out_buffer[MB_CUR_MAX];
			size_t wc = c16rtomb(out_buffer, c16, &ps);

			if (wc == (size_t) -1) {
				perror("Invalid UTF-16 sequence");
				exit(1);
			}

			if (fwrite(out_buffer, sizeof(out_buffer[0]), wc, ostream) < wc) {
				perror("Failed to write to stdout");
				exit(1);
			}

			i += rc;
		}
	}
}
