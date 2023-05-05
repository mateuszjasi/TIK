#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct HuffmanNodesList {
	int symbol, freq, left_symbol, right_symbol;
	struct HuffmanNodesList *next;
};
typedef struct HuffmanNodesList HuffmanNodesList;

struct HuffmanNode {
	int symbol, freq;
	struct HuffmanNode *left, *right;
};
typedef struct HuffmanNode HuffmanNode;

struct MinHeap {
	int size, capacity;
	struct HuffmanNode** array;
};
typedef struct MinHeap MinHeap;

struct CodeTable {
	int symbol, size;
	unsigned short code;
};
typedef struct CodeTable CodeTable;

int CompareHuffmanNodes(const void *item1, const void *item2) {
	HuffmanNode *node1 = (HuffmanNode *)item1;
	HuffmanNode *node2 = (HuffmanNode *)item2;
	int compareResult = (node1->freq - node2->freq);
	if(!compareResult)
		compareResult = (node1->symbol - node2->symbol);
	return -compareResult;
}

void SortHuffmanModel(HuffmanNode *huffmanModelArray, int modelArrayLength) {
	qsort(huffmanModelArray, modelArrayLength, sizeof(HuffmanNode), CompareHuffmanNodes);
}

HuffmanNode *CreateDataModel(HuffmanNode *huffmanModelArray, char *fileName, int readBytesLength, int *readCount) {
	int i, x;
	unsigned char buffer[1];
	FILE *fileHandle;
	if((fileHandle = fopen(fileName, "rb"))== NULL) {
		printf("Nie znaleziono pliku z danymi!");
		return NULL;
	}
	while(fread(buffer, sizeof(unsigned char), readBytesLength, fileHandle)) {
		x = 0;
		for(i=0;i<*readCount;i++) {
			if(huffmanModelArray[i].symbol == buffer[0]) {
				huffmanModelArray[i].freq++;
				x = 1;
				break;
			}
		}
		if(!x) {
			(*readCount)++;
			huffmanModelArray = (HuffmanNode*) realloc(huffmanModelArray, sizeof(HuffmanNode) * (*readCount));
			huffmanModelArray[(*readCount)-1].symbol = buffer[0];
			huffmanModelArray[(*readCount)-1].freq = 1;
		}
	}
	fclose(fileHandle);
	SortHuffmanModel(huffmanModelArray, *readCount);
	return huffmanModelArray;
}

void WriteModelIntoFile(char *fileName, HuffmanNode *huffmanModelArray, int readCount) {
	FILE *fileHandle;
	int i;
	if((fileHandle = fopen(fileName, "wb")) == NULL)
		printf("Nie powiodlo sie otworzenie pliku do zapisu\n");
	else {
		fprintf(fileHandle, "%d\n", readCount);
		for(i=0;i<readCount;i++)
			fprintf(fileHandle, "%d:%d\n", huffmanModelArray[i].symbol, huffmanModelArray[i].freq);
		fclose(fileHandle);
	}
}

HuffmanNode* newNode(int symbol, int freq) {
	HuffmanNode* temp = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    temp->left = temp->right = NULL;
    temp->symbol = symbol;
    temp->freq = freq;
    return temp;
}

