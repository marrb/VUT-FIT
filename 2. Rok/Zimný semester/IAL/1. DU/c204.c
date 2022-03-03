
/* ******************************* c204.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c204 - Převod infixového výrazu na postfixový (s využitím c202)     */
/*  Referenční implementace: Petr Přikryl, listopad 1994                      */
/*  Přepis do jazyka C: Lukáš Maršík, prosinec 2012                           */
/*  Upravil: Kamil Jeřábek, září 2019                                         */
/*           Daniel Dolejška, září 2021                                       */
/* ************************************************************************** */
/*
** Implementujte proceduru pro převod infixového zápisu matematického výrazu
** do postfixového tvaru. Pro převod využijte zásobník (Stack), který byl
** implementován v rámci příkladu c202. Bez správného vyřešení příkladu c202
** se o řešení tohoto příkladu nepokoušejte.
**
** Implementujte následující funkci:
**
**    infix2postfix ... konverzní funkce pro převod infixového výrazu
**                      na postfixový
**
** Pro lepší přehlednost kódu implementujte následující pomocné funkce:
**    
**    untilLeftPar ... vyprázdnění zásobníku až po levou závorku
**    doOperation .... zpracování operátoru konvertovaného výrazu
**
** Své řešení účelně komentujte.
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c204.h"

int solved;

/**
 * Pomocná funkce untilLeftPar.
 * Slouží k vyprázdnění zásobníku až po levou závorku, přičemž levá závorka bude
 * také odstraněna.
 * Pokud je zásobník prázdný, provádění funkce se ukončí.
 *
 * Operátory odstraňované ze zásobníku postupně vkládejte do výstupního pole
 * znaků postfixExpression.
 * Délka převedeného výrazu a též ukazatel na první volné místo, na které se má
 * zapisovat, představuje parametr postfixExpressionLength.
 *
 * Aby se minimalizoval počet přístupů ke struktuře zásobníku, můžete zde
 * nadeklarovat a používat pomocnou proměnnou typu char.
 *
 * @param stack Ukazatel na inicializovanou strukturu zásobníku
 * @param postfixExpression Znakový řetězec obsahující výsledný postfixový výraz
 * @param postfixExpressionLength Ukazatel na aktuální délku výsledného postfixového výrazu
 */
void untilLeftPar( Stack *stack, char *postfixExpression, unsigned *postfixExpressionLength ) {
    //Check if the stack is empty
    while(!Stack_IsEmpty(stack)){
        char content;
        //if not, go trough the stack until you find '('
        Stack_Top(stack, &content); //Get the top character of the stack
        Stack_Pop(stack); //Remove the top from the stack
        if(content == '('){
        //If the function finds '(' the executing stops
            return;
        }
        postfixExpression[(*postfixExpressionLength)++] = content; //Insert the character in the postfix array
        }
    }

/**
 * Pomocná funkce doOperation.
 * Zpracuje operátor, který je předán parametrem c po načtení znaku ze
 * vstupního pole znaků.
 *
 * Dle priority předaného operátoru a případně priority operátoru na vrcholu
 * zásobníku rozhodneme o dalším postupu.
 * Délka převedeného výrazu a taktéž ukazatel na první volné místo, do kterého
 * se má zapisovat, představuje parametr postfixExpressionLength, výstupním
 * polem znaků je opět postfixExpression.
 *
 * @param stack Ukazatel na inicializovanou strukturu zásobníku
 * @param c Znak operátoru ve výrazu
 * @param postfixExpression Znakový řetězec obsahující výsledný postfixový výraz
 * @param postfixExpressionLength Ukazatel na aktuální délku výsledného postfixového výrazu
 */
void doOperation( Stack *stack, char c, char *postfixExpression, unsigned *postfixExpressionLength ) {
    //If the stack is empty, push char 
    if(Stack_IsEmpty(stack)){
        Stack_Push(stack, c);
        return;
    }

    char stackTOP;
    Stack_Top(stack, &stackTOP);
    
    //If there is a left bracket on the top of the stack, push char
    if(stackTOP == '('){
        Stack_Push(stack, c);
        return;
    }

    //If there is an operator with lower priority push char
    if((stackTOP == '+' || stackTOP == '-') && (c == '*' || c == '/')){
        Stack_Push(stack, c);
        return;
    }
    else{
        //If there is an operator with higher or same priority, delete it and send to output string
        postfixExpression[(*postfixExpressionLength)++] = stackTOP;
        Stack_Pop(stack);
        //Repeat until we can push it to the stack
        doOperation(stack, c, postfixExpression, postfixExpressionLength);
        return;
    }
}

