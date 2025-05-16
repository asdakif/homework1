#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 64
#define MAX_LINE 1024

// Struct Definitions
// Linked list node for items (ingredients, potions, trophies)
typedef struct Item {
    char name[MAX_NAME]; 
    int quantity;        
    struct Item* next;   
} Item;

// Linked list node for formula recipes
typedef struct Formula {
    char name[MAX_NAME];   
    Item* ingredients;   
    struct Formula* next;  // Next formula node
} Formula;

// Linked list node for bestiary knowledge
typedef struct Bestiary {
    char monster[MAX_NAME];
    Item* counters;         // Effective potions or signs
    struct Bestiary* next; 
} Bestiary;

// Global lists to hold datas of all things
Item* ingredients = NULL;
Item* potions = NULL;
Item* trophies = NULL;
Formula* formulas = NULL;
Bestiary* bestiary = NULL;
 /** 
 * Finds an item by name in the linked list.
 * 
 * @param head Pointer to head of the list.
 * @param name Item name.
 * @return Pointer to found item or NULL if not found.
 */
Item* find(Item* head, const char* name) {
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}
/**
 * Decreases the quantity of an item.
 * @param item Item that is added. 
 * @param quantity Amount to add.
 */
void add(Item** head, const char* name, int quantity) {
    Item* item = find(*head, name);
    if (item) item->quantity += quantity;
    else {
        Item* newItem = malloc(sizeof(Item));
        strcpy(newItem->name, name);
        newItem->quantity = quantity;
        newItem->next = *head;
        *head = newItem;
    }
}
/**
 * Decreases the quantity of an item.
 * @param item Item that is removed. 
 * @param quantity Amount to remove.
 */

void removeItem(Item* item, int quantity) {
    if (item->quantity >= quantity)
        item->quantity -= quantity;
}

