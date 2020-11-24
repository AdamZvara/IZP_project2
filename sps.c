#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Structure for cells in rows
typedef struct {
    int size;
    int cap;
    char *text;
} Tcell;

//Strucure for rows in table
typedef struct {
    int size;
    int cap;
    Tcell *cells;
} Trow;

//Table structure
typedef struct {
    int size;
    int cap;
    Trow *rows;
} Ttable;

typedef struct {
    char **argv;
    int argc;
} Targs;

/* Set default values to an empty cell
 * @return : empty cell 
 */
Tcell cell_init(){
    Tcell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    return new_cell;
}   

/* Set default values of an empty row
 * @return : empty row
 */
Trow row_init(){
    Trow new_row;
    new_row.size = 0;
    new_row.cap = 0; 
    new_row.cells = NULL;
    return new_row;
}

//Same as row_init function
void table_init(Ttable *table){
    table->size = 0;
    table->cap = 0;
    table->rows = NULL;
}

/*
 * Change cell capacity to a higher number
 * @param : pointer to cell to change its capacity
 * @param : new capacity
 */
void cell_resize(Tcell *cell, int new_cap){
    char *resized;
    resized = realloc(cell->text, new_cap * sizeof(char));
    if (resized != NULL){
        cell->text = resized;
        cell->cap = new_cap;
        for (int i = cell->size; i < cell->cap; i++){ //initialize all chars to '\'
            cell->text[i] = '\0';
        }
    }
}

/*
 * Change row capacity to a higher number
 * @param : pointer to row
 * @param : new row capacity
 */
void row_resize(Trow *row, int cells_n){
    void *resized;
    resized = realloc(row->cells, cells_n * sizeof(Tcell));

    if (resized != NULL){
        row->cells = resized;
        row->cap = cells_n;
    }
}

/*
 * Same as row_resize
 */
void table_resize(Ttable *table, int rows_n){
    void *resized;
    resized = realloc(table->rows, rows_n * sizeof(Trow));

    if (resized != NULL){
        table->rows = resized;
        table->cap = rows_n;
    }
}

/*
 * Append single character to an existing cell
 * @param : pointer to cell
 * @param : character to append
 */
void cell_append(Tcell *cell, char item){
    if (cell->cap == cell->size){
    
        //if cell is empty, set its capacity to 1
        //if cell isnt empty, double the capacity
        cell_resize(cell, cell->cap ? cell->cap * 2 : 1);     
    }
    if (cell->cap > cell->size){
        cell->text[cell->size] = item;
        cell->size++;
    }
}
/*
 * Same as cell_append function
 */ 
void row_append(Trow *row, int current_cell){
    if (row->cap == row->size){
        row_resize(row, row->cap + 1);
    }
    if (row->size < row->cap){
        row->cells[current_cell] = cell_init();
        row->size++;
    }
}

/*
 * Same as row_append function
 */
void table_append(Ttable *table, int current_row){
    if (table->cap == table->size){
        table_resize(table, current_row+1);
    }
    if (table->size < table->cap){
        table->rows[current_row] = row_init();
        table->size++;
    }
}

/*
 * Print the content of cell
 * @param : pointer to cell
 */
void cell_print(Tcell *cell){
    for (int i = 0; i < cell->size; i++){
        printf("%c", cell->text[i]);
    }
}

/*
 * Same as cell print function
 */
void row_print(Trow *row){
    for (int i = 0; i < row->size; i++){
        cell_print(&row->cells[i]);
        //put delimiter after each cell
        if (i != row->size-1){
            putchar(':');
        }
    }
} 

void table_print(Ttable *table){
    for (int i = 0; i < table->size; i++){
        row_print(&table->rows[i]);
        if (i != table->size-1){
            //put newline character at the end of a row
            putchar('\n');
        }
    }
}

/*
 * Free pointer to a cell
 * @param : pointer to cell
 */
void cell_destroy(Tcell *cell){
    if (cell->cap)
        free(cell->text);
}

/*
 * Same as cell_destroy function
 */
void row_destroy(Trow *row){
    for (int i = 0; i < row->size; i++){
        cell_destroy(&row->cells[i]);
    }

    if (row->cap)
        free(row->cells);
}

/*
 * Same as row_destroy function
 */
void table_destroy(Ttable *table){
    for (int i = 0; i < table->size; i++){
        row_destroy(&table->rows[i]);
    }

    if (table->cap){
        free(table->rows);
    }
}

void end(Ttable *table){
    table_print(table);
    table_destroy(table);
}

char * find_delim(const Targs args){
    char *delim = " ";
    if (args.argc >= 3){
        if (!strcmp(args.argv[1], "-d")){
            delim = args.argv[2];
        } 
    }
    return delim;
}

int main(int argc, char **argv){
    Ttable table;
    table_init(&table);

    Targs args = {argv, argc};

    char *delims = find_delim(args);
    (void) delims;
    int c, current_cell = 0, current_row = 0;
    while ((c = fgetc(stdin)) != EOF){
        if (table.rows == NULL){
            table_append(&table, current_row);
        }
        
        if (c == '\n'){ 
            current_cell = 0;
            current_row++;
            table_append(&table, current_row); 
            continue;
        } 

        if (table.rows[current_row].cells == NULL){
            row_append(&table.rows[current_row], current_cell);
        }   
        
        if (c == ' '){
            current_cell++;
            row_append(&table.rows[current_row], current_cell);
            continue;
        }
 
        cell_append(&table.rows[current_row].cells[current_cell], c);
    }

    end(&table);
    return 0;
}


