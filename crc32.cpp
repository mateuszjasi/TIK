#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printFile(char *fileNameInput) {
	printf("\nInput file:\n");
	FILE *fileHandle = fopen(fileNameInput, "rb");
	if(fileHandle != NULL) {		
		unsigned char buffer[1];
		while(fread(buffer, sizeof(unsigned char), 1, fileHandle))
			printf("%c", buffer[0]);
	}
	fclose(fileHandle);
}

void printHexFile(char *fileNameInput) {
	printf("\n\nInput file in Hex:\n");
	FILE *fileHandle = fopen(fileNameInput, "rb");
	if(fileHandle != NULL) {		
		unsigned char buffer[1];
		while(fread(buffer, sizeof(unsigned char), 1, fileHandle))
			printf("0x%X ", buffer[0]);
	}
	fclose(fileHandle);
}

long int toEOF(FILE *fileHandle) {
    long int curPos = ftell(fileHandle);
    fseek(fileHandle, 0, SEEK_END);
    long int endPos = ftell(fileHandle);
    fseek(fileHandle, curPos, SEEK_SET);
    return endPos - curPos;
}

unsigned int calculateCRC32(char *fileNameInput, bool check) {
	unsigned int i, CRC32, mask, buffer[1];
	FILE *fileHandle = fopen(fileNameInput, "rb");
	CRC32 = 0xFFFFFFFF;
	while(fread(buffer, sizeof(char), 1, fileHandle)) {
		CRC32 = CRC32 ^ buffer[0];
	    for(i=0; i<8; i++) {
	        mask = -(CRC32 & 1);
	        CRC32 = (CRC32>>1) ^ (0xEDB88320 & mask);
	    }
	    if(toEOF(fileHandle) <= 4 && check)
	    	break;
	}
	fclose(fileHandle);
	return ~CRC32;
}

void addCRC32(char *fileNameOutput, unsigned int CRC32) {
	int i;
	FILE *fileHandle = fopen(fileNameOutput, "ab");
	for(i=3; i>=0; i--)
		fprintf(fileHandle, "%c", CRC32 >> i * 8);
	fclose(fileHandle);
}

unsigned int loadCRC32(char *fileNameInput) {
	unsigned char buffer[1];
	unsigned int fileCRC32;
	FILE *fileHandle = fopen(fileNameInput, "rb");
	fseek(fileHandle, -4, SEEK_END);
	while(fread(buffer, sizeof(char), 1, fileHandle))
		fileCRC32 = (fileCRC32 << 8) + buffer[0];
	fclose(fileHandle);
	return fileCRC32;
}

int main() {
	char fileNameInput[50];
	unsigned int CRC32;
	int select;
	do {
		system("cls");
		printf("1. Calculate CRC\n2. Check CRC\n3. Exit\n\n");
		scanf("%d", &select);
		switch(select) {
			case 1:
				printf("Enter the filename: ");
				scanf("%s", fileNameInput);
				//strcpy(fileNameInput, "test.txt");
				printFile(fileNameInput);
				printHexFile(fileNameInput);
				CRC32 = calculateCRC32(fileNameInput, 0);
				printf("\n\nCalulated CRC32: 0x%X", CRC32);
				addCRC32(fileNameInput, CRC32);
				printf("\nCRC added to file.\n");
				system("pause");
				break;
			case 2:
				printf("Enter the filename: ");
				scanf("%s", fileNameInput);
				//strcpy(fileNameInput, "test.txt");
				printFile(fileNameInput);
				printHexFile(fileNameInput);
				printf("\n\nChecking CRC...:\n");
				CRC32 = calculateCRC32(fileNameInput, 1);
				if(CRC32 == loadCRC32(fileNameInput))
					printf("CRC is correct.\n\n");
				else {
					printf("File is corrupted.\n");
					printf("CRC32 calculated for the data: 0x%X\n\n", CRC32);
				}
				system("pause");
				break;
		}
	} while(select != 3);
    return 0;
}
