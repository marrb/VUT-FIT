#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_ARGUMENT_SIZE 10
#define MAX_DELIMITER_COUNT 30
#define MAX_CHARACTERS_PER_CELL 100
#define ERR 1

typedef struct selection{
    char argument[MAX_ARGUMENT_SIZE]; //Použítý príkaz na selkciu riadkov
    char str[MAX_CHARACTERS_PER_CELL];  //Retazec na ulozenie parametra v beginswith a contains
    int start_row; //Zaciatocný riadok v argumente rows
    int end_row;    //Koncový riadok v argumente rows
    int col_num;    //Číslo stĺpca ktorý sa ma prehladávať pri selekcii
    int position_of_argument; //Pozicia príkazu selekcie v shell
    bool r_active; //Použitý ROWS ?
    bool b_active; //Použitý BEGINSWITH ?
    bool c_active; //Použitý CONTAINS ?
}selections;

void arow(int r,char table[][500],int pdl,char delimiter);
void irow(int x, int r,char table[][500],int pdl,char delimiter);
void drow(int x, int r,char table[][500]);
void drows(int x, int *y, int r,char table[][500]);
void acol(int r, char table[][500], char delimiter);
void icol(int x, int r, char table[][500], char delimiter, int pdl);
void dcol(int x, int r, char table[][500], char delimiter, int pdl);
void dcols(int x, int *y, int r, char table[][500], char delimiter, int pdl);
void cset(int r, int c, char table[][500], char delimiter, char STR[100], struct selection s);
void tolowup(int r, int c, char table[][500], char delimiter, struct selection s, char argument[10]);
void roint_f(int r, int c, char table[][500], char delimiter, struct selection s, char argument[10]);
void copy(int x, int y, int r, char table[][500], char delimiter, struct selection s);
void swap_f(int x, int y, int r, char table[][500], char delimiter, struct selection s);
void move(int x, int y, int r, char table[][500], char delimiter, int pdl, struct selection s);

struct selection Get_Info(char *argv[], int argc, int position, int r){ //Vracia (štruktúru) info o požitom príkaze na selekciu a jeho parametre
    struct selection s1;
    s1.r_active = 0;
    s1.b_active = 0;
    s1.c_active = 0;

    while(position < argc){
        if(!strcmp(argv[position], "rows")){
            strcpy(s1.argument, argv[position]);
            if(!strcmp(argv[position+1], "-")){
                s1.start_row = r+1;
            }

            else if(!strcmp(argv[position+2], "-")){
                s1.end_row = r+1;
            }
            else{
            s1.start_row = atoi(argv[position+1]);
            s1.end_row = atoi(argv[position+2]);
            }

            s1.position_of_argument = position;
            s1.r_active = 1;
            return s1;
        }
        else if(!strcmp(argv[position], "beginswith")){
            strcpy(s1.argument, argv[position]);
            s1.position_of_argument = position;
            
            if(strlen(argv[position+2]) > MAX_CHARACTERS_PER_CELL){
                fprintf(stderr, "WARNING: 'STR' parameter on 'beginswith' exceeded 100 characters");
                s1.b_active = 0;
                return s1;
            }
            else{
                strcpy(s1.str, argv[position+2]);
                s1.col_num = atoi(argv[position+1]);
                s1.b_active = 1;
                return s1;
            }
        }
        else if(!strcmp(argv[position], "contains")){
            strcpy(s1.argument, argv[position]);
            s1.position_of_argument = position;

            if(strlen(argv[position+2]) > MAX_CHARACTERS_PER_CELL){
                fprintf(stderr, "WARNING: 'STR' parameter on 'contains' exceeded 100 characters");
                s1.c_active = 0;
                return s1;
            }
            else{
                strcpy(s1.str, argv[position+2]);
                s1.col_num = atoi(argv[position+1]);
                s1.c_active = 1;
                return s1;
            }

        }
        position++;
    }
    return s1;
}

bool beginswith(struct selection s, char table[][500], char delimiter, int i){ //Vracia 1 ak sa má riadok spracovávať, 0 ak nie

    int pocet_znakov_v_riadku = 0;
    int pos_x;
    int pos_y;
    int pocet_delimiterov = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};
    char buffer2[MAX_CHARACTERS_PER_CELL] = {0};

    strcpy(buffer, s.str);

        while(table[i][pocet_znakov_v_riadku] != '\0'){             //Prejde celý riadok a nájde začiatočnú a koncovú pozícu hľadaného stĺpca
            if(pocet_delimiterov == s.col_num-1 && zapis_x == 0){
                pos_x = pocet_znakov_v_riadku;
                zapis_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }


            if((pocet_delimiterov == s.col_num && zapis_y == 0)){
                pos_y = pocet_znakov_v_riadku-1;
                zapis_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){   //Ak sa koncový znak nenašiel tak nastaví pozíciu Y podľa toho či je to posledný riadok v tabuľke alebo nie
            pos_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_y = pocet_znakov_v_riadku-1;
        }

        pos_y = pos_x+strlen(buffer)-1;
        strncpy(buffer2, table[i]+pos_x, pos_y-pos_x+1);

        if(strstr(buffer, buffer2)){
            return true;
        }
        else{
            return false;
        }
}