/**
 * Konverzní funkce infix2postfix.
 * Čte infixový výraz ze vstupního řetězce infixExpression a generuje
 * odpovídající postfixový výraz do výstupního řetězce (postup převodu hledejte
 * v přednáškách nebo ve studijní opoře).
 * Paměť pro výstupní řetězec (o velikosti MAX_LEN) je třeba alokovat. Volající
 * funkce, tedy příjemce konvertovaného řetězce, zajistí korektní uvolnění zde
 * alokované paměti.
 *
 * Tvar výrazu:
 * 1. Výraz obsahuje operátory + - * / ve významu sčítání, odčítání,
 *    násobení a dělení. Sčítání má stejnou prioritu jako odčítání,
 *    násobení má stejnou prioritu jako dělení. Priorita násobení je
 *    větší než priorita sčítání. Všechny operátory jsou binární
 *    (neuvažujte unární mínus).
 *
 * 2. Hodnoty ve výrazu jsou reprezentovány jednoznakovými identifikátory
 *    a číslicemi - 0..9, a..z, A..Z (velikost písmen se rozlišuje).
 *
 * 3. Ve výrazu může být použit předem neurčený počet dvojic kulatých
 *    závorek. Uvažujte, že vstupní výraz je zapsán správně (neošetřujte
 *    chybné zadání výrazu).
 *
 * 4. Každý korektně zapsaný výraz (infixový i postfixový) musí být uzavřen
 *    ukončovacím znakem '='.
 *
 * 5. Při stejné prioritě operátorů se výraz vyhodnocuje zleva doprava.
 *
 * Poznámky k implementaci
 * -----------------------
 * Jako zásobník použijte zásobník znaků Stack implementovaný v příkladu c202.
 * Pro práci se zásobníkem pak používejte výhradně operace z jeho rozhraní.
 *
 * Při implementaci využijte pomocné funkce untilLeftPar a doOperation.
 *
 * Řetězcem (infixového a postfixového výrazu) je zde myšleno pole znaků typu
 * char, jenž je korektně ukončeno nulovým znakem dle zvyklostí jazyka C.
 *
 * Na vstupu očekávejte pouze korektně zapsané a ukončené výrazy. Jejich délka
 * nepřesáhne MAX_LEN-1 (MAX_LEN i s nulovým znakem) a tedy i výsledný výraz
 * by se měl vejít do alokovaného pole. Po alokaci dynamické paměti si vždycky
 * ověřte, že se alokace skutečně zdrařila. V případě chyby alokace vraťte namísto
 * řetězce konstantu NULL.
 *
 * @param infixExpression Znakový řetězec obsahující infixový výraz k převedení
 *
 * @returns Znakový řetězec obsahující výsledný postfixový výraz
 */
char *infix2postfix( const char *infixExpression ) {
    //Allocate memory for the postfix expresion
    char *postfixExpresion = (char*) malloc(MAX_LEN * sizeof(char));

    //If malloc fails return NULL
    if(!postfixExpresion){
        return NULL;
    }

    //Allocate memory for the stack
    Stack *stack = (Stack *) malloc(sizeof(Stack));

    //If malloc fails return NULL and free the memory
    if(!stack){
        free(stack);
        return NULL;
    }

    //Initialize the stack
    Stack_Init(stack);
    unsigned int postCounter = 0;

    //Go trough the input array 1 by 1
    for(int i = 0; infixExpression[i] != '\0'; i++){
        //Check if it is an operand
        if((infixExpression[i] >= 'a' && infixExpression[i] <= 'z') || (infixExpression[i] >= 'A' && infixExpression[i] <= 'Z') || (infixExpression[i] >= '0' && infixExpression[i] <= '9')){
            postfixExpresion[postCounter++] = infixExpression[i];
        }
        //Else if it is a left bracket push it to the stack
        else if(infixExpression[i] == '('){
            Stack_Push(stack, infixExpression[i]);
        }
        //Else if it is an operator, use function doOperation
        else if(infixExpression[i] == '+' || infixExpression[i] == '-' || infixExpression[i] == '*' || infixExpression[i] == '/'){
            doOperation(stack, infixExpression[i], postfixExpresion, &postCounter);
        }
        //Else if it is a right bracket, use untilLeftPar
        else if(infixExpression[i] == ')'){
            untilLeftPar(stack, postfixExpresion, &postCounter);
        }
        //Else if it is a '=', go trough the stack and remove chars 1 by 1 until the end
        else if(infixExpression[i] == '='){
            while(!Stack_IsEmpty(stack)){
                Stack_Top(stack, &(postfixExpresion[postCounter++]));
                Stack_Pop(stack);
            }
            //When the stack is empty, send the '=' to the output
            postfixExpresion[postCounter++] = '=';
        }
    }
    //Free stack memory
    free(stack);

    //Terminate the output string with NULL char
    postfixExpresion[postCounter++] = '\0';
    
    //Return the output string
    return postfixExpresion;
}

/* Konec c204.c */
