#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define ERR 1
const int SIZE = 50; //Velkosť inkrementácie pri alokacii tabulky, ak je subor väčší ako 50 riadkov
int cmd_location;

void get_DELIM(char *argv[], char **DELIM);
struct Table LoadFile(char *filename, struct Table *T1, int *alloc);
struct Table Allign(struct Table *T1, char *delimiter);
struct selection Get_Selection(struct selection *s, char *argv[], bool *s_active, struct Table T);
struct command Edit_Table(char *argv[], struct command c, struct Table *T, char *delimiter, int alloc_size);
struct command Get_Command(struct command *c, char *argv[], struct Table T);
struct Table Make_Changes(struct command c, struct Table *T, char *delimiter);
struct Table arow(struct command c, struct Table *T, char *delimiter);
void check_X_Y(struct selection s, struct Table T);
struct Table irow(struct command c, struct Table *T, char *delimiter);
struct Table drow(struct command c, struct Table *T);
struct Table icol(struct command c, struct Table *T, char *delimiter);
void Get_Position(int col, int row, int *pos_x, int *pos_y, struct Table T, char *delimiter, int *pocet_znakov_v_riadku);
struct Table acol(struct command c, struct Table *T, char *delimiter);
struct Table dcol(struct command c, struct Table *T, char *delimiter);
struct Table set(struct command c, struct Table *T, char *delimiter);
struct Table clear(struct command c, struct Table *T, char *delimiter);

typedef struct rows{
int row_len;
int num_of_rows;
}rows;

typedef struct cols{
int pos_x;
int pos_y;
int num_of_cols;
}cols;

typedef struct Table{
struct rows r; 
struct cols c;
char **table;
}Table;

typedef struct selection{
int row;
int col;
int row2;
int col2;
int arg_pos;
}selection;

typedef struct command{
struct selection S;
char *str;
}command;

//MAIN---------------------------------------------------------------------------------
int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Please input arguments!");
        return ERR;
    }

    char *DELIM;
    Table T1;
    command C;
    cmd_location = argc-2;
    int alloc_size;

    
            //FILENAME
    LoadFile(argv[argc-1], &T1, &alloc_size);

    //Ziska DELIM
    get_DELIM(argv, &DELIM);
    
    //Zarovnanie tabulky
    Allign(&T1, DELIM);

    //Prejde príkazy
    Edit_Table(argv, C, &T1, DELIM, alloc_size); 

    for(int i = 0; i < T1.r.num_of_rows; i++)printf("%s", T1.table[i]);

    free(DELIM);
    free(T1.table);
    exit(0);
} //MAIN-END--------------------------------------------------------------------------------


//GET_DELIM-------------------------------------------------------------------------------
void get_DELIM(char *argv[], char **DELIM){
    int l = 0;
    int len;

    //Hladanie DELIMU
    if(!strcmp(argv[1], "-d")){

        //ALOKACIA + kontrola
        char buffer[1000];
        strcpy(buffer, argv[2]);
        int buf_len = strlen(buffer);
        char *line = (char *)malloc((buf_len + 1) * sizeof(char));
        strcpy(line, buffer);
        *DELIM = line;
       if(*DELIM == NULL){
           fprintf(stderr, "DELIM could not be alocated!");
           exit(ERR);
       }

       //VYHODENIE rovnakých znakov
       len = strlen(*DELIM);
       for(int i = 0; i < len; i++){
           for(int j = i + 1; j <= len; j++){
               if(DELIM[l][i] == '\\' || DELIM[l][i] == '"'){
                   fprintf(stderr, "ERROR: DELIM contains not allowed characters!");
                   exit(ERR);
               }
               if(DELIM[l][i] == DELIM[l][j]){
                   DELIM[l][j] = DELIM[l][len];
                   DELIM[l][len] = '\0';
                   len--;
               }
           }
       }
    }
    //V pripade ze nieje delim zadany
    else{
        char buffer[] = " ";
        int buf_len = strlen(buffer);
        char *line = (char *)malloc((buf_len + 1) * sizeof(char));
        strcpy(line, buffer);
        *DELIM = line;
    }
}

