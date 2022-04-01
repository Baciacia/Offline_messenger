#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/file.h>
#include <pthread.h>

#define MAX 1000

char* inregistrare(void * arg);
char* login(void * arg);
int exist(const char *file);
int exist2(const char *file);
int exist3(const char *file);
void logout(char file[MAX], int socket_nou);
void adauga(char tintas[MAX], int socket_nou, char tinta[MAX]);
void seehistory(char tintas[MAX], int socket_nou, char tinta[MAX]);
int prieteni(char tintas[MAX], char tinta[MAX]);
void Startconv(char tintas[MAX], int socket_nou, char tinta[MAX]);
void see(char username[MAX],int socket_nou);
void quit(int socket_nou);

char convoffline[500];
pthread_mutex_t lck = PTHREAD_MUTEX_INITIALIZER;

struct mesaje{
    char mesaje[300];
    char tinta[100];
    char tintas[100];
}vector[100];


int k = 0;
int degeaba = 0;

typedef struct thData
{
    int id;
    int cl;
    char username[MAX];

}thData;

static void *treat(void *);

int main(void)
{
    FILE *file;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
    char msg[MAX];

    /// facem socketul serverului
    int s_socket;
    s_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(s_socket == -1)
    {
        printf("Eroare la creearea socketului!\n");
        exit(1);
    }
    int cal=1;
    setsockopt(s_socket,SOL_SOCKET,SO_REUSEADDR, &cal, sizeof(cal));
    ///adresa serverului
    struct sockaddr_in adresa_server;
    struct sockaddr_in adr_serv;
    adresa_server.sin_family = AF_INET;
    adresa_server.sin_port = htons(9002);
    adresa_server.sin_addr.s_addr = INADDR_ANY;

    ///legam socketul de adresa IP;
    if(bind(s_socket, (struct sockaddr*) &adresa_server, sizeof(adresa_server)) == -1)
    {
        printf("Eroare la bind!\n");
        exit(1);
    }

    ///ascultam sa vedem daca vin clienti sa se conecteze;
    if(listen(s_socket, 5) == -1)
    {
        printf("Eroare la listen!\n");
        exit(1);
    }

    while(1)
    {
        thData *td;
        int cerere = -10;
        int lungime = sizeof(adr_serv);
        int socket_nou = accept(s_socket, (struct sockaddr*)&adr_serv,(socklen_t *) &lungime);
        if(socket_nou < 0)
        {
            printf("Eroare la accept!\n");
            exit(1);
        }
        printf("Conexiune facuta la server!\n\n");
        bzero(msg, MAX);
        td=(struct thData*)malloc(sizeof(struct thData));	
	    td->id = i++;
        td->cl = socket_nou;
        strcpy(td->username,""); 
	    pthread_create(&th[i], NULL, &treat, td);
    }
    return 0;
}