bool contains(struct selection s, char table[][500], char delimiter, int i){ //Vracia 1 ak sa má riadok spracovávať, 0 ak nie

    int pocet_znakov_v_riadku = 0;
    int pos_x;
    int pos_y;
    int pocet_delimiterov = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};
    char buffer2[MAX_CHARACTERS_PER_CELL] = {0};

    strcpy(buffer, s.str);

        while(table[i][pocet_znakov_v_riadku] != '\0'){          //Prejde celý riadok a nájde začiatočnú a koncovú pozícu hľadaného stĺpca
            if(pocet_delimiterov == s.col_num-1 && zapis_x == 0){
                pos_x = pocet_znakov_v_riadku;
                zapis_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }


            if((pocet_delimiterov == s.col_num && zapis_y == 0)){
                pos_y = pocet_znakov_v_riadku-1;
                zapis_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){ //Ak sa koncový znak nenašiel tak nastaví pozíciu Y podľa toho či je to posledný riadok v tabuľke alebo nie
            pos_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_y = pocet_znakov_v_riadku-1;
        }
        strncpy(buffer2, table[i]+pos_x, pos_y-pos_x+1);
        char *ptr = strstr(buffer2, buffer);

        if(ptr != NULL){
            return 1;
        }
        else{
            return 0;
        }
}


int roundNo(float num) //Zaokruhluje číslo
{ 
    return num < 0 ? num - 0.5 : num + 0.5; 
} 

