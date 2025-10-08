#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE (1ULL << 32) // 256^4

static int32_t *table32 = NULL;

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

void init_table32()
{
	// ~4 GiB RAM
	table32 = (int32_t *) calloc(TABLE_SIZE, sizeof(int32_t));

	if (!table32)
	{
		fprintf(stderr, "calloc failed\n");
		exit(1);
	}

	char key[5] = {0}; // 4 chars + null

	// TODO optimize
	// Brute force all 4-byte combinations
	for (uint64_t i = 0; i < TABLE_SIZE; ++i)
	{
		key[0] = (char)((i      ) & 0xFF);
		key[1] = (char)((i >>  8) & 0xFF);
		key[2] = (char)((i >> 16) & 0xFF);
		key[3] = (char)((i >> 24) & 0xFF);

		table32[i] = parse_4digits(key);
	}

	printf("Table initialized.\n");
}

int32_t parseInt8a(const char *str)
{
	// TODO check length
	const uint32_t idx1 = *(const uint32_t *)&str[0];
	const uint32_t idx2 = *(const uint32_t *)&str[4];

	const int32_t high = table32[idx1];
	const int32_t low  = table32[idx2];
	return (int32_t)(high * 10000) + (int32_t)low;
}

int main()
{
	init_table32();

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

		const int32_t val = parseInt8a(input);
		printf("parseInt8(\"%s\") = %d\n", tests[i], val);
	}

	free(table32);
	return 0;
}
