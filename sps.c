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
#define CMD_MAX 1000
#define CMD_LEN 1000
#define CMD_DELIM ";"
#define SELECT_DELIM ','

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

/* Set default values to an empty structure (cell, row, table)
 * @return : empty initialized structure (cell, row)
 */
Cell cell_init(){
    Cell new_cell;
    new_cell.size = new_cell.cap = 0;
    new_cell.text = NULL;
    new_cell.delim = false;
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
void table_print(Table *table,  char delim, FILE *dst){
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
    int quotes_active = -1; //changing sign to + or - depending whether quotes are active

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
        } 
        else if (c == '"'){
            quotes_active *= -1;
            continue;

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
            row_append(&table->rows[current_row], current_cell);
            continue;
        }
    
        if (isdelim(c, delims)){
            table->rows[current_row].cells[current_cell].delim = true;
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
void end(Table *table){
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

/*
 * Add more rows or columns if the selection is bigger than the table
 * @param table: pointer to a table structure
 * @param new_rows: expected number of rows in updated table
 * @param new_cols: expected number of columns in updated table
 */
void table_expand(Table *table, int new_rows, int new_cols){
    for (int i = table->size; i < new_rows; i++){
        table_append(table, i);
        fill_table(table);
    }

    for (int i = 0; i < table->size; i++){
        for (int j = table->rows[i].size; j < new_cols; j++){
            row_append(&table->rows[i], j);
        }
    }
}

/*
 * Temporary function for selection to print out selected cells
 */
void print_selection(Selection *sc, Table *table){  
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            cell_print(&table->rows[i].cells[j], stdout);
            putchar(' ');
        }
    }
    
}

/*
 * If selected area is bigger than the table, resize it
 * @param sc: updated selection 
 * @param table: pointer to table structure
 */
void check_table_size(Selection *sc, Table *table){
    if (sc->end_row > table->size){
        table_expand(table, sc->end_row, 0);
    } 
    if (sc->end_col > get_max_row(*table)){
        table_expand(table, 0, sc->end_col);
    }
}

/*
 * Return index of n-th character in a string
 * @param c: character to find in string
 * @param string: string itself
 * @param n: number of occurences of char in string
 * @return: index of found character or 0 if no character was found 
 */
int char_index(char c, char *string, int n){
    int counter2 = 0;
    for (size_t i = 0; string[i] != '\0'; i++){
        if (string[i] == c){
            counter2++;
            if (counter2 == n){
                return i;
            }
        }
    }
    return 0;
}

/*
 * Return how many characters are in a string
 * @param c: character to find
 * @param string: string itself
 * @return: number of character occurences
 */
int char_in_string(char c, char *string){
    int counter = 0;
    char *temp = strchr(string,c);
    while (temp != NULL){
        counter++;
        temp = strchr(temp+1, c);
    }

    return counter;
}

/*
 * Function used to identify if char '_' is used in selection
 * @param arg: selection text
 * @param n: number of underscores
 * @param c: what character should follow after underscore
 * @return: 1 if underscore was found, 0 if no underscore was found
 */
int underscore(char *arg, int n, char c){
    int pos = char_index(SELECT_DELIM, arg, n);
    if (arg[pos+1] == '_'){
        if (arg[pos+2] != c){
            return 1;
        } else {
            arg[pos+1] = '0';
        }
    }   
    return 0;
}

/*
 * After underscore is found in string, set values of selection to parameter values
 * @param sc: selection struct
 * @param table: table struct
 * @param par: first parameter to set 
 * @param par2: second parameter to set
 */
void set_params(Selection *sc, Table *table, int par, int par2){
    if (par == 0){  //underscore was found here
        sc->end_row = table->size;
    } else {
        sc->end_row = par;
    }

    if (par2 == 0){
        sc->end_col = table->rows[0].size; //underscore was found here
    } else {
        sc->end_col = par2;
    }
}

/*
 * Convert number from string format do double
 * @param string: string to convert
 * @return: successfully converted double 
 */
int string_to_double(char *string, double *num){
    double temp;
    if (string != NULL){
        if (sscanf(string, "%lf", &temp)){
            *num = temp;
            return 0;
        } else {
            return 1;
        }
    }
    return 1;
}

/*
 * Find first refference value in a table and store it
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

/*
 * Max/Min selection specifier - function to find minimal or maximal numeric value in selection
 * @param sc: selection struct
 * @param table: table struct
 * @param str: can be "min" or "max"
 * @return: 1 if any error occurs
 */
int m_selection(Selection *sc, Table *table, char *str){
    double reff; 
    double content;
    int r_index = 1, c_index = 1;
    if (find_refference(sc, table, &reff, &r_index, &c_index))
        return 1;
    
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
    return 0;
}

/*
 * Find first occurance of string in a table
 * @param sc: selection struct
 * @param table: table struct
 * @param string: string to find in table
 */
void find_selection(Selection *sc, Table *table, char *string){
    string[strlen(string)-1] = '\0'; //remove ] from the end
    for (int i = sc->start_row-1; i < sc->end_row; i++){
        for (int j = sc->start_col-1; j < sc->end_col; j++){
            if (!strcmp(table->rows[i].cells[j].text, string)){
                sc->start_row = sc->end_col = i;
                sc->start_row = sc->end_col = j;
                return;
            }
        }
    }
}

/*
 * Uses already created selection to specify it to a certain cell in a table
 * @param sc: selection struct
 * @param arg: command from user
 * @param table: table struct
 * @return: 0 if every function was successfull (or no function was found)
 *          1 if any error occurs
 */
int specify_selection(Selection *sc, char *arg, Table *table){
    char par1[100], par2[100];

    if (!strcmp(arg, "[max]")){
        if (m_selection(sc, table, "max")){
            return 1;
        }
    } 
    else if (!strcmp(arg, "[min]")){
        if (m_selection(sc, table, "min")){
            return 1;
        }
    } 
    else if (char_in_string(' ', arg)){
        if (sscanf(arg, "%99s %s", par1, par2) == 2 && !strcmp(par1,"[find"))
            find_selection(sc, table, par2);
    }

    return 0;
}


/*
 * Function for exctacting parameters from single selection in format [int,int]
 * @param sc: pointer to a selection structure
 * @param arg: argument in format [int,int]
 * @param table: pointer to a table structure
 */ 
int simple_selection(Selection *sc, char *arg, Table *table){
    int par1 = -1, par2 = -1, count; 
    count = sscanf(arg, "[%d,%d]", &par1, &par2);

    if (count != 2){
        if (underscore(arg, 0, SELECT_DELIM)) {
            return 1;
        } 
        
        if (underscore(arg, 1, ']')) {
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
    
    if (par1 == 0){
        sc->start_row = par1+1; 
    } else {
        sc->start_row = par1; 
    }

    if (par2 == 0){
        sc->start_col = par2+1;
    } else {
        sc->start_col = par2;
    }

    return 0;
}

/*
 * Similiar to simple_selection but works for 4 numbers
 */
int advanced_selection(Selection *sc, char *arg, Table *table){
    int par1 = -1, par2 = -1, par3 = -1, par4 = -1, count; 
    count = sscanf(arg, "[%d,%d,%d,%d]", &par1, &par2, &par3, &par4);

    if (par1 <= 0 || par2 <= 0){
        return 1;
    }    

    if (count != 4){
        if (underscore(arg, 2, SELECT_DELIM)) {
            return 1;
        } 
        
        if (underscore(arg, 3, ']')) {
            return 1;
        }

        sscanf(arg, "[%d,%d,%d,%d]", &par1, &par2, &par3, &par4);
    
        set_params(sc, table, par3, par4);
    } else {
        if (par3 > 0 && par4 > 0 && par1 <= par3 && par2 <= par4){
            
            sc->start_row = par1;
            sc->end_col = par2;
            sc->end_row = par3;
            sc->end_col = par4;
        } else return 1;
    }
    
    if (par1 == 0){
        sc->start_row = par1+1; 
    } else {
        sc->start_row = par1; 
    }

    if (par2 == 0){
        sc->start_col = par2+1;
    } else {
        sc->start_col = par2;
    }

    return 0;
}

int set_selection(Selection *sc, char *arg, Table *table){
    int counter = char_in_string(SELECT_DELIM, arg);

    int error;
    if (counter == 0){
        error = specify_selection(sc, arg, table);
    } else if (counter == 1){
        error = simple_selection(sc, arg, table);
    } else if (counter == 3){
        error = advanced_selection(sc, arg, table);
    } else {
        error = 1;
    }

    if (error){
        return 1;
    }
    check_table_size(sc, table);
    
    //printf("%d %d %d %d ", sc->start_row, sc->end_row, sc->start_col, sc->end_col);
    printf("Selection:\n");
    print_selection(sc, table);
    putchar('\n');
    return 0;
}
    

/*
 * Process commands - separate them and indentify, call appropriate function
 * @param argv: argument structure
 * @param sc: selection
 */
int parse_commands(char **argv, Selection *sc, Table *table){
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
            if (set_selection(sc, curr_cmnd, table)){
                printf("chybne argumenty selekcie\n");
                return 1;
            }
        }
        curr_cmnd = strtok(NULL, CMD_DELIM);
    }
    return 0;
}

int main(int argc, char **argv){
    if (argc < 3){
        //TODO print error
        printf("malo argumentov\n");
        return 1;
    }
   
    FILE *fr;
    fr = open_file(argv[argc-1]);
    if (fr == NULL){
        //TODO error while opening the file
        printf("no file was opened\n");
        return 1;
    }
 
    Args args = {argv, argc};
    char *delims = find_delim(args);

    Table table;
    table_init(&table);
    create_table(&table, fr, delims);    
    fill_table(&table);

    Selection sc = {1,1,1,1};

    if (parse_commands(argv, &sc, &table)){
        end(&table);
        return 1;
    }
    
    putchar('\n');
    table_print(&table, DELIM, stdout); //TODO change to dst
    rewind(fr);
    end(&table);
    fclose(fr);
    return 0;
}


