/*

    SECRET CODE is a 2-player board game, also known as MasterMind.
    There's a slight difference, in Mastermind a player needs to create a code
    of 4 numbers, here we use colors instead.
    The available colors are: white, red, yellow, orange, blue, pink, purple
    and green.
    The first player creates a code of 4 different colors and the second one
    needs to guess the code in less than 10 attempts.
    In each attempt, the second player "proposes" a code, a combination of
    4 different colors to the other player who responds with colors.
    The response is always composed of 4 "elements", because the code is always
    of 4 colors.
    There are 3 types of elements:
    - empty, means that one of the colors you put in your attempt is not in
        the code
    - yellow, means that one of the colors you put in your attempt is in the
        code, but you put it in the wrong position
    - red, means that one of the colors you put in your attempt is in the code
        and in the correct position.
    It is very very important that nobody tells the second player which color
    corresponds to which "element", guessing the secret code would be too easy
    that way.
    Some example of responses:
    4 reds: you won, you guessed all colors and placed them in the correct
            positions.
    4 yellows: almost there, you guessed the colors but they're all in the wrong
            positions.
    4 emptys: none of the colors is in the code, this may seem like the worst
            attempt ever, but actually now you know that you don't need to touch
            these colors again.
    1 red, 2 yellows: typical mid-game response, one is correct, 2 are in wrong
            positions and one is not in the code, but who is who?

    This program plays as the second player, who tries to guess your secret code
    in less than 10 attempts.

    In the real game, a response like "3 reds, 1 yellow" cannot exist because it
    is equal to "4 reds". However, this program doesn't recognize this one.

    Obviously, responses need to be consistent, this program isn't that clever.

    After many many tweaks I finally created a perfect player that never loses.
    The maximum number of attempts I made him try is 9, let me know if you can
    make it go to 10.

    ----------------------------------------------------------------------------

    Compile with:
    gcc secret_code.c -o secret_code

    Run with:
    ./secret_code

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define abs(n) n<0?-n:n


typedef struct s{
    char* name;
    bool visited;
}color;

color colori[8] = { {"red", false},
                    {"yellow", false},
                    {"green", false},
                    {"orange", false},
                    {"pink", false},
                    {"purple", false},
                    {"blue", false},
                    {"white", false} };



typedef struct s2{
    float score;
    color* ordine[4];
    struct s2* next;
}solution;

solution *soluzioni = NULL;
solution *tmp = NULL;


color* codice[4] = {NULL};

unsigned int total_solutions = 0;

int tentativi = 10;

int num_rossi = 0;
int num_gialli = 0;



void write_color(color* pc);
void delete_color();
void add_solution();
void delete_solution(solution *ps);
void print_solutions();
void print_graph();

void explore_combinations(int depth, int index_color);

solution* find_solution();

void set_best_solution();

int evaluate_solution(solution* sol);

int main ( void ) {

    for(int i=0; i<8; i++){
        colori[i].visited = true;
        write_color(&colori[i]);
        explore_combinations(4, i);
        delete_color();
        colori[i].visited = false;
    }

    for(int i=0; i<8; i++) colori[i].visited = false;
    for(int i=0; i<4; i++) codice[i] = NULL;
    printf("I\'m ready to play!\nPress ENTER when you\'re ready too.\n\n");
    getchar();


    while(tentativi && num_rossi!=4){
        printf("(%d solutions remaining)\n",total_solutions);
        printf("Attempt n.%d :\n",11-tentativi);
        set_best_solution();
        for(int i=0; i<4; i++){
            printf("%s%s",codice[i]->name,i!=3?" - ":"\n");
        }

        do{
            if(num_gialli+num_rossi>4 || num_gialli+num_rossi<0) printf("Incorrect numbers.\n");
            int ver;
            do{
                printf("How many reds? ");
                ver = scanf("%d",&num_rossi);
                if(num_rossi>4 || num_rossi<0) ver = 0;
                fflush(stdin);
            }while(!ver);
            do{
                printf("How many yellows? ");
                ver = scanf("%d",&num_gialli);
                if(num_gialli>4 || num_gialli<0) ver = 0;
                fflush(stdin);
            }while(!ver);
        }while(num_gialli+num_rossi>4 || num_gialli+num_rossi<0);

        tmp = soluzioni;
        while(tmp != NULL){
            if(!evaluate_solution(tmp)) delete_solution(tmp);
            tmp = tmp->next;
        }
        printf("\n\n");

        tentativi--;
    }

    if(num_rossi == 4) printf("I won =)!!!\n\n\n");
    else printf("I\'m not that good. =(\n\n\n");



    goto fine;
    fine:
        tmp = soluzioni;
        while(tmp != NULL){
            tmp = tmp->next;
            free(soluzioni);
            soluzioni = tmp;
        }
    system("pause");
    return 0;
}

void write_color(color* pc){
    int i;
    for(i=0; i<4 && codice[i]!=NULL; i++) ;
    codice[i] = pc;
}

void delete_color(){
    int i;
    for(i=3; i>=0 && codice[i]==NULL; i--) ;
    codice[i] = NULL;
}

void add_solution(){
    tmp = (solution*) malloc(sizeof(solution));
    tmp->score = 0;
    for(int i=0; i<4; i++) tmp->ordine[i] = codice[i];
    tmp->next = soluzioni;
    soluzioni = tmp;
    total_solutions++;
}

void delete_solution(solution* ps){
    if(ps == soluzioni){
        ps = ps->next;
        free(soluzioni);
        soluzioni = ps;
        total_solutions--;
        return;
    }
    solution* sol = soluzioni;
    while(sol->next!=ps) sol = sol->next;
    sol->next = ps->next;
    free(ps);
    total_solutions--;
}

solution* find_solution(){
    tmp = soluzioni;
    int found;
    while(tmp!=NULL){
        found = 0;
        for(int i=0; i<4; i++){
            if(tmp->ordine[i] == codice[i]) found++;
        }
        if(found == 4) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void explore_combinations(int depth, int index_color){
    if(depth==1 && find_solution()==NULL){
        add_solution();
        return;
    }

    for(int i=0; i<8; i++){
        if(colori[i].visited == false){
            colori[i].visited = true;
            write_color(&colori[i]);
            explore_combinations(depth-1, i);
            delete_color();
            colori[i].visited = false;
        }
    }
}

void set_best_solution(){
    int max = soluzioni->score;
    solution* best = soluzioni;
    tmp = soluzioni;
    while(tmp != NULL){
        if(tmp->score > max){
            max = tmp->score;
            best = tmp;
        }
        tmp = tmp->next;
    }
    for(int i=0; i<4; i++) codice[i] = best->ordine[i];
}

void print_solutions(){
    printf("Solutions:\n");
    tmp = soluzioni;
    while(tmp != NULL){
        for(int i=0; i<4; i++)
            printf("%s - ",tmp->ordine[i]->name);
        printf("\n");
        tmp = tmp->next;
    }
    printf("---\n");
}

void print_graph(){
    for(int i=0; i<8; i++){
        printf("%-9s %s\n",colori[i].name,colori[i].visited==true?"YES":"NO");
    }
}

int evaluate_solution(solution* sol){
    //Returns 0 if sol needs to be deleted
    //        1 otherwise
    int g=0, r=0;
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            if(codice[i] == sol->ordine[j]){
                if(i==j) r++;
                else g++;
            }
        }
    }

    if(num_gialli+num_rossi == 4){
        if(g+r == 4 && r!=4){
            sol->score += (float)(abs(num_rossi-r) + abs(num_gialli-g)*1.1);
            return 1;
        }
        else return 0;
    }
    else if(num_gialli+num_rossi == 0){
        if(g+r>0) return 0;
        else{
            sol->score += (float)((4-r)*1.1 + (4-g));
            return 1;
        }
    }
    else if(g+r==4 || g+r==0) return 0;
    else{
        sol->score += (float)( abs(num_rossi-r) + abs(num_gialli-g) );
        return 1;
    }
}