//LOAD_FILE--------------------------------------------------------------------------------------
struct Table LoadFile(char *filename, struct Table *T1, int *alloc){

    FILE *subor;
    char buffer[1000];
    T1->r.num_of_rows = 0;
    int alloc_size = SIZE;

    //Otvorenie súboru a kontrola otvorenia
    subor = fopen(filename, "r");
    if(!subor){
        fprintf(stderr, "ERROR: File could not be opened!");
        exit(ERR);
    }
    //Alokacia pola na 50 riadkov
    T1->table = (char **)malloc(SIZE * sizeof(char *));
    
    while(fgets(buffer, 1000, subor)){

        //Ak sa pole naplni tak sa realokuje
        if(T1->r.num_of_rows == alloc_size){
            alloc_size += SIZE;

            char **temp = realloc(T1->table, alloc_size * sizeof(char *));
            if(!temp){
                fprintf(stderr, "ERROR: Can't reallocate!");
                exit(ERR);
            }
            T1->table = temp;
        }
        //Dlzka bufferu
        int buf_len = strlen(buffer);

        //Alokacia riadku na dlzku buffera
        char *line = (char *)malloc((buf_len + 1) * sizeof(char));

        //Skopirovanie riadku do docasnej premennej
        strcpy(line, buffer);

        //Predanie riadku do tabulky
        T1->table[T1->r.num_of_rows] = line;
        T1->r.num_of_rows++;
    }
    fclose(subor);
    *alloc = alloc_size;
    return *T1;
}

//ALLIGN----------------------------------------------------------------------------------------

struct Table Allign(struct Table *T1, char *delimiter){
    int col_max = 0;
    int counter = 0;
    int char_num = 0;
    int len = strlen(delimiter);

    //Hladanie koľko má tabuľka maximum stĺpcov
    for(int i = 0; i < T1->r.num_of_rows; i++){
        while(T1->table[i][char_num] != '\0'){
            for(int j = 0; j < len; j++){
                if(T1->table[i][char_num] == delimiter[j]){
                    counter++;
                }
            }
            char_num++;
        }
        if(counter > col_max) col_max = counter;
        counter = 0;
        char_num = 0;
    }

    T1->c.num_of_cols = col_max;
    counter = 0;
    char_num = 0;

    //Pridanie stĺpcov tam kde chýbajú
    for(int i = 0; i < T1->r.num_of_rows; i++){
        while(T1->table[i][char_num] != '\0'){
            for(int j = 0; j < len; j++){
                if(T1->table[i][char_num] == delimiter[j]){
                    counter++;
                }
            }
            char_num++;
        }
        int size = strlen(T1->table[i]);

        //Realokuje a prida delim znak
        if(counter < col_max){
            int difference = col_max - counter;
            T1->table[i] = realloc(T1->table[i], size+1 + difference);

            if(!T1->table[i]){
                fprintf(stderr, "ERROR: Can't realloc!");
                exit(ERR);
            }

            T1->table[i][size + difference] = '\0';

            if(T1->table[i][size-1] == '\n'){
                T1->table[i][size-1] = '\0';
                for(int j = 0; j < difference; j++){
                    T1->table[i][size-1+j] = delimiter[0];
                }
                T1->table[i][size-1 + difference] = '\n';
            }
            else{
                for(int j = 0; j < difference; j++){
                    T1->table[i][size+j] = delimiter[0];
                }
            }
            

        }
        counter = 0;
        char_num = 0;
    }
    return *T1;
}
//EDIT_TABLE------------------------------------------------------------------------------------------------

struct command Edit_Table(char *argv[], struct command c, struct Table *T, char *delimiter, int alloc_size){
    bool s_active;
    c.S.arg_pos = 0;

