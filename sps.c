/*
 * @author: Adam Zvara
 * @login: xzvara01
 * @file: sps.c
 * @date: 5.12.2020
 * @brief: Program for parsing and editing a file, 
 *         with table structure based on user inputed commands
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DELIM delims[0]
#define CMD_MAX 1000
#define CMD_LEN 1000
#define CMD_DELIM ";" //string format for strtok
#define SELECTION_DELIM ',' 
#define CELL_TEXT table->rows[i].cells[j].text
#define TEMPORARY_MAX 10

//Structure for cells in rows
typedef struct {
    int size;
    int cap;
    char *text;
    bool delim;
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

//Stores POSITIONS not INDEXES of selection
typedef struct {
    int start_row;
    int end_row;
    int start_col;
    int end_col;
} Selection;

//Structure for temporary variables
typedef struct {
    Cell variables[TEMPORARY_MAX];
} Temporary;

  /**************************/
 /******CELL FUNCTIONS******/
/**************************/

/* Initialize a new cell with default values
 * @return : empty initialized cell
 */
Cell cell_init(){
    Cell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    new_cell.delim = false;
    return new_cell;
} 

/*
 * Increase capacity of a cell (expand cell.text)
 * @param cell: cell struct
 * @param new_cap: new capacity
 */
void cell_resize(Cell *cell, int new_cap){
    void *resized;
    resized = realloc(cell->text, new_cap * sizeof(char));
    if (resized != NULL){
        cell->text = resized;
        cell->cap = new_cap;
        for (int i = cell->size; i < cell->cap; i++){ //set all new chars to '\0'
            cell->text[i] = '\0';
        }
    }
}

/* Append a character to an existing cell. Resize the cell if needed */
void cell_append(Cell *cell, char c){
    if (cell->cap == cell->size){
        cell_resize(cell, cell->cap ? cell->cap * 2 : 1);     
    }
    if (cell->cap > cell->size){
        cell->text[cell->size] = c;
        cell->size++;
    }
}

/* Used in cell_rewrite (set "string") to look for delimiters in string 
 * @param string: string to check
 * @param delims: all delimiters
 */
bool contains_delim(char *string, char *delims){
    for (unsigned i = 0; i < strlen(delims); i++){
        for (unsigned j = 0; j < strlen(string); j++){
            if (delims[i] == string[j])
                return true;
        }
    }
    return false;
}

/* Rewrite text in a cell with another string
 * @param cell: cell struct
 * @param string: string to write to a cell ("\0" clears the cell)
 */
void cell_rewrite(Cell *cell, char *string, char *delims){
    cell->size = cell->cap = 0;
    for (size_t i = 0; i < strlen(string); i++){
        cell_append(cell, string[i]);
    }

    if (contains_delim(string, delims)){
        cell->delim = true;
    } else {
        cell->delim = false;
    }
}

/* Print the content of cell to given file
 * @param: pointer to structure cell
 * @param dst: destination file
 */
void cell_print(Cell *cell, FILE *dst){
    if (cell->delim){
        putchar('"');
    }
    for (int i = 0; i < cell->size; i++){
        fputc(cell->text[i], dst);
    }
    if (cell->delim){
        putchar('"');
    }
}

/* Extract text from a cell to a given string
 * @param cell: cell sturct
 * @param string: destination of extraction
 */
void get_cell_text(Cell *cell, char *string){
    int i = 0;
    for (i = 0; i < cell->size; i++){
        string[i] = cell->text[i];
    }
    string[i] = '\0';
}

/* Swap 2 whole cells at any positions in a table
 * @param table: table struct
 * @param src_row, src_col: row and column indexes of one cell
 * @param dst_row, dst_col: row and column indexed of other cell
 */
void cell_swap(Table *table, int src_row, int src_col, int dst_row, int dst_col){
    Cell tmp;
    tmp = table->rows[src_row].cells[src_col];
    table->rows[src_row].cells[src_col] = table->rows[dst_row].cells[dst_col];
    table->rows[dst_row].cells[dst_col] = tmp;
}