void swapNode(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
	int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if(left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;
    if(right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;
    if(smallest != idx) {
        swapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }	
}

MinHeap* createAndBuildMinHeap(HuffmanNode *huffmanModelArray, int readCount) {
	int i;
	MinHeap *minHeap = (MinHeap*)malloc(sizeof(MinHeap));
	minHeap->size = 0;
	minHeap->capacity = readCount;
	minHeap->array = (HuffmanNode**)malloc(minHeap->capacity * sizeof(HuffmanNode*));
	for(i=0;i<readCount;i++)
		minHeap->array[i] = newNode(huffmanModelArray[i].symbol, huffmanModelArray[i].freq);
	minHeap->size = readCount;
	int n = minHeap->size - 1;
	for(i=(n-1)/2;i>=0;i--)
        minHeapify(minHeap, i);
	return minHeap;
}

HuffmanNode* extractMin(MinHeap* minHeap) {
    HuffmanNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, HuffmanNode* huffmanNode) {
    minHeap->size++;
    int i = minHeap->size - 1;
    while(i && huffmanNode->freq < minHeap->array[(i - 1) / 2]->freq) {
		minHeap->array[i] = minHeap->array[(i - 1) / 2];
		i = (i - 1) / 2;
    }
    minHeap->array[i] = huffmanNode;
}

HuffmanNode* CreateTree(HuffmanNode *huffmanModelArray, int readCount) {
	int i=-1;
	HuffmanNode *left, *right, *parent;
	MinHeap *minHeap = createAndBuildMinHeap(huffmanModelArray, readCount);
	while (minHeap->size != 1) {
		left = extractMin(minHeap);
        right = extractMin(minHeap);
        parent = newNode(i--, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        insertMinHeap(minHeap, parent);
	}
	return extractMin(minHeap);
}

void WriteTreeIntoFile(HuffmanNode *codeTree, int parent, FILE **fileHandle) {
	if(codeTree->symbol < 0)
		fprintf(*fileHandle, "#%d:%d", -codeTree->symbol, codeTree->freq);
	else
		fprintf(*fileHandle, "%d:%d", codeTree->symbol, codeTree->freq);
	fprintf(*fileHandle, "\tchildLeft:");
	if(codeTree->left)
		if(codeTree->left->symbol < 0)
			fprintf(*fileHandle, "#%d", -codeTree->left->symbol);
		else
			fprintf(*fileHandle, "%d", codeTree->left->symbol);
	else
		fprintf(*fileHandle, "-");
	fprintf(*fileHandle, "\tchildRight:");
	if(codeTree->right)
		if(codeTree->right->symbol < 0)
			fprintf(*fileHandle, "#%d", -codeTree->right->symbol);
		else
			fprintf(*fileHandle, "%d", codeTree->right->symbol);
	else
		fprintf(*fileHandle, "-");
	fprintf(*fileHandle, "\tParent:");
	if(parent < 0)
		fprintf(*fileHandle, "#%d\n", -parent);
	else if(parent == 0)
		fprintf(*fileHandle, "-\n");
	else
		fprintf(*fileHandle, "%d\n", codeTree->symbol);
	if(codeTree->left != NULL) WriteTreeIntoFile(codeTree->left, codeTree->symbol, fileHandle);
	if(codeTree->right != NULL) WriteTreeIntoFile(codeTree->right, codeTree->symbol, fileHandle);
}

void WriteCodeTableIntoFile(HuffmanNode *codeTree, int *temp, int x, FILE *fileHandle) {
	if(codeTree->left) {
		temp[x] = 0;
		WriteCodeTableIntoFile(codeTree->left, temp, x+1, fileHandle);
	}
	if(codeTree->right) {
		temp[x] = 1;
		WriteCodeTableIntoFile(codeTree->right, temp, x+1, fileHandle);
	}
	if(!(codeTree->left || codeTree->right)) {
		fprintf(fileHandle, "%d-", codeTree->symbol);
		int i;
		for(i=0;i<x;i++)
			fprintf(fileHandle, "%d", temp[i]);
		fprintf(fileHandle, "\n");
	}
}

void FreeTree(HuffmanNode *codeTree) {
	if(codeTree->left)
		FreeTree(codeTree->left);
	if(codeTree->right)
		FreeTree(codeTree->right);
	free(codeTree);
}

CodeTable *ReadCodeTableFromFile(char *fileName, int *readCount) {
	FILE *fileHandle;
	CodeTable *codeTable;
	unsigned char buffer[1];
	int i, mode = 1;
	*readCount = 0;
	if((fileHandle = fopen(fileName, "rb")) == NULL) {
		printf("Nie znaleziono pliku z danymi!\n");
		return NULL;
	}
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle)) {
		if(buffer[0] == 13)
			break;
		*readCount = (*readCount) * 10 + buffer[0] - 48;
	}
	codeTable = (CodeTable*) malloc (sizeof(CodeTable)*(*readCount));
	for(i=0;i<*readCount;i++) {
		codeTable[i].symbol = 0;
		codeTable[i].code = 0;
		codeTable[i].size = 0;
	}
	i = 0;
	while(i < *readCount) {
		fread(buffer, sizeof(unsigned char), 1, fileHandle);
		if(buffer[0] == 10)
			continue;
		if(buffer[0] == 13) {
			i++;
			mode = 1;
			continue;
		}
		if(buffer[0] == '-') {
			mode = 0;
			continue;
		}
		if(mode)
			codeTable[i].symbol = codeTable[i].symbol * 10 + buffer[0] - 48;
		else {
			codeTable[i].size++;
			codeTable[i].code = codeTable[i].code << 1;
			codeTable[i].code += buffer[0] - 48;
		}
	}
	fclose(fileHandle);
	return codeTable;
}

int WriteCompressedFile(char *fileName, char *fileNameInput, int readCount, CodeTable *codeTable) {
	FILE *input, *output;
	if((input = fopen(fileNameInput, "rb")) == NULL) {
		printf("Nie znaleziono pliku z danymi!\n");
		return 1;
	}
	if((output = fopen(fileName, "wb")) == NULL) {
		printf("Nie udalo sie utworzyc pliku!\n");
		return 1;
	}
	unsigned char buffer[1], temp = 0;
	int i, x, temp_size = 0;
	while(fread(buffer, sizeof(unsigned char), 1, input)) {
		for(i=0;i<readCount;i++) {
			if(codeTable[i].symbol == buffer[0]) {
				if(temp_size + codeTable[i].size < 8) {
					temp = temp << codeTable[i].size;
					temp += codeTable[i].code;
					temp_size += codeTable[i].size;
				} else if(temp_size + codeTable[i].size < 16) {
					temp = temp << 8 - temp_size;
					temp += codeTable[i].code >> codeTable[i].size + temp_size - 8;
					fprintf(output, "%c", temp);
					temp = codeTable[i].code;
					temp_size = codeTable[i].size + temp_size - 8;
				} else {
					temp = temp << 8 - temp_size;
					temp += codeTable[i].code >> codeTable[i].size + temp_size - 8;
					fprintf(output, "%c", temp);
					temp = codeTable[i].code >> codeTable[i].size + temp_size - 16;
					fprintf(output, "%c", temp);
					temp = codeTable[i].code;
					temp_size = codeTable[i].size + temp_size - 16;
				}
			}
		}
	}
	if(temp_size) {
		temp = temp << 8 - temp_size;
		fprintf(output, "%c", temp);
	}
	fclose(input);
	fclose(output);
	return 0;
}

HuffmanNodesList* CreateListNode(FILE *fileHandle) {
	unsigned char buffer[1];
	HuffmanNodesList *codeTree = (HuffmanNodesList*) malloc(sizeof(HuffmanNodesList));;
	codeTree->next = NULL;
	if(fread(buffer, sizeof(unsigned char), 1, fileHandle) == 0)
		return NULL;
	if(buffer[0] == '#') {
		fread(buffer, sizeof(unsigned char), 1, fileHandle);
		codeTree->symbol = -(buffer[0] - 48);
	} else
		codeTree->symbol = buffer[0] - 48;
	//symbol
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle)) {
		if(buffer[0] == ':')
			break;
		if(codeTree->symbol > 0)
			codeTree->symbol = codeTree->symbol * 10 + buffer[0] - 48;
		else
			codeTree->symbol = codeTree->symbol * 10 - (buffer[0] - 48);
	}
	//frequency
	fread(buffer, sizeof(unsigned char), 1, fileHandle);
	codeTree->freq = buffer[0] - 48;
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle)) {
		if(buffer[0] == '\t')
			break;
		codeTree->freq = codeTree->freq * 10 + buffer[0] - 48;
	}
	//left
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle))
		if(buffer[0] == ':')
			break;
	fread(buffer, sizeof(unsigned char), 1, fileHandle);
	if(buffer[0] == '-')
		codeTree->left_symbol = 0;
	else if(buffer[0] == '#') {
		fread(buffer, sizeof(unsigned char), 1, fileHandle);
		codeTree->left_symbol = -(buffer[0] - 48);
	} else
		codeTree->left_symbol = buffer[0] - 48;
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle)) {
		if(buffer[0] == '\t')
			break;
		if(codeTree->left_symbol > 0)
			codeTree->left_symbol = codeTree->left_symbol * 10 + buffer[0] - 48;
		else
			codeTree->left_symbol = codeTree->left_symbol * 10 - (buffer[0] - 48);
	}
	//right
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle))
		if(buffer[0] == ':')
			break;
	fread(buffer, sizeof(unsigned char), 1, fileHandle);
	if(buffer[0] == '-')
		codeTree->right_symbol = 0;
	else if(buffer[0] == '#') {
		fread(buffer, sizeof(unsigned char), 1, fileHandle);
		codeTree->right_symbol = -(buffer[0] - 48);
	} else
		codeTree->right_symbol = buffer[0] - 48;
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle)) {
		if(buffer[0] == '\t')
			break;
		if(codeTree->right_symbol > 0)
			codeTree->right_symbol = codeTree->right_symbol * 10 + buffer[0] - 48;
		else
			codeTree->right_symbol = codeTree->right_symbol * 10 - (buffer[0] - 48);
	}
	//parent - nie potrzebny, pomijam
	while(fread(buffer, sizeof(unsigned char), 1, fileHandle))
		if(buffer[0] == '\n')
			break;
	return codeTree;
}

