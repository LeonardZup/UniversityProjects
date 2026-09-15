#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

#define port_lavagna 5678

#define MAX_COMMAND_LENGTH 64
#define MAX_TEXT_LENGTH 128     //lunghezza testo attività
#define MAX_PAYLOAD_LENGTH 512

#define MAX_CARDS 50       //massimo di card nella lavagna
#define BUFFER_SIZE 1024
#define MAX_UTENTI 20       //valore massimo di utenti

#define COL_WIDTH 15        //larghezza colonna per la stampa della lavagna

#define PING_TIMEOUT 30     //timeout per il ping

enum card_state {TO_DO, DOING, DONE};

struct card{
    int id;
    enum card_state colonna;        //colonna a cui appartiene la card
    char attivita[MAX_TEXT_LENGTH];     //testo dell'attività
    int utente;     //porta dell'utente
    time_t ultima_modifica;     //timestamp ultima modifica  
};

struct card lavagna[MAX_CARDS] = {0};
int num_cards = 0;

struct utente{
    int sock_utente;
    int porta;
    int card_gestita;
};

//lista degli utenti registrati e contatore
struct utente lista_utenti[MAX_UTENTI] = {0};
int num_utenti = 0;

//lista di attesa per gli utenti non ancora registrati e contatore
int attesa_utenti[MAX_UTENTI] = {0};
int num_attesa = 0;

//buffer per lo scambio dei messaggi
char buffer[BUFFER_SIZE];

//buffer in cui salvare il payload
char payload[MAX_PAYLOAD_LENGTH];

//campi dei messaggi ricevuti
#define MAX_FIELD_LENGTH 128
char rec_command[MAX_COMMAND_LENGTH] = {0};
int rec_port = 0;
char rec_payload[MAX_PAYLOAD_LENGTH] = {0};

//vettore per memorizzare i ping inviati, usato per evitare di inviare ping ripetuti
int ping_sent[MAX_CARDS] = {0};

/*======================================================================================================*/
//funzioni ausiliarie

//funzione da chiamare nella qsort per l'ordinamento degli utenti in base al numero di porta
int compare_users(const void *a, const void *b) {
  struct utente *utente_a = (struct utente*)a;
  struct utente *utente_b = (struct utente*)b;

  return (utente_a->porta - utente_b->porta);
}

//funzione per la codifica del messaggio
int codifica_messaggio(char *comando, char *buffer, int buffer_size, char *payload){
    //costruisco la stringa da inviare
    int len = snprintf(buffer, BUFFER_SIZE, "%s|%d|%s\n", comando, port_lavagna, payload);
    return len;
}

//funzione per la decodifica deimessaggi
void decodifica_messaggio (char *messaggio, int lunghezza){
    sscanf(messaggio, "%99[^|]|%d|%99[^\n]", rec_command, &rec_port, rec_payload);
}

//funzione per la creazione delle prime 10 card
void riempi_lavagna(){
    for (int i = 0; i < 10; i++){
        lavagna[i].id = i + 1;
        lavagna[i].colonna = TO_DO;
        snprintf(lavagna[i].attivita, MAX_TEXT_LENGTH, "task %d", i + 1);
        lavagna[i].utente = 0;
        lavagna[i].ultima_modifica = time(NULL);
        num_cards++;
    }
}

//funzione per la creazione di una nuova card
void create_card(){
    lavagna[num_cards].id = num_cards + 1;
    lavagna[num_cards].colonna = TO_DO;
    snprintf(lavagna[num_cards].attivita, MAX_TEXT_LENGTH, rec_payload, num_cards + 1);
    lavagna[num_cards].utente = 0;
    lavagna[num_cards].ultima_modifica = time(NULL);
    num_cards++;
}