/* Destroy instantance of a cell and set its size/capacity to default value */
void cell_destroy(Cell *cell){
    free(cell->text);
    cell->cap = cell->size = 0;
    
}

/* Remove cell at a given position in a row
 * @param row: row struct
 * @param index: index of a cell to remove
 */
void cell_delete(Row *row, int index){
        cell_destroy(&row->cells[index]);
        for (int i = index+1; i < row->size; i++){
            memcpy(&row->cells[i-1], &row->cells[i], sizeof(Cell));
        }
        row->size--;
}

  /*****************************/
 /*******ROW FUNCTIONS*********/
/*****************************/

/* Initialize a new row with default values
 * @return: empty initialized row
 */
Row row_init(){
    Row new_row;
    new_row.size = new_row.cap = 0;
    new_row.cells = NULL;
    return new_row;
}

/* Make space for new cells in a row 
 * @param row: row struct
 * @param cells_n: new maximal ammount of cells
 * @see: cell_resize
 */
void row_resize(Row *row, int cells_n){
    void *resized;
    resized = realloc(row->cells, cells_n * sizeof(Cell));

    if (resized != NULL){
        row->cells = resized;
        row->cap = cells_n;
    }
}

/* Append new empty cell to a row. Resize the row if needed */
void row_append(Row *row){
    if (row->size+1 > row->cap){
        row_resize(row, row->cap + 1);
    }
    if (row->size+1 <= row->cap){
        row->cells[row->size] = cell_init();
        row->size++;
    }
}

/* Insert a new empty cell with default values (text = "\0")
 * @param row: row struct
 * @param index: identifies where to insert the new cell
 */
void row_insert(Row *row, int index){
    Cell new_cell = cell_init();
    cell_append(&new_cell, '\0');
    row_append(row);
    
    int i;
    for (i = row->size-1; i != index; i--){ 
        //move all cells to the right by one
        memcpy(&row->cells[i], &row->cells[i-1], sizeof(Cell));
    }
    row->cells[i] = new_cell;
}

/* @see: cell_print
 * @param delim: delim to separate the cells
 * @param dst: destination file
 */
void row_print(Row *row, char delim, FILE *dst){
    for (int i = 0; i < row->size; i++){
        cell_print(&row->cells[i], dst);
        //put delimiter after each cell
        if (i != row->size-1){
            fputc(delim, dst);
        }
    }
}

/* Destroy all instances of cells in a row */ 
void row_destroy(Row *row){
    for (int i = 0; i < row->size; i++){
        cell_destroy(&row->cells[i]);
    }

    if (row->cap)
        free(row->cells);
}

/* Delete row at given position
 * @param table: table struct
 * @param index: index of row to delete
 */
void row_delete(Table *table, int index){
    row_destroy(&table->rows[index]);
    for (int i = index+1; i < table->size; i++){
        memcpy(&table->rows[i-1], &table->rows[i], sizeof(Row));
    }
    table->size--;
}

  /*****************************/
 /*******TABLE FUNCTIONS*******/
/*****************************/

/* Set default values to an empty table */
void table_init(Table *table){
    table->size = 0;
    table->cap = 0;
    table->rows = NULL;
}

/* Make space for new rows in the table
 * @param table: pointer to a table
 * @param rows_n: new maximal ammount of rows
 * @see: cell_resize
 */
void table_resize(Table *table, int rows_n){
    void *resized;
    resized = realloc(table->rows, rows_n * sizeof(Row));

    if (resized != NULL){
        table->rows = resized;
        table->cap = rows_n;
    }
}

/* Create a new row with default values at the end of the table */
void table_append(Table *table){
    if (table->cap == table->size){
        table_resize(table, table->size+1);
    }
    if (table->size < table->cap){
        table->rows[table->size] = row_init();
        table->size++;
    }
}

/* Insert a new row with same number of initialized cells as the other rows 
 * @param table: table struct
 * @param index: index in table, where new row is created
 */
