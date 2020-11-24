/* @author: Adam Zvara
 * @login: xzvara01
 * @file: sps.c
 * @date: 24.10.2020
 * @brief: A short program for parsing and editing a file - table 
 */
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

//Pack argv and argc into one structure Targs
typedef struct {
    char **argv;
    int argc;
} Targs;

/* Set default values to an empty structure (cell, row, table)
 * @return : empty initialized structure (cell, row)
 */
Tcell cell_init(){
    Tcell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    return new_cell;
}   

// @see: cell_init()
Trow row_init(){
    Trow new_row;
    new_row.size = new_row.cap = 0;
    new_row.cells = NULL;
    return new_row;
}

// @see row_init function
void table_init(Ttable *table){
    table->size = table->cap = 0;
    table->cap = 0;
    table->rows = NULL;
}

/*
 * Make (cell, row, table) structure larger - increase its capacity 
 * @param: pointer to structure (cell, row, table) to change its capacity
 * @param: new capacity
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

// @see: cell_resize
 
void row_resize(Trow *row, int cells_n){
    void *resized;
    resized = realloc(row->cells, cells_n * sizeof(Tcell));

    if (resized != NULL){
        row->cells = resized;
        row->cap = cells_n;
    }
}

// @see: cell_resize
void table_resize(Ttable *table, int rows_n){
    void *resized;
    resized = realloc(table->rows, rows_n * sizeof(Trow));

    if (resized != NULL){
        table->rows = resized;
        table->cap = rows_n;
    }
}

/*
 * Append single (character, cell, row) to an existing (cell, row, table)
 * @param: pointer to structure (cell, row, table)
 * @param: (character, cell, row) to append
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

// @see: cell_append()
void row_append(Trow *row, int current_cell){
    if (row->cap == row->size){
        row_resize(row, row->cap + 1);
    }
    if (row->size < row->cap){
        row->cells[current_cell] = cell_init();
        row->size++;
    }
}

// @see: cell_append()
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
 * Print the content of (cell, table, row)
 * @param: pointer to structure (cell, row, table)
 */
void cell_print(Tcell *cell){
    for (int i = 0; i < cell->size; i++){
        printf("%c", cell->text[i]);
    }
}

// @see: cell_print
// @params: delim to separate the cells
void row_print(Trow *row, char delim){
    for (int i = 0; i < row->size; i++){
        cell_print(&row->cells[i]);
        //put delimiter after each cell
        if (i != row->size-1){
            putchar(delim);
        }
    }
} 

// @see: cell_print
// @params: parsing delim so i can print it in row_print
void table_print(Ttable *table, char delim){
    for (int i = 0; i < table->size; i++){
        row_print(&table->rows[i], delim);
        if (i != table->size-1){
            //put newline character at the end of a row
            putchar('\n');
        }
    }
}

/*
 * Free pointer to a (cell, row, table)
 * @param: pointer to structure (cell, row, table)
 */
void cell_destroy(Tcell *cell){
    if (cell->cap)
        free(cell->text);
}

// @see: cell_destroy
void row_destroy(Trow *row){
    for (int i = 0; i < row->size; i++){
        cell_destroy(&row->cells[i]);
    }

    if (row->cap)
        free(row->cells);
}

// @see: cell_destroy
void table_destroy(Ttable *table){
    for (int i = 0; i < table->size; i++){
        row_destroy(&table->rows[i]);
    }

    if (table->cap){
        free(table->rows);
    }
}


/*
 * Checks if char is a delim
 * @params: character to check
 * @params: delimiters
 * @return: true if successful
 * @return: false if unsuccessful
 */
bool isdelim(char c, char *delims){
    for (size_t i = 0; i < strlen(delims); i++){
        if (c == delims[i]) {
            return true;
        }
    }
    return false;
}

void create_table(Ttable *table, FILE *fr, char *delims){
    int c, current_cell = 0, current_row = 0;
    while ((c = fgetc(fr)) != EOF){
        if (table->rows == NULL){
            table_append(table, current_row);
        }

        if (c == '\n'){ 
            current_cell = 0;
            current_row++;
            table_append(table, current_row); 
            continue;
        } 

        if (table->rows[current_row].cells == NULL){
            row_append(&table->rows[current_row], current_cell);
        }   
        
        if (isdelim(c, delims)){
            current_cell++;
            row_append(&table->rows[current_row], current_cell);
            continue;
        }
 
        cell_append(&table->rows[current_row].cells[current_cell], c);
    }

}

/*
 * Function for printing the table and freeing its content
 * @param: pointer to a structure (table)
 * @param: MAIN DELIMITER - first delimiter in delims
 */
void end(Ttable *table, char delim){
    table_print(table, delim);
    table_destroy(table);
}

/*
 * Locate delim in program arguments 
 * @params: args structure
 * @return: string with space or string of delimiters
 */
char * find_delim(const Targs args){
    char *delim = " ";
    if (args.argc >= 3){
        if (!strcmp(args.argv[1], "-d")){
            delim = args.argv[2];
        } 
    }
    return delim;
}

/*
 * Open a file and return pointer to it
 * @params: name of a file
 * @return: pointer to an opened file
 */
FILE * open_file(char *name){
    FILE *fr;
    fr = fopen(name, "r");
    return fr;
}   


int main(int argc, char **argv){
    if (argc < 3){
        //TODO print error
        printf("malo argumentov\n");
        return 1;
    }

   
    Targs args = {argv, argc};

    char *delims = find_delim(args);
    (void) delims;
    
    FILE *fr;
    fr = open_file(argv[argc-1]);
    if (fr == NULL){
        //TODO error while opening the file
        printf("no file was opened\n");
        return 1;
    }
    
    Ttable table;
    table_init(&table);
    create_table(&table, fr, delims);
   
    end(&table, delims[0]);
    return 0;
}


