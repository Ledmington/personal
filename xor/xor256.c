#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t digest[32] = {0xe3, 0xb0, 0xc4, 0x42,
					  0x98, 0xfc, 0x1c, 0x14,
					  0x9a, 0xfb, 0xf4, 0xc8,
					  0x99, 0x6f, 0xb9, 0x24,
					  0x27, 0xae, 0x41, 0xe4,
					  0x64, 0x9b, 0x93, 0x4c,
					  0xa4, 0x95, 0x99, 0x1b,
					  0x78, 0x52, 0xb8, 0x55};

int main(int argc, char *argv[]) {

	if(argc != 2) {
		fprintf(stderr, "Error: wrong number of parameters\n");
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE *fin = fopen(argv[1], "rb");
	if(fin == NULL) {
		fprintf(stderr, "Error while opening the file \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	int index = 0;
	while(!feof(fin)) {
		uint8_t c = (uint8_t)fgetc(fin);
		digest[index%32] ^= c;
		index++;
	}

	for(int i=0; i<32; i++) {
		printf("%02x", digest[i]);
	}
	printf("\n");

	fclose(fin);

	return EXIT_SUCCESS;
}