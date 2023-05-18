#include <stdio.h>

void stringLength(char* str);

int main() {
	char input[] = "Hello, World!";
	stringLength(input);
	return 0;
}

void stringLength(char* str) {
    int length = 0;

    while (str[length] != '\0') {
        length++;
    }

    printf("%d",length);
}