HuffmanNodesList* CreateList(FILE *fileHandle) {
	HuffmanNodesList *head = CreateListNode(fileHandle);
	HuffmanNodesList *temp = head;
	while(!feof(fileHandle)) {
		temp->next = CreateListNode(fileHandle);
		if(temp->next != NULL)
			temp = temp->next;
	}
	return head;
}

HuffmanNode* SearchNode(HuffmanNodesList *list, int symbol) {
	HuffmanNodesList *tempList = list;
	while(tempList != NULL) {
		if(tempList->symbol == symbol) {
			HuffmanNode *codeTree = (HuffmanNode*) malloc(sizeof(HuffmanNode));
			codeTree->symbol = tempList->symbol;
			codeTree->freq = tempList->freq;
			codeTree->left = (tempList->left_symbol == 0)? NULL : SearchNode(list, tempList->left_symbol);
			codeTree->right = (tempList->right_symbol == 0)? NULL : SearchNode(list, tempList->right_symbol);
			return codeTree;
		} else
			tempList = tempList->next;
	}
	return NULL;
}

HuffmanNode* ReadTreeFromFile(FILE *fileHandle) {
	HuffmanNodesList *list = CreateList(fileHandle);
	HuffmanNode *codeTree = (HuffmanNode*) malloc(sizeof(HuffmanNode));
	codeTree->symbol = list->symbol;
	codeTree->freq = list->freq;
	codeTree->left = list->left_symbol == '-' ? NULL : SearchNode(list, list->left_symbol);
	codeTree->right = list->right_symbol == '-' ? NULL : SearchNode(list, list->right_symbol);
	while(list != NULL) {
		HuffmanNodesList *tempList = list;
		list = list->next;
		free(tempList);
	}
	return codeTree;
}