static void *treat(void * arg)
{

    pthread_mutex_lock(&lck);
    pthread_detach(pthread_self());
    pthread_mutex_unlock(&lck);
    FILE *file;
    int cerere = -10;
    char msg[MAX];
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    comenzi:
        read(tdL.cl, msg, sizeof(msg)); 
        if(strncmp("/login", msg, 6) == 0)
        {
            cerere = 0;
        }
        else 
        {   
            if(strncmp("/register", msg, 9) == 0)
            {
                cerere = 1;
            }
            else
            {   
                if(strcmp("/quit\n",msg) == 0)
                {
                    cerere = 2;
                }
                else
                {
                    printf("errrrroare\n");
                    strcpy(msg,"\nPoti folosi doar 3 comenzi: /login, /quit, /register !\n\n");
                    write(tdL.cl, msg, sizeof(msg));
                    goto comenzi;
                }
            }
        }
        if(cerere == 2)
        {
            quit(tdL.cl);
        }
        int logat = 0;
        if(cerere == 1)
        {
            strcpy(tdL.username, inregistrare(&tdL));
            logat = 1;
        }
        else
            {
                strcpy(tdL.username, login(&tdL));   
                logat = 1;
            }

        ///SUNTEM LOGATI;
        if(logat)
        {
            int comenzi = 1;
            while(comenzi)
            {
                int com;
                read(tdL.cl,msg,sizeof(msg));
                if(strncmp("/help",msg,5) == 0)
                    com = 1;
                if(strncmp("/vorbeste",msg,9) == 0)
                    com = 2;
                if(strncmp("/seehistory",msg,11) == 0)
                    com = 3;
                if(strncmp("/adauga",msg,7) == 0)
                    com = 4;
                if(strncmp("/logout",msg,7) == 0)
                    com = 5;
                if(strcmp("/see\n",msg) == 0)
                    com = 6;
                switch(com)
                {
                    case 1:
                    strcpy(msg,"Acestra sunt comenzile disponibile :/see, /adauga, /vorbeste, /seehistory, /logout\n\n");
                    write(tdL.cl,msg,sizeof(msg));
                    break;

                    case 2:
                    strcpy(msg,"Cu ce utilizator doriti sa vorbiti?\n\n");
                    write(tdL.cl,msg,sizeof(msg));
                    bzero(msg,sizeof(msg));
                    read(tdL.cl,msg,sizeof(msg));
                    msg[strlen(msg)-1] = 0;
                    printf("%s\n%s\nStartconv\n",tdL.username,msg);
                    Startconv(tdL.username,tdL.cl,msg);
                    break;

                    case 3:
                    strcpy(msg,"Cu ce utilizator doriti sa vedeti conversatia?\n\n");
                    write(tdL.cl,msg,sizeof(msg));
                    bzero(msg,sizeof(msg));
                    read(tdL.cl,msg,sizeof(msg));
                    msg[strlen(msg)-1] = 0;
                    printf("%s\n%s\nseehistory\n",tdL.username,msg);
                    seehistory(tdL.username,tdL.cl,msg);
                    break;

                    case 4:
                    strcpy(msg,"Pe cine doriti sa adaugati ca prieten?\n\n");
                    write(tdL.cl,msg,sizeof(msg));
                    bzero(msg,sizeof(msg));
                    read(tdL.cl,msg,sizeof(msg));
                    msg[strlen(msg)-1] = 0;
                    printf("%s\n%s\nadauga\n",tdL.username,msg);
                    adauga(tdL.username,tdL.cl,msg);
                    break;

                    case 5:
                    printf("%s\nlogout\n",tdL.username);
                    logout(tdL.username,tdL.cl);
                    comenzi = 0;
                    goto comenzi;

                    case 6:
                    printf("%s\n%s\nsee\n",tdL.username,msg);
                    see(tdL.username,tdL.cl);
                    break;

                    default:
                    strcpy(msg,"Comanda necunoscuta! Pentru ajutor folositi comanda /help!\n\n");
                    write(tdL.cl,msg,sizeof(msg));
                    break;

                }    
            }
        }
        else;
        close ((intptr_t)arg);
        close(tdL.cl);
        pthread_exit(0);
        return(NULL);
    }

char* inregistrare(void * arg)
{
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    int socket_nou = tdL.cl;
    char parola1[MAX], parola2[MAX];
    bzero(parola1, sizeof(parola1));
    bzero(parola2, sizeof(parola2));
    int inr = -1,confparola = 1;
    char msg[MAX];

    strcpy(msg, " Alegeti un username!\n\n");
    write(socket_nou, msg, sizeof(msg));

    a:
    read(socket_nou, msg, sizeof(msg));
    msg[strlen(msg)-1] = 0;
    if(exist(msg))
    {
        strcpy(msg," Username folosit deja!\n\n");
        write(socket_nou, msg, sizeof(msg));
        goto a;
    }

    else
    {

        strcpy(tdL.username,msg);
        char fisier[31];
        strcpy(fisier, msg);
        strcpy(msg," Alegeti o parola!\n\n");
        write(socket_nou, msg, sizeof(msg));
        par:
        read(socket_nou, parola1, sizeof(parola1));
        strcpy(msg, " Confirmati parola introducand-o din nou!\n\n");
        write(socket_nou, msg, sizeof(msg));
        read(socket_nou, parola2, sizeof(parola2));
        char folder[MAX];
        if(strcmp(parola1,parola2) == 0)  
        {          
            bzero(folder, MAX);
            strcpy(folder,"USERS/");
            strcat(folder, fisier);
            FILE *file1 = fopen(folder, "w");
            fprintf(file1,"%s", parola1);
            fclose(file1);
            inr = 0;
        }
    
    }
    if(inr == -1)
    {
        strcpy(msg, " Inregistrare nereusita!\nScrieti o noua parola!\n");
        write(socket_nou, msg, sizeof(msg));
        goto par;
    }
    else    
    {
        char fold[MAX];
        bzero(fold, MAX);
        strcpy(fold,"online/");
        strcat(fold, tdL.username);
        FILE *file1 = fopen(fold, "w");
        strcpy(msg, " Inregistrare reusita!\nAti fost conectat automat la server!\n\n");
        write(socket_nou, msg, sizeof(msg));
    }
     
    char *ptr = (char *) malloc(MAX);
    strcpy(ptr, tdL.username);
    return ptr;
}

