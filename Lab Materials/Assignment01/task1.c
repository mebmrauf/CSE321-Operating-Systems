#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max 500

void main(int argc, char *argv[]) {
    if (argc >= 2) {
        FILE *file = fopen(argv[1], "w");
        if (file == NULL) {
            printf("Error opening/creating the file");
            return;
        }

        char input[max];
        printf("Enter strings to write (enter '-1' to stop):\n");

        while (1) {
            printf("> ");
            if (fgets(input, max, stdin) == NULL) {
                break;
            }

            input[strcspn(input, "\n")] = '\0';
            if (strcmp(input, "-1") == 0) {
                break;
            }
            fprintf(file, "%s\n", input);
        }

        fclose(file);
    } else {
        printf("Please enter a filename.\n");
    }
}