    while(argv[cmd_location][c.S.arg_pos] != '\0'){

        //Zisti aka je selekcia riadkov
        Get_Selection(&c.S, argv, &s_active, *T);
        if(s_active == 0){
            fprintf(stderr, "ERROR: Selection syntax error!");
            exit(ERR);
        }

        if(argv[cmd_location][c.S.arg_pos-1] != ';'){
            c.S.arg_pos += 1;
        }

        if(argv[cmd_location][c.S.arg_pos-2] == '\0'){
            c.S.arg_pos = c.S.arg_pos-1;
            continue;
        }

        //Zisti aky príkaz bol použitý pre danú selekciu
        Get_Command(&c, argv, *T);

        if(argv[cmd_location][c.S.arg_pos-1] != ';'){
            c.S.arg_pos += 1;
        }

        //V prípade že sa tabulka rozšíri nad alokovany počet riadkov, tak sa realokuje a prída riadky o počte SIZE
        if(T->r.num_of_rows == alloc_size){
            alloc_size += SIZE;

            char **temp = realloc(T->table, alloc_size * sizeof(char *));
            if(!temp){
                fprintf(stderr, "ERROR: Can't reallocate!");
                exit(ERR);
            }
            T->table = temp;
        }

        //Urobí zmeny v tabuľke
        Make_Changes(c, T, delimiter);

        if(argv[cmd_location][c.S.arg_pos-1] == '\0'){
            c.S.arg_pos -= 1;
        }
    }
    return c;
}

//GET_SELECTION---------------------------------------------------------------------------------------------

struct selection Get_Selection(struct selection *s, char *argv[], bool *s_active, struct Table T){
    if(argv[cmd_location][s->arg_pos] != '['){
        s_active = 0;
        return *s;
    }

    int com_count = 0;
    int arg_pointer = s->arg_pos;
    char buffer[1000] = {0};
    int com_pos;
    
    //Spočíta poćet hodnôt v selekcii (či sa jedná o výber [r,s] alebo [r,s,r,s] alebo [str])
    while(argv[cmd_location][arg_pointer] != ';' && argv[cmd_location][arg_pointer] != '\0'){
        if(argv[cmd_location][arg_pointer] == ','){
            com_count++;
            com_pos = arg_pointer;
        }
        arg_pointer++;
    }
                    //dlzka buffera
    memset(buffer, 0, 1000);

    //Ak je výber v tvare [r,s]
    if(com_count == 1){
        strncpy(buffer, argv[cmd_location]+s->arg_pos+1, com_pos - s->arg_pos -1);
        if(!strcmp(buffer, "_")){
            s->row = -5;
        }
        else{
        s->row = atoi(buffer);
        }

        memset(buffer, 0, 1000);

        strncpy(buffer, argv[cmd_location]+com_pos+1, arg_pointer-1 - com_pos-1);

        if(!strcmp(buffer, "_")){
            s->col = -5;
        }
        else{
        s->col = atoi(buffer);
        }

        check_X_Y(*s, T);
        *s_active = 1;
    }

    //Ak je výber v tvare [r,s,r,s]
    else if(com_count == 3){
        int counter = 0;
        int temp;
        arg_pointer = s->arg_pos;
        while(argv[cmd_location][arg_pointer] != ';' && argv[cmd_location][arg_pointer] != '\0'){

            if(argv[cmd_location][arg_pointer] == ','){
                com_pos = arg_pointer;
                counter++;
            }

            if(counter == 1 && arg_pointer == com_pos){
                strncpy(buffer, argv[cmd_location]+s->arg_pos+1, com_pos - s->arg_pos -1);
                s->row = atoi(buffer);
                temp = com_pos;
                memset(buffer, 0, 1000);
            }

            else if(counter == 2 && arg_pointer == com_pos){
                strncpy(buffer, argv[cmd_location]+temp+1, com_pos - temp-1);
                s->col = atoi(buffer);
                temp = com_pos;
                memset(buffer, 0, 1000);
            }

            else if(counter == 3 && arg_pointer == com_pos){
                strncpy(buffer, argv[cmd_location]+temp+1, com_pos - temp-1);
                if(!strcmp(buffer, "_")){
                    s->row2 = -1;
                }
                else{
                    s->row2 = atoi(buffer);
                }
                temp = com_pos;
                memset(buffer, 0, 1000);
            }
            arg_pointer++;
        }

                strncpy(buffer, argv[cmd_location]+temp+1, arg_pointer - temp-2);
                if(!strcmp(buffer, "_")){
                    s->col2 = -1;
                }
                else{
                    s->col2 = atoi(buffer);
                }
                memset(buffer, 0, 1000);

                // TODO check_X_Y();
                // TODO check_X_Y();
                *s_active = 1;
    }
    