bool check_X(int x_Value, char argument[10]){  //Kontroluje správnosť parametru pre príkazy s 1 parametrom, vracia 0 ak je v poriadku, 1 ak je chybný a následne vypíše chybu
    if(!strcmp(argument, "drow") || !strcmp(argument, "irow")){
        if(x_Value < 1 || !x_Value ){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
    }
    else if(!strcmp(argument, "icol") || !strcmp(argument, "dcol") || !strcmp(argument, "cset") || !strcmp(argument, "tolower") || !strcmp(argument, "toupper") || !strcmp(argument, "round") || !strcmp(argument, "int")){
        if(!x_Value || x_Value < 1){
        fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
        return 1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

bool check_X_Y(int x_Value, int y_Value, char argument[10]){ //Kontroluje správnosť parametru pre príkazy s 2 parametrami, vracia 0 ak sú v poriadku, 1 ak sú chybné a následne vypíše chybu
    if(!strcmp(argument, "drows")){
        if(x_Value > y_Value || x_Value < 1 || y_Value < x_Value || !x_Value || !y_Value){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
    }

    else if(!strcmp(argument, "dcols") || !strcmp(argument, "copy")){
        if(x_Value > y_Value || x_Value < 1 || y_Value < x_Value || !x_Value || !y_Value){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

bool check_CSM(int x_Value, int y_Value, char argument[10]){  //Kontroluje správnosťi parametrov pre príkazy Copy, Swap, Move
        if(x_Value < 1 || y_Value < 1 || !x_Value || !y_Value){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
}

bool check_argc(int argPointer,int argc, char argument[10]){ //Kontrola či bol parameter pre príkazy zadaný, zabezpečuje aby sa nepriradila hodnota, ktorá sa už nenachádza v rozmedzí argc
    if(argPointer+1 == argc){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
}

bool check_Selection(int start_row, int end_row, char argument[10], int pdl_Value, int col_num){ //Kontroluje správnosťi parametrov pre príkazy selekcie riadkov
    if(!strcmp(argument, "rows")){
        if(start_row > end_row || start_row < 1 || end_row < start_row || !start_row || !end_row){
            fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
            return 1;
        }
        else{
            return 0;
        }
        return 0;
    }

    else if(!strcmp(argument, "contains") || !strcmp(argument, "beginswith")){
        if(col_num > pdl_Value+1 || !col_num || col_num < 1){
        fprintf(stderr, "WARNING: Invalid parameter on '%s' argument! \n", argument);
        return 1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

int main(int argc, char ** argv){ 

    char c; //Charakter na zápis do tabuľky
    char table[1024][500] = {0}; //Tabuľka
    char delimiter[MAX_DELIMITER_COUNT];//Reťazec pre delimiter, reťazec DELIM môže mať max. 30 znakov

    bool del_a = 0; //Nastaví sa na 1 ak je zvolený delimiter
    bool f_row = 1; //Označuje prvý riadok v tabuľke, ktorý nieje prázdny
    bool data_edit = 0; //Nastaví sa na 1 ak už bol použitý príkaz na spracovanie dát v tabuľke

    int r = 0;  //Poćet riadkov
    int s = 0; //Počet znakov v riadku
    int pdl = 0; //Počet delimiterov v 1 riadku

    selections s1;
    selections s2;
    
    //CHECK ARGUMENTS AND FILE ---------------------------------------------------------------------------------

    if(argc < 2){ //Kontroluje či boli zadané argumenty
        fprintf(stderr, "Error: Please input arguments!");
        return ERR;
    }
    else if ((fseek(stdin, 0, SEEK_END), ftell(stdin)) > 0) //Ďalej kontroluje či je súbor prázdny
    {
        rewind(stdin);
    }
    else{
        fprintf(stderr, "ERROR: File is empty or not found!");
        return ERR;
    }

    //DELIMITER ----------------------------------------------------------------------------------------------

    delimiter[0]=' '; //Delimiter sa nastaví na default hodnotu

    for(int i = 1; i < argc; i++){  //Skrontroluje či bol zadaný reťazec delim
        if(!strcmp(argv[i], "-d") && argc != i+1){
            if(strlen(argv[i+1])> MAX_DELIMITER_COUNT){
                fprintf(stderr, "ERROR: Delimiter exceeded 30 characters!");
                return ERR;
            }
            strcpy(delimiter, argv[i+1]);
            del_a = 1; //Nastaví že bol zadaný
            break;
        }
    }

    //GET TABLE ----------------------------------------------------------------------------------------------

    while((c = getchar()) == '\n'){ //Zápis tabuľky do pola, najprv zapíše a preskočí prázdne riadky
        table[r][s]=c;
        r++;
    }

    ungetc(c, stdin); //Vráti posledny znak naspäť na vstup

    int num_chars_between_delimiters = 0; //Počet znakov v stĺpci

    while((c = getchar()) != EOF){
        if(del_a == 1 && c != delimiter[0]){ //Ak sa znak nachádza v reťazci DELIM tak ho zamení za prvý znak v DELIM
            for(int i = 1; i < MAX_DELIMITER_COUNT; i++){
                if(c == delimiter[i]){
                    c = delimiter[0];
                    break;
                }
            }
        }
        
        if(c == delimiter[0]) { //Ak sa našiel znak z reťazca DELIM, vynuluje počet znakov v stĺpci
            num_chars_between_delimiters = 0;
        }
        else {
            num_chars_between_delimiters ++;
        }


        if(num_chars_between_delimiters > MAX_CHARACTERS_PER_CELL) {
            fprintf(stderr, "ERROR: Max number of characters in cell exceeded!");
            return ERR;
        }


        table[r][s]=c;
        s++;

        if(r == 1023 || s == 499){  //Kontrola či zadaná tabuľka nieje moc veľká
            fprintf(stderr, "Error: File too big!");
            return ERR;
        }

        if(c == delimiter[0] && f_row == 1){
            pdl++; //Ak najde delimiter a nachádzame sa v 1 riadku, tak pripočita 1 k počtu delimiterov (stĺpec)
        }

        if(c == '\n'){
            f_row = 0;
            r++;
            s = 0;
            num_chars_between_delimiters = 0;
        }
    }

//GET_SELECTION------------------------------------------------------------------------------------

s1 = Get_Info(argv, argc, 1, r);  //Skontroluje čo bol použitý príkaz na selekciu riadkov
s2 = Get_Info(argv, argc, s1.position_of_argument+1, r); 

if(s2.r_active == 1 || s2.b_active == 1 || s2.c_active == 1){ //Ak boli použíte 2 vracia chybu
    fprintf(stderr, "ERROR: Only 1 command for selection is allowed!");
    return ERR;
}

if(s1.end_row > r+1){ 
    s1.end_row = r+1;
}

check_Selection(s1.start_row, s1.end_row, s1.argument, pdl, s1.col_num); //Kontrola správnosti parametrov selekcie

//TABLE EDIT----------------------------------------------------------------------------------------

    int x,y; //Premenné pre parametre argumentov
    int argPointer = 1;

    while(argPointer < argc){ //Prejdenie argumentov a ich vykonanie
        if(!strcmp(argv[argPointer], "arow")){
            arow(r, table, pdl, delimiter[0]);
            r++;
            argPointer++;
            continue;
        }

        else if(!strcmp(argv[argPointer], "irow")){
            x = atoi(argv[argPointer+1]); 
            if(check_X(x, argv[argPointer])) return ERR;

            if(x > r+1){
                argPointer+=2;
                continue;
            }

            else{
                irow(x, r, table, pdl, delimiter[0]);
                r++;
                argPointer+=2;
                continue;
            }
        }

        else if(!strcmp(argv[argPointer], "drow")){
            x = atoi(argv[argPointer+1]);
            y = x;
            if(check_X(x, argv[argPointer])) return ERR;

            else if(x > r+1){
                argPointer+=3;
                continue;
            }
            
            else{
            drows(x, &y, r, table);
            r = r-((y+1)-x);
            argPointer+=2;
            continue;
            }
        }

        else if(!strcmp(argv[argPointer], "drows")){
            if(check_argc(argPointer, argc, argv[argPointer])) return ERR;
            x = atoi(argv[argPointer+1]);
            y = atoi(argv[argPointer+2]);

            if(check_X_Y(x, y, argv[argPointer])) return ERR;

            else if(x > r+1){
                argPointer+=3;
                continue;
            }

            else{
            drows(x, &y, r, table);
            r = r-((y+1)-x);
            argPointer+=3;
            continue;
            }
        }

        else if(!strcmp(argv[argPointer], "acol")){
            acol(r, table, delimiter[0]);
            pdl++;
            argPointer++;
            continue;
        }

        else if(!strcmp(argv[argPointer], "icol")){
            x = atoi(argv[argPointer+1]);
            if(check_X(x, argv[argPointer])) return ERR;
                
            else if(x > pdl+1){
                argPointer+=2;
                continue;
            }

            else{
                icol(x, r, table, delimiter[0], pdl);
                pdl++;
                argPointer+=2;
                continue;
            }
        }

        else if(!strcmp(argv[argPointer], "dcol")){
            x = atoi(argv[argPointer+1]);
            y = x;
            if(check_X(x, argv[argPointer])) return ERR;

            else if(x > pdl+1){
                argPointer+=2;
                continue;
            }

            else{
                dcols(x, &y, r, table, delimiter[0], pdl);
                argPointer+=2;
                pdl--;
                continue;
            }
        }
        
        else if(!strcmp(argv[argPointer], "dcols")){
            if(check_argc(argPointer, argc, argv[argPointer])) return ERR;
            x = atoi(argv[argPointer+1]);
            y = atoi(argv[argPointer+2]);

            if(check_X_Y(x, y, argv[argPointer])) return ERR;

            else if(x > pdl+1){
                argPointer+=3;
                continue;
            }

            else{
                dcols(x, &y, r, table, delimiter[0], pdl);
                pdl = pdl-((y+1)-x);
                argPointer+=3;
                continue;
            }
        }

        else if(!strcmp(argv[argPointer], "cset")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }

            if(s1.position_of_argument > argPointer){
                s1.b_active = 0;
                s1.c_active = 0;
                s1.r_active = 0;
            }

            x = atoi(argv[argPointer+1]);
            if(check_X(x, argv[argPointer])){
                return ERR;
            }

            else if(x > pdl+1){
                argPointer+=2;
                continue;
            }


            if(strlen(argv[argPointer+2]) > MAX_CHARACTERS_PER_CELL){
                fprintf(stderr, "String 'STR' in 'cset' exceeded maximum characters per cell!");
                return ERR;
            }

            cset(r, x, table, delimiter[0], argv[argPointer+2], s1);
            data_edit = 1;
        }

        else if(!strcmp(argv[argPointer], "tolower") || !strcmp(argv[argPointer], "toupper")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }

            x = atoi(argv[argPointer+1]);
            if(x > pdl+1){
                argPointer+=2;
                continue;
            }

            if(check_X(x, argv[argPointer])){
                return ERR;
            }
                tolowup(r, x, table, delimiter[0], s1, argv[argPointer]);
                data_edit = 1;
        }

        else if(!strcmp(argv[argPointer], "round") || !strcmp(argv[argPointer], "int")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }
            x = atoi(argv[argPointer+1]);

            if(x > pdl+1){
                argPointer++;
                continue;
            }

            if(check_X(x, argv[argPointer])){
                return ERR;
            }

                roint_f(r, x, table, delimiter[0], s1, argv[argPointer]);
                data_edit = 1;
        }

        else if(!strcmp(argv[argPointer], "copy")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }
            x = atoi(argv[argPointer+1]);
            y = atoi(argv[argPointer+2]);

            if(check_CSM(x, y, argv[argPointer])){
                return ERR;
            }

            if(x > pdl+1 || y > pdl+1 || x == y){
                argPointer+=3;
                continue;
            }

            copy(x, y, r, table, delimiter[0], s1);
            data_edit = 1;
        }

        else if(!strcmp(argv[argPointer], "swap")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }

            x = atoi(argv[argPointer+1]);
            y = atoi(argv[argPointer+2]);

            if(check_CSM(x, y, argv[argPointer])){
                return ERR;
            }

            if(x > pdl+1 || y > pdl+1 || x == y){
                argPointer++;
                continue;
            }

                swap_f(x, y, r, table, delimiter[0], s1);
                data_edit = 1;
        }
        else if(!strcmp(argv[argPointer], "move")){
            if(data_edit == 1){
                fprintf(stderr, "ERROR: Only 1 command for data processing allowed!");
                return ERR;
            }
            x = atoi(argv[argPointer+1]);
            y = atoi(argv[argPointer+2]);

            if(check_CSM(x, y, argv[argPointer])){
                return ERR;
            }

            if(x > pdl+1 || y > pdl+1 || x == y){
                argPointer++;
                continue;
            }

            move(x, y, r, table, delimiter[0], pdl, s1);
            data_edit = 1;
        }
        argPointer++;
    }  
    
    for(int i = 0; i <= r; i ++) printf("%s", table[i]);
    return 0;
}

//AROW FUNCTION-------------------------------------------------------------------------------

void arow(int r, char table[][500], int pdl, char delimiter){
    int pocet_znakov_v_riadku = 0;
    while(table[r][pocet_znakov_v_riadku] != '\0'){ //Spočíta počet znakov v poslednom riadku
        pocet_znakov_v_riadku++;
    }

    table[r][pocet_znakov_v_riadku] = '\n'; //Za posledny znak zapíše znak konca riadku
    table[r][pocet_znakov_v_riadku+1]= '\0';

    for (int i = 0; i < pdl; i++){ //Prída do riadku delimitre (stĺpce)
        table[r+1][i] = delimiter;
    }
}

//IROW FUNCTION------------------------------------------------------------------------------

void irow(int x, int r, char table[][500], int pdl, char delimiter){
    int curentPointer = r+1;
    int pocet_znakov_v_riadku = 0;

    while(x <= curentPointer){
        while(table[curentPointer-1][pocet_znakov_v_riadku] != '\0'){ //Spočíta počet znakov v riadku
            pocet_znakov_v_riadku++;
        }

        strncpy(table[curentPointer], table[curentPointer-1], pocet_znakov_v_riadku); //Kopíruje riadky do riadku pod nimi (zväčśuje tabuľku)

        memset(table[curentPointer-1], 0, pocet_znakov_v_riadku); //Vymaže miesto kde sa nachádzal riadok
        curentPointer--;
        pocet_znakov_v_riadku = 0;
    }

    for(int i = 0; i < pdl; i++){
        table[curentPointer][i] = delimiter; //Prída stĺpce do pridaného riadku
    }
    table[curentPointer][pdl] = '\n'; 
}

//DROWS + DROWS FUNCTION-----------------------------------------------------------------------------

void drows(int x, int *y, int r,char table[][500]){

    int x_Pointer = x-1;
    int y_Pointer = *y-1;
    int pocet_znakov_v_riadku = 0;

    if(y_Pointer > r){
        y_Pointer = r;
        *y = r+1;
    }

while (x_Pointer  <= y_Pointer){
    while(table[x_Pointer][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku, následne ich vymaže 
        pocet_znakov_v_riadku++;
    }

    memset(table[x_Pointer], 0, pocet_znakov_v_riadku);
    x_Pointer++;
    pocet_znakov_v_riadku = 0;
}
x_Pointer = x - 1;
while(y_Pointer+1 <= r){
    while(table[y_Pointer+1][pocet_znakov_v_riadku] != '\0'){ 
        pocet_znakov_v_riadku++;
    }
    strncpy(table[x_Pointer], table[y_Pointer+1], pocet_znakov_v_riadku); //Premiestni ostatné riadky (ktore ostali na konci) za posledný riadok
    memset(table[y_Pointer+1], 0, pocet_znakov_v_riadku);
    x_Pointer++;
    y_Pointer++;
    pocet_znakov_v_riadku = 0;
}
}

//ACOL FUNCTION-----------------------------------------------------------------------------

void acol(int r, char table[][500], char delimiter){
    int pocet_znakov_v_riadku = 0;
    for(int i = 0; i <= r; i++){

        while(table[i][0] == '\n'){
            i++;
        }
        
        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku
            pocet_znakov_v_riadku++;
        }

        if(i == r){ //Ak sa jedná o posledný riadok
        table[i][pocet_znakov_v_riadku] = delimiter; //Za posledny znak dá delimiter
        table[i][pocet_znakov_v_riadku+1] = '\0';
        pocet_znakov_v_riadku = 0;
        }
        else{ //Ak sa jedná o iný ako posledný
        table[i][pocet_znakov_v_riadku-1] = delimiter;
        table[i][pocet_znakov_v_riadku] = '\n';
        table[i][pocet_znakov_v_riadku+1] = '\0';
        pocet_znakov_v_riadku = 0;
        }
    }
}

//ICOL FUNCTION----------------------------------------------------------------------------

void icol(int x, int r, char table[][500], char delimiter, int pdl){
    int pocet_znakov_v_riadku = 0;
    int pozicia_na_zapis = 0;
    int pocet_delimiterov = 0;

    if(x > pdl+1){
        return;
    }

    for(int i = 0; i <= r; i++){
        while(table[i][0] == '\n'){ //Preskočí prázdne riadky
            i++;
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Hľadá  začiatok zadaného stĺpca

            if(pocet_delimiterov < x-1){
                pozicia_na_zapis++;
            }
            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }
            pocet_znakov_v_riadku++;
        }

        while(pozicia_na_zapis-1 <= pocet_znakov_v_riadku){ //Posunie všetky znaky do prava a potom na pozíciu pridá stĺpec
            table[i][pocet_znakov_v_riadku+2] = table[i][pocet_znakov_v_riadku+1];
            pocet_znakov_v_riadku--;
        }
        table[i][pozicia_na_zapis] = delimiter;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
        pozicia_na_zapis = 0;
    }
}

//DCOLS + DCOL FUNCTION------------------------------------------------------------------------

void dcols(int x, int *y, int r, char table[][500], char delimiter, int pdl){
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_x = 0;
    int pos_y = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;

    if(*y > pdl+1){
        *y = pdl+1;
    }

    for(int i = 0; i <= r; i++){

        if(table[i][0] == '\n') continue;

        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku a zapíše pozícu začiatočného a koncového znaku na vymazanie

            if(pocet_delimiterov == x-1 && zapis_x == 0){
                pos_x = pocet_znakov_v_riadku;
                zapis_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == *y && zapis_y == 0)){
                pos_y = pocet_znakov_v_riadku;
                zapis_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_x = pos_x-1;
            pos_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_x = pos_x-1;
            pos_y = pocet_znakov_v_riadku-1;
        }

        memset(table[i]+pos_x, 0, pos_y-pos_x+1); //Mazanie stĺpca

        while(pos_y < pocet_znakov_v_riadku){ //Posunie znaky na správne miesto
            table[i][pos_x] = table[i][pos_y+1];
            table[i][pos_y+1] = 0;
            pos_x++;
            pos_y++;
        }

        zapis_y = 0;
        zapis_x = 0;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
    }
}

//TOLOWER + TOUPPER------------------------------------------------------------------------------------------------

void tolowup(int r, int c, char table[][500], char delimiter, struct selection s, char argument[10]){

    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_x = 0;
    int pos_y = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;
    int i = 0;
    int end_row = r;

    if(s.r_active == 1){ //Ak je vybratá slekcia ROWS tak sa nastavia príslušné hodnoty
        i = s.start_row-1;
        end_row = s.end_row-1;
    }
    else{
        i = 0;
        end_row = r;
    }

    for(; i <= end_row; i++){

        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){  //Ak je vybratá slekcia BEGINSWITH tak kontroluje či sa má práve vybratý riadok spracovať
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){ //Ak je vybratá slekcia CONTAINS tak kontroluje či sa má práve vybratý riadok spracovať
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku a zapíše pozícu začiatočného a koncového znaku vo vybranom stĺpci

            if(pocet_delimiterov == c-1 && zapis_x == 0){
                pos_x = pocet_znakov_v_riadku;
                zapis_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == c && zapis_y == 0)){
                pos_y = pocet_znakov_v_riadku-1;
                zapis_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_y = pocet_znakov_v_riadku-1;
        }
        if(!strcmp(argument, "tolower")){ //Pri príkaze tolower sa UPPER-CASE znaky zmenia NA LOWER-CASE
            while(pos_x <= pos_y){
                table[i][pos_x] = tolower(table[i][pos_x]);
                pos_x++;
        }
        }
        else if(!strcmp(argument, "toupper")){ //Pri príkaze toupper zase naopak
            while(pos_x <= pos_y){
            table[i][pos_x] = toupper(table[i][pos_x]);
            pos_x++;
        }
        }
        pocet_delimiterov = 0;
        pocet_znakov_v_riadku = 0;
        zapis_y = 0;
        zapis_x = 0;
    }
}

//ROUND + INT----------------------------------------------------------------------------------------------------

void roint_f(int r, int c, char table[][500], char delimiter, struct selection s, char argument[10]){
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_x = 0;
    int pos_y = 0;
    bool zapis_x = 0;
    bool zapis_y = 0;
    int i = 0;
    int end_row = r;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};
    unsigned int buf_pointer = 0;

    if(s.r_active == 1){
        i = s.start_row-1;
        end_row = s.end_row-1;
    }

    for(; i <= end_row; i++) {                                                  
        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku a zapíše pozícu začiatočného a koncového znaku vo vybranom stĺpci

            if(pocet_delimiterov == c-1 && zapis_x == 0){
                pos_x = pocet_znakov_v_riadku;
                zapis_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == c && zapis_y == 0)){
                pos_y = pocet_znakov_v_riadku-1;
                zapis_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_y = pocet_znakov_v_riadku-1;
        }                                                                         
 
        if(pos_y < 0){
            pocet_delimiterov = 0;
            pocet_znakov_v_riadku = 0;
            zapis_y = 0;
            zapis_x = 0;
        }

        memset(buffer, 0, MAX_CHARACTERS_PER_CELL);
        strncpy(buffer, table[i] + pos_x, pos_y - pos_x + 1);

        bool nextIteration = true;
        int numberWidth = pos_y - pos_x + 1;

        for(int i = 0; i <= numberWidth; i ++) {
            if(isalpha(buffer[i])) {
                nextIteration = false;
                break;
            }
        }

        if(pos_y < pos_x){
            nextIteration = false;
        }

        if(nextIteration == true){ //Zaokrúhlenie čísla
        float convertedNumber = atof(buffer);
        if(!strcmp(argument, "round")){
            convertedNumber = roundNo(convertedNumber);
        }
        sprintf(buffer, "%d", (int)convertedNumber);
        
        for(int n = pos_x;buf_pointer <= strlen(buffer)-1; n++){
            table[i][n] = buffer[buf_pointer];
            buf_pointer++;
        }

        pos_x = pos_x + (strlen(buffer)-1);

        while(pos_y < pocet_znakov_v_riadku){
            table[i][pos_x+1]=table[i][pos_y+1];
            pos_x++;
            pos_y++;
        }

        memset(table[i]+pos_x,0, pos_y - pos_x+1);

        pocet_delimiterov = 0;
        pocet_znakov_v_riadku = 0;
        zapis_y = 0;
        zapis_x = 0;
        buf_pointer = 0;
}
        else{
            pocet_delimiterov = 0;
            pocet_znakov_v_riadku = 0;
            zapis_y = 0;
            zapis_x = 0;
            buf_pointer = 0;
        }
    }
}

//COPY-----------------------------------------------------------------------------------

void copy(int x, int y, int r, char table[][500], char delimiter, struct selection s){
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_copy_x;
    int pos_copy_y;
    int pos_dest_x;
    int temp;
    int pos_dest_y;
    int difference = 0;
    bool zapis_copy_x = 0;
    bool zapis_copy_y = 0;
    bool zapis_dest_x = 0;
    bool zapis_dest_y = 0;
    int i = 0;
    int end_row = r;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};

    if(s.r_active == 1){
        i = s.start_row-1;
        end_row = s.end_row-1;
    }

    for(; i <= end_row; i++){
        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){ //Počita počet znakov v riadku a zapíše pozícu začiatočného a koncového znaku vo vybraných stĺpcoch

            if(pocet_delimiterov == x-1 && zapis_copy_x == 0){
                pos_copy_x = pocet_znakov_v_riadku;
                zapis_copy_x = 1;
            }

            if(pocet_delimiterov == y-1 && zapis_dest_x == 0){
                pos_dest_x = pocet_znakov_v_riadku;
                zapis_dest_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == x && zapis_copy_y == 0)){
                pos_copy_y = pocet_znakov_v_riadku-1;
                zapis_copy_y = 1;
            }

            if((pocet_delimiterov == y && zapis_dest_y == 0)){
                pos_dest_y = pocet_znakov_v_riadku-1;
                zapis_dest_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_copy_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_copy_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_copy_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_copy_y = pocet_znakov_v_riadku-1;
        }
        if(zapis_dest_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_dest_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_dest_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_dest_y = pocet_znakov_v_riadku-1;
        }

        memset(buffer, 0, MAX_CHARACTERS_PER_CELL);

        difference = (pos_dest_y-pos_dest_x+1)-(pos_copy_y-pos_copy_x+1); //Vypočíta o koľko menši alebo väčší je 1 reťazec od druhého
        strncpy(buffer, table[i]+pos_copy_x, (pos_copy_y-pos_copy_x+1));

        temp = pocet_znakov_v_riadku;

        while(difference < 0){ //Podľa toho či je kopírovaný reťazec menśi alebo väčší tak zmenši alebo zväčši stĺpec
            while(temp >= pos_dest_y+1){
                table[i][temp+1] = table[i][temp];
                temp--;
            }
            pocet_znakov_v_riadku++;
            difference++;
            temp = pocet_znakov_v_riadku;
        }

        temp = (pos_dest_x+(pos_copy_y-pos_copy_x+1));

        while(difference > 0){
            while(temp < pocet_znakov_v_riadku){
                table[i][temp] = table[i][temp+1];
                temp++;
            }
            pocet_znakov_v_riadku--;
            difference--;
            temp = (pos_dest_x+(pos_copy_y-pos_copy_x+1));
        }
        memset(table[i]+pos_dest_x, 0, (pos_copy_y-pos_copy_x+1)); //vymaže stĺpec a prepíśe ho kopírovaným
        strncpy(table[i]+pos_dest_x, buffer, strlen(buffer)); //

        zapis_copy_x = 0;
        zapis_copy_y = 0;
        zapis_dest_x = 0;
        zapis_dest_y = 0;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
        difference = 0;
    }
}

void swap_f(int x, int y, int r, char table[][500], char delimiter, struct selection s){ //Pracuje na podobnom prinćipe ako copy len sa diferencia počíta 2 krát
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_buf1_x;
    int pos_buf1_y; 
    int pos_buf2_x;  
    int pos_buf2_y;  
    int temp;
    int difference = 0;
    bool zapis_x_b1 = 0;
    bool zapis_y_b1 = 0;
    bool zapis_x_b2 = 0;
    bool zapis_y_b2 = 0;
    int i = 0;
    int end_row = r;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};
    char buffer2[MAX_CHARACTERS_PER_CELL] = {0};

    if(x > y){
        temp = y;
        y = x;
        x = temp;
    }

    if(s.r_active == 1){
        i = s.start_row-1;
        end_row = s.end_row-1;
    }

    for(; i <= end_row; i++){
        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){

            if(pocet_delimiterov == x-1 && zapis_x_b1 == 0){
                pos_buf1_x = pocet_znakov_v_riadku;
                zapis_x_b1 = 1;
            }

            if(pocet_delimiterov == y-1 && zapis_x_b2 == 0){
                pos_buf2_x = pocet_znakov_v_riadku;
                zapis_x_b2 = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == x && zapis_y_b1 == 0)){
                pos_buf1_y = pocet_znakov_v_riadku-1;
                zapis_y_b1 = 1;
            }

            if((pocet_delimiterov == y && zapis_y_b2 == 0)){
                pos_buf2_y = pocet_znakov_v_riadku-1;
                zapis_y_b2 = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y_b1 == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_buf1_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y_b1 == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_buf1_y = pocet_znakov_v_riadku-1;
        }
        if(zapis_y_b2 == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_buf2_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y_b2 == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_buf2_y = pocet_znakov_v_riadku-1;
        }

        memset(buffer, 0, MAX_CHARACTERS_PER_CELL);
        memset(buffer2, 0, MAX_CHARACTERS_PER_CELL);

        strncpy(buffer, table[i] + pos_buf1_x, pos_buf1_y - pos_buf1_x + 1);
        strncpy(buffer2, table[i] + pos_buf2_x, pos_buf2_y - pos_buf2_x + 1);

        difference = (pos_buf1_y - pos_buf1_x+1)-(pos_buf2_y - pos_buf2_x+1);
        temp = pos_buf1_x+(pos_buf2_y-pos_buf2_x+1);

        while(difference > 0){
            while(temp <= pocet_znakov_v_riadku){
                table[i][temp] = table[i][temp+1];
                temp++;
            }
            difference--;
            temp = pos_buf1_x+(pos_buf2_y-pos_buf2_x+1);
            pocet_znakov_v_riadku--;
            pos_buf2_x--;
            pos_buf2_y--;
        }

        temp = pocet_znakov_v_riadku;

        while(difference < 0){
            while(temp >=pos_buf1_y+1){
                table[i][temp+1] = table[i][temp];
                temp--;
            }
            difference++;
            pocet_znakov_v_riadku++;
            temp = pocet_znakov_v_riadku;
            pos_buf2_x++;
            pos_buf2_y++;
        }

        strncpy(table[i]+pos_buf1_x, buffer2, strlen(buffer2));

        temp = pos_buf2_x+(pos_buf1_y-pos_buf1_x+1);
        difference = (pos_buf2_y - pos_buf2_x+1)-(pos_buf1_y - pos_buf1_x+1);

        while(difference > 0){
            while(temp <= pocet_znakov_v_riadku){
                table[i][temp] = table[i][temp+1];
                temp++;
            }
            difference--;
            temp = pos_buf2_x+(pos_buf1_y-pos_buf1_x+1);
            pocet_znakov_v_riadku--;
        }

        temp = pocet_znakov_v_riadku;

        while(difference < 0){
            while(temp >= pos_buf2_y+1){
                table[i][temp+1] = table[i][temp];
                temp--;
            }
            difference++;
            pocet_znakov_v_riadku++;
            temp = pocet_znakov_v_riadku;
        }

        strncpy(table[i]+pos_buf2_x, buffer, strlen(buffer));

        zapis_x_b1 = 0;
        zapis_y_b1 = 0;
        zapis_x_b2 = 0;
        zapis_y_b2 = 0;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
        difference = 0;
    }
}

