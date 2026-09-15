#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

#define port_lavagna 5678

#define BUFFER_SIZE 1024
#define MAX_UTENTI 20

#define MAX_COMMAND_LENGTH 64       //lunghezza massima del comando
#define MAX_PAYLOAD_LENGTH 512      //lunghezza massima del payload

int port_utente;        //porta dell'utennte, inserita al momento dell'esecuzione
int user_state = 0;        //stato di attività dell'utente: 0 se in attesa o in gestione di una card, 1 se ha terminato il task (dopo la sleep)

char buffer[BUFFER_SIZE];

char payload[BUFFER_SIZE];

//campi dei messaggi ricevuti
char rec_command[MAX_COMMAND_LENGTH] = {0};
int rec_port = 0;
char rec_payload[MAX_PAYLOAD_LENGTH] = {0};

#define MAX_TEXT_LENGTH 128
//card in gestione dall'utente
struct card{
    int id;
    char attivita[MAX_TEXT_LENGTH];     //testo dell'attività
} card_gestita;

//lista degli utenti connessi alla lavagna (a cui inviare la revisione) e contatore
int lista_utenti[MAX_UTENTI] = {0};
int num_utenti = 0;

//lista dei socket degli utenti da cui si attende approvazione e contatore
int lista_socket[MAX_UTENTI] = {0};
int num_socket = 0;
//contatore delle approvazioni ricevute
int approvazioni_ricevute;

//lista degli utenti a cui mandare l'approvazione (che hanno richiesto la revisione)
int approvazione_utenti[MAX_UTENTI] = {0};
int num_approvazione = 0;

//funzione per la codifica del messaggio
/*----formato messaggio: "COMANDO|PORTA_SORGENTE|PAYLOAD" (il payload è variabile in base al comando da inviare, può essere vuoto)----*/
int codifica_messaggio(char *comando, char *buffer, int buffer_size, char *payload){
    //costruisco la stringa da inviare
    int len = snprintf(buffer, BUFFER_SIZE, "%s|%d|%s\n", comando, port_utente, payload);
    return len;
}

//funzione per la decodifica dei messaggi
void decodifica_messaggio (char *messaggio, int lunghezza){
    sscanf(messaggio, "%99[^|]|%d|%99[^\n]", rec_command, &rec_port, rec_payload);
}

/*=====================================================================================*/
//inizio main