    //Ak je výber v tvare [str]
    else if(com_count == 0){
        if(s->col != -5){
            strncpy(buffer, argv[cmd_location]+s->arg_pos+1, arg_pointer - s->arg_pos-2);
            if(!s->col){
                fprintf(stderr, "ERROR: Selection syntax error");
                exit(ERR);
            }

            if(!strcmp(buffer, "max")){
                s->col = T.c.num_of_cols+1;
            }

            else if(!strcmp(buffer, "min")){
                s->col = 1;
            }
            *s_active = 1;
        }
    }

    else{
        fprintf(stderr, "ERROR: Selection syntax failure!");
        exit(ERR);
    }
    s->arg_pos = arg_pointer+1;
    return *s;
}

//GET_COMMAND-------------------------------------------------------------------------------------
struct command Get_Command(struct command *c, char *argv[], struct Table T){
   int arg_pointer = c->S.arg_pos;
    char buffer[1000] = {0};
    bool temp = 0;

    if(argv[cmd_location][arg_pointer] == '\0'){
        return *c;
    }

    //Ak sú 2 alebo viac selekcii za sebou
    if(argv[cmd_location][c->S.arg_pos] == '['){
        c->S.arg_pos = arg_pointer;
        while(argv[cmd_location][c->S.arg_pos] == '['){
            Get_Selection(&c->S, argv, &temp, T);
        }
    }
    arg_pointer = c->S.arg_pos;

    while(argv[cmd_location][arg_pointer] != ';'  && argv[cmd_location][arg_pointer] != '\0'){
        arg_pointer++;
    }

    strncpy(buffer, argv[cmd_location]+c->S.arg_pos, arg_pointer - c->S.arg_pos);
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    c->str = line;
    c->S.arg_pos = arg_pointer;
    return *c;
}

//MAKE_CHANGES------------------------------------------------------------------------------------
struct Table Make_Changes(struct command c, struct Table *T, char *delimiter){

//Prikazy na upravu tabulky***********************************

    if(!strcmp(c.str, "irow")){
        irow(c, T, delimiter);
    }

    else if(!strcmp(c.str, "arow")){
        arow(c, T, delimiter);
    }

    else if(!strcmp(c.str, "drow")){
        drow(c, T);
    }

    else if(!strcmp(c.str, "icol")){

        if(c.S.col == -5){
            c.S.col = 1;
        }

        if(c.S.row == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                command c2 = c;
                c2.S.row = i;
                icol(c2, T, delimiter);
            }
        }

        else{
            icol(c, T, delimiter);
        }
        T->c.num_of_cols++;
    }  

//ACOL--------------------------------------------------------------------------------
    else if(!strcmp(c.str, "acol")){

        if(c.S.col == -5){
            c.S.col = T->c.num_of_cols+1;

        }

        if(c.S.row == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                command c2 = c;
                c2.S.row = i;
                acol(c2, T, delimiter);
            }
        }

        else{
            acol(c, T, delimiter);
        }
        T->c.num_of_cols++;
    }

//DCOL------------------------------------------------------------------------------
    else if(!strcmp(c.str, "dcol")){
        if(c.S.row == -5 && c.S.col == -5){
            drow(c, T);
        }

        else if(c.S.row == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                command c2 = c;
                c2.S.row = i;
                dcol(c2, T, delimiter);
            }
        }

        else if(c.S.col == -5){
            drow(c,T);
        }

        else{
            dcol(c, T, delimiter);
        }
    }

//*************************************************************

//Príkazy na úpravu obsahu buniek********************************************