int WriteDecompressedFile(char *fileNameInput, char *fileNameOutput, HuffmanNode *codeTree) {
	FILE *input, *output;
	if((input = fopen(fileNameInput, "rb")) == NULL) {
		printf("Nie powiodlo sie otworzenie pliku do odczytu\n");
		return 1;
	}
	unsigned char temp, buffer[0];
	int readCount = 0, tempSize = 0;
	int *decode = (int*) malloc(sizeof(int) * codeTree->freq);
	HuffmanNode *tempTree = codeTree;
	while(readCount < codeTree->freq) {
		if(tempSize == 0) {
			fread(buffer, sizeof(unsigned char), 1, input);
			temp = buffer[0];
			tempSize = 8;
		}
		if((temp >> tempSize - 1)%2)
			tempTree = tempTree->right;
		else
			tempTree = tempTree->left;
		tempSize--;
		if(tempTree->left == NULL && tempTree->right == NULL) {
			decode[readCount] = tempTree->symbol;
			readCount++;
			tempTree = codeTree;
		}
	}
	fclose(input);
	if((output = fopen(fileNameOutput, "wb")) == NULL) {
		printf("Nie powiodlo sie otworzenie pliku do zapisu\n");
		return 1;
	}
	for(readCount = 0; readCount < codeTree->freq; readCount++)
		fprintf(output, "%c", decode[readCount]);
	free(decode);
	fclose(output);
	return 0;
}

