/* Iosif Constantin-Andrei – grupa 312CC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct page
{
    int id;
    char url[50];
    char *description;
} page;

typedef struct nodeStack
{
    page *pagina;
    struct nodeStack *next;
} nodeStack;

typedef struct stack
{
    struct nodeStack *top;
} stack;

typedef struct tab
{
    int id;
    page *currentPage;
    stack *backwardStack;
    stack *forwardStack;
} tab;

typedef struct nodeList
{
    tab *tab;
    struct nodeList *next;
    struct nodeList *prev;

} nodeList;

typedef struct tabsList
{
    nodeList *head;
    nodeList *tail;
} tabsList;

typedef struct browser
{
    tab *current;
    tabsList *list;
} browser;

// Functie pentru crearea paginii implicite
page pagina_implicita()
{
    char *s = "Computer Science";
    page pagina_implicita;
    pagina_implicita.id = 0;
    memset(pagina_implicita.url, 0, 50);
    strcpy(pagina_implicita.url, "https://acs.pub.ro/");
    pagina_implicita.description = malloc(strlen(s) + 1);
    strcpy(pagina_implicita.description, s);
    return pagina_implicita;
}

// Functie pentru initializarea unui nou tab
tab init_tab(page pages[])
{
    tab tab0;
    tab0.id = 0;
    tab0.currentPage = &pages[0];
    stack *backward = malloc(sizeof(stack));
    stack *forward = malloc(sizeof(stack));
    backward->top = NULL;
    forward->top = NULL;
    tab0.backwardStack = backward;
    tab0.forwardStack = forward;
    return tab0;
}

// Functie pentru initializarea browserului
browser *init_browser(page pages[])
{
    browser *browser0;
    browser0 = malloc(sizeof(browser));
    tab *tab0;
    tab0 = malloc(sizeof(tab));
    *tab0 = init_tab(pages);
    browser0->current = tab0;

    nodeList *santinela;
    santinela = malloc(sizeof(nodeList));
    santinela->tab = malloc(sizeof(tab));
    santinela->next = santinela;
    santinela->prev = santinela;
    santinela->tab->id = -1;

    nodeList *node;
    node = malloc(sizeof(nodeList));
    node->tab = tab0;
    node->next = santinela;
    node->prev = santinela;
    santinela->next = node;
    santinela->prev = node;

    tabsList *list;
    list = malloc(sizeof(tabsList));
    list->head = santinela;
    list->tail = node;
    browser0->list = list;

    return browser0;
}

// Functie pentru eliberarea memoriei unei stive
void free_stack(stack *s)
{
    if (!s)
        return;
    nodeStack *temp = s->top;
    while (temp != NULL)
    {
        nodeStack *next = temp->next;
        free(temp);
        temp = next;
    }
    free(s);
}

// Functie pentru eliberarea memoriei listei de taburi
void free_tabs_list(tabsList *list)
{
    if (!list)
        return;

    nodeList *santinela = list->head;
    nodeList *temp = santinela->next;

    while (temp != santinela)
    {
        nodeList *next = temp->next;

        if (temp->tab)
        {
            free_stack(temp->tab->backwardStack);
            free_stack(temp->tab->forwardStack);
            free(temp->tab);
        }

        free(temp);
        temp = next;
    }
    if (santinela->tab)
    {
        free(santinela->tab);
    }
    free(santinela);
    free(list);
}

// Functie pentru eliberarea memoriei browserului
void free_browser(browser *browser)
{
    if (!browser)
        return;
    free_tabs_list(browser->list);
    free(browser);
}

void NEW_TAB(browser *browser, page pages[], int *tabMax)
{
    tab *newTab;
    newTab = malloc(sizeof(tab));

    // Initializeaza tab-ul cu pagina implicita
    *newTab = init_tab(pages);
    newTab->id = *tabMax + 1;
    (*tabMax)++;

    // Creeaza nodul din lista corespunzator noului tab
    nodeList *node;
    node = malloc(sizeof(nodeList));
    node->tab = newTab;

    // Conecteaza nodul in lista
    node->prev = browser->list->tail;
    browser->list->tail->next = node;
    browser->list->tail = node;
    node->next = browser->list->head;
    browser->list->head->prev = node;

    // Seteaza tab-ul curent
    browser->current = node->tab;
}

void CLOSE(browser *browser, FILE *fout)
{
    // Conditia pentru a nu inchide tab-ul cu ID 0
    if (browser->current->id == 0)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Gasim nodul din lista care contine tab-ul curent
    nodeList *temp = browser->list->head->next;
    while (temp != browser->list->head && temp->tab != browser->current)
    {
        temp = temp->next;
    }

    // Daca tabul nu exista afisam mesajul de eroare
    if (temp == browser->list->head)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Setam noul tab curent
    if (temp->prev != browser->list->head)
    {
        browser->current = temp->prev->tab;
    }
    else if (temp->next != browser->list->head)
    {
        browser->current = temp->next->tab;
    }

    // Scoatem nodul din lista
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;

    // Actualizam coada daca era ultimul tab
    if (browser->list->tail == temp)
    {
        if (temp->prev != browser->list->head)
        {
            browser->list->tail = temp->prev;
        }
        else
        {
            browser->list->tail = browser->list->head->prev;
        }
    }

    // Eliberam memoria alocata pentru stive si tab
    free_stack(temp->tab->backwardStack);
    free_stack(temp->tab->forwardStack);
    free(temp->tab);
    free(temp);
}

void addStack(stack *s, page *page)
{
    nodeStack *nou = malloc(sizeof(nodeStack));
    nou->pagina = page;
    nou->next = NULL;
    if (s->top == NULL)
    {
        s->top = nou;
    }
    else
    {
        nou->next = s->top;
        s->top = nou;
    }
}

void popStack(stack *s)
{
    if (s->top == NULL)
        return;
    nodeStack *temp = s->top;
    s->top = s->top->next;
    free(temp);
}

int isEmptyStack(stack *s)
{
    if (s->top == NULL)
    {
        return 1;
    }
    else
        return 0;
}

// Functie ce creeaza o copie inversata a unei stive
stack *invers(stack *s)
{
    stack *temp = malloc(sizeof(stack));
    temp->top = NULL;

    nodeStack *current = s->top;
    while (current != NULL)
    {
        // Copiem elementele in ordine inversa
        nodeStack *nou = malloc(sizeof(nodeStack));
        nou->pagina = current->pagina;
        nou->next = temp->top;
        temp->top = nou;
        current = current->next;
    }

    return temp;
}

void OPEN(browser *browser, int ID_cautat, FILE *fout)
{
    nodeList *temp;

    // Cautam tab-ul cu ID-ul dat
    temp = browser->list->head->next;
    while (temp->tab->id != ID_cautat && temp != browser->list->head)
    {
        temp = temp->next;
    }

    // Daca nu exista afisam mesajul de eroare
    if (temp == browser->list->head)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Setam noul tab current
    browser->current = temp->tab;
}

void NEXT(browser *browser)
{
    // Cautam nodul din lista care contine tab-ul curent
    nodeList *temp = browser->list->head->next;
    while (temp != browser->list->head && temp->tab != browser->current)
    {
        temp = temp->next;
    }

    // Verificam care este urmatorul tab si il setam ca tabul curent
    // Daca am ajuns la sfarsitul listei, revenim la inceput
    if (temp->next == browser->list->head)
    {
        temp = browser->list->head->next;
    }
    else
    {
        temp = temp->next;
    }
    browser->current = temp->tab;
}

void PREV(browser *browser)
{
    // Cautam nodul din lista care contine tab-ul curent
    nodeList *temp = browser->list->head->next;
    while (temp != browser->list->head && temp->tab != browser->current)
    {
        temp = temp->next;
    }

    // Verificam care este tab-ul precedent si il setam ca tabul curent
    // Daca am ajuns la inceputul listei, mergem la final
    if (temp->prev == browser->list->head)
    {
        temp = browser->list->head->prev;
    }
    else
    {
        temp = temp->prev;
    }

    browser->current = temp->tab;
}

void PAGE(browser *browser, int ID_cautat, page pages[], int nrPagini, FILE *fout)
{
    int i;
    // Cautam pagina cu ID-ul dorit in vectorul de pagini
    for (i = 0; i < nrPagini; i++)
    {
        if (pages[i].id == ID_cautat)
            break;
    }
    // Daca nu exista pagina cu ID-ul cautat afisam mesajul de eroare
    if (i == nrPagini)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Adaugam pagina curenta in stiva BACKWARD
    addStack(browser->current->backwardStack, browser->current->currentPage);

    // Golim stiva FORWARD
    while (!isEmptyStack(browser->current->forwardStack))
    {
        popStack(browser->current->forwardStack);
    }

    // Setam noua pagina ca pagina curenta
    browser->current->currentPage = &pages[i];
}

void BACKWARD(browser *browser, FILE *fout)
{
    if (isEmptyStack(browser->current->backwardStack))
    {
        // Afisam mesajul de eroare daca stiva BACKWARD e goala
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Salvam pagina curenta in stiva FORWARD
    addStack(browser->current->forwardStack, browser->current->currentPage);

    // Setam pagina anterioara ca pagina curenta
    browser->current->currentPage = browser->current->backwardStack->top->pagina;

    // Stergem din stiva BACKWARD
    popStack(browser->current->backwardStack);
}

void FORWARD(browser *browser, FILE *fout)
{
    if (isEmptyStack(browser->current->forwardStack))
    {
        // Afisam mesajul de eroare daca stiva FORWARD e goala
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    // Salvam pagina curenta in stiva BACKWARD
    addStack(browser->current->backwardStack, browser->current->currentPage);

    // Setam pagina urmatoare ca pagina curenta
    browser->current->currentPage = browser->current->forwardStack->top->pagina;

    // Stergem din stiv FORWARD
    popStack(browser->current->forwardStack);
}

void PRINT(browser *browser, FILE *fout)
{

    // Cautam tab-ul curent in lista
    nodeList *temp = browser->list->head->next;
    while (temp != browser->list->head && temp->tab != browser->current)
    {
        temp = temp->next;
    }

    // Daca tab-ul nu a fost gasit, afisam mesajul de eroare
    if (temp == browser->list->head)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }
    nodeList *start = temp;

    // Afisam circular incepand de la tab-ul curent
    do
    {
        fprintf(fout, "%d ", temp->tab->id);
        temp = temp->next;
        if (temp == browser->list->head)
        {
            temp = temp->next; // sarim peste santinela
        }
    } while (temp != start);

    fprintf(fout, "\n");
    fprintf(fout, "%s\n", browser->current->currentPage->description);
}

void PRINT_HISTORY(browser *browser, int ID_cautat, FILE *fout)
{
    // Cautam tab-ul cu ID-ul dorit
    nodeList *temp = browser->list->head->next;
    while (temp->tab->id != ID_cautat && temp != browser->list->head)
    {
        temp = temp->next;
    }

    // Daca tab-ul nu a fost gasit, afisam mesajul de eroare
    if (temp == browser->list->head)
    {
        fprintf(fout, "403 Forbidden\n");
        return;
    }

    stack *tempStack;
    nodeStack *tempPage;

    // Afisam stiva FORWARD folosind o copie inversata temporara
    if (isEmptyStack(temp->tab->forwardStack) == 0)
    {
        tempStack = invers(temp->tab->forwardStack);
        tempPage = tempStack->top;
        while (tempPage != NULL)
        {
            fprintf(fout, "%s\n", tempPage->pagina->url);
            tempPage = tempPage->next;
        }
        free_stack(tempStack);
    }

    // Afisam pagina curenta
    fprintf(fout, "%s\n", temp->tab->currentPage->url);
    tempPage = temp->tab->backwardStack->top;

    // Afisam stiva BACKWARD
    if (isEmptyStack(temp->tab->backwardStack) == 0)
    {
        while (tempPage != NULL)
        {

            fprintf(fout, "%s\n", tempPage->pagina->url);
            tempPage = tempPage->next;
        }
    }
}

// Functie ce converteste un numar salvat in forma de char in int
int conversie(char *s)
{
    int j, n;
    j = 0;
    n = 0;
    while (s[j] != '\0' && s[j] != '\n')
    {
        n = n * 10 + s[j] - '0';
        j++;
    }
    return n;
}

// Functie care elimina caracterul '\n' de la finalul unui sir
void prelucrareString(char *s)
{
    int len;
    len = strlen(s);
    if (s[len - 1] == '\n')
    {
        s[len - 1] = '\0';
    }
}

// Functie care citeste datele despre pagini si le salveaza in vectorul pages[]
void citire_Pagini(page pages[], int nrPagini, FILE *fin)
{
    char text[100];

    // Pagina cu indexul 0 reprezinta pagina implicita
    for (int i = 0; i < nrPagini - 1; i++)
    {
        // Citim si convertim id-ul
        fgets(text, 100, fin);
        pages[i + 1].id = conversie(text);

        // Citim si prelucram URL-ul
        fgets(pages[i + 1].url, 50, fin);
        prelucrareString(pages[i + 1].url);

        // Citim si prelucram descrierea si o alocam dinamic
        fgets(text, 100, fin);
        prelucrareString(text);
        pages[i + 1].description = malloc(strlen(text) + 1);
        strcpy(pages[i + 1].description, text);
    }
}

int main()
{
    // Deschidem fisierele de intrare si iesire
    FILE *fin = fopen("tema1.in", "r");
    FILE *fout = fopen("tema1.out", "w");

    int nrPagini = 0, j = 0;
    char aux0[100];

    // Citim numarul de pagini din fisier
    fgets(aux0, 100, fin);
    nrPagini = conversie(aux0);

    // Adaugam 1 deoarece salvam pagina implicita pe pages[0]
    nrPagini = nrPagini + 1;

    // Alocam dinamic vectorul de pagini
    page *pages;
    pages = malloc(sizeof(page) * nrPagini);

    // Initializam pagina implicita
    pages[0] = pagina_implicita();

    if (nrPagini > 1)
    {
        citire_Pagini(pages, nrPagini, fin);
    }

    // Initializam browser-ul
    browser *browser = init_browser(pages);

    int nrInstructiuni;
    int tabMax;
    tabMax = 0;
    char aux[100];
    char *instructiune;

    // Citim numarul de instructiuni
    fgets(aux, 100, fin);
    nrInstructiuni = conversie(aux);

    int i, ID_cautat;
    for (i = 0; i < nrInstructiuni; i++)
    {
        ID_cautat = 0;

        // Citim si prelucram instructiunea si alocam dinamic
        fgets(aux, 100, fin);
        instructiune = malloc((strlen(aux) + 1) * sizeof(char));
        strcpy(instructiune, aux);
        prelucrareString(instructiune);

        // Verificam ce instructiune primim si o executam
        if (strcmp(instructiune, "NEW_TAB") == 0)
        {
            NEW_TAB(browser, pages, &tabMax);
        }
        else if (strcmp(instructiune, "CLOSE") == 0)
        {
            CLOSE(browser, fout);
        }
        else if (strcmp(instructiune, "PRINT") == 0)
        {
            PRINT(browser, fout);
        }
        else if (strcmp(instructiune, "FORWARD") == 0)
        {
            FORWARD(browser, fout);
        }
        else if (strcmp(instructiune, "BACKWARD") == 0)
        {
            BACKWARD(browser, fout);
        }
        else if (strcmp(instructiune, "NEXT") == 0)
        {
            NEXT(browser);
        }
        else if (strcmp(instructiune, "PREV") == 0)
        {
            PREV(browser);
        }
        else if (strncmp(instructiune, "OPEN", 4) == 0)
        {
            ID_cautat = conversie(instructiune + 5);
            OPEN(browser, ID_cautat, fout);
        }

        else if (strncmp(instructiune, "PAGE", 4) == 0)
        {
            ID_cautat = conversie(instructiune + 5);
            PAGE(browser, ID_cautat, pages, nrPagini, fout);
        }
        else if (strncmp(instructiune, "PRINT_HISTORY", 13) == 0)
        {
            ID_cautat = conversie(instructiune + 14);
            PRINT_HISTORY(browser, ID_cautat, fout);
        }

        // Eliberam memoria folosita de instructiune
        free(instructiune);
    }

    // Eliberam memoria alocata pentru descrierea fiecarei pagini
    for (int i = 0; i < nrPagini; i++)
    {
        free(pages[i].description);
    }

    // Eliberam vectorul de pagini si browser-ul
    free(pages);
    free_browser(browser);

    // Inchidem fisierele de intrare si iesire
    fclose(fin);
    fclose(fout);
}