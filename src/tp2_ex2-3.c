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

char *separators = " ,;\t\n{}[]";

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
    TLex *lexData = malloc(sizeof(TLex));
    lexData->data = _data;
    lexData->startPos = _data;
    lexData->nbLignes = 1;
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
    //free((*_lexData)->tableSymboles);
    for (int i = 0; i < (*_lexData)->nbSymboles; ++i)
    {
        if ((*_lexData)->tableSymboles[i].type == JSON_STRING)
        {
            free((*_lexData)->tableSymboles[i].val.chaine);
        }
    }
    free((*_lexData)->tableSymboles);
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
    printf("LexData\ndata: \"%s\"\nstartPos: \"%s\"\nnbLignes: %d\nnbSymboles: %d\ntableSymboles :\n", _lexData->data, _lexData->startPos, _lexData->nbLignes, _lexData->nbSymboles);
    for (int i = 0; i < _lexData->nbSymboles; i++)
    {

        switch (_lexData->tableSymboles[i].type)
        {
        case JSON_STRING:
            printf("\tChaine\t: %s\n", _lexData->tableSymboles[i].val.chaine);
            break;

        case JSON_INT_NUMBER:
            printf("\tEntier\t: %d\n", _lexData->tableSymboles[i].val.entier);
            break;

        case JSON_REAL_NUMBER:
            printf("\tReel\t: %f\n", _lexData->tableSymboles[i].val.reel);
            break;
        }
    }
}

/**
 * \fn int changeTableSymboleSize(TLex *_lexData)
 * \brief Fonction qui incrémente le nombre de symbole de la table des symbole et augmente sa taille
 * 
 * \param[inout] _lexData donnees de l'analyseur lexical
 * \return reallocation reussie
 */
int changeTableSymboleSize(TLex *_lexData)
{
    _lexData->tailleTableSymboles += sizeof(TSymbole);
    _lexData->tableSymboles = realloc(_lexData->tableSymboles, _lexData->tailleTableSymboles * 2);

    return _lexData->tableSymboles ? 0 : 1;
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

    if (!changeTableSymboleSize(_lexData))
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
    if (!changeTableSymboleSize(_lexData))
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
    if (!changeTableSymboleSize(_lexData))
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

    char *obj = NULL;
    int objSize = 0;

    while (ended == 0 && *_lexData->startPos != '\0')
    {
        current = *_lexData->startPos;
        
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
                ended = JSON_LCB;
                break;

            case '}':
                state = 18;
                ended = JSON_RCB;
                break;

            case '[':
                state = 19;
                ended = JSON_LB;
                break;

            case ']':
                state = 20;
                ended = JSON_RB;
                break;

            case ':':
                state = 21;
                ended = JSON_COLON;
                break;

            case ',':
                state = 22;
                ended = JSON_COMMA;
                break;

            case '"':
                state = 23;
                break;

            case '-':
                state = 27;
                break;

            case '0':
                state = 28;
                break;

            default:
                if (current >= '1' && current <= '9')
                    state = 29;
                else if (isSep(current))
                {
                    /*if(current == "\n"){
                        ++_lexData->nbLignes;
                    }*/
                    state = 0;
                }
                else
                    ended = JSON_LEX_ERROR;
                break;
            }

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
                ended = JSON_TRUE;
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
                ended = JSON_FALSE;
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
                ended = JSON_NULL;
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
                ended = JSON_STRING;
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

        case 27:
            if (current == '0')
                state = 27;
            else if (current >= '1' && current <= '9')
                state = 29;
            else
                ended = JSON_LEX_ERROR;
            break;

        case 28:
            if (current == '.')
            {
                state = 30;
            }
            else if (isSep(current))
            {
                state = 31;
                ended = JSON_INT_NUMBER;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }
            break;

        case 29:
            if (current >= '0' && current <= '9')
                state = 29;
            else if (current == '.')
            {
                state = 30;
            }
            else if (isSep(current))
            {
                state = 31;
                ended = JSON_INT_NUMBER;
            }
            else if (toupper(current) == 'E')
            {
                state = 32;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }
            break;

        case 30:
            if (current >= '0' && current <= '9')
            {
                state = 33;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }
            break;

        case 31:
            if (current >= '0' && current <= '9')
            {
                state = 33;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }
            break;

        case 32:
            if (current == '+' || current == '-' || (current >= '0' && current <= '9'))
            {
                state = 35;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }

            break;

        case 33:
            if (current >= '0' && current <= '9')
            {
                state = 33;
            }
            else if (toupper(current) == 'E')
            {
                state = 32;
            }
            else if (isSep(current))
            {
                state = 34;
                ended = JSON_REAL_NUMBER;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }
            break;

        case 35:
            if (current >= '0' && current <= '9')
            {
                state = 35;
            }
            else if (isSep(current))
            {
                state = 36;
                ended = JSON_REAL_NUMBER;
            }
            else
            {
                ended = JSON_LEX_ERROR;
            }

            break;
        }

        obj = realloc(obj, sizeof(char) * (objSize + 1));
        obj[objSize] = current;
        ++objSize;

        ++_lexData->startPos;
    }

    char *tmp;
    obj = realloc(obj, sizeof(char) * (objSize + 1));
    obj[objSize] = '\0';

    switch (ended)
    {
    case JSON_STRING:
        tmp = strdup(obj);
        addStringSymbolToLexData(_lexData, tmp);
        break;

    case JSON_INT_NUMBER:
        addIntSymbolToLexData(_lexData, atoi(obj));
        break;

    case JSON_REAL_NUMBER:
        addRealSymbolToLexData(_lexData, atof(obj));
        break;
    }

    free(obj);

    return ended;
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
    while (i != JSON_LEX_ERROR && *lex_data->startPos != '\0')
    {
        printf("lex()=%d\n", i);
        i = lex(lex_data);
    }
    printLexData(lex_data);
    deleteLexData(&lex_data);

    return 0;
}