void table_insert(Table *table, int index){
    Row new_row = row_init();
    for (int i = 0; i < table->rows->size; i++){
        row_append(&new_row);
        cell_append(&new_row.cells[i], '\0');
    }
    table_append(table);
    int i;
    for (i = table->size-1; i != index; i--){
        //move all rows by 1 to the right
        memcpy(&table->rows[i], &table->rows[i-1], sizeof(Row));
    }
    table->rows[index] = new_row;
}

/* Get length of the longest row
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

/* Fill the table with empty cells so each row has equal ammount of cells */
void fill_table(Table *table){
    int max_row = get_max_row(*table);
    
    for (int i = 0; i < table->size; i++){
        while (table->rows[i].size != max_row){
            row_append(&table->rows[i]);
        }
    }  
}

/* @see: cell_print
 * @param delim: delimiter of cells in the table
 * @param dst: destination file (stdout for testing purposes)
 */
void table_print(Table *table,  char delim, FILE *dst){
    for (int i = 0; i < table->size; i++){
        row_print(&table->rows[i], delim, dst);
        if (i != table->size-1){
            fputc('\n', dst);
        }
    } fputc('\n', dst);
}

/* Destroy all instances of rows in a table */
void table_destroy(Table *table){
    for (int i = 0; i < table->size; i++){
        row_destroy(&table->rows[i]);
    }

    if (table->cap){
        free(table->rows);
    }
}

/* Add more rows or columns if the selection is bigger than the table
 * @param table: pointer to a table structure
 * @param new_rows: expected number of rows in updated table
 * @param new_cols: expected number of columns in updated table
 */
void table_expand(Table *table, int new_rows, int new_cols){
    for (int i = table->size; i < new_rows; i++){
        table_append(table);
        fill_table(table);
    }

    for (int i = 0; i < table->size; i++){
        for (int j = table->rows[i].size; j < new_cols; j++){
            row_append(&table->rows[i]);
        }
    }
} 

/* Initialize temporary variables with default cells */
void variables_init(Temporary *tmp_vars){
    for (int i = 0; i < TEMPORARY_MAX; i++){
        tmp_vars->variables[i] = cell_init();
    }  
}

/* Destroy all instances of temporary variables */
void variables_destroy(Temporary *tmp_vars){
    for (int i = 0; i < TEMPORARY_MAX; i++){
        cell_destroy(&tmp_vars->variables[i]);
    }
}

/* Checks if char is a delim
 * @param c: character to check
 * @param delims: delimiters
 * @return: true if successful, false if unsuccessful
 */
bool isdelim(char c, char *delims){
    for (size_t i = 0; i < strlen(delims); i++){
        if (c == delims[i]) {
            return true;
        }
    }
    return false;
}

/* Take input from a file and insert it into table using cell,row,table structures
 * @param table: table struct
 * @param source: source file
 * @param delims: delimiters from argv
 */
void create_table(Table *table, FILE *source, char *delims){
    int c, current_cell = 0, current_row = 0;
    int quotes_active = -1; //changing sign to + or - depending whether quotes are active

    while ((c = fgetc(source)) != EOF){
        if (table->rows == NULL){
            table_append(table); 
        }
        if (table->rows[current_row].cells == NULL){
            row_append(&table->rows[current_row]);
        }

        if (c == '\n'){ 
            current_cell = 0;
            current_row++;
            table_append(table); 
            continue;
        } 
        else if (c == '"'){
            quotes_active *= -1;

        } 
        else if (c == '\\'){           
            if ((c = fgetc(source)) != EOF){
                if (isdelim(c, delims)){
                    table->rows[current_row].cells[current_cell].delim = true;
                }
                cell_append(&table->rows[current_row].cells[current_cell], c);
                continue;
            }
        }

        if (isdelim(c, delims) && quotes_active == -1){
            current_cell++;
            row_append(&table->rows[current_row]);
            continue;
        }
    
        if (isdelim(c, delims)){
            table->rows[current_row].cells[current_cell].delim = true;
            }

        if (c != '"'){
            cell_append(&table->rows[current_row].cells[current_cell], c);
        }
    }

    //delete last row, because of \n from last line in file
    row_destroy(&table->rows[table->size]-1);
    table->size--;
}