//SET--------------------------------------------------------------------------------------
    else if(!strncmp(c.str, "set", 3)){

        if(c.S.row == -5 && c.S.col == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                c.S.row = i;
                for(int j = 1; j <= T->c.num_of_cols+1; j++){
                    c.S.col = j;
                    set(c, T, delimiter);
                }
            }
        }

        else if(c.S.row == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                c.S.row = i;
                set(c, T, delimiter);
            }
        }

        else if(c.S.col == -5){
            for(int i = 1; i <= T->c.num_of_cols+1; i++){
                c.S.col = i;
                set(c, T, delimiter);
            }
        }

        else{
            set(c, T, delimiter);
        }
    }

    //CLEAR------------------------------------------------------------------------------------
    else if(!strcmp(c.str, "clear")){

        if(c.S.row == -5 && c.S.col == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                c.S.row = i;
                for(int j = 1; j <= T->c.num_of_cols+1; j++){
                    c.S.col = j;
                    clear(c, T, delimiter);
                }
            }
        }

        else if(c.S.row == -5){
            for(int i = 1; i <= T->r.num_of_rows; i++){
                c.S.row = i;
                clear(c, T, delimiter);
            }
        }

        else if(c.S.col == -5){
            for(int i = 1; i <= T->c.num_of_cols+1; i++){
                c.S.col = i;
                clear(c, T, delimiter);
            }
        }

        else{
            clear(c, T, delimiter);
        }
    }

    else{
        fprintf(stderr, "ERROR: Unrecognized command!");
        exit(ERR);
    }
//***************************************************************************
return *T;
}
//---------------------------------------------------------------------------------------------

//Check Selection------------------------------------------------------------
void check_X_Y(struct selection s, struct Table T){

        if(s.row < 1 || s.col < 1 || s.row > T.r.num_of_rows || !s.row || !s.col){
            if(s.row == -5 || s.col == -5){
                return;
            }
            fprintf(stderr, "ERROR: Invalid Selection \n");
            exit(ERR);
        }
        else{
            return;
}
}

//AROW-------------------------------------------------------------------------
struct Table arow(struct command c, struct Table *T, char *delimiter){
    int current_Pointer = T->r.num_of_rows-1;
    char delim[1000] = {0};
    int row = c.S.row-1;

    //Ak row = '_'
    if(c.S.row == -5){
        row = T->r.num_of_rows-1;
    }

    //Presunie riadky 
    while(current_Pointer > row){
        T->table[current_Pointer+1] = T->table[current_Pointer];
        current_Pointer--;
    }
    T->r.num_of_rows += 1;

    if(row+1 != T->r.num_of_rows-1){
        for(int i = 0; i < T->c.num_of_cols; i++) delim[i] = delimiter[0];
        delim[T->c.num_of_cols] = '\n';
    }

    else
    {   
        delim[0] = '\n';
        for(int i = 1; i < T->c.num_of_cols+1; i++) delim[i] = delimiter[0];
    }

    //Pridelí prázdny riadok tam kde má byť
    int buf_len = strlen(delim);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, delim);
    T->table[row+1] = line;
    return *T;
}

//IROW--------------------------------------------------------------------------
struct Table irow(struct command c, struct Table *T, char *delimiter){

    int current_Pointer = T->r.num_of_rows-1;
    char delim[1000] = {0};
    int row = c.S.row-1;

    //Ak row = '_'
    if(c.S.row == -5){
        row = 0;
    }

    //Presunie riadky
    while(current_Pointer >= row){
        T->table[current_Pointer+1] = T->table[current_Pointer];
        current_Pointer--;
    }
    T->r.num_of_rows += 1;

        for(int i = 0; i < T->c.num_of_cols; i++) delim[i] = delimiter[0];
        delim[T->c.num_of_cols] = '\n';

    //Pridelí prázdny riadok tam kde má byť
        int buf_len = strlen(delim);
        char *line = (char *)malloc((buf_len + 1) * sizeof(char));
        strcpy(line, delim);
        T->table[row] = line;
        return *T;
}

//DROW---------------------------------------------------------------------------
struct Table drow(struct command c, struct Table *T){

    int current_Pointer = c.S.row-1;

