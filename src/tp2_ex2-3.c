/**
 * \file tp2_lex.c
 * \brief analyseur lexical pour le langage JSON
 * \author NM
 * \version 0.1
 * \date 25/11/2015
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <regex.h>

#define JSON_LEX_ERROR -1   /**< code d'erreur lexicale */
#define JSON_TRUE 1         /**< entite lexicale true */
#define JSON_FALSE 2        /**< entite lexicale false */
#define JSON_NULL 3         /**< entite lexicale null */
#define JSON_LCB 4          /**< entite lexicale { */
#define JSON_RCB 5          /**< entite lexicale } */
#define JSON_LB 6           /**< entite lexicale [ */
#define JSON_RB 7           /**< entite lexicale ] */
#define JSON_COMMA 8        /**< entite lexicale , */
#define JSON_COLON 9        /**< entite lexicale : */
#define JSON_STRING 10      /**<entite lexicale chaine de caracteres */
#define JSON_INT_NUMBER 11  /**< entite lexicale nombre entier */
#define JSON_REAL_NUMBER 12 /**< entite lexicale nombre reel */

/**
 * \union TSymbole
 * \brief union permettant de manipuler un entier/reel/chaine pour la table
 * des symboles
 */
typedef struct
{
    int type; /**< l'un des 3 types suivants : JSON_STRING/JSON_INT_NUMBER/
JSON_REAL_NUMBER */
    union
    {
        int entier;
        float reel;
        char *chaine;
    } val; /**< valeur associer a un element de la table des symboles */
} TSymbole;

/**
 * \struct TLex
 * \brief structure contenant tous les parametres/donnees pour
 * l'analyse lexicale
 */
typedef struct
{
    char *data;              /**< chaine a parcourir */
    char *startPos;          /**< position de depart pour la prochaine analyse */
    int nbLignes;            /**< nb de lignes analysees */
    TSymbole *tableSymboles; /**< tableau des symboles : chaines/entier/reel */
    int nbSymboles;          /**< nb de symboles stockes dans tableSymboles */
    int tailleTableSymboles; /**< taille memoire du tableau tableSymboles */
} TLex;

char *separators = ":,;\n";

/**
 * \fn int isSep(char _symb)
 * \brief fonction qui teste si un symbole fait partie des separateurs
 *
 * \param[in] _symb symbole a analyser
 * \return 1 (vrai) si _symb est un separateur, 0 (faux) sinon
 */
int isSep(const char _symb)
{
    int founded = 0;

    for (int i = 0; !founded && separators[i] != '\0'; ++i)
    {
        if (separators[i] == _symb)
        {
            founded = 1;
        }
    }
    return founded;
}

/**
 * \fn TLex * initLexData(char * _data)
 * \brief fonction qui reserve la memoire et initialise les
 * donnees pour l'analyseur lexical
 *
 * \param[in] _data chaine a analyser
 * \return pointeur sur la structure de donnees creee
 */
TLex *initLexData(char *_data)
{
    TLex *lexData;
    lexData->data = _data;
    lexData->startPos = _data;
    lexData->nbLignes = 0;
    lexData->tableSymboles = NULL;
    lexData->nbSymboles = 0;
    lexData->tailleTableSymboles = 0;

    return lexData;
}

/**
 * \fn void deleteLexData(TLex ** _lexData)
 * \brief fonction qui supprime de la memoire les donnees pour
 * l'analyseur lexical
 *
 * \param[inout] _lexData donnees de l'analyseur lexical
 * \return neant
 */
void deleteLexData(TLex **_lexData)
{
    free((*_lexData)->data);
    free(*_lexData);
}

/**
 * \fn void printLexData(TLex * _lexData)
 * \brief fonction qui affiche les donnees pour
 * l'analyseur lexical
 *
 * \param[in] _lexData données de l'analyseur lexical
 * \return neant
 */
void printLexData(TLex *_lexData)
{
    printf("LexData\ndata: %s\nstartPos: %s\nnbLignes: %d\nnbSymboles: %d", _lexData->data, _lexData->startPos, _lexData->nbLignes, _lexData->nbSymboles);
}
/**
 * \fn void addIntSymbolToLexData(TLex * _lexData, const int _val)
 * \brief fonction qui ajoute un symbole entier a la table des symboles
 *
 * \param[inout] _lexData donnees de l'analyseur lexical
 * \param[in] _val valeur entiere e ajouter
 * \return neant
 */
void addIntSymbolToLexData(TLex *_lexData, const int _val)
{
    int toContinue = 1;

    if (_lexData->nbSymboles > _lexData->tailleTableSymboles)
    {
        _lexData->tailleTableSymboles += 4;
        toContinue = realloc(_lexData->tableSymboles, _lexData->tailleTableSymboles);
    }

    if (toContinue)
    {
        _lexData->tableSymboles[_lexData->nbSymboles].type = JSON_INT_NUMBER;
        _lexData->tableSymboles[_lexData->nbSymboles].val.entier = _val;
        ++_lexData->nbSymboles;
    }
}

/**
 * \fn void addRealSymbolToLexData(TLex * _lexData, const float _val)
 * \brief fonction qui ajoute un symbole reel a la table des symboles
 *
 * \param[inout] _lexData donnees de l'analyseur lexical
 * \param[in] _val valeur reelle a ajouter
 */