/* Locate delim in program arguments 
 * @return: string with space or string of delimiters
 */
char * find_delim(const Args args){
    char *delim = " ";
    if (!strcmp(args.argv[1], "-d")){
        delim = args.argv[2];
    } 
    return delim;
}



/* Temporary function for selection to print out selected cells */
void print_selection(Selection *sc, Table *table){  
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            cell_print(&table->rows[i].cells[j], stdout);
            putchar(' ');
        }
    }  
}

/* Resizing the table if new selection bigger than the table size */
void check_table_size(Selection *sc, Table *table){
    if (sc->end_row > table->size){
        table_expand(table, sc->end_row, 0);
    } 
    if (sc->end_col > get_max_row(*table)){
        table_expand(table, 0, sc->end_col);
    }
}

/* Return index of n-th character in a string
 * @param c: string: character to find in string
 * @param n: occurance of number in a string
 * @return: index of found character or 0 if no character was found 
 */
int char_index(char c, char *string, int n){
    int counter = 0;
    for (int i = 0; string[i] != '\0'; i++){
        if (string[i] == c){
            counter++;
            if (counter == n){
                return i;
            }
        }
    }
    return 0;
}

/* Returns number of characters in a string */
int char_in_string(char c, char *string){
    int counter = 0;
    char *temp = strchr(string,c);
    while (temp != NULL){
        counter++;
        temp = strchr(temp+1, c);
    }
    return counter;
}

/* Convert number from string format do double
 * @param string: string to convert
 * @param num: store string into this number
 * @return: 0 if number was successfully extracted from string, otherwise return 1
 */
int string_to_double(char *string, double *num){
    double temp;
    if (string != NULL && strcmp(string,"")){
        if (sscanf(string, "%lf", &temp)){
            *num = temp;
            return 0;
        } else {
            return 1;
        }
    }
    return 1;
}

/* Find first refference value in a table and store it 
 * Refference value serves as first valid value to compare other values with
 * @return: 0 if valid value was found, 1 if no valid value was found
 */
int find_refference(Selection *sc, Table *table, double *reff, int *r_index, int *c_index){
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            if (!string_to_double(table->rows[i].cells[j].text, reff)){
                *r_index = i;
                *c_index = j;
                return 0;
            }
        }
    }
    return 1;
}

/* Max/Min selection specifier - function to find minimal or maximal numeric value in selection
 * @param sc: selection struct
 * @param table: table struct
 * @param str: can be "min" or "max"
 */
void m_selection(Selection *sc, Table *table, char *str){
    double reff; 
    double content;
    int r_index = 1, c_index = 1;
    if (find_refference(sc, table, &reff, &r_index, &c_index))
        return;
    
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            if (string_to_double(table->rows[i].cells[j].text, &content)){
                continue;
            }
            if (!strcmp(str, "max")){
                if (content > reff){
                    reff = content;
                    r_index = i;
                    c_index = j;
                }
            } else {
                if (content < reff){
                    reff = content;
                    r_index = i;
                    c_index = j;
                }
            }
        }
    }
    sc->start_row = sc->end_row = r_index+1;
    sc->start_col = sc->end_col = c_index+1;
}

/* Find first occurance of string in a table
 * @param sc: selection struct
 * @param table: table struct
 * @param string: string to find in table
 */
void find_selection(Selection *sc, Table *table, char *string){
    string[strlen(string)-1] = '\0'; //remove ] from the end
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            char temp[table->rows[i].cells[j].size+1];
            get_cell_text(&table->rows[i].cells[j], temp); 
            if (!strcmp(temp, string)){
                sc->start_row = sc->end_row = i+1;
                sc->start_col = sc->end_col = j+1;
                return;
            }
        }
    }
}

