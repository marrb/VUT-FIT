/*
 * Binárny vyhľadávací strom — iteratívna varianta
 *
 * S využitím dátových typov zo súboru btree.h, zásobníkov zo súborov stack.h a
 * stack.c a pripravených kostier funkcií implementujte binárny vyhľadávací
 * strom bez použitia rekurzie.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializácia stromu.
 *
 * Užívateľ musí zaistiť, že incializácia sa nebude opakovane volať nad
 * inicializovaným stromom. V opačnom prípade môže dôjsť k úniku pamäte (memory
 * leak). Keďže neinicializovaný ukazovateľ má nedefinovanú hodnotu, nie je
 * možné toto detekovať vo funkcii.
 */
void bst_init(bst_node_t **tree) {
  *tree = NULL;
}

/*
 * Nájdenie uzlu v strome.
 *
 * V prípade úspechu vráti funkcia hodnotu true a do premennej value zapíše
 * hodnotu daného uzlu. V opačnom prípade funckia vráti hodnotu false a premenná
 * value ostáva nezmenená.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  if(tree != NULL){
    if(tree->key == key){
      *value = tree->value;
      return true;
    }
    bst_node_t *Pine = tree;

    while(key < Pine->key || key > Pine->key){
      if(key < Pine->key){
        if(Pine->left == NULL){
          return false;
        }
        Pine = Pine->left;
      }
      if(key > Pine->key){
        if(Pine->right == NULL){
          return false;
        }
        Pine = Pine->right;
      }
    }
    *value = Pine->value;
    return true;
  }
  return false;
}

/*
 * Vloženie uzlu do stromu.
 *
 * Pokiaľ uzol so zadaným kľúčom v strome už existuje, nahraďte jeho hodnotu.
 * Inak vložte nový listový uzol.
 *
 * Výsledný strom musí spĺňať podmienku vyhľadávacieho stromu — ľavý podstrom
 * uzlu obsahuje iba menšie kľúče, pravý väčšie.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  
  if((*tree) == NULL){
    *tree = malloc(sizeof(struct bst_node));

    (*tree)->key = key;
    (*tree)->value = value;
    (*tree)->left = NULL;
    (*tree)->right = NULL;
    return;
  }

  bst_node_t *Point_Pine = NULL;
  bst_node_t *Pine = (*tree);

  while(Pine != NULL && key != Pine->key){

    Point_Pine = Pine;

    if(key < Pine->key){
      Pine = Pine->left;
    }
    else if(key > Pine->key){
      Pine = Pine->right;
    }
  }
  if(Pine == NULL){
    bst_node_t *New = malloc(sizeof(struct bst_node));
    New->key = key;
    New->value = value;
    New->left = NULL;
    New->right = NULL;

    if(key < Point_Pine->key){
      Point_Pine->left = New;
    }
    else{
      Point_Pine->right = New;
    }
  }
  else{
    Pine->value = value;
  }
}

/*
 * Pomocná funkcia ktorá nahradí uzol najpravejším potomkom.
 *
 * Kľúč a hodnota uzlu target budú nahradené kľúčom a hodnotou najpravejšieho
 * uzlu podstromu tree. Najpravejší potomok bude odstránený. Funkcia korektne
 * uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkcia predpokladá že hodnota tree nie je NULL.
 *
 * Táto pomocná funkcia bude využitá pri implementácii funkcie bst_delete.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  bst_node_t *Pine = (*tree);
  bst_node_t *Prev = NULL;
  while(Pine->right != NULL){
    Prev = Pine;
    Pine = Pine->right;
  }

  target->key = Pine->key;
  target->value = Pine->value;

  bst_node_t *backup = Pine->left;
  free(Pine);
  Pine = backup;
  Prev->right = Pine;
}

/*
 * Odstránenie uzlu v strome.
 *
 * Pokiaľ uzol so zadaným kľúčom neexistuje, funkcia nič nerobí.
 * Pokiaľ má odstránený uzol jeden podstrom, zdedí ho otec odstráneného uzla.
 * Pokiaľ má odstránený uzol oba podstromy, je nahradený najpravejším uzlom
 * ľavého podstromu. Najpravejší uzol nemusí byť listom!
 * Funkcia korektne uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkciu implementujte iteratívne pomocou bst_replace_by_rightmost a bez
 * použitia vlastných pomocných funkcií.
 */