int main(int argc, char *argv[]){
    int sock_lavagna, sock_utente, new_sock;
    struct sockaddr_in address_lavagna, address_utente;
    socklen_t addrlen_utente = sizeof(address_utente);
    char *ip_addr = "127.0.0.1";

    int max_fd;
    fd_set read_fds;
    int activity;

    //lunghezza del messaggio ricevuto (usata con codifica_messaggio())
    int message_len;

    //memorizzazione della porta utente
    if (argc == 2){
        int val = atoi(argv[1]);
        if (val > port_lavagna){
            port_utente = val;
        } else{
            fprintf(stderr, "Porta non valida\n");
            exit(EXIT_FAILURE);
        }
    } else{
        fprintf(stderr, "Porta non specificata\n");
        exit(EXIT_FAILURE);
    }

    //creazione del socket
    if ((sock_lavagna = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    //configurazione dell'indirizzo
    address_lavagna.sin_family = AF_INET;
    address_lavagna.sin_port = htons(port_lavagna);
    if (inet_pton(AF_INET, ip_addr, &address_lavagna.sin_addr) <= 0){
        perror("Errore nella conversione dell'IP");
        close(sock_lavagna);
        exit(EXIT_FAILURE);
    }

    //connessione
    if (connect(sock_lavagna, (struct sockaddr*)&address_lavagna, sizeof(address_lavagna)) < 0){
        perror("Errore nella connessione");
        close(sock_lavagna);
        exit(EXIT_FAILURE);
    }

    printf("Utente connesso alla lavagna\n");

    //invio della richiesta di registrazione
    message_len = codifica_messaggio("HELLO", buffer, BUFFER_SIZE, "");
    send(sock_lavagna, buffer, message_len, 0);
    printf("Registrazione alla lavagna effettuata\n");

    //creazione del socket di ascolto dell'utente
    if ((sock_utente = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    //configurazione dell'indirizzo
    address_lavagna.sin_family = AF_INET;
    address_lavagna.sin_port = htons(port_utente);
    address_lavagna.sin_addr.s_addr = INADDR_ANY;

    //binding dell'indirizzo
    if (bind(sock_utente, (struct sockaddr*)&address_lavagna, sizeof(address_lavagna)) < 0){
        perror("Errore nel binding dell'indirizzo");
        close(sock_utente);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_utente, 20) < 0){
        perror("Errore nella listen");
        close(sock_utente);
        exit(EXIT_FAILURE);
    }

    printf("Utente pronto per l'ascolto\n");

    while (1){
        //pulizia del set di socket
        FD_ZERO(&read_fds);
        
        //aggiunta dell'input da linea di comando
        FD_SET(STDIN_FILENO, &read_fds);
        max_fd = STDIN_FILENO;
        
        //aggiunta del socket di ascolto per gli altri utenti
        FD_SET(sock_utente, &read_fds);
        if (sock_utente > max_fd){
            max_fd = sock_utente;
        }
        
        //aggiunta dei socket per comuinicare con la lavagna
        FD_SET(sock_lavagna, &read_fds);
        if (sock_lavagna > max_fd){
            max_fd = sock_lavagna;
        }

        //aggiunta dei socket degli utenti a da cui ricevere l'approvazione
        for (int i = 0; i < num_socket; i++) {
            FD_SET(lista_socket[i], &read_fds);
            if (lista_socket[i] > max_fd) {
                max_fd = lista_socket[i];
            }
        }

        //aggiunta dei socket degli utenti a cui inviare l'approvazionne
        for (int i = 0; i < num_approvazione; i++) {
            FD_SET(approvazione_utenti[i], &read_fds);
            if (approvazione_utenti[i] > max_fd){
                max_fd = approvazione_utenti[i];
            }
        }

        //controllo delle attività sul socket 
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)){
            perror("Errore nel select");
        }

        //controllo di messaggi da linea di comando
        if (FD_ISSET(STDIN_FILENO, &read_fds)){
            //lettura del comando
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                buffer[strcspn(buffer, "\n")] = 0;

                if (strcmp(buffer, "QUIT") == 0){
                    message_len = codifica_messaggio("QUIT", buffer, BUFFER_SIZE, "");
                    send(sock_lavagna, buffer, message_len, 0);
                }
                if (strcmp(buffer, "SHOW_LAVAGNA") == 0){
                    message_len = codifica_messaggio("SHOW_LAVAGNA", buffer, BUFFER_SIZE, "");
                    send(sock_lavagna, buffer, message_len, 0);
                } 
                if (strcmp(buffer, "CREATE_CARD") == 0){
                    //inserimento del testo della card da creare e invio del messaggio
                    printf("Inserire il testo attività della nuova card:\n");
                    fgets(payload, sizeof(payload), stdin);
                    message_len = codifica_messaggio("CREATE_CARD", buffer, BUFFER_SIZE, payload);
                    send(sock_lavagna, buffer, message_len, 0);
                }
                if (strcmp(buffer, "REQUEST_USER_LIST") == 0){
                    message_len = codifica_messaggio("REQUEST_USER_LIST", buffer, BUFFER_SIZE, "");
                    send(sock_lavagna, buffer, message_len, 0);
                }
            }
        }
        
        //controllo di messaggi ricevuti dalla lavagna
        if (FD_ISSET(sock_lavagna, &read_fds)){
            int readval = recv(sock_lavagna, buffer, BUFFER_SIZE, 0);
            if (readval == 0){
                //chiusura della connessione
                close(sock_lavagna);
                exit(EXIT_FAILURE);
            } else{
                buffer[readval] = '\0';
                decodifica_messaggio(buffer, readval);
                printf("ricevuto: %s", buffer);

                if (strcmp(rec_command, "PING_USER") == 0){
                    //invio del messaggio PONG_LAVAGNA
                    message_len = codifica_messaggio("PONG_LAVAGNA", buffer, BUFFER_SIZE, "");
                    printf("invio di PONG alla lavagna\n");
                    send(sock_lavagna, buffer, message_len, 0);
                }
                if (strcmp(rec_command, "HANDLE_CARD") == 0) {
                    //variabili per salvare le informazioni nel payload
                    int rec_id;
                    char rec_attivita[MAX_TEXT_LENGTH];
                    int rec_num_utenti;
                    char rec_lista_utenti[MAX_PAYLOAD_LENGTH];

                    sscanf(rec_payload, "%d-%[^-]-%d-%s", &rec_id, rec_attivita, &rec_num_utenti, rec_lista_utenti);

                    card_gestita.id = rec_id;
                    strcpy(card_gestita.attivita, rec_attivita);

                    char* ptr = rec_lista_utenti;
                    num_utenti = rec_num_utenti - 1;
                    int offset;

                    //decodifica della lista ricevuta e aggiornamento della lista degli utenti
                    for (int j = 0; j < num_utenti; j++){
                        sscanf(ptr, "%d%n", &lista_utenti[j], &offset);
                        ptr += offset;

                        if (*ptr == ','){
                            ptr++;
                        }
                    }

                    //invio di ACK_CARD alla lavagna
                    sprintf(payload, "%d", rec_id);
                    message_len = codifica_messaggio("ACK_CARD", buffer, BUFFER_SIZE, payload);
                    buffer[message_len] = '\0';
                    send(sock_lavagna, buffer, message_len, 0);
                    printf("invio di ACK_CARD alla lavagna\n");

                    //simulazione dello svolgimento del task
                    user_state = 1;
                    approvazioni_ricevute = 0;
                    sleep(20);
                    
                    printf("task terminato, invio richieste di approvazione\n");

                    //richiesta della lista di utenti connessi
                    message_len = codifica_messaggio("REQUEST_USER_LIST", buffer, BUFFER_SIZE, "");
                    send(sock_lavagna, buffer, message_len, 0);
                    printf("invio di REQUEST_USER_LIST alla lavagna\n");
                }
                if (strcmp(rec_command, "SEND_USER_LIST") == 0){
                    int rec_num_utenti;
                    char rec_lista_utenti[BUFFER_SIZE] = {0};
                    sscanf(rec_payload, "%d-%s", &rec_num_utenti, rec_lista_utenti);

                    char* ptr = rec_lista_utenti;
                    num_utenti = rec_num_utenti - 1;
                    int offset;

                    //decodifica della lista ricevuta e aggiornamento della lista degli utenti
                    for (int j = 0; j < num_utenti; j++){
                        sscanf(ptr, "%d%n", &lista_utenti[j], &offset);
                        ptr += offset;

                        if (*ptr == ','){
                            ptr++;
                        }
                    }

                    //se l'utente ha terminato il task si stabilisce la connessione con gli utenti attivi e si inviano le richieste di revisione
                    if (user_state == 1 && num_utenti != 0){
                        num_socket = 0;
                        for (int j = 0; j < num_utenti; j++){
                            int port_revisione = lista_utenti[j];
                            int sock_revisione;
                            struct sockaddr_in address_revisione;

                            //creazione del socket
                            if ((sock_revisione = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                                perror("Errore nella creazione del socket");
                                exit(EXIT_FAILURE);
                            }

                            memset(&address_revisione, 0, sizeof(address_revisione));

                            //configurazione dell'indirizzo
                            address_revisione.sin_family = AF_INET;
                            address_revisione.sin_port = htons(port_revisione);
                            if (inet_pton(AF_INET, ip_addr, &address_revisione.sin_addr) <= 0){
                                perror("Errore nella conversione dell'IP");
                                close(sock_revisione);
                                exit(EXIT_FAILURE);
                            }

                            //connessione
                            if (connect(sock_revisione, (struct sockaddr*)&address_revisione, sizeof(address_revisione)) < 0){
                                perror("Errore nella connessione");
                                close(sock_revisione);
                                exit(EXIT_FAILURE);
                            }
                            printf("Conessione per la review effettuata\n");

                            //aggiunta del socket del nuovo utente alla lista 
                            lista_socket[num_socket] = sock_revisione;
                            num_socket++;

                            //invio della revisione con REVIEW_CARD
                            snprintf(payload, BUFFER_SIZE, "%d", 0);        //per le richieste di revisione si inserisce 0 come payload
                            message_len = codifica_messaggio("REVIEW_CARD", buffer, BUFFER_SIZE, payload);
                            printf("invio della revisione agli utenti\n");
                            send(sock_revisione, buffer, message_len, 0);
                        }
                    }
                    
                    //se il task e terminato ma non ci sono utenti si richiede nuovamente la lista degli utenti connessi
                    if (user_state == 1 && num_utenti == 0){
                        sleep(5);
                        message_len = codifica_messaggio("REQUEST_USER_LIST", buffer, BUFFER_SIZE, "");
                        send(sock_lavagna, buffer, message_len, 0);
                        printf("nuovo invio di REQUEST_USER_LIST alla lavagna");      
                    }
                }
            }
        }

        //controllo di connessioni da altri utenti
        if (FD_ISSET(sock_utente, &read_fds)){
            new_sock = accept(sock_utente, (struct sockaddr*)&address_utente, &addrlen_utente);
            if (new_sock < 0) {
                perror("Errore nella accept utente");
                continue;
            }

            //aggiunta del nuovo socket alla lista degli utenti a cui inviare l'approvazione
            approvazione_utenti[num_approvazione] = new_sock;
            num_approvazione++;
        }

        //controllo di messaggi dagli utenti da cui si attende l'approvazione
        for (int i = 0; i < num_socket; i++) {
            if (FD_ISSET(lista_socket[i], &read_fds)) {
                int readval = recv(lista_socket[i], buffer, BUFFER_SIZE, 0);
                //se l'utente si è disconnesso si rimuove dall'elenco delle approvazioni attese
                if (readval == 0) {
                    close(lista_socket[i]);
                    lista_socket[i] = lista_socket[num_socket - 1];
                    num_socket--;
                    i--;

                    num_utenti--;

                    //se alla disconnessione dell'utente tutte le altre approvazioni erano ricevute si termina la card inviando CARD_DONE 
                    if (approvazioni_ricevute == num_utenti){
                        sprintf(payload, "%d", card_gestita.id);
                        message_len = codifica_messaggio("CARD_DONE", buffer, BUFFER_SIZE, payload);
                        send(sock_lavagna, buffer, message_len, 0);
                        printf("invio di CARD_DONE alla lavagna\n");

                        //reset delle variabili
                        num_socket = 0;
                        approvazioni_ricevute = 0;
                        user_state = 0;
                        card_gestita.id = 0;
                        strcpy(card_gestita.attivita, "");
                    }
                } else {
                    buffer[readval] = '\0';
                    decodifica_messaggio(buffer, BUFFER_SIZE);
                    if (strcmp(rec_command, "REVIEW_CARD") == 0) {
                        //se REVIEW_CARD è un messaggio di approvazione (payload = 1) l'utente invia CARD_DONE alla lavagna e si disconnette dagli altri utenti
                        int review_type = atoi(rec_payload);
                        if (review_type == 1){
                            //registrazione del giudizio positivo e chiusura della connessione relativa
                            approvazioni_ricevute++;
                            close(lista_socket[i]);

                            lista_socket[i] = lista_socket[num_socket - 1];
                            num_socket--;
                            i--;
                            printf("ricevuta una approvazione\n");

                            //se tutti i giudizi sono stati ricevuti si comunica alla lavagna la terminazione del task con CARD_DONE
                            if (approvazioni_ricevute == num_utenti){
                                sprintf(payload, "%d", card_gestita.id);
                                message_len = codifica_messaggio("CARD_DONE", buffer, BUFFER_SIZE, payload);
                                send(sock_lavagna, buffer, message_len, 0);
                                printf("invio di CARD_DONE alla lavagna\n");

                                num_socket = 0;
                                approvazioni_ricevute = 0;
                                user_state = 0;
                                card_gestita.id = 0;
                                strcpy(card_gestita.attivita, "");
                            }
                        }   
                    }
                }
            }
        }

        //controllo di messaggi dagli utenti che vogliono inviare revisioni
        for (int i = 0; i < num_approvazione; i++) {
            //se l'entrata è vuota vado alla prossima
            if (approvazione_utenti[i] == 0){
                continue;
            }
            if (FD_ISSET(approvazione_utenti[i], &read_fds)) {
                int readval = recv(approvazione_utenti[i], buffer, BUFFER_SIZE, 0);
                if (readval == 0) {
                    close(approvazione_utenti[i]);
                    approvazione_utenti[i] = approvazione_utenti[num_approvazione - 1];
                    num_approvazione--;
                    i--;
                } else {
                    buffer[readval] = '\0';
                    decodifica_messaggio(buffer, BUFFER_SIZE);
                    if (strcmp(rec_command, "REVIEW_CARD") == 0) {
                        //se REVIEW_CARD è un messaggio di revisione (payload = 0) l'utente risponde con un REVIEW_CARD di approvazionne (payload 1)
                        int review_type = atoi(rec_payload);
                        if (review_type == 0){
                            snprintf(payload, BUFFER_SIZE, "%d", 1);
                            message_len = codifica_messaggio("REVIEW_CARD", buffer, BUFFER_SIZE, payload);
                            send(approvazione_utenti[i], buffer, message_len, 0);
                            printf("inviata una approvazione\n");
                            close(approvazione_utenti[i]);

                            approvazione_utenti[i] = approvazione_utenti[num_approvazione - 1];
                            num_approvazione--;
                            i--;
                        }  
                    }
                }
            }
        }    
    }

    close(sock_lavagna);
    close(sock_utente);
    return 0;
}