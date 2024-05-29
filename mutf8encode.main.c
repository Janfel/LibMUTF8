#include <mutf8.h>

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

#define BUFFER_SIZE 4096

int main()
{
	setlocale(LC_ALL, "");

	char buffer[BUFFER_SIZE];
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
			char16_t c16 = 0;
			size_t rc = mbrtoc16(&c16, &buffer[i], (prefix + count) - i, &ps);

			if (rc == (size_t) -1) {
				perror("Invalid multibyte sequence");
				exit(1);
			}

			if (rc == (size_t) -2) {
				prefix = (prefix + count) - i;
				memmove(buffer, &buffer[i], prefix * sizeof(buffer[0]));
				break;
			}

			do {
				uint8_t out_buffer[MUTF8_MAX];
				size_t wc = c16tomutf8(out_buffer, c16);

				if (fwrite(out_buffer, sizeof(out_buffer[0]), wc, ostream) < wc) {
					perror("Failed to write to stdout");
					exit(1);
				}
			} while (mbrtoc16(&c16, NULL, 0, &ps) == (size_t) -3);

            i += rc ? rc : 1;
		}
	}
}