//funzione chiamata nella handle card per la costruzione della lista degli utenti
int aux_handle_card(int utente){
    //si sceglie la prima card libera e si codifica il messaggio con id card, testo attivita, numero utenti e lista utenti (meno l'utente a cui inviare)
    int card_id = 0;

    for (int i = 0; i < MAX_CARDS; i++){
        //se la card è stata creata e non è assegnata a nessun utente si sceglie per assegnarla all'utente
        if ((lavagna[i].id != 0) && (lavagna[i].colonna == TO_DO)){
            card_id = lavagna[i].id;
            break;
        }
    }

    if (card_id == 0){
        return -1;
    }

    //costruzione dell'elenco di utenti connessi
    char send_lista_utenti[128] = {0};
    char send_porta[8] = {0};

    int count = 0;
    for (int i = 0; i < num_utenti; i++){
       if (lista_utenti[i].porta != utente) {
            if (count > 0) {
                strcat(send_lista_utenti, ",");
            }
            sprintf(send_porta, "%d", lista_utenti[i].porta);
            strcat(send_lista_utenti, send_porta);
            count++;
        }
    }

    //costruzione del payload completo
    snprintf(payload, MAX_PAYLOAD_LENGTH, "%d-%s-%d-%s", lavagna[card_id - 1].id, lavagna[card_id - 1].attivita, num_utenti, send_lista_utenti);

    return 0;
}

void handle_card(){
    //ricerca dell'utente con numero di porta piu basso e invio del comando HANDLE_CARD
    for (int j = 0; j < num_utenti; j++) {
        if (lista_utenti[j].card_gestita == 0) {
            //costruisco il payload
            //se ci sono card libere si invia HANDLE_CARD
            if (aux_handle_card(lista_utenti[j].porta) < 0){
                break;
            }
            int message_len = codifica_messaggio("HANDLE_CARD", buffer, BUFFER_SIZE, payload);
            printf("invio card %s\n", buffer);
            send(lista_utenti[j].sock_utente, buffer, message_len, 0);
            break;
        }
    }
}

void move_card(char *comando, int id_card){
    int user_index;
    //ricerca dell'utente nella lista
    for (int i = 0; i < num_utenti; i++){
        if (lista_utenti[i].porta == rec_port){
            user_index = i;
            break;
        }
    }

    //spostamento della card in DOING nel caso di comando ACK_CARD
    if (strcmp(comando, "ACK_CARD") == 0){
        lista_utenti[user_index].card_gestita = id_card;
        lavagna[id_card - 1].utente = lista_utenti[user_index].porta;
        lavagna[id_card - 1].colonna = DOING;
        lavagna[id_card - 1].ultima_modifica = time(NULL);
    }

    //spostamento della card in DONE nel caso di CARD_DONE
    if (strcmp(comando, "CARD_DONE") == 0){
        lista_utenti[user_index].card_gestita = 0;
        lavagna[id_card - 1].colonna = DONE;
        lavagna[id_card - 1].ultima_modifica = time(NULL);
    }
}

//funzione per la stampa della lavagna, viene chiamata ogni volta che si verifica un evento che ne richiede l'aggiornamento della visualizzazione
void show_lavagna(){
    //pulizia della tabella precedente
    system("clear");

    //suddivisione delle card in colonne
    struct card todo[MAX_CARDS];
    struct card doing[MAX_CARDS];
    struct card done[MAX_CARDS];

    int todo_count = 0;
    int doing_count = 0;
    int done_count = 0;

    for (int i = 0; i < MAX_CARDS; i++){
        if (lavagna[i].id != 0){
            switch (lavagna[i].colonna){
                case TO_DO:
                    todo[todo_count] = lavagna[i];
                    todo_count++;
                    break;
                case DOING:
                    doing[doing_count] = lavagna[i];
                    doing_count++;
                    break;
                case DONE:
                    done[done_count] = lavagna[i];
                    done_count++;
                    break;
            }
        }
    }

    //stampa dell'header della lavagna
    printf("                     LAVAGNA - %d                    \n", port_lavagna);
    printf("-------------------------------------------------------\n");
    printf("|      TO DO      |      DOING      |      DONE       |\n");
    printf("|-----------------------------------------------------|\n");

    int max_rows = 0;
    if (todo_count > max_rows){
        max_rows = todo_count;
    }
    if (doing_count > max_rows){
        max_rows = doing_count;
    }
    if (done_count > max_rows){
        max_rows = done_count;
    }

    //stampa delle card
    for (int i = 0; i < max_rows; i++){
        char todo_id[COL_WIDTH] = {""};        
        char doing_id[COL_WIDTH] = {""};
        char done_id[COL_WIDTH] = {""};
        
        char todo_str[COL_WIDTH] = {""};
        char doing_str[COL_WIDTH] = {""};
        char done_str[COL_WIDTH] = {""};

        //preparazione delle stringhe da stampare
        if (i < todo_count){
            snprintf(todo_id, COL_WIDTH, "Task %d", todo[i].id);
            strncpy(todo_str, todo[i].attivita, COL_WIDTH);
        }
        if (i < doing_count){
            snprintf(doing_id, COL_WIDTH, "Task %d", doing[i].id);
            strncpy(doing_str, doing[i].attivita, COL_WIDTH);
        }
        if (i < done_count){
            snprintf(done_id, COL_WIDTH, "Task %d", done[i].id);
            strncpy(done_str, done[i].attivita, COL_WIDTH);
        }

        //stampa degli id
        printf("| %-*s | %-*s | %-*s |\n", COL_WIDTH, todo_id, COL_WIDTH, doing_id, COL_WIDTH, done_id);

        //stampa dei testi attività
        printf("| %-*s | %-*s | %-*s |\n", COL_WIDTH, todo_str, COL_WIDTH, doing_str, COL_WIDTH, done_str);

        if (i == max_rows - 1){
            printf("-------------------------------------------------------\n");
        } else{
            printf("|   -----------   |   -----------   |   -----------   |\n");
        }
    }
}