/* Set values to temporary selection */
void tmp_selection_set(Selection *sc, Selection *tmp_sc){
    tmp_sc->start_row = sc->start_row;
    tmp_sc->end_row = sc->end_row;
    tmp_sc->start_col = sc->start_col;
    tmp_sc->end_col = sc->end_col;
}

/* Get values from temporary selection and set them to current selection */
void tmp_selection_use(Selection *sc, Selection *tmp_sc){
    sc->start_row = tmp_sc->start_row;
    sc->end_row = tmp_sc->end_row;
    sc->start_col = tmp_sc->start_col;
    sc->end_col = tmp_sc->end_col;
}

/* Uses already created selection to specify it to a certain cell in a table
 * @param sc: selection struct
 * @param arg: command from user
 * @param table: table struct
 * @return: 0 if every function was successfull (or no function was found)
 *          1 if any error occurs
 */
int specify_selection(Selection *sc, Selection *tmp_sc, char *arg, Table *table){
    char par1[6], par2[100];

    if (!strcmp(arg, "[max]")){
        m_selection(sc, table, "max");
    } 
    else if (!strcmp(arg, "[min]")){
        m_selection(sc, table, "min");
    } 
    else if (char_in_string(' ', arg)){
        if (sscanf(arg, "%5s %99s", par1, par2) == 2 && !strcmp(par1,"[find")){
            find_selection(sc, table, par2);
        } 
    }
    else if (!strcmp(arg, "[set]")){
        tmp_selection_set(sc, tmp_sc);
    }
    else if (!strcmp(arg, "[_]")){
        tmp_selection_use(sc, tmp_sc);
    }
    
    return 0;
}

/* Function used to identify if special char is valid in selection command
 * @param arg: selection command
 * @param n: ammound of SELECTION_DELIMITERS (,) before special char
 * @param c: what character should follow after special char
 * @return: 1 if special char was found, the string is modified 
 */
int special_chars(char *arg, int n, char c, char special){
    int pos = char_index(SELECTION_DELIM, arg, n);
    if (arg[pos+1] == special){
        if (arg[pos+2] != c){
            return 1;
        } else {
            arg[pos+1] = '0';
        }
    }   
    return 0;
}

/* After special char is found in string, set values of selection to parameter values
 * @param par, par2: parameters to set
 */
void set_params(Selection *sc, Table *table, int par, int par2){
    if (par == 0){  
        sc->end_row = table->size;
    } else {
        sc->end_row = par;
    }

    if (par2 == 0){
        sc->end_col = table->rows[0].size;
    } else {
        sc->end_col = par2;
    }
}

/* Function for exctacting parameters from selection in format [int,int]
 * @param arg: argument in format [int,int]
 */ 
int simple_selection(Selection *sc, char *arg, Table *table){
    int par1 = -1, par2 = -1, count; 
    count = sscanf(arg, "[%d,%d]", &par1, &par2);

    //setting end values
    if (count != 2){
        if (special_chars(arg, 0, SELECTION_DELIM, '_')) { //looking for [_,]
            return 1;
        } 
        
        if (special_chars(arg, 1, ']', '_')) { //looking for [,_]
            return 1;
        }

        sscanf(arg, "[%d,%d]", &par1, &par2);
        set_params(sc, table, par1, par2);
        
    } else {
        if (par1 > 0 && par2 > 0){
            sc->end_row = par1;
            sc->end_col = par2;
        } else return 1;
    }
    
    //setting start values
    if (par1 == 0){ // [_,num]
        sc->start_row = par1+1; 
    } else {
        sc->start_row = par1; 
    }

    if (par2 == 0){ //[num,_]
        sc->start_col = par2+1;
    } else {
        sc->start_col = par2;
    }

    return 0;
}

