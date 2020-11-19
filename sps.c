#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int size;
    int cap;
    char *text;
} Tcell;

typedef struct {
    int size;
    int cap;
    Tcell *cells;
} Trow;



//initialize empty row 
void row_init(Trow *row){
    row->size = 0;
    row->cap = 0; 
    row->cells = NULL;
}

//initialize empty cell
Tcell cell_init(){
    Tcell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    return new_cell;
}   

//make cell bigger, if the size is same as capacity 
void cell_resize(Tcell *cell, int new_cap){
    char *resized;
    resized = realloc(cell->text, new_cap * sizeof(char));
    if (resized != NULL){
        cell->text = resized;
        cell->cap = new_cap;
    }
}

//append a character to a cell and increase its size
void cell_append(Tcell *cell, char item){
    if (cell->cap == cell->size){
        cell_resize(cell, cell->cap ? cell->cap * 2 : 1);
    }
    if (cell->cap > cell->size){
        cell->text[cell->size] = item;
        cell->size++;
    }
}

void cell_print(Tcell *cell){
    for (int i = 0; i < cell->size; i++){
        printf("%c", cell->text[i]);
    }  putchar('\n');

}

void cell_destroy(Tcell *cell){
    if (cell->cap)
        free(cell->text);
}

//expand row by a number of cells
void row_expand(Trow *row, int cells_n){
    void *resized;
    resized = realloc(row->cells, cells_n * sizeof(Tcell));

    if (resized != NULL){
        row->cells = resized;
        row->cap = cells_n;
    }
}

void row_append(Trow *row, int current_cell){
    if (row->cap == row->size){
        row_expand(row, row->cap ? row->cap * 2 : 1);
    }
    if (row->size < row->cap){
        row->cells[current_cell] = cell_init();
        row->size++;
    }
}

void row_print(Trow *row){
    for (int i = 0; i < row->size; i++){
        cell_print(&row->cells[i]);
    }
}

void row_destroy(Trow *row){
    for (int i = 0; i < row->size; i++){
        cell_destroy(&row->cells[i]);
    }

    if (row->cap)
        free(row->cells);
}

int main(){
    Trow row;
    row_init(&row);

    int c, current_cell = 0;
    while ((c = fgetc(stdin)) != '\n'){
        if (c == ' '){
            current_cell++;
            row_append(&row, current_cell);
            continue;
        }
        if (row.cells == NULL){
            row_append(&row, current_cell);
        }    
        cell_append(&row.cells[current_cell], c);
    }

    row_print(&row);
    printf("%d ", row.cap);
    row_destroy(&row);
    return 0;
}


