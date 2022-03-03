
/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
 * uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c206.h"

int error_flag;
int solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error() {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = TRUE;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init( DLList *list ) {
    list->firstElement = NULL;
    list->lastElement = NULL;
    list->activeElement = NULL;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose( DLList *list ) {
    DLLElementPtr currentFirst = list->firstElement;

    //Go trough the list and free the memory of its elements
    while(list->firstElement != NULL){
        list->firstElement = list->firstElement->nextElement;
        free(currentFirst);
        currentFirst = list->firstElement;
    }
    //Revert the list back to initialization stage
    list->firstElement = NULL;
    list->lastElement = NULL;
    list->activeElement = NULL;
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst( DLList *list, int data ) {
    //Allocate memory for new element
    DLLElementPtr NewElementPTR = (DLLElementPtr) malloc(sizeof(struct DLLElement));
    //If malloc fails call for an error
    if(!NewElementPTR){
        DLL_Error();
        return;
    }
    NewElementPTR->data = data; //Insert data into the new element
    NewElementPTR->previousElement = NULL; //Previous element of the new first element is NULL
    NewElementPTR->nextElement = list->firstElement; //Next element of the new element is the old first element

    //If the list wasnt empty
    if(list->firstElement != NULL){
        //The previous element of the old first element is the new element
        list->firstElement->previousElement = NewElementPTR;
    }
    else{
        //Else if the list was empty, then the last element is also the new first element
        list->lastElement = NewElementPTR;
    }
    //Set the first element to the new element
    list->firstElement = NewElementPTR;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast( DLList *list, int data ) {
    //Allocate memory for new element
    DLLElementPtr NewElementPTR = (DLLElementPtr) malloc(sizeof(struct DLLElement));
    //If malloc fails call for an error
    if(!NewElementPTR){
        DLL_Error();
        return;
    }

    NewElementPTR->data = data; //Insert data into the new element
    NewElementPTR->nextElement = NULL; //The next element of the new last one is NULL
    NewElementPTR->previousElement = list->lastElement; //The previous element of the new last element is the old last element

    //If the list wasnt empty
    if(list->lastElement != NULL){
        //The next element of the old last element is the new element
        list->lastElement->nextElement = NewElementPTR;
    }
    //Else if the list was empty
    else{
        //The new last element is also the first element
        list->firstElement = NewElementPTR;
    }
    //Set the new element as the last one
    list->lastElement = NewElementPTR;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz (nepočítáme-li return),
 * aniž byste testovali, zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First( DLList *list ) {
    list->activeElement = list->firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz (nepočítáme-li return),
 * aniž byste testovali, zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last( DLList *list ) {
    list->activeElement = list->lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst( DLList *list, int *dataPtr ) {
    //If the list is empty call an error
    if(!list->firstElement){
        DLL_Error();
        return;
    }
    *dataPtr = list->firstElement->data;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast( DLList *list, int *dataPtr ) {
    //If the list is empty call an error
    if(!list->firstElement){
        DLL_Error();
        return;
    }
    *dataPtr = list->lastElement->data;
}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst( DLList *list ) {
    //If the list is empty
    if(!list->firstElement){
        return;
    }

    DLLElementPtr OldFirstPTR;
    OldFirstPTR = list->firstElement;

    //If the first element is active, remove the activity
    if(list->firstElement == list->activeElement){
        list->activeElement = NULL;
    }

    //If the list has only 1 element, NULL both the first and last element
    if(list->firstElement == list->lastElement){
        list->firstElement = NULL;
        list->lastElement = NULL;
    }
    //Else if not, correct the first and previous elements
    else{
        list->firstElement = list->firstElement->nextElement;
        list->firstElement->previousElement = NULL;
    }
    //Free the memory
    free(OldFirstPTR);
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast( DLList *list ) {
    //If the list is empty
    if(!list->firstElement){
        return;
    }

    DLLElementPtr OldLastPTR;
    OldLastPTR = list->lastElement;

    //If the Last element is active, remove the activity
    if(list->lastElement == list->activeElement){
        list->activeElement = NULL;
    }

    //If there is only 1 element in the list, NULL both
    if(list->lastElement == list->firstElement){    
        list->lastElement = NULL;
        list->firstElement = NULL;
    }
    //Else if there is more elements, corrct the last and next element
    else{
        list->lastElement = list->lastElement->previousElement;
        list->lastElement->nextElement = NULL;
    }
    //Free the memory
    free(OldLastPTR);
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter( DLList *list ) {
    //If the list doesnt have an active element or the last element is active, do nothing
    if(!list->activeElement || (list->lastElement == list->activeElement)){
        return;
    }

    DLLElementPtr ElePTR;
    ElePTR = list->activeElement->nextElement; //Element being deleted

    list->activeElement->nextElement = ElePTR->nextElement; //Next element of the active element is the one after the element being deleted

    //If the element being deleted is the last one in the list, the active element is the new last element
    if(list->lastElement == ElePTR){
        list->lastElement = list->activeElement;
    }
    //Else if it is not the last one, the previous element of the element after the one being deleted is the active element
    else{
        ElePTR->nextElement->previousElement = list->activeElement;
    }
    free(ElePTR);
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore( DLList *list ) {
    //If the list doesnt have an active element or the first element is active, do nothing
    if(!list->activeElement || (list->firstElement == list->activeElement)){
        return;
    }

    DLLElementPtr ElePTR;
    ElePTR = list->activeElement->previousElement; //Element being deleted

    list->activeElement->previousElement = ElePTR->previousElement; //Previous element of the active element is the one before the element being deleted

    //If the element being deleted is the first one in the list, the active element is the new first element
    if(list->firstElement == ElePTR){
        list->firstElement = list->activeElement;
    }
    //Else if it is not the first one, the next element of the element before the one being deleted is the active element
    else{
        ElePTR->previousElement->nextElement = list->activeElement;
    }
    free(ElePTR);
}

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter( DLList *list, int data ) {
    //If the list doesnt have a active element, do nothing
    if(!list->activeElement){
        return;
    }

    //Allocate memory for new element
    DLLElementPtr NewElementPTR = (DLLElementPtr) malloc(sizeof(struct DLLElement));
    //If malloc fails call for an error
    if(!NewElementPTR){
        DLL_Error();
        return;
    }

    NewElementPTR->data = data; //Insert data into the new element
    NewElementPTR->previousElement = list->activeElement; //Previous element of the new element is the active element
    NewElementPTR->nextElement = list->activeElement->nextElement; //Next element of the new elemenet is the next element of the active element
    list->activeElement->nextElement = NewElementPTR; //Next element of the active element is the new element

    //If the active element is last, the new element will be the new last
    if(list->activeElement == list->lastElement){
        list->lastElement = NewElementPTR;
    }
    //Else if not, then we correct previous element of the new next element
    else{
        NewElementPTR->nextElement->previousElement = NewElementPTR;
    }



}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore( DLList *list, int data ) {
    //If the list doesnt have a active element, do nothing
    if(!list->activeElement){
        return;
    }

    //Allocate memory for new element
    DLLElementPtr NewElementPTR = (DLLElementPtr) malloc(sizeof(struct DLLElement));
    //If malloc fails call for an error
    if(!NewElementPTR){
        DLL_Error();
        return;
    }

    NewElementPTR->data = data; //Insert data into the new element
    NewElementPTR->previousElement = list->activeElement->previousElement; //Previous element of the new element is the active element
    NewElementPTR->nextElement = list->activeElement; //Next element of the new elemenet is the active element
    list->activeElement->previousElement = NewElementPTR; //Previous element of the active element is the new element

    //If the active element is first, the new element will be the new first
    if(list->activeElement == list->firstElement){
        list->firstElement = NewElementPTR;
    }
    //Else if not, then we correct the next element of the previous element
    else{
        NewElementPTR->previousElement->nextElement = NewElementPTR;
    }
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue( DLList *list, int *dataPtr ) {
    //If the list doesnt have a active element, call an error
    if(!list->activeElement){
        DLL_Error();
        return;
    }

    *dataPtr = list->activeElement->data;
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue( DLList *list, int data ) {
    //If the list doesnt have a active element, do nothing
    if(!list->activeElement){
        return;
    }

    list->activeElement->data = data;
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next( DLList *list ) {
    //If the list doesnt have a active element, do nothing
    if(!list->activeElement){
        return;
    }

    //If the last element is active, list becomes innactive
    if(list->activeElement == list->lastElement){
        list->activeElement = NULL;
    }
    else{
        //Else, next element becomes active
        list->activeElement = list->activeElement->nextElement;
    }
}


/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous( DLList *list ) {
    //If the list doesnt have a active element, do nothing
    if(!list->activeElement){
        return;
    }

    //If the first element is active, list becomes innactive
    if(list->activeElement == list->firstElement){
        list->activeElement = NULL;
    }
    else{
        //Else, the previous element becomes active
        list->activeElement = list->activeElement->previousElement ;
    }
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
int DLL_IsActive( DLList *list ) {
    return (!list->activeElement) ? 0:1;
}

/* Konec c206.c */