    //Ak row = '_'
    if(c.S.row == -5){
        current_Pointer = 0;
        while(current_Pointer < T->r.num_of_rows){
            free(T->table[current_Pointer]);
            current_Pointer++;
        }
        T->r.num_of_rows = 0;
    }

    //Inak prepíše riadky (Na miesto riadku na vymazanie da riadok ktorý je od neho nižšie a tak ďalej)
    else{
        while(current_Pointer+1 < T->r.num_of_rows){
            T->table[current_Pointer] = T->table[current_Pointer+1];
            current_Pointer++;
        }
        T->r.num_of_rows -= 1;
    }
    return *T;
}

//GET_POSITION-------------------------------------------------------------------
//Najde pozíciu prvého a posledného znaku vybranej bunky
void Get_Position(int col, int row, int *pos_x, int *pos_y, struct Table T, char *delimiter, int *pocet_znakov_v_riadku){
    int pocet_znakov = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;
    int pocet_delimiterov = 0;
    int del_len = strlen(delimiter);

    while(T.table[row][pocet_znakov] != '\0'){

        if(pocet_delimiterov == col-1 && zapis_x == 0){
            *pos_x = pocet_znakov;
            zapis_x = 1;
        }

        for(int i = 0; i < del_len; i++){
            if(T.table[row][pocet_znakov] == delimiter[i]) pocet_delimiterov++;
        }

        if(pocet_delimiterov == col && zapis_y == 0){
            *pos_y = pocet_znakov-1;
            zapis_y = 1;
        }
        pocet_znakov++;
    }

    if(zapis_y == 0 && T.table[row][pocet_znakov-1] == '\n'){
        *pos_y = pocet_znakov-2;
        zapis_y = 1;
    }

    else if(zapis_y == 0 && T.table[row][pocet_znakov-1] != '\n'){
        *pos_y = pocet_znakov-1;
        zapis_y = 1;
    }

    if(zapis_x == 0 || zapis_y == 0){
        fprintf(stderr, "ERROR: Selection syntax error!");
        exit(ERR);
    }
    *pocet_znakov_v_riadku = pocet_znakov;
}

//ICOL---------------------------------------------------------------------------
struct Table icol(struct command c, struct Table *T, char *delimiter){

    int pos_x;
    int pos_y;
    int pocet_znakov_v_riadku;
    int current_pointer = c.S.row-1;
    char buffer[1000] = {0};
    int char_pointer = 0;

    //Zoberie pozíciu bunky a skopíruje riadok do buffera
    Get_Position(c.S.col ,current_pointer, &pos_x, &pos_y, *T, delimiter, &pocet_znakov_v_riadku);
    strcpy(buffer, T->table[current_pointer]);

    char_pointer = pocet_znakov_v_riadku;

    //Upraví buffer
    while(char_pointer >= pos_x){
        buffer[char_pointer+1] = buffer[char_pointer];
        char_pointer--;
    }

    //Predá buffer tabulke
    buffer[pos_x] = delimiter[0];
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    free(T->table[current_pointer]);
    T->table[current_pointer] = line;

    return *T;
}

//ACOL----------------------------------------------------------------------------
struct Table acol(struct command c, struct Table *T, char *delimiter){
    int pos_x;
    int pos_y;
    int pocet_znakov_v_riadku;
    int current_pointer = c.S.row-1;
    char buffer[1000] = {0};
    int char_pointer = 0;

    //Zoberie pozíciu bunky a skopíruje riadok do buffera
    Get_Position(c.S.col ,current_pointer, &pos_x, &pos_y, *T, delimiter, &pocet_znakov_v_riadku);
    strcpy(buffer, T->table[current_pointer]);

    char_pointer = pocet_znakov_v_riadku;
    
    //Upraví buffer
    while(char_pointer >= pos_y+1){
        buffer[char_pointer+1] = buffer[char_pointer];
        char_pointer--;
    }

    buffer[pos_y+1] = delimiter[0];
    
    //Predá buffer tabulke
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    free(T->table[current_pointer]);
    T->table[current_pointer] = line;
    return *T;
}

