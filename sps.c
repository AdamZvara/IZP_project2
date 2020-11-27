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

#define DELIM delims[0]

//Structure for cells in rows
typedef struct {
    int size;
    int cap;
    char *text;
} Cell;

//Strucure for rows in table
typedef struct {
    int size;
    int cap;
    Cell *cells;
} Row;

//Table structure
typedef struct {
    int size;
    int cap;
    Row *rows;
} Table;

//Pack argv and argc into one structure Targs
typedef struct {
    char **argv;
    int argc;
} Args;

typedef struct {
    char *content;
} Commands;

typedef struct {
    int r_start;
    int r_end;
    int c_start;
    int c_end;
} Selection;

/* Set default values to an empty structure (cell, row, table)
 * @return : empty initialized structure (cell, row)
 */
Cell cell_init(){
    Cell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    return new_cell;
}   

// @see: cell_init()
Row row_init(){
    Row new_row;
    new_row.size = new_row.cap = 0;
    new_row.cells = NULL;
    return new_row;
}

// @see row_init function
void table_init(Table *table){
    table->size = table->cap = 0;
    table->cap = 0;
    table->rows = NULL;
}

/*
 * Make (cell, row, table) structure larger - increase its capacity 
 * @param: pointer to structure (cell, row, table) to change its capacity
 * @param: new capacity
 */
void cell_resize(Cell *cell, int new_cap){
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
 
void row_resize(Row *row, int cells_n){
    void *resized;
    resized = realloc(row->cells, cells_n * sizeof(Cell));

    if (resized != NULL){
        row->cells = resized;
        row->cap = cells_n;
    }
}

// @see: cell_resize
void table_resize(Table *table, int rows_n){
    void *resized;
    resized = realloc(table->rows, rows_n * sizeof(Row));

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
void cell_append(Cell *cell, char item){
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
void row_append(Row *row, int current_cell){
    if (row->cap == row->size){
        row_resize(row, row->cap + 1);
    }
    if (row->size < row->cap){
        row->cells[current_cell] = cell_init();
        row->size++;
    }
}

// @see: cell_append()
void table_append(Table *table, int current_row){
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
void cell_print(Cell *cell, FILE *dst){
    for (int i = 0; i < cell->size; i++){
        fputc(cell->text[i], dst);
    }
}

// @see: cell_print
// @params: delim to separate the cells
void row_print(Row *row, char delim, FILE *dst){
    for (int i = 0; i < row->size; i++){
        cell_print(&row->cells[i], dst);
        //put delimiter after each cell
        if (i != row->size-1){
            fputc(delim, dst);
        }
    }
} 

// @see: cell_print
// @params: passing delim for printing it in row_print
void table_print(Table *table, char delim, FILE *dst){
    for (int i = 0; i < table->size; i++){
        row_print(&table->rows[i], delim, dst);
        if (i != table->size-1){
            fputc('\n', dst);
        }
    } fputc('\n', dst);
}

/*
 * Free pointer to a structure (cell, row, table)
 * @param: pointer to structure (cell, row, table)
 */
void cell_destroy(Cell *cell){
    if (cell->cap)
        free(cell->text);
}

// @see: cell_destroy
void row_destroy(Row *row){
    for (int i = 0; i < row->size; i++){
        cell_destroy(&row->cells[i]);
    }

    if (row->cap)
        free(row->cells);
}

// @see: cell_destroy
void table_destroy(Table *table){
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


/*
 * Take input from a file and insert it into table
 * @params table: pointer to a table structre
 * @params: source file
 * @params: delimiters
 */
void create_table(Table *table, FILE *source, char *delims){
    int c, current_cell = 0, current_row = 0;
    int quotes_active = -1;

    while ((c = fgetc(source)) != EOF){
        if (table->rows == NULL){
            table_append(table, current_row);
        }
        if (table->rows[current_row].cells == NULL){
            row_append(&table->rows[current_row], current_cell);
        }

        if (c == '\n'){ 
            current_cell = 0;
            current_row++;
            table_append(table, current_row); 
            continue;
        } else if (c == '"'){
            quotes_active *= -1;
        } else if (c == '\\'){           
            cell_append(&table->rows[current_row].cells[current_cell], c);
            if ((c = fgetc(source)) != EOF){
                cell_append(&table->rows[current_row].cells[current_cell], c);
                continue;
            }
        }

        if (isdelim(c, delims) && quotes_active == -1){
            current_cell++;
            row_append(&table->rows[current_row], current_cell);
            continue;
        }
 
        cell_append(&table->rows[current_row].cells[current_cell], c);
    }

    //delete last row
    row_destroy(&table->rows[table->size]-1);
    table->size--;
}

/*
 * Function for printing the table and freeing its content
 * @params table: pointer to a table structre
 * @param: MAIN DELIMITER - first delimiter in delims
 */
void end(Table *table, char delim, FILE *dst){
    (void) dst;
    table_print(table, delim, stdout);//TODO change to dst
    table_destroy(table);
}

/*
 * Locate delim in program arguments 
 * @params args: agrv, argc in one structure
 * @return: string with space or string of delimiters
 */
char * find_delim(const Args args){
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
 * @params src: source file name
 * @return: pointer to an opened file
 */
FILE * open_file(char *src){
    FILE *fr;
    fr = fopen(src, "r+");
    return fr;
}   

/*
 * Get length of the longest row
 * @params table: pointer to a table structre
 * @return: maximal row size in a table
 */
int get_max_row(Table table){
    int max_row = 0;
    
    for (int i = 0; i < table.size; i++){
        if (table.rows[i].size > max_row){
            max_row = table.rows[i].size;        
        }
    }

    return max_row;
}

/*
 * Fill the table with empty cells so each row has equal ammount of cells
 * @params table: pointer to a table structre
 */
void fill_table(Table *table){
    int max_row = get_max_row(*table);
    
    for (int i = 0; i < table->size; i++){
        while (table->rows[i].size != max_row){
            row_append(&table->rows[i], table->rows[i].size);
        }
    }  
}

//FIXME pracovat na ulozeni argumentov do struktury a rozdelenie podla ;
void get_commands(Args args, Commands *cmds){
    cmds->content = malloc(20);
    for (size_t i = 0; i < strlen(args.argv[1]); i++){
        /*if (i == ';'){
            //new command found
        }*/
        if (strlen(cmds->content) < )
        cmds->content[i] = args.argv[1][i];
    }
    cmds->content[19] = '\0';
    printf("%s\n", cmds->content);
}

int main(int argc, char **argv){
    if (argc < 3){
        //TODO print error
        printf("malo argumentov\n");
        return 1;
    }
   
    Args args = {argv, argc};
    char *delims = find_delim(args);
    Table table;
    table_init(&table);
    Commands cmds = {NULL};

    FILE *fr;
    fr = open_file(argv[argc-1]);
    if (fr == NULL){
        //TODO error while opening the file
        printf("no file was opened\n");
        return 1;
    }
   
    create_table(&table, fr, delims);    
    fill_table(&table);

    get_commands(args, &cmds);

    rewind(fr);
    end(&table, DELIM, fr);
    fclose(fr);
    return 0;
}


