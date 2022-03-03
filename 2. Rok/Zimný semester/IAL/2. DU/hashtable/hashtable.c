/*
 * Tabuľka s rozptýlenými položkami
 *
 * S využitím dátových typov zo súboru hashtable.h a pripravených kostier
 * funkcií implementujte tabuľku s rozptýlenými položkami s explicitne
 * zreťazenými synonymami.
 *
 * Pri implementácii uvažujte veľkosť tabuľky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptyľovacia funkcia ktorá pridelí zadanému kľúču index z intervalu
 * <0,HT_SIZE-1>. Ideálna rozptyľovacia funkcia by mala rozprestrieť kľúče
 * rovnomerne po všetkých indexoch. Zamyslite sa nad kvalitou zvolenej funkcie.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializácia tabuľky — zavolá sa pred prvým použitím tabuľky.
 */
void ht_init(ht_table_t *table) {
  for(int i = 0; i < HT_SIZE; i++){
    (*table)[i] = NULL;
  }
}

/*
 * Vyhľadanie prvku v tabuľke.
 *
 * V prípade úspechu vráti ukazovateľ na nájdený prvok; v opačnom prípade vráti
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {
  ht_item_t *PTR = (*table)[get_hash(key)];

  while(PTR != NULL){
    if(PTR->key == key) return PTR;
    PTR = PTR->next;
  }
  return NULL;
}

/*
 * Vloženie nového prvku do tabuľky.
 *
 * Pokiaľ prvok s daným kľúčom už v tabuľke existuje, nahraďte jeho hodnotu.
 *
 * Pri implementácii využite funkciu ht_search. Pri vkladaní prvku do zoznamu
 * synonym zvoľte najefektívnejšiu možnosť a vložte prvok na začiatok zoznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {
  ht_item_t *PTR = ht_search(table, key);
  if(PTR != NULL){
    PTR->value = value;
    return;
  }

  ht_item_t *New = malloc(sizeof(struct ht_item));
  New->value = value;
  New->key = key;
  New->next = (*table)[get_hash(key)];
  (*table)[get_hash(key)] = New;
}

/*
 * Získanie hodnoty z tabuľky.
 *
 * V prípade úspechu vráti funkcia ukazovateľ na hodnotu prvku, v opačnom
 * prípade hodnotu NULL.
 *
 * Pri implementácii využite funkciu ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {
  ht_item_t *PTR = ht_search(table, key);
  if(PTR == NULL) return NULL;
  return &PTR->value;
}

/*
 * Zmazanie prvku z tabuľky.
 *
 * Funkcia korektne uvoľní všetky alokované zdroje priradené k danému prvku.
 * Pokiaľ prvok neexistuje, nerobte nič.
 *
 * Pri implementácii NEVYUŽÍVAJTE funkciu ht_search.
 */
void ht_delete(ht_table_t *table, char *key) {
  ht_item_t *PTR = (*table)[get_hash(key)];

  if(PTR == NULL){
    return;
  }

  if(PTR->key == key){
    if(PTR->next == NULL){
      (*table)[get_hash(key)] = NULL;
      free(PTR);
    }
    else{
      (*table)[get_hash(key)] = PTR->next;
      free(PTR);
    }
  }
  else{
    ht_item_t *Prev = NULL;
    while(PTR != NULL){
      Prev = PTR;
      PTR = PTR->next;

      if(PTR->key == key){
        Prev->next = PTR->next;
        free(PTR);
        return;
      }
    }
  }
}

/*
 * Zmazanie všetkých prvkov z tabuľky.
 *
 * Funkcia korektne uvoľní všetky alokované zdroje a uvedie tabuľku do stavu po
 * inicializácii.
 */
void ht_delete_all(ht_table_t *table) {
  for(int i = 0; i < HT_SIZE; i++){
    ht_item_t *PTR = (*table)[i];
    ht_item_t *Next = NULL;
    
    while(PTR != NULL){
      Next = PTR->next;
      free(PTR);
      PTR = Next;
    }
  }
  ht_init(table);
}
