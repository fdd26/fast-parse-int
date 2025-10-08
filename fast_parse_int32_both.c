#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE (1ULL << 32) // 256^4 entries

static int16_t *table16 = NULL;
static int32_t *table32 = NULL;

// Parse a 4-character string to int, with rules:
// skip leading spaces, optional '+', then digits, stop at non-digit
// If invalid, return 0
int parse_4digits(const char *s)
{
	int i = 0;

	// skip leading spaces
	while ((i < 4) && s[i] && (s[i] <= ' '))
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

	// Faster without static constants
	const char ALLOWED_CHARS[] = { '+', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };
	const int  ALLOWED_COUNT = sizeof(ALLOWED_CHARS);

	char key[5] = {0};

	// Iterate only allowed chars per position (12^4 = 20,736 entries)
	for (int i0 = 0; i0 < ALLOWED_COUNT; ++i0)
	for (int i1 = 1; i1 < ALLOWED_COUNT; ++i1)
	for (int i2 = 1; i2 < ALLOWED_COUNT; ++i2)
	for (int i3 = 1; i3 < ALLOWED_COUNT; ++i3)
	{
		key[0] = ALLOWED_CHARS[i0];
		key[1] = ALLOWED_CHARS[i1];
		key[2] = ALLOWED_CHARS[i2];
		key[3] = ALLOWED_CHARS[i3];

		const uint32_t idx = *(uint32_t *)key;
		table32[i] = (int32_t) parse_4digits(idx);
	}

	printf("Table initialized.\n");
}

void init_table16() {
	table16 = (int16_t *) calloc(TABLE_SIZE, sizeof(uint16_t));
	if (!table16) {
		fprintf(stderr, "calloc failed\n");
		exit(1);
	}

	// Faster without static constants
	const char ALLOWED_CHARS[] = { '+', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };
	const int  ALLOWED_COUNT = sizeof(ALLOWED_CHARS);

	char key[5] = {0};

	// Iterate only allowed chars per position (12^4 = 20,736 entries)
	for (int i0 = 0; i0 < ALLOWED_COUNT; ++i0)
	for (int i1 = 1; i1 < ALLOWED_COUNT; ++i1)
	for (int i2 = 1; i2 < ALLOWED_COUNT; ++i2)
	for (int i3 = 1; i3 < ALLOWED_COUNT; ++i3)
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

int32_t parseInt8a(const char *input)
{
	char buffer[32] = {
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
		// Faster without \0 above
		// so forcefully add \0 below instead after memcpy()
	};

	const size_t len0 = strlen(input);
	size_t len = len0;
	if (len > 8) len = 8;  // truncate to 8 max

	// Right-align copy to last 8 positions
	memcpy(&buffer[31-len], input, len);

	// Ensure buffer is NUL terminated
	buffer[31] = 0;

	// Now parse the last 8 bytes
	const uint32_t idx1 = *(uint32_t *)&buffer[24];
	const uint32_t idx2 = *(uint32_t *)&buffer[28];

	const uint32_t high = (uint32_t) table32[idx1];
	const uint32_t low  = (uint32_t) table32[idx2];
	const uint32_t val  = (high * 10000) + low;

	//printf("LEN0[%u], LEN[%u], INPUT[%s], BUFFER[%s], IDX1[%u], IDX2[%u], HIGH[%u], LOW[%u], VAL[%u]\n", len, len0, input, buffer, idx1, idx2, high, low, val);

	return val;
}

int32_t parseInt8b(const char *input)
{
	char buffer[32] = {
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
		// Faster without \0 above
		// so forcefully add \0 below instead after memcpy()
	};

	const size_t len0 = strlen(input);
	size_t len = len0;
	if (len > 8) len = 8;  // truncate to 8 max

	// Right-align copy to last 8 positions
	memcpy(&buffer[31-len], input, len);

	// Ensure buffer is NUL terminated
	buffer[31] = 0;

	// Now parse the last 8 bytes
	const uint32_t idx1 = *(uint32_t *)&buffer[24];
	const uint32_t idx2 = *(uint32_t *)&buffer[28];

	const uint32_t high = (uint32_t) table16[idx1];
	const uint32_t low  = (uint32_t) table16[idx2];
	const uint32_t val  = (high * 10000) + low;

	//printf("LEN0[%u], LEN[%u], INPUT[%s], BUFFER[%s], IDX1[%u], IDX2[%u], HIGH[%u], LOW[%u], VAL[%u]\n", len, len0, input, buffer, idx1, idx2, high, low, val);

	return val;
}

int main()
{
	init_table32();
	init_table16();

	static const char *tests[] = {
		"00000000", // 0
		"00000001", // 1
		"00001234", // 1234
		"00012345", // 12345
		"99999999", // 99999999
		"+0012345", // invalid 7-char, you should pad to 8 chars
		"  +12345", // spaces + plus

		// now works
		"+1",
		"+2",
		"+0002",
		"-0",

		// Invalid so 0
		"-1",

		NULL
	};

	for (int i = 0; tests[i]; ++i)
	{
		const size_t len = strlen(tests[i]);

		const char* input = tests[i];

		const int32_t vala = parseInt8a(input);
		const int32_t valb = parseInt8b(input);
		printf("parseInt8(\"%s\") = %d = %d\n", tests[i], vala, valb);
	}

	free(table32);
	free(table16);
	return 0;
}