void request_user_list(int utente){
    //costruzione dell'elenco di utenti connessi
    char send_lista_utenti[128] = {0};
    char send_porta[8] = {0};

    int count = 0;
    for (int i = 0; i < num_utenti; i++){
       if (lista_utenti[i].porta != utente) {
            if (count > 0) {
                strcat(send_lista_utenti, ",");
            }
            sprintf(send_porta, "%d", lista_utenti[i].porta);
            strcat(send_lista_utenti, send_porta);
            count++;
        }
    }

    //costruzione del payload completo
    snprintf(payload, MAX_PAYLOAD_LENGTH, "%d-%s", num_utenti, send_lista_utenti);
}

/*======================================================================================================*/
//inizio main


int main(){
    FILE *f = fopen("debug_lavagna.log", "w");

    int sock_lavagna, new_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    int max_fd;
    fd_set read_fds;
    int activity;

    //lunghezza del messaggio ricevuto (usata con codifica_messaggio())
    int message_len;

    //creazione del socket di ascolto della lavagna
    if ((sock_lavagna = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    //configurazione dell'indirizzo
    address.sin_family = AF_INET;
    address.sin_port = htons(port_lavagna);
    address.sin_addr.s_addr = INADDR_ANY;

    //binding dell'indirizzo
    if (bind(sock_lavagna, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Errore nel binding dell'indirizzo");
        close(sock_lavagna);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_lavagna, 20) < 0){
        perror("Errore nella listen");
        close(sock_lavagna);
        exit(EXIT_FAILURE);
    }

    //inizializzazione della lavagna con le 10 card iniziali
    riempi_lavagna();
    
    //stampa della lavagna
    show_lavagna();

    while (1){
        //per il timeout
        struct timeval tv;

        //si controlla se ci sono card attive da troppo tempo e nel caso si manda PING_USER
        time_t current_time = time(NULL);
        int tempo_trascorso;
        for (int i = 0; i < MAX_CARDS; i++){
            tempo_trascorso = difftime(current_time, lavagna[i].ultima_modifica);
            if (lavagna[i].colonna == DOING && ping_sent[i] == 0){
                if (tempo_trascorso > PING_TIMEOUT){
                    fprintf(f, "troppo tempo invio ping\n");
                    fflush(f);
                    ping_sent[i] = 1;
                    for (int j = 0; j < num_utenti; j++){
                        if (lista_utenti[j].porta == lavagna[i].utente){
                            message_len = codifica_messaggio("PING_USER", buffer, BUFFER_SIZE, "");
                            send(lista_utenti[j].sock_utente, buffer, message_len, 0);
                            ping_sent[i] = 1;
                            break;
                        }
                    }
                    
                }
            }
            if (ping_sent[i] == 1 && tempo_trascorso > PING_TIMEOUT + 30){
                fprintf(f, "troppo tempo termino il task\n");
                fflush(f);
                lavagna[i].colonna = TO_DO;
                lavagna[i].ultima_modifica = time(NULL);
                lavagna[i].utente = 0;
                ping_sent[i] = 0;

                //dopo che la card viene reinserita in TO_DO si controlla se puo essere assegnata immediatamente a un altro utente
                handle_card();

                show_lavagna();
            }
        }

        //pulizia dei set di socket
        FD_ZERO(&read_fds);

        //aggiunta dei socket della lavagna al set
        FD_SET(sock_lavagna, &read_fds);
        max_fd = sock_lavagna;

        //aggiunta degli utenti non ancora registrati al set
        for (int i = 0; i < num_attesa; i++) {
            FD_SET(attesa_utenti[i], &read_fds);
            if (attesa_utenti[i] > max_fd){
                max_fd = attesa_utenti[i];
            }
        }

        //aggiunta degli utenti registrati al set
        for (int i = 0; i < num_utenti; i++){
            FD_SET(lista_utenti[i].sock_utente, &read_fds);
            if (lista_utenti[i].sock_utente > max_fd){
                max_fd = lista_utenti[i].sock_utente;
            }
        }


        //imposto il timeout
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        //controllo delle attività sul socket
        activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);

        if ((activity < 0) && (errno != EINTR)){
            perror("Errore nel select");
        }

        //controllo delle nuove connessioni
        if (FD_ISSET(sock_lavagna, &read_fds)){
            if ((new_sock = accept(sock_lavagna, (struct sockaddr*)&address, &addrlen)) < 0) {
                perror("Errore nella accept");
                exit(EXIT_FAILURE);
            }

            fprintf(f, "Nuova connessione stabilita con client (socket fd: %d)\n", new_sock);
            fflush(f);

            //aggiunta del nuovo socket alla lista degli utenti
            if (num_attesa < MAX_UTENTI){
                attesa_utenti[num_attesa] = new_sock;
                num_attesa++;
                fprintf(f, "Aggiunto nuovo client\n");
                fflush(f);
            }
        }

        //controllo di messaggi dagli utenti in attesa
        for (int i = 0; i < num_attesa; i++) {
            if (FD_ISSET(attesa_utenti[i], &read_fds)) {
                int readval = recv(attesa_utenti[i], buffer, BUFFER_SIZE, 0);
                //se l'utente si è disconnesso si chiude il socket e si rimuove l'utente dalla lista
                if (readval == 0) {
                    close(attesa_utenti[i]);
                    attesa_utenti[i] = attesa_utenti[num_attesa - 1];
                    num_attesa--;
                    i--;
                } else {
                    //decodifica del messaggio e controllo sul comando
                    buffer[readval] = '\0';
                    decodifica_messaggio(buffer, BUFFER_SIZE);
                    if (strcmp(rec_command, "HELLO") == 0) {
                        //inserimento dell'utente nella lista dei registrati
                        lista_utenti[num_utenti].sock_utente = attesa_utenti[i];
                        lista_utenti[num_utenti].porta = rec_port;
                        lista_utenti[num_utenti].card_gestita = 0;
                        num_utenti++;

                        //rimozione del socket dalla lista degli utenti non registrati
                        FD_CLR(attesa_utenti[i], &read_fds);
                        attesa_utenti[i] = attesa_utenti[num_attesa - 1];
                        num_attesa--;
                        i--;

                        qsort(lista_utenti, num_utenti, sizeof(struct utente), compare_users);
                
                        fprintf(f, "Utente %d registrato\n", rec_port);
                    
                        //invio del comando HANDLE_CARD per assegnare la card
                        handle_card();
                    }
                }
            }
        }

        //controllo di messaggi da parte di utenti connessi
        for (int i = 0; i < num_utenti; i++){
            if (FD_ISSET(lista_utenti[i].sock_utente, &read_fds)){
                int readval = recv(lista_utenti[i].sock_utente, buffer, BUFFER_SIZE, 0);
                //se l'utente si è disconnesso si chiude la connessione e si rimuove dalla lista
                if (readval == 0){
                    fprintf(f, "Utente porta %d disconnesso\n", lista_utenti[i].porta);
                    fflush(f);

                    //chiusura del socket associato all'utente
                    close(lista_utenti[i].sock_utente);

                    //rimozione dell'utente dall'elenco
                    lista_utenti[i] = lista_utenti[num_utenti - 1];
                    num_utenti--;
                    i--;
                    
                    if (num_utenti > 0) {
                        qsort(lista_utenti, num_utenti, sizeof(struct utente), compare_users);
                    }

                    show_lavagna();
                } else{
                    //decodifica del messaggio ricevuto e controllo sul tipo di comando
                    buffer[readval] = '\0';
                    decodifica_messaggio(buffer, readval);
                    fprintf(f, "ricevuto: %s da %d\n", buffer, rec_port);
                    fflush(f);
                    if (strcmp(rec_command, "CREATE_CARD") == 0){
                        //prelievo del testo della card
                        lavagna[num_cards].id = num_cards + 1;
                        lavagna[num_cards].colonna = TO_DO;
                        strcpy(lavagna[num_cards].attivita, rec_payload);
                        lavagna[num_cards].utente = 0;
                        lavagna[num_cards].ultima_modifica = time(NULL);
                        num_cards++;

                        //se ci sono utenti liberi si assegna subito la card
                        handle_card();

                        show_lavagna();
                    }
                    if (strcmp(rec_command, "QUIT") == 0){
                        fprintf(f, "Utente porta %d disconnesso\n", lista_utenti[i].porta);
                        fflush(f);
                        //riassegnazione della card in TO DO
                        if (lista_utenti[i].card_gestita != 0) {
                            int id_card = lista_utenti[i].card_gestita - 1;
                            lavagna[id_card].colonna = TO_DO;
                            lavagna[id_card].utente = 0;
                            lavagna[id_card].ultima_modifica = time(NULL);
                        }

                        //chiusura del socket associato all'utente
                        close(lista_utenti[i].sock_utente);

                        //rimozione dell'utente dall'elenco
                        lista_utenti[i] = lista_utenti[num_utenti - 1];
                        num_utenti--;
                        i--;
    
                        if (num_utenti > 0) {
                            qsort(lista_utenti, num_utenti, sizeof(struct utente), compare_users);
                        }

                        show_lavagna();
                    }
                    if (strcmp(rec_command, "SHOW_LAVAGNA") == 0){
                        show_lavagna();
                    }
                    if (strcmp(rec_command, "PONG_LAVAGNA") == 0){
                        //resetto il ping per la card associata all'utente
                        ping_sent[lista_utenti[i].card_gestita - 1] = 0;
                        lavagna[lista_utenti[i].card_gestita - 1].ultima_modifica = time(NULL);
                    }
                    if (strcmp(rec_command, "ACK_CARD") == 0){
                        //prelievo dell'id della card accettata dall'utente
                        int rec_id = atoi(rec_payload);
                        
                        //spostamento della card inviata nella colonna DOING
                        move_card(rec_command, rec_id);
                
                        show_lavagna();
                    }
                    if (strcmp(rec_command, "REQUEST_USER_LIST") == 0){
                        //costruzione del payload con la lista degli utenti e invio del messaggio SEND_USER_LIST
                        request_user_list(rec_port);
                        message_len = codifica_messaggio("SEND_USER_LIST", buffer, BUFFER_SIZE, payload);
                        send(lista_utenti[i].sock_utente, buffer, message_len, 0);
                        fprintf(f, "invio: %s\n", buffer);
                        fflush(f);
                    }
                    if (strcmp(rec_command, "CARD_DONE") == 0){
                        //prelievo dell'id della card terminata
                        int rec_id = atoi(rec_payload);

                        //spostamento della card in CARD_DONE
                        move_card(rec_command, rec_id);

                        ping_sent[rec_id - 1] = 0;

                        //assegnazione di una nuova card se disponibile all'utente che ha appena terminato la card
                        handle_card();

                        show_lavagna();
                    }
                }
            }
        }   
    }

    close(sock_lavagna);
    return 0;
}