void move(int x, int y, int r, char table[][500], char delimiter, int pdl, struct selection s){
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_buf1_x;
    int pos_buf1_y; 
    int pos_buf2_x;  
    int pos_buf2_y;  
    int temp;
    int difference = 0;
    bool zapis_x_b1 = 0;
    bool zapis_y_b1 = 0;
    bool zapis_x_b2 = 0;
    bool zapis_y_b2 = 0;
    int i = 0;
    int end_row = r;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};

    if(s.r_active == 1){
        i = s.start_row-1;
        end_row = s.end_row-1;
    }

    for(; i <= end_row; i++){
        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){

            if(pocet_delimiterov == x-1 && zapis_x_b1 == 0){
                pos_buf1_x = pocet_znakov_v_riadku;
                zapis_x_b1 = 1;
            }

            if(pocet_delimiterov == y-1 && zapis_x_b2 == 0){
                pos_buf2_x = pocet_znakov_v_riadku;
                zapis_x_b2 = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }

            if((pocet_delimiterov == x && zapis_y_b1 == 0)){
                pos_buf1_y = pocet_znakov_v_riadku-1;
                zapis_y_b1 = 1;
            }

            if((pocet_delimiterov == y && zapis_y_b2 == 0)){
                pos_buf2_y = pocet_znakov_v_riadku-1;
                zapis_y_b2 = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_y_b1 == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_buf1_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y_b1 == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_buf1_y = pocet_znakov_v_riadku-1;
        }
        if(zapis_y_b2 == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_buf2_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_y_b2 == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_buf2_y = pocet_znakov_v_riadku-1;
        }

        memset(buffer, 0, MAX_CHARACTERS_PER_CELL);
        strncpy(buffer, table[i] + pos_buf1_x, pos_buf1_y - pos_buf1_x + 1);
        buffer[pos_buf1_y - pos_buf1_x + 1] = delimiter;

        if(x == pdl+1){
            memset(table[i]+pos_buf1_x-1, 0, pos_buf1_y - pos_buf1_x + 4);
            if(i != r){
                table[i][pos_buf1_x-1]='\n';
                table[i][pos_buf1_x]='\0';
            }
            else{
                table[i][pos_buf1_x-1]='\0';
            }
            
        }
        else{
            memset(table[i]+pos_buf1_x, 0, pos_buf1_y - pos_buf1_x +2);
            difference = pos_buf1_y - pos_buf1_x+2;
            temp = pos_buf1_x;
            while(difference > 0){
                while(temp+1 <= pocet_znakov_v_riadku){
                    table[i][temp] = table[i][temp+1];
                    temp++;
                }
                pocet_znakov_v_riadku--;
                temp = pos_buf1_x;
                difference--;
                if(y != 1){
                pos_buf2_x--;
                pos_buf2_y--;
                }
            }
        }

        difference = -(pos_buf1_y-pos_buf1_x+2);
        temp = pocet_znakov_v_riadku;

        while(difference < 0){
            while(temp >= pos_buf2_x){
                table[i][temp+1] = table[i][temp];
                temp--;
            }
            pocet_znakov_v_riadku++;
            temp = pocet_znakov_v_riadku;
            difference++;
        }

        strncpy(table[i]+pos_buf2_x, buffer, pos_buf1_y-pos_buf1_x+2);

        zapis_x_b1 = 0;
        zapis_y_b1 = 0;
        zapis_x_b2 = 0;
        zapis_y_b2 = 0;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
    }
}