int main() {
	int i, readBytesLength = 1, readCount;
	char fileName[50], fileNameOutput[50], fileNameInput[50];
	HuffmanNode *codeTree;
	FILE *fileHandle;
	do {
		readCount = 0;
		printf("TiK Laboratorium nr 3\n1. Kompresja\n2. Dekompresja\n3. Wyjscie\n-->");
		scanf("%d", &i);
		if(i == 1) {
			printf("Plik wejsciowy: ");
			scanf("%s", fileNameInput);
			//strcpy(fileNameInput, "test2.txt");
			HuffmanNode *huffmanModelArray = (HuffmanNode*) malloc(sizeof(HuffmanNode));
			huffmanModelArray = CreateDataModel(huffmanModelArray, fileNameInput, readBytesLength, &readCount);
			if(huffmanModelArray == NULL) {
				free(huffmanModelArray);
				continue;
			}
			printf("Plik wyjsciowy: ");
			scanf("%s", fileName);
			//strcpy(fileName, "wynik");
			strcpy(fileNameOutput, fileName);
			strcat(fileNameOutput,".model");
			WriteModelIntoFile(fileNameOutput, huffmanModelArray, readCount);
			printf("\nModel zrodla zostal zapisany do pliku\n");
			codeTree = CreateTree(huffmanModelArray, readCount);
			strcpy(fileNameOutput, fileName);
			strcat(fileNameOutput,".graf");
			if((fileHandle = fopen(fileNameOutput, "w")) == NULL)
				printf("Nie powiodlo sie otworzenie pliku do zapisu\n");
			else {
				WriteTreeIntoFile(codeTree, 0, &fileHandle);
				fclose(fileHandle);
				printf("Drzewo kodowania + ");
			}
			strcpy(fileNameOutput, fileName);
			strcat(fileNameOutput,".code");
			if((fileHandle = fopen(fileNameOutput, "w")) == NULL)
				printf("Nie powiodlo sie otworzenie pliku do zapisu\n");
			else {
				int *temp = (int*) malloc(sizeof(int) * readCount);
				fprintf(fileHandle, "%d\n", readCount);
				WriteCodeTableIntoFile(codeTree, temp, 0, fileHandle);
				free(temp);
				fclose(fileHandle);
				printf("tablica kodowa zostaly zapisane do pliku\n");
			}
			free(huffmanModelArray);
			FreeTree(codeTree);
			CodeTable *codeTable = ReadCodeTableFromFile(fileNameOutput, &readCount);
			if(WriteCompressedFile(fileName, fileNameInput, readCount, codeTable))
				continue;
			printf("Zapisano skompresowany plik\n\n");
			free(codeTable);
		} else if(i == 2) {
			printf("Plik wejsciowy: ");
			scanf("%s", fileName);
			//strcpy(fileName, "wynik");
			printf("Plik wyjsciowy: ");
			scanf("%s", fileNameOutput);
			//strcpy(fileNameOutput, "wynik.txt");
			strcpy(fileNameInput, fileName);
			strcat(fileNameInput,".graf");
			if((fileHandle = fopen(fileNameInput, "rb")) == NULL)
				printf("Nie powiodlo sie otworzenie pliku do odczytu\n");
			else {
				codeTree = ReadTreeFromFile(fileHandle);
				fclose(fileHandle);
			}
			if(WriteDecompressedFile(fileName, fileNameOutput, codeTree)) {
				printf("\bBlad dekompresji!\n\n");
				FreeTree(codeTree);
				continue;
			}
			printf("\nZdekodowano dane\n\n");
			FreeTree(codeTree);
		}
	} while(i != 3);
	return 0;
}