char* login(void * arg)
{
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    int socket_nou = tdL.cl;
    char msg[MAX];
    char parola[21];
    strcpy(msg, " Scrieti username-ul!\n");
    write(socket_nou, msg, sizeof(msg));
    c:
    read(socket_nou, msg, sizeof(msg));
    msg[strlen(msg)-1] = 0;
    char fisier[31];
    char folder[MAX];
    char aux[MAX];
    strcpy(fisier,msg);
    bzero(folder, MAX);
    strcpy(folder,"USERS/");
    strcat(folder,fisier);
    if(exist(fisier))
    {
        bzero(aux, sizeof(aux));
        strcpy(aux, msg);
        strcpy(msg," Introduceti parola!\n");
        write(socket_nou, msg, sizeof(msg));
        int fd = open(folder, O_RDONLY);
        FILE *file1 = fopen(folder, "r");
        fread(parola,sizeof(parola),1,file1);
        fclose(file1);
        parola_gresita:
        read(socket_nou, msg, sizeof(msg));
        if(strcmp(parola, msg) == 0)
        {
            if(exist3(aux))
            {
                strcpy(msg,"Cineva este deja conectat!\nScrieti un alt username!\n\n");
                write(socket_nou, msg, sizeof(msg));
                goto c; 
            }
            else
            {
                char fold[MAX];
                bzero(fold, MAX);
                strcpy(fold,"online/");
                strcat(fold, aux);
                FILE *file1 = fopen(fold, "w");
                strcpy(tdL.username,aux);
                strcpy(msg,"Logare reusita!\n/see - verificare mesaje!\n/help - pentru informatii!\n\n");
                write(socket_nou, msg, sizeof(msg));

                int verif = 1;
                while(verif)
                {
                    read(socket_nou,msg,sizeof(msg));
                    if(strncmp(msg,"/see", 4) == 0)
                    {
                        see(tdL.username,socket_nou);
                        verif = 0;
                    }
                    else
                    {
                        strcpy(msg,"Va rog verificati-va mesajele!\n\n");
                        write(socket_nou,msg,sizeof(msg));
                    }
                }
            }
        }
        else
        {
            strcpy(msg,"Logare nereusita!\nScrieti parola din nou!\n\n");
            write(socket_nou, msg, sizeof(msg));
            goto parola_gresita;
        }
         
    }
    else
    {
        strcpy(msg,"Username neexistent!\nScrieti un username nou!\n");
        write(socket_nou, msg, sizeof(msg));
        goto c;
    }

    char *ptr = (char *) malloc(MAX);
    strcpy(ptr, tdL.username);
    return ptr;
}

int exist(const char *file)
{
    DIR *d;
    int gasit = 0;
    struct dirent *dir;
    d = opendir("USERS/");
    if (d) 
        while ((dir = readdir(d)) != NULL) 
        {
            if(strcmp(dir->d_name,file) == 0)
                 gasit = 1;
        }
    else gasit = 13444;
    
    closedir(d);
    return gasit;
}