/*Similiar to simple_selection but uses 4 numbers*/
int advanced_selection(Selection *sc, char *arg, Table *table){
    int par1 = -1, par2 = -1, par3 = -1, par4 = -1, count; 
    count = sscanf(arg, "[%d,%d,%d,%d]", &par1, &par2, &par3, &par4);

    if (par1 <= 0 || par2 <= 0){
        return 1;
    }    
    
    if (count != 4){
        if (special_chars(arg, 2, SELECTION_DELIM, '-')) { // [num,num,-,num]
            return 1;
        } 
        if (special_chars(arg, 3, ']', '-')) { // [num,num,num,-]
            return 1;
        }
        sscanf(arg, "[%d,%d,%d,%d]", &par1, &par2, &par3, &par4);
        set_params(sc, table, par3, par4);

    } else {
        if (par3 > 0 && par4 > 0 && par1 <= par3 && par2 <= par4){
            sc->start_row = par1;
            sc->start_col = par2;
            sc->end_row = par3;
            sc->end_col = par4;
        } else return 1;
    }
    
    if (par3 == 0 || par4 == 0){
        sc->start_row = par1; 
        sc->start_col = par2;
    }

    return 0;
}

/* Function to determine, what type of selection was found
 * @param sc: selection struct
 * @param arg: command from user
 * @param table: table struct
 * return: 1 if any error has occured, 0 if everything was successful
 */
int set_selection(Selection *sc, Selection *tmp_sc, char *arg, Table *table){
    int counter = char_in_string(SELECTION_DELIM, arg); //number of commas 

    int error;
    if (counter == 0){
        error = specify_selection(sc, tmp_sc, arg, table); //[max]
    } else if (counter == 1){
        error = simple_selection(sc, arg, table); //[int,int]
    } else if (counter == 3){
        error = advanced_selection(sc, arg, table); //[int,int,int,int]
    } else {
        error = 1;
    }

    if (error){
        return 1;
    }
    check_table_size(sc, table);
    
    /*debug mode*/
    printf("Selection:\n");
    print_selection(sc, table);
    putchar('\n'); putchar('\n');

    return 0;
}

/* Functions for editing structure of the whole table */
int edit_tstruc(Selection *sc, char *arg, Table *table, char *delims){
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            if (!strcmp(arg, "irow")){
                table_insert(table, i);
            }
            else if (!strcmp(arg, "arow")){
                table_insert(table, i+1);
            }
            else if (!strcmp(arg, "icol")){
                row_insert(&table->rows[i], j);
            }
            else if (!strcmp(arg, "acol")){
                row_insert(&table->rows[i], j+1);
            }
            else if (!strcmp(arg, "dcol")){
                cell_delete(&table->rows[i], j);
            }
            else if (!strcmp(arg, "clear")){
                cell_rewrite(&table->rows[i].cells[j], "\0", delims);
            }
        }
        if (!strcmp(arg, "drow")){ 
            row_delete(table, i);
        }
    }
    return 0;
}

/* Edits temporary variables based on user command */
int edit_variables(Selection *sc, Table *table, Temporary *tmp_vars, char *arg, char *param,
                   char *delims){
    int var;
    if (sscanf(param, "%d", &var) != 1){
        return 1;
    }
    if (var < 0 || var >= 10){
        return 1;
    }

    if (!strcmp(arg, "def")){
        int len = table->rows[sc->end_row-1].cells[sc->end_col-1].size;
        char text[len+1];
        get_cell_text(&table->rows[sc->end_row-1].cells[sc->end_col-1], text);
        cell_rewrite(&tmp_vars->variables[var], text, delims); 
    }
    else if (!strcmp(arg, "use")){
        for (int row = sc->start_row-1; row < sc->end_row; row++){
            for (int col = sc->start_col-1; col < sc->end_col; col++){
                int len = tmp_vars->variables[var].size;
                char text[len+1];
                get_cell_text(&tmp_vars->variables[var], text);
                cell_rewrite(&table->rows[row].cells[col], text, delims);
            }
        }
    } 
    else if (!strcmp(arg, "inc")){
        double num;
        char text[50];
        if (tmp_vars->variables[var].text != NULL){
            if (sscanf(tmp_vars->variables[var].text, "%lf", &num)){
                num++;
                sprintf(text, "%g", num);
            } else {
                num = 1.0;
                sprintf(text, "%g", num);
            }
        } else {
            num = 1.0;
            sprintf(text, "%g", num);
        }

        cell_rewrite(&tmp_vars->variables[var], text, delims);
    }

    return 0;
}