void printAll(Item* head) {
    if (!head) {
        printf("None\n");
        return;
    }

    Item* arr[100];
    int n = 0;

    while (head) {
        if (head->quantity > 0)
            arr[n++] = head;
        head = head->next;
    }

    if (n == 0) {
        printf("None\n");
        return;
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (strcmp(arr[i]->name, arr[j]->name) > 0) {
                Item* temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        printf("%d %s", arr[i]->quantity, arr[i]->name);
        if (i != n - 1) printf(", ");
    }
    printf("\n");
}
void printSortedByQuantity(Item* head) {
    if (!head) {
        printf("None\n");
        return;
    }

    Item* arr[100];
    int n = 0;
    while (head) {
        if (head->quantity > 0) arr[n++] = head;
        head = head->next;
    }

    if (n == 0) {
        printf("None\n");
        return;
    }
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (arr[i]->quantity < arr[j]->quantity ||
               (arr[i]->quantity == arr[j]->quantity && strcmp(arr[i]->name, arr[j]->name) > 0)) {
                Item* temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        printf("%d %s", arr[i]->quantity, arr[i]->name);
        if (i != n - 1) printf(", ");
    }
    printf("\n");
}
/**
 * Handles the "Geralt loots" command.
 * Parses input line, extracts ingredients and adds them.
 */
void handleLoot(char* line) {
    char* loot = strtok(line + 12, ",");// Skips "Geralt loots"
    while (loot) {
        int q;
        char name[MAX_NAME];
        sscanf(loot, "%d %s", &q, name);
        add(&ingredients, name, q);
        loot = strtok(NULL, ",");
    }
    printf("Alchemy ingredients obtained\n");
}

/**
 * Handles the "Geralt trades" command.
 * Checks if Geralt has enough trophies, removes them, and gives ingredients in return.
 */
void handleTrade(char* line) {
    char* trade = strstr(line, "for");  // Split between trophy and ingredients
    if (!trade) {
        printf("INVALID\n");
        return;
    }

    // Extract the trophy part before for
    char trophiesPart[MAX_LINE];
    strncpy(trophiesPart, line + 13, trade - line - 13);
    trophiesPart[trade - line - 13] = '\0';

    char* trophy = strtok(trophiesPart, ",");
    int canTrade = 1;

    // does Geralt have all required trophies
    while (trophy) {
        int q;
        char name[MAX_NAME];
        sscanf(trophy, "%d %s", &q, name);
        strcat(name, " trophy");
        Item* t = find(trophies, name);
        if (!t || t->quantity < q){
            canTrade = 0;
        }  
        trophy = strtok(NULL, ",");
    }

    if (!canTrade) {
        printf("Not enough trophies\n");
        return;
    }
    // Remove the trophies
    trophy = strtok(trophiesPart, ",");
    while (trophy) {
        int q;
        char name[MAX_NAME];
        sscanf(trophy, "%d %s", &q, name);
        strcat(name, " trophy");
        Item* t = find(trophies, name);
        removeItem(t, q);
        trophy = strtok(NULL, ",");
    }

    // Add new ingredients Geralt gets
    trophy = strtok(trade + 4, ",");
    while (trophy) {
        int q;
        char name[MAX_NAME];
        sscanf(trophy, "%d %s", &q, name);
        add(&ingredients, name, q);
        trophy = strtok(NULL, ",");
    }

    printf("Trade successful\n");
}
/**
 * Handles the "Geralt learns" command.
 * Supports both potion formulas and monster effectiveness.
 */
void handleLearn(char* line) {
    // Learning a potion formula
    if (strstr(line, "consists of")) {
        char potion[MAX_NAME];
        char* start = line + 14;
        char* mid = strstr(line, " potion consists of");
        int len = mid - start;
        strncpy(potion, start, len);
        potion[len] = '\0';

        // Create new formula and parse ingredients
        Formula* f = malloc(sizeof(Formula));
        strcpy(f->name, potion);
        f->ingredients = NULL;

        char* recipe = mid + strlen(" potion consists of ");
        char* recipeToken = strtok(recipe, ",");
        while (recipeToken) {
            int q;
            char name[MAX_NAME];
            sscanf(recipeToken, "%d %s", &q, name);
            add(&f->ingredients, name, q);
            recipeToken = strtok(NULL, ",");
        }

        f->next = formulas;
        formulas = f;

        printf("New alchemy formula obtained: %s\n", potion);
    }

    // Learning what is effective against a monster
    else if (strstr(line, "is effective against")) {
        char monster[MAX_NAME];
        char thing[MAX_NAME];
        
        char* start = line + strlen("Geralt learns ");
        char* mid = strstr(start, " is effective against ");

        if (!mid) {
            printf("INVALID\n");
            return;
        }

        // Parse sign/potion and monster name
        int len = mid - start;
        strncpy(thing, start, len);
        thing[len] = '\0';
        char* space = strchr(thing, ' ');
        if (space) *space = '\0';

        sscanf(mid + strlen(" is effective against "), "%[^\n]", monster);

        // Add to bestiary
        Bestiary* b = bestiary;
        while (b && strcmp(b->monster, monster)) b = b->next;

        if (!b) {
            b = malloc(sizeof(Bestiary));
            strcpy(b->monster, monster);
            b->counters = NULL;
            b->next = bestiary;
            bestiary = b;
        }
        add(&b->counters, thing, 1);
        printf("New bestiary entry added: %s\n", monster);
    }
}

/**
 * Handles the "Geralt brews" command.
 * Checks if ingredients are available and brews the potion if possible.
 */
void handleBrew(char* line) {
    char potion[MAX_NAME];
    sscanf(line + 13, "%[^\n]", potion);

    // Find formula for the potion
    Formula* f = formulas;
    while (f && strcmp(f->name, potion)) f = f->next;

    if (!f) {
        printf("No formula for %s\n", potion);
        return;
    }

    // Check if Geralt has enough ingredients
    Item* cur = f->ingredients;
    while (cur) {
        Item* have = find(ingredients, cur->name);
        if (!have || have->quantity < cur->quantity) {
            printf("Not enough ingredients\n");
            return;
        }
        cur = cur->next;
    }

    // Brew the potion by removing ingredients and adding potion
    cur = f->ingredients;
    while (cur) {
        Item* have = find(ingredients, cur->name);
        removeItem(have, cur->quantity);
        cur = cur->next;
    }

    add(&potions, potion, 1);
    printf("Alchemy item created: %s\n", potion);
}


/**
 * Handles the "Geralt encounters a "monster"" command.
 * Checks if Geralt has knowledge about the monster, otherwise he escapes.
 */
void handleEncounter(char* line) {
    char dummy[5], monster[MAX_NAME];
    sscanf(line + 18, "%s %[^\n]", dummy, monster);  // Skip "Geralt encounters a"

    Bestiary* b = bestiary;
    while (b && strcmp(b->monster, monster)) b = b->next;

    if (!b) {
        printf("Geralt is unprepared and barely escapes with his life\n");
        return;
    }

    // Add trophy if monster defeated
    char trophyName[MAX_NAME];
    snprintf(trophyName, MAX_NAME, "%s trophy", monster);
    add(&trophies, trophyName, 1);
    printf("Geralt defeats %s\n", monster);
}

/**
 * Handles all types of queries from the user (e.g., Total potion X?, What is in X?).
 * Parses and prints appropriate data based on the question type.
 *
 * @param line The full input query line.
 */
void handleQuestions(char* line) {
    if (strstr(line, "Total ingredient?")) {
         printAll(ingredients);
    }
    else if (strstr(line, "Total potion")) {
        char name[MAX_NAME];
        sscanf(line + strlen("Total potion "), "%[^\n]", name);
        int len = strlen(name);
        name[len - 1] = '\0';  // Remove trailing '?'
        Item* t = find(potions, name);
        printf("%d\n", t ? t->quantity : 0);
    }
    else if (strstr(line, "Total trophy")) {
        char name[MAX_NAME];
        sscanf(line + strlen("Total trophy "), "%[^\n]", name);
        int len = strlen(name);
        name[len - 1] = '\0';
        strcat(name, " trophy");
        Item* t = find(trophies, name);
        printf("%d\n", t ? t->quantity : 0);
    }
    else if (strstr(line, "What is in")) {
        char potion[MAX_NAME];
        sscanf(line + strlen("What is in "), "%[^\n]", potion);
        int len = strlen(potion);
        if (potion[len - 1] == '?') potion[len - 1] = '\0';
        Formula* f = formulas;
        while (f && strcmp(f->name, potion)) f = f->next;
        if (!f) {
            printf("No formula for %s\n", potion);
            return;
        }
        printSortedByQuantity(f->ingredients);
    }
    else if (strstr(line, "What is effective against")) {
        char monster[MAX_NAME];
        sscanf(line + strlen("What is effective against "), "%[^\n]", monster);
        int len = strlen(monster);
        if (monster[len - 1] == '?') monster[len - 1] = '\0';

        Bestiary* b = bestiary;
        while (b && strcmp(b->monster, monster)) b = b->next;

        if (!b) {
            printf("No knowledge of %s\n", monster);
            return;
        }

        // Print known effective items
        Item* cur = b->counters;
        int first = 1;
        while (cur) {
            if (!first) { 
                printf(", ");}
            printf("%s", cur->name);
            first = 0;
            cur = cur->next;
        }
        printf("\n");
    }
}
/**
 * Main function of Witcher Tracker project.
 * Handles command processing and input reading from file or terminal.
 */
int main() {
    char line[MAX_LINE];
    FILE* input = stdin; // Default input is terminal

    while (1) {
        // Starting point
        if (input == stdin) {
            printf(">> ");
            fflush(stdout);
        }
        // Read line from input
        if (!fgets(line, MAX_LINE, input)){
                break;
        }
        
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        // Exit condition
        if (!strcmp(line, "Exit")){
             break;
        }
        // Check what is the input
        if (strstr(line, "Geralt loots")){
            handleLoot(line);
        }
        else if (strstr(line, "Geralt trades")){
            handleTrade(line);
        }
            
        else if (strstr(line, "Geralt learns")){
            handleLearn(line);
        }
          
        else if (strstr(line, "Geralt brews")){
            handleBrew(line);
        }
        else if (strstr(line, "Geralt encounters a")){
            handleEncounter(line);
        }
        else if (strstr(line, "What is effective against")){
            handleQuestions(line);
        }
        else if (strstr(line, "What is in")){
            handleQuestions(line);
        }
        else if (strstr(line, "Total")){
            handleQuestions(line);
        }
        else{
            printf("INVALID\n");
        }
    }
    return 0;
}