int exist2(const char *file)
{
    DIR *d;
    int gasit = 0;
    struct dirent *dir;
    d = opendir("Conversatii/");
    if (d) 
        while ((dir = readdir(d)) != NULL) 
        {
            if(strcmp(dir->d_name,file) == 0)
                 gasit = 1;
        }
    else gasit = 13444;
    
    closedir(d);
    return gasit;
}

int exist3(const char *file)
{
    DIR *d;
    int gasit = 0;
    struct dirent *dir;
    d = opendir("online/");
    if (d) 
        while ((dir = readdir(d)) != NULL) 
        {
            if(strcmp(dir->d_name,file) == 0)
                 gasit = 1;
        }
    else gasit = 13444;
    
    closedir(d);
    return gasit;
}

int prieteni(char tintas[MAX], char tinta[MAX])
{
    DIR *d;
    char file[MAX];
    bzero(file,sizeof(file));
    int gasit = 0;
    struct dirent *dir;
    d = opendir("Conversatii/");
    ///printf("%s_%s", tintas, tinta);
    if(strcmp(tintas,tinta) < 0)
    {
        bzero(file,sizeof(file));
        strcat(file,tintas);
        strcat(file, "_");
        strcat(file, tinta);
    }
    else
    {   
        bzero(file,sizeof(file));
        strcat(file,tinta);
        strcat(file, "_");
        strcat(file, tintas);
    }
    if (d) 
    {
        while ((dir = readdir(d)) != NULL) 
        {
            if(strcmp(dir->d_name,file) == 0)
                 gasit = 1;
        }
    }
    else gasit = 0;

    closedir(d);
    return gasit;

}

void logout(char file[MAX],int socket_nou)
{
    char msg[MAX];
    char folder[MAX];
    strcpy(folder,"online/");
    strcat(folder,file);
    int fd = open(folder, O_RDONLY);
    if(fd)
    {
        int rem = remove(folder);
        if(rem == 0)
        {
            strcpy(msg,"Te-ai deconectat de la messenger!\n Pentru a va conecta folositi comanda /login!\n\n");
        }
        else 
        {
            strcpy(msg,"nu este conectat!\n");
        }
    }       
    else 
    {
        strcpy(msg,"fisier negasit!!!!\n");
    }

    write(socket_nou,msg,sizeof(msg));
}

void adauga(char tintas[MAX], int socket_nou, char tinta[MAX])
{
    char msg[MAX];
    if(exist(tinta) == 0)
    {
        strcpy(msg,"Nu exista acest utilizator!\nFolositi din nou comanda de adaugare sau oricare alta comanda!\n\n");
        write(socket_nou,msg,sizeof(msg));
        return;
    }
    char file[150];
    bzero(file,sizeof(file));
    char folder[256];
    strcpy(folder,"Conversatii/");
    if(strcmp(tintas,tinta) < 0)
    {
        bzero(file,sizeof(file));
        strcat(file,tintas);
        strcat(file, "_");
        strcat(file, tinta);
    }
    else
    {
        bzero(file,sizeof(file));
        strcat(file,tinta);
        strcat(file, "_");
        strcat(file, tintas);
    }
    if(exist2(file) == 0)
    {
        strcat(folder,file);
        FILE *conv = fopen(folder,"w");
        int fd = open(folder, O_RDONLY);
        if(conv == NULL)
        {
            printf("aici e erroarea!\n");
        }
        fprintf(conv,"%s", "Acum sunteti prieteni!\n\n");
        flock(fd,LOCK_EX);
        fclose(conv);
        strcpy(msg,"L-ai adaugat cu succes ca prieten!\n\n");
        write(socket_nou,msg,sizeof(msg));
    }
    else
    {
        strcpy(msg,"Deja sunteti prieteni!\n\n");
        write(socket_nou,msg,sizeof(msg));
    }
    
}

