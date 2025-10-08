// Tested in godbolt

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE (1ULL << 32) // 256^4 entries

static int16_t *table16 = NULL;

// Parse a 4-character string to int, with rules:
// skip leading spaces, optional '+', then digits, stop at non-digit
// If invalid, return 0
int parse_4digits(const char *s)
{
	int i = 0;

	// skip leading spaces
	while ((i < 4) && isspace((unsigned char)s[i]))
	{ ++i; }

	// optional '+'
	if ((i < 4) && (s[i] == '+'))
	{ ++i; }

	int val = 0;
	int digits_found = 0;
	for (; i < 4; ++i)
	{
		const char ch = s[i];
		if (ch >= '0' && ch <= '9')
		{
			val = (val * 10) + (ch - '0');
			++digits_found;
		}
		else if (ch == ' ')
		{
			// trailing spaces allowed
			continue;
		}
		else
		{
			// invalid char
			return 0;
		}
	}

	return digits_found ? val : 0;
}

void init_table16() {
	table16 = (int16_t *) calloc(TABLE_SIZE, sizeof(uint16_t));
	if (!table16) {
		fprintf(stderr, "calloc failed\n");
		exit(1);
	}

	const char ALLOWED_CHARS[] = { '+', '-', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };
	const int  ALLOWED_COUNT = sizeof(ALLOWED_CHARS);

	char key[5] = {0};

	// Iterate only allowed chars per position (12^4 = 20,736 entries)
	for (int i0 = 0; i0 < ALLOWED_COUNT; ++i0)
	for (int i1 = 2; i1 < ALLOWED_COUNT; ++i1)
	for (int i2 = 2; i2 < ALLOWED_COUNT; ++i2)
	for (int i3 = 2; i3 < ALLOWED_COUNT; ++i3)
	{
		key[0] = ALLOWED_CHARS[i0];
		key[1] = ALLOWED_CHARS[i1];
		key[2] = ALLOWED_CHARS[i2];
		key[3] = ALLOWED_CHARS[i3];

		const uint32_t idx = *(uint32_t *)key;
		table16[idx] = (int16_t) parse_4digits(key);
	}

	printf("Optimized table16 initialized (only valid chars).\n");
}

int32_t parseInt8b(const char *str)
{
	// TODO check length
	const uint32_t idx1 = *(const uint32_t *)&str[0];
	const uint32_t idx2 = *(const uint32_t *)&str[4];

	const int16_t high = table16[idx1];
	const int16_t low  = table16[idx2];
	return (int32_t)(high * 10000) + (int32_t)low;
}

int main()
{
	init_table16();

	const char *tests[] = {
		"00000000", // 0
		"00000001", // 1
		"00001234", // 1234
		"00012345", // 12345
		"99999999", // 99999999
		"+0012345", // invalid 7-char, you should pad to 8 chars
		"  +12345", // spaces + plus
		NULL
	};

	for (int i = 0; tests[i]; ++i)
	{
		char input[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

		const size_t len = strlen(tests[i]);

		// Copy input padded with spaces on the right (or left)
		// Here right-pad with spaces:
		for (size_t j = 0; j < len && j < 8; ++j)
		{
			input[j] = tests[i][j];
		}

		const int32_t val = parseInt8b(input);
		printf("parseInt8(\"%s\") = %d\n", tests[i], val);
	}

	free(table16);
	return 0;
}