//DCOL---------------------------------------------------------------------------
struct Table dcol(struct command c, struct Table *T, char *delimiter){
    int pos_x;
    int pos_y;
    int pocet_znakov_v_riadku;
    int current_pointer = c.S.row-1;
    char buffer[1000] = {0};

    //Zoberie pozíciu bunky a skopíruje riadok do buffera
    Get_Position(c.S.col ,current_pointer, &pos_x, &pos_y, *T, delimiter, &pocet_znakov_v_riadku);
    strcpy(buffer, T->table[current_pointer]);

    //Upraví buffer
    if(c.S.col == T->c.num_of_cols+1){
        memset(buffer+pos_x-1, 0, pos_y-pos_x+1);
        while(pos_y+1 <= pocet_znakov_v_riadku){ //Posunie znaky na správne miesto
        buffer[pos_x-1] = buffer[pos_y+1];
        buffer[pos_y+1] = 0;
        pos_x++;
        pos_y++;
    }
    }

    else{
        memset(buffer+pos_x, 0, pos_y-pos_x+2);
        while(pos_y+2 <= pocet_znakov_v_riadku){ //Posunie znaky na správne miesto
        buffer[pos_x] = buffer[pos_y+2];
        buffer[pos_y+2] = 0;
        pos_x++;
        pos_y++;
    }
    }

    //Predá buffer tabulke
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    free(T->table[current_pointer]);
    T->table[current_pointer] = line;

    return *T;
}

//SET--------------------------------------------------------------------------
struct Table set(struct command c, struct Table *T, char *delimiter){
    int pos_x;
    int pos_y;
    int pocet_znakov_v_riadku;
    int current_pointer = c.S.row-1;
    char buffer[1000] = {0};
    char str[1000] = {0};
    
    //Zoberie pozíciu bunky a skopíruje riadok do buffera
    strncpy(str, c.str+4, strlen(c.str) - 4);
    Get_Position(c.S.col ,current_pointer, &pos_x, &pos_y, *T, delimiter, &pocet_znakov_v_riadku);
    strcpy(buffer, T->table[current_pointer]);
    int difference = ((pos_y - pos_x+1) - strlen(str));

    //Upraví buffer
    while(difference > 0){
        int temp = pos_y+1;

        while(temp <= pocet_znakov_v_riadku){
            buffer[temp-1] = buffer[temp];
            temp++;
        }
        pos_y--;
        temp = pos_y+1;
        difference--;
        pocet_znakov_v_riadku--;
    }

    while(difference < 0){
        int temp = pocet_znakov_v_riadku;

        while(temp >= pos_y+1){
            buffer[temp+1] = buffer[temp];
            temp--;
        }
        pocet_znakov_v_riadku++;
        difference++;
    }

    strncpy(buffer+pos_x, str, strlen(str));

    //Predá buffer tabulke
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    free(T->table[current_pointer]);
    T->table[current_pointer] = line;

    return *T;
}

//CLEAR-----------------------------------------------------------------------------------
struct Table clear(struct command c, struct Table *T, char *delimiter){
    int pos_x;
    int pos_y;
    int pocet_znakov_v_riadku;
    int current_pointer = c.S.row-1;
    char buffer[1000] = {0};

    //Zoberie pozíciu bunky a skopíruje riadok do buffera
    Get_Position(c.S.col ,current_pointer, &pos_x, &pos_y, *T, delimiter, &pocet_znakov_v_riadku);
    strcpy(buffer, T->table[current_pointer]);

    int temp = pos_x;

    //Upraví buffer
    for(int i = 0; i < pos_y-pos_x+1; i++){
        while(temp+1 <= pocet_znakov_v_riadku){
            buffer[temp] = buffer[temp+1];
            temp++;
        }
        temp = pos_x;
    }

    //Predá buffer tabulke
    int buf_len = strlen(buffer);
    char *line = (char *)malloc((buf_len + 1) * sizeof(char));
    strcpy(line, buffer);
    free(T->table[current_pointer]);
    T->table[current_pointer] = line;

    return *T;
}