/* Extract values from string in format [int,int] to integers par1, par2
 * @param arg: string in format [int,int]
 * @param par1,par2: integers to store output values to
 */
int args_to_int(Table *table, char *arg, int *par1, int *par2){
    if (sscanf(arg, "[%d,%d]", par1, par2) != 2){
        return 1;
    }
    if (*par1 <= 0 || *par2 <= 0 || *par1 > table->size || *par2 > table->rows[0].size ){
        return 1;
    }
    return 0;
}

/* Functions for editing data in table 
 * @param arg: first part of user command (before space character)
 * @param param: second part of user command (parameters)
 */
int edit_tdata(Selection *sc, Table *table, char *arg, char *param, char *delims){
    int par1, par2, counter = 0;
    double temp_value = 0, num = 0;
    char sum[50];

    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            if (!strcmp(arg, "set")){
                cell_rewrite(&table->rows[i].cells[j], param, delims);
            }
            else if (!strcmp(arg, "swap")){
                if (args_to_int(table, param, &par1, &par2)){
                    return 1;
                }
                cell_swap(table, i, j, par1-1, par2-1);
            }
            else if (!strcmp(arg, "sum")){
                if (args_to_int(table, param, &par1, &par2)){
                    return 1;
                }
                num = 0;
                if (!string_to_double(CELL_TEXT, &num)){
                    temp_value += num;
                }
            }
            else if (!strcmp(arg, "avg")){
                if (args_to_int(table, param, &par1, &par2)){
                    return 1;
                }
                num = 0;
                if (!string_to_double(CELL_TEXT, &num)){
                    temp_value += num;
                    counter++;
                }
                if (i+1 == sc->end_row && j+1 == sc->end_col){
                    temp_value /= counter;
                }
            }
            else if (!strcmp(arg, "count")){
                if (args_to_int(table, param, &par1, &par2)){
                    return 1;
                }
                num = 0;
                if (CELL_TEXT != NULL && strcmp(CELL_TEXT, "")){
                    counter++;
                }
                if (i+1 == sc->end_row && j+1 == sc->end_col){
                    temp_value = counter;    
                }
            }
            else if (!strcmp(arg, "len")){
                if (args_to_int(table, param, &par1, &par2)){
                    return 1;
                }
                num = 0;
                if (CELL_TEXT != NULL && strcmp(CELL_TEXT, "")){
                    temp_value = table->rows[i].cells[j].size;
                } else {
                    temp_value = 0;
                }
            }

        }
    }

    if (!strcmp(arg, "sum") || !strcmp(arg, "avg") ||
        !strcmp(arg, "count") || !strcmp(arg, "len")){
        sprintf(sum, "%g", temp_value);
        cell_rewrite(&table->rows[par1-1].cells[par2-1], sum, delims);
    }

    return 0;
}

/* Seperate string into 2 parts by given delimiter, format and return first or second part
 * @param curr_cmnd: command from user to process
 * @param delim: delimiter of substrings
 * @param format: format, in which whey will be split
 * @param n: return first or second part
 */
char * string_separate(char *curr_cmnd, char delim, char *format, int n){
    int len = char_index(delim, curr_cmnd, 1);
    char *arg = malloc(len+1);
    char *param = malloc(strlen(curr_cmnd)-len);
    if (arg == NULL || param == NULL){
        return NULL;
    }

    if (sscanf(curr_cmnd, format, arg, param) != 2){
        free(arg); free(param);
        return NULL;
    }

    if (n == 1){
        free(param);
        return arg;
    } else {
        free(arg);
        return param;
    }
}