void bst_delete(bst_node_t **tree, char key) {
    if((*tree) == NULL){
      return;
    }

    bst_node_t *Pine = (*tree);
    bst_node_t *Prev = NULL;

    while(key < Pine->key || key > Pine->key){
      if(key < Pine->key){
        if(Pine->left == NULL){
          return;
        }
        Prev = Pine;
        Pine = Pine->left;
      }
      if(key > Pine->key){
        if(Pine->right == NULL){
          return;
        }
        Prev = Pine;
        Pine = Pine->right;
      }
    }
    if(Pine->left == NULL && Pine->right == NULL){
      //No subtrees
      if(Pine->key < Prev->key) Prev->left = NULL;
      else Prev->right = NULL;
      free(Pine);
      Pine = NULL;
    }
    else if(Pine->left != NULL && Pine->right != NULL){
      //Both subtrees
      bst_replace_by_rightmost(Pine, &Pine->left);
    }
    else{
      //1 subtree
      bst_node_t *backup;
      if(Pine->left == NULL){
        backup = Pine->right;
        free(Pine);
        Pine = backup;
        if(Pine->key < Prev->key) Prev->left = Pine;
        else Prev->right = Pine;
      }
      else{
        backup = Pine->left;
        free(Pine);
        Pine = backup;
        if(Pine->key < Prev->key) Prev->left = Pine;
        else Prev->right = Pine;
      }
    }
}


/*
 * Zrušenie celého stromu.
 *
 * Po zrušení sa celý strom bude nachádzať v rovnakom stave ako po
 * inicializácii. Funkcia korektne uvoľní všetky alokované zdroje rušených
 * uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_dispose(bst_node_t **tree) {
  if((*tree) == NULL) return;

  stack_bst_t Stack;
  stack_bst_init(&Stack);
  stack_bst_push(&Stack, (*tree));

  while(!stack_bst_empty(&Stack)){
    bst_node_t *Pine = stack_bst_top(&Stack);
    stack_bst_pop(&Stack);

    if(Pine->left != NULL){
      stack_bst_push(&Stack, Pine->left);
    }
    if(Pine->right != NULL){
      stack_bst_push(&Stack, Pine->right);
    }

    free(Pine);
  }
  (*tree) = NULL;
}

/*
 * Pomocná funkcia pre iteratívny preorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu.
 * Nad spracovanými uzlami zavola bst_print_node a uloží ich do zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit) {
    while(tree != NULL){
      bst_print_node(tree);
      stack_bst_push(to_visit, tree);
      tree = tree->left;
    }
}

/*
 * Preorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_preorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_preorder(bst_node_t *tree) {
  if(tree == NULL) return;

  stack_bst_t Stack;
  stack_bst_init(&Stack);

  bst_leftmost_preorder(tree, &Stack);

  while(!stack_bst_empty(&Stack)){
    tree = stack_bst_top(&Stack);
    stack_bst_pop(&Stack);
    if(tree->right != NULL){
      bst_leftmost_preorder(tree->right, &Stack);
    }
  }
}

/*
 * Pomocná funkcia pre iteratívny inorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit) {
   while(tree != NULL){
    stack_bst_push(to_visit, tree);
    tree = tree->left;
  }
}

/*
 * Inorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_inorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_inorder(bst_node_t *tree) {
  if(tree == NULL) return;

  stack_bst_t Stack;
  stack_bst_init(&Stack);

  bst_leftmost_inorder(tree, &Stack);

  while(!stack_bst_empty(&Stack)){
    tree = stack_bst_top(&Stack);
    bst_print_node(tree);
    stack_bst_pop(&Stack);
    if(tree->right != NULL){
      bst_leftmost_inorder(tree->right, &Stack);
    }
  }
}

/*
 * Pomocná funkcia pre iteratívny postorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov. Do zásobníku bool hodnôt ukladá informáciu že uzol
 * bol navštívený prvý krát.
 *
 * Funkciu implementujte iteratívne pomocou zásobníkov uzlov a bool hodnôt a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit) {

    while(tree != NULL){
      stack_bst_push(to_visit, tree);
      stack_bool_push(first_visit, true);
      tree = tree->left;
    }
}

/*
 * Postorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_postorder a
 * zásobníkov uzlov a bool hodnôt bez použitia vlastných pomocných funkcií.
 */
void bst_postorder(bst_node_t *tree) {
  if(tree == NULL) return;

  stack_bst_t Stack;
  stack_bst_init(&Stack);

  stack_bool_t B_Stack;
  stack_bool_init(&B_Stack);

  bst_leftmost_postorder(tree, &Stack, &B_Stack);

  while(!stack_bst_empty(&Stack)){
    tree = stack_bst_top(&Stack);
    bool Left_Node = stack_bool_top(&B_Stack);
    stack_bool_pop(&B_Stack);

    if(Left_Node){
      stack_bool_push(&B_Stack, false);
      bst_leftmost_postorder(tree->right, &Stack, &B_Stack);
    }
    else{
      stack_bst_pop(&Stack);
      bst_print_node(tree);
    }
  }
}