void seehistory(char tintas[MAX], int socket_nou, char tinta[MAX]) 
{
    char msg[MAX];
    if(exist(tinta) == 0)
    {
        strcpy(msg,"Nu exista acest utilizator!\n\n");
        write(socket_nou,msg,sizeof(msg));
        return;
    }

    if(prieteni(tintas,tinta) || prieteni(tinta, tintas))
    {
        char folder[256];
        char file[150];
        strcpy(folder,"Conversatii/");
        if(strcmp(tintas,tinta) < 0)
        {
            strcat(file,tintas);
            strcat(file, "_");
            strcat(file, tinta);
        }
        else
        {
            strcat(file,tinta);
            strcat(file, "_");
            strcat(file, tintas);
        }
        strcat(folder,file);
        FILE *conv;
        char msg[1000];
        char citire[100];      
        conv = fopen(folder,"r"); 
        bzero(msg, sizeof(msg));
        strcat(msg, "\n");
        while(fgets(citire,1000,conv) != NULL)    //citeste pana la enter; nasoll
        {
            strcat(msg,citire);
            strcat(msg, "\n");
        }
        write(socket_nou,msg, sizeof(msg));
        fclose(conv);
    }  
    else printf("nu s prieteni!\n\n");
}

void Startconv(char tintas[MAX], int socket_nou, char tinta[MAX])
{
    printf("startconv se deschide!\n\n");
    char msg[MAX];
    if(exist(tinta) == 0)
    {
        strcpy(msg,"Nu exista acest utilizator!\nFolositi o noua comanda!\n\n");
        write(socket_nou,msg,sizeof(msg));
        return;
    }
    if(prieteni(tintas,tinta) || prieteni(tinta,tintas))
    {
        printf("merge\n\n");
        char folder[256];
        char file[150];
        bzero(file, sizeof(file));
        bzero(folder, sizeof(folder));
        strcpy(folder,"Conversatii/");
        if(strcmp(tintas,tinta) < 0)
        {
            strcat(file,tintas);
            strcat(file, "_");
            strcat(file, tinta);
        }
        else
        {
            strcat(file,tinta);
            strcat(file, "_");
            strcat(file, tintas);
        }
        
        strcat(folder,file);      
        FILE *conv;
        conv = fopen(folder,"a");
        if(conv == NULL)
        {
            printf("aici e erroarea bossule!\n");
        }
        strcpy(msg,"Puteti incepe conversatia!\n\n");
        write(socket_nou,msg,sizeof(msg));
        while(1)
        {
            msgvb:
            bzero(msg,sizeof(msg));
            read(socket_nou,msg,sizeof(msg));                         
            if(strcmp("/exit\n", msg) == 0)
            {
                printf("ati iesit!!\n\n");
                strcpy(msg,"Conversatie terminata!\n Puteti folosi alte comenzi!\n\n");
                write(socket_nou,msg,sizeof(msg));
                goto iesire;
            }
            if(strcmp("/see\n", msg) == 0)
            {
                printf("ati dat sa vdeti msg!!\n\n");
                see(tintas, socket_nou);
                goto msgvb;
            }
            pthread_mutex_lock(&lck);
            strcat(vector[k].mesaje, msg);
            strcat(vector[k].tinta, tinta);
            strcat(vector[k].tintas, tintas);
            k++;
            pthread_mutex_unlock(&lck);

            fprintf(conv,"[%s] : ",tintas);                            
            fprintf(conv,"%s", msg);                       
            strcpy(msg,"Mesaj trimis cu succes!\n\n");
            write(socket_nou,msg,sizeof(msg));
        }
        iesire:
        fclose(conv);
        
        
    }
}

void see(char username[100],int socket_nou)
{
    char msg[MAX];
    int gasit = 0;
    for(int i = 0; i < k; i++)
    {
        if(strcmp(username,vector[i].tinta) == 0)
        {
            
            strcpy(msg,"Mesaj de la ");
            strcat(msg,"[");
            strcat(msg,vector[i].tintas);
            strcat(msg,"] : ");
            strcat(msg,vector[i].mesaje);
            
            write(socket_nou, msg, sizeof(msg));
            gasit = 1;
        }
    }
    if(gasit == 0)
    {
        strcpy(msg,"Nu aveti niciun mesaj primit!\n\n");
        write(socket_nou, msg, sizeof(msg));
    }
}

void quit(int socket_nou)
{
    close(socket_nou);
}

/*  

*/