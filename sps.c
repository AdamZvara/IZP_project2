#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *text;
    int length;
} cell_t;

typedef struct {
    cell_t *cells;
} row_t;

void initialize_cell(row_t row){
    cell_t new_cell;
    new_cell.text = malloc(sizeof(char));
    new_cell.length = 0;
    row.cells = malloc(sizeof(cell_t));
}

int main(){
    int c = 0, len = 0, counter = -1;
    row_t row;

    while (true){
        if (c == '\n') break;
        if (!c || c == ' '){ //new cell in a row
            len = 0;
            initialize_cell(row);
            counter++;
        }
        len++;

        /*
            new_cell.text = realloc(a.text, len * sizeof(char));
            a.text[len-1] = c;
            a.length = len;
            b.text = realloc(b.text, len * sizeof(char));
            b.text[len-1] = c;
            b.length = len;
        
        row.cells[0] = a;
        */
    }

    printf("%s %d\n", row.cells[0].text, row.cells[0].length);
    //printf("%s %d\n", b.text, b.length);
    
    return 0;
}