void addRealSymbolToLexData(TLex *_lexData, const float _val)
{
    int toContinue = 1;

    if (_lexData->nbSymboles > _lexData->tailleTableSymboles)
    {
        _lexData->tailleTableSymboles += 4;
        toContinue = realloc(_lexData->tableSymboles, _lexData->tailleTableSymboles);
    }

    if (toContinue)
    {
        _lexData->tableSymboles[_lexData->nbSymboles].type = JSON_REAL_NUMBER;
        _lexData->tableSymboles[_lexData->nbSymboles].val.reel = _val;
        ++_lexData->nbSymboles;
    }
}

/**
 * \fn void addStringSymbolToLexData(TLex * _lexData, char * _val)
 * \brief fonction qui ajoute une chaine de caracteres a la table des symboles
 *
 * \param[inout] _lexData donnees de l'analyseur lexical
 * \param[in] _val chaine a ajouter
 */
void addStringSymbolToLexData(TLex *_lexData, char *_val)
{
    int toContinue = 1;

    if (_lexData->nbSymboles > _lexData->tailleTableSymboles)
    {
        _lexData->tailleTableSymboles += 4;
        toContinue = realloc(_lexData->tableSymboles, _lexData->tailleTableSymboles);
    }

    if (toContinue)
    {
        _lexData->tableSymboles[_lexData->nbSymboles].type = JSON_STRING;
        _lexData->tableSymboles[_lexData->nbSymboles].val.chaine = _val;
        ++_lexData->nbSymboles;
    }
}

/**
 * \fn int lex(Lex * _lexData)
 * \brief fonction qui effectue l'analyse lexicale (contient le code l'automate fini)
 *
 * \param[inout] _lexData donnees de suivi de l'analyse lexicale
 * \return code d'identification de l'entite lexicale trouvee
 */
int lex(TLex *_lexData)
{
    char current;
    int state = 0;
    int ended = 0;
    int founded = 0;
    regex_t preg;

    while (!ended)
    {
        current = *_lexData->data;

        switch (state)
        {
        case 0:
            switch (current)
            {
            case 't':
                state = 1;
                break;

            case 'f':
                state = 6;
                break;

            case 'n':
                state = 12;
                break;

            case '{':
                state = 17;
                ended = 1;
                break;

            case '}':
                state = 18;
                ended = 1;
                break;

            case '[':
                state = 19;
                ended = 1;
                break;

            case ']':
                state = 20;
                ended = 1;
                break;

            case ':':
                state = 21;
                ended = 1;
                break;

            case ',':
                state = 22;
                ended = 1;
                break;

            case '"':
                state = 23;
                break;

            default:
                if (current == '0')
                    state = 27;
                else if (current >= '1' && current <= '9')
                    state = 29;
                else
                    ended = JSON_LEX_ERROR;
                break;

                break;

            case 1:
                if (current == 'r')
                    state = 2;
                else
                    ended = JSON_LEX_ERROR;
                break;

            case 2:
                if (current == 'u')
                    state = 3;
                else
                    ended = JSON_LEX_ERROR;
                break;

            case 3:
                if (current == 'e')
                    state = 4;
                else
                    ended = JSON_LEX_ERROR;
                break;

            case 4:
                //On change d'état si on à trouvé un séparateur
                if (isSep(current))
                {
                    state = 5;
                    ended = 1;
                }
                else
                    ended = JSON_LEX_ERROR;

                break;

            case 6:
                if (current == 'a')
                {
                    state = 7;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 7:
                if (current == 'l')
                {
                    state = 8;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 8:
                if (current == 's')
                {
                    state = 9;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 9:
                if (current == 'e')
                {
                    state = 10;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 10:

                //On change d'état si on à trouvé un séparateur
                if (isSep(current))
                {
                    state = 11;
                    ended = 1;
                }
                else
                    ended = JSON_LEX_ERROR;
                break;

            case 12:
                if (current == 'u')
                {
                    state = 13;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 13:
                if (current == 'l')
                {
                    state = 14;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 14:
                if (current == 'l')
                {
                    state = 15;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 15:
                if (isSep(current))
                {
                    state = 16;
                    ended = 1;
                }
                else
                {
                    ended = JSON_LEX_ERROR;
                }
                break;

            case 23:
                if (current == '"')
                {
                    state = 26;
                    ended = 1;
                }
                else if (current == '\\')
                {
                    state = 25;
                }
                else
                {
                    state = 23;
                }
                break;

            case 25:
                state = 23;
                break;

            case 27 :
                if(!regcomp(&preg, "[1-9]") && ){
                    
                }
                break;

            }

            switch (ended)
            {
            case JSON_LEX_ERROR:
                fprintf(stderr, "Erreur : caractere %c inconnu (etat %d)\n", current, state);
                return JSON_LEX_ERROR;
            case 2:
                fprintf(stderr, "Erreur : etat %d inconnu\n", state);
                return JSON_LEX_ERROR;
            default:
                return state;
            }

            return ended;
        }
    }
}

    /**
 * \fn int main()
 * \brief fonction principale
 */
    int main()
    {
        char *test;
        int i;
        TLex *lex_data;

        test = strdup("{\"obj1\": [ {\"obj2\": 12, \"obj3\":\"text1 \\\"and\\\" text2\"},\n{\"obj4\":314.32} ], \"obj5\": true }");

        printf("%s", test);
        printf("\n");

        lex_data = initLexData(test);
        i = lex(lex_data);
        while (i != JSON_LEX_ERROR)
        {
            printf("lex()=%d\n", i);
            i = lex(lex_data);
        }
        printLexData(lex_data);
        deleteLexData(&lex_data);
        free(test);
        return 0;
    }