/* Process commands - separate them, indentify, call appropriate function */
int parse_commands(char **argv, Selection *sc, Selection *tmp_sc, Table *table, 
                   Temporary *tmp_vars, char *delims){
    int pos;
    if (!strcmp("-d", argv[1])){
        pos = 3;
    } else {
        pos = 1;
    }
    
    char *cmd_seq = argv[pos];
    char *curr_cmnd = strtok(cmd_seq, CMD_DELIM);
    while (curr_cmnd != NULL){
        if (curr_cmnd[0] == '['){
            if (set_selection(sc, tmp_sc, curr_cmnd, table)){
                printf("Chybne argumenty selekcie\n");
                return 1;
            }
        } 
        else if (!char_in_string(' ', curr_cmnd)){
            edit_tstruc(sc, curr_cmnd, table, delims);
        }
        else if(char_in_string('_', curr_cmnd)){
            char *arg, *param;
            arg = string_separate(curr_cmnd, ' ', "%s _%s", 1);
            param = string_separate(curr_cmnd, ' ', "%s _%s", 2);
            if (arg == NULL || param == NULL){ //no parameter or arg was found
                printf("Chybne zadane prikazy\n");
                free(arg); free(param);
                return 1;
            }
            if (edit_variables(sc, table, tmp_vars, arg, param, delims)){ //parameter was not valid
                printf("Chybne zadane prikazy\n");
                free(arg); free(param);
                return 1;
            }
            free(arg); free(param);
        }
        else {
            char *arg, *param;
            arg = string_separate(curr_cmnd, ' ', "%s %s", 1);
            param = string_separate(curr_cmnd, ' ', "%s %s", 2);
            if (arg == NULL || param == NULL){ //no parameter or arg was found
                printf("Chybne zadane prikazy\n");
                free(arg); free(param);
                return 1;
            }
            if (edit_tdata(sc, table, arg, param, delims)){ //parameter was not valid
                printf("Chybne zadane prikazy\n");
                free(arg); free(param);
                return 1;
                }
            free(arg); free(param);
        }
        curr_cmnd = strtok(NULL, CMD_DELIM);
    }
    return 0;
}

/* Remove excess (most right empty) colums from table */
void excess_columns(Table *table){
    int empty_rows;
    for (int j = table->rows[0].size-1; j >= 0; j--){
        empty_rows = 0;
        for (int i = 0; i < table->size; i++){
            if (!table->rows[i].cells[j].size || !*table->rows[i].cells[j].text){
                empty_rows++;
            }
        }
        if (empty_rows == table->size){
            for (int i = 0; i < table->size; i++){
                cell_delete(&table->rows[i], table->rows[i].size-1);
            }
        }
    }
}

int main(int argc, char **argv){
    if (argc < 3){
        fprintf(stderr, "Minimalny pocet argumentov je 3\n");
        return 1;
    }
   
    Args args = {argv, argc};
    char *delims = find_delim(args);
    Table table;
    table_init(&table);
    
    FILE *file;
    file = fopen(argv[argc-1], "r");
    if (file == NULL){
        fprintf(stderr, "Nastala chyba pri otvarani suboru\n");
        return 1;
    }

    create_table(&table, file, delims);    
    fill_table(&table);
    
    //fclose(file); //comment for debug mode

    Selection sc = {1,1,1,1}; //default selection is first row,column
    Selection tmp_sc = {1,1,1,1};
    Temporary tmp_vars;

    variables_init(&tmp_vars);

    if (parse_commands(argv, &sc, &tmp_sc, &table, &tmp_vars, delims)){
        table_destroy(&table);
        variables_destroy(&tmp_vars);
        return 1;
    }

    /*
    file = fopen(argv[argc-1], "w");                                  //          //
    if (file == NULL){                                                // comment  //
        fprintf(stderr, "Nastala chyba pri otvarani suboru\n");       //   for    //
        return 1;                                                     //  debug   //
    } */                                                                //          //
    
    fill_table(&table); 
    excess_columns(&table);

    //table_print(&table, DELIM, file);                                // comment for debug
    table_print(&table, DELIM, stdout);                            //uncomment for debug
    
    table_destroy(&table);
    variables_destroy(&tmp_vars);
    fclose(file);
    return 0;
}


