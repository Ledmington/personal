#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define uint unsigned int
#define uchar unsigned char

void add(uchar *a, uchar *b, uchar *res, uint len) {
	int carry = 0;
	for (int i = len - 1; i >= 0; i--) {
		res[i] = (carry + (a[i] - 'a') + (b[i] - 'a')) % 26 + 'a';
		// printf("%d + %d + %d = %d\n", a[i]-'a', b[i]-'a', carry, res[i]-'a');
		carry = (carry + (a[i] - 'a') + (b[i] - 'a')) > 25;
	}
}

void sub(uchar *a, uchar *b, uchar *res, uint len) {
	uchar tmp[len];
	strcpy(tmp, a);

	for (int i = len - 1; i >= 0; i--) {
		if (tmp[i] < b[i]) {
			tmp[i - 1]--;
			res[i] = (tmp[i] - 'a' + 26 - (b[i] - 'a')) + 'a';
		} else {
			res[i] = (tmp[i] - 'a' - (b[i] - 'a')) + 'a';
		}
	}
}

void div2(uchar *s, uint len) {
	for (int i = 0; i < len; i++) {
		if ((s[i] - 'a') % 2 == 1) {
			if (i < len - 1) s[i + 1] += 26;
		}
		s[i] = ((s[i] - 'a') / 2) + 'a';
	}
}

void init(uchar *s, uchar c, uint len) {
	for (int i = 0; i < len - 1; i++) {
		s[i] = c;
	}
	s[len - 1] = '\0';
}

int main(void) {
	uint len;

	printf(
		"\nWelcome to Word Guesser!\n"
		" The rules are simple: i will try to guess your word by proposing you "
		"some guesses.\n"
		" (Only lowercase letters)\n\n");

	printf("How long is your word? ");
	scanf("%u", &len);
	getchar();

	uchar a[len + 1];
	uchar b[len + 1];
	uchar mid[len + 1];
	uchar tmp[len + 1];

	uchar response;

	init(a, 'a', len + 1);
	init(b, 'z', len + 1);
	init(mid, 'a', len + 1);
	init(tmp, 'a', len + 1);

	printf("\nLet's go!\n");

	int nattempts = (int)(log(pow(26, (double)len)) / log(2));

	do {
		printf("\nOnly %d attempts remaining.\n", nattempts);

		// Computing mid = a+(b-a)/2 with tmp as temporary result
		sub(b, a, tmp, len);
		div2(tmp, len);
		add(a, tmp, mid, len);

		printf("Is \"%s\" your word (y/n)? ", mid);

		while ((response = getchar()) == '\n')
			;
		getchar();

		switch (response) {
			case 'y':
				printf(
					"Yey! I won!\n"
					"Goodbye!\n\n");
				return EXIT_SUCCESS;
				break;
			case 'n':
				break;
			default:
				printf("ERROR: unknown character \'%c\'\n", response);
				return EXIT_FAILURE;
		}

		printf("Is it \"greater\" than your word (y/n)? ");

		while ((response = getchar()) == '\n')
			;
		getchar();

		switch (response) {
			case 'y':
				strncpy(b, mid, len);
				break;
			case 'n':
				strncpy(a, mid, len);
				break;
			default:
				printf("ERROR: unknown character \'%c\'\n", response);
				exit(EXIT_FAILURE);
		}

		nattempts--;
	} while (strncmp(b, a, len) > 0);

	return EXIT_SUCCESS;
}