void cset(int r, int c, char table[][500], char delimiter, char STR[100], struct selection s){ //Taktiež pracuje podobne 
    int pocet_znakov_v_riadku = 0;
    int pocet_delimiterov = 0;
    int pos_dest_x;
    int pos_dest_y;
    int temp;
    int difference = 0;
    bool zapis_dest_x = 0;
    bool zapis_dest_y = 0;
    int i = 0;
    int end_row = r;
    char buffer[MAX_CHARACTERS_PER_CELL] = {0};

    if(s.r_active == 1){
        i = s.start_row-1;
        end_row = s.end_row-1;
    }

    strcpy(buffer, STR);

    for(; i <= end_row; i++){
        if(table[i][0] == '\n') continue;

        if(s.b_active == 1){
            if(!beginswith(s, table, delimiter, i)){
                continue;
            }
        }

        if(s.c_active == 1){
            if(!contains(s, table, delimiter, i)){
                continue;
            }
        }

        while(table[i][pocet_znakov_v_riadku] != '\0'){

            if(pocet_delimiterov == c-1 && zapis_dest_x == 0){
                pos_dest_x = pocet_znakov_v_riadku;
                zapis_dest_x = 1;
            }

            if(table[i][pocet_znakov_v_riadku] == delimiter){
                pocet_delimiterov++;
            }


            if((pocet_delimiterov == c && zapis_dest_y == 0)){
                pos_dest_y = pocet_znakov_v_riadku-1;
                zapis_dest_y = 1;
            }
            pocet_znakov_v_riadku++;
        }

        if(zapis_dest_y == 0 && table[i][pocet_znakov_v_riadku-1] == '\n'){
            pos_dest_y = pocet_znakov_v_riadku-2;
        }
        else if(zapis_dest_y == 0 && table[i][pocet_znakov_v_riadku-1] != '\n'){
            pos_dest_y = pocet_znakov_v_riadku-1;
        }
        difference = (pos_dest_y-pos_dest_x+1)-strlen(buffer);

        temp = pocet_znakov_v_riadku;

        while(difference < 0){
            while(temp >= pos_dest_y+1){
                table[i][temp+1] = table[i][temp];
                temp--;
            }
            pocet_znakov_v_riadku++;
            difference++;
            temp = pocet_znakov_v_riadku;
        }

       temp = (pos_dest_x+strlen(buffer));

        while(difference > 0){
            while(temp < pocet_znakov_v_riadku){
                table[i][temp] = table[i][temp+1];
                temp++;
            }
            pocet_znakov_v_riadku--;
            difference--;
            temp = (pos_dest_x+strlen(buffer));
        }
        strncpy(table[i]+pos_dest_x, buffer, strlen(buffer));

        zapis_dest_x = 0;
        zapis_dest_y = 0;
        pocet_znakov_v_riadku = 0;
        pocet_delimiterov = 0;
        difference = 0;
    }
}

        
