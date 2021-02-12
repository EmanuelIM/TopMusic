/* cliTcpConc.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
//COPYRIGHT Pagina cursului Computer Networks: https://profs.info.uaic.ro/~computernetworks/ 
//COPYRIGHT TCP/IP Client/Server Concurrent Model & Implementation. https://profs.info.uaic.ro/~gcalancea/laboratories.html (program bazat pe modelul prezent la adresa)
//COPYRIGHT MySQL C API programming tutorial http://zetcode.com/db/mysqlc/ (partile de mysql au fost documentate de aici)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <mysql.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd,opt = 0, done = 0,msg = 3, admin = 0, logged_in = 0, lines,votes;
  struct sockaddr_in server;
  char buf[10],user[250], pass[250];
  char genre[100],name[100], link[500],artist[100],coment[500];
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  /* stabilim portul */
  port = atoi (argv[2]);
  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
      perror ("Eroare la socket().\n");
      return errno;
  }
  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
      perror ("[client]Eroare la connect().\n");
      return errno;
  }
  printf("Bine ai venit in aplicatia TopMusic! Te rugam sa te inregistrezi sau sa te loghezi.\n");
  printf("1. Logheaza-te\n");
  printf("2. Inregistreaza-te\n");
  printf("3. Iesire\n\n");
  while(logged_in != 1)
  {
    printf ("[:Meniu de logare:] Introduceti un numar: ");
    scanf("%d",&opt);
    switch(opt)
    {
        case 1:
            if (write (sd,&opt,sizeof(int)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("Introduceti user-ul:\n");
            read(0,user,sizeof(user));
            strtok(user,"\n");
            printf("Introduceti parola:\n");
            read(0,pass,sizeof(pass));
            strtok(pass,"\n");
            write(sd, user, sizeof(user));
            write(sd, pass, sizeof(pass));
            if (read (sd, &msg,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if(msg == 1)
            {
                printf("Logat ca administrator! Bine ai venit!\n");
                logged_in = 1;
                admin = 1;
            }
            else if(msg == 2)
            {
                printf("Logat ca utilizator normal! Bine ai venit!\n");
                logged_in = 1;
            }
            else if(msg == 3)
            {
                printf("Parola gresita.");
            }
            else
            {
                printf("Utilizator inexistent.");
            }
            break;
         case 2:
            if (write (sd,&opt,sizeof(int)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("[:Inregistrare:] Introduceti user-ul cu care doriti sa va inregistrati:\n");
            read(0,user,sizeof(user));
            strtok(user,"\n");
            printf("Introduceti parola noului cont:\n");
            read(0,pass,sizeof(pass));
            strtok(pass,"\n");
            write(sd, user, sizeof(user));
            write(sd, pass, sizeof(pass));
            if (read (sd, &msg,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if(msg == 1)
            {
                printf("[:Eroare:] Utilizator deja existent. Te rog sa alegi altul.\n");
            }
            else if(msg == 2)
            {
                printf("[:Succes:] Cont creat cu succes :D. Poti incerca sa te loghezi. \n");
            }
            break;
        case 3:
            if (write (sd,&opt,sizeof(int)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            } 
            else
            {
                logged_in = 1;
                done = 1;
                close (sd);
            }
            break;
        default: printf("[:Eroare:] Optiune incorecta! :( Alege un numar din cele prezente pe ecran\n");
    }
  }
  if(done == 1)
  printf("La revedere! \n");
  else
  {
    int afis_din_nou = 1;
    while(logged_in == 1)
    {
        if(afis_din_nou)
        {
            printf("Alege dintre urmatoarele optiuni\n");
            printf("1. Afiseaza topul melodiilor [sortate in functie de numarul de voturi]\n");
            printf("2. Afiseaza topul melodiilor [apartinand doar unui gen specificat]\n");
            printf("3. Adaugati o melodie la top\n");
            printf("4. Afiseaza optiunile din nou.\n");
            printf("5. Iesire\n");
            if(admin) printf("6. Restrictioneaza dreptul de vot al unui utilizator \n");
            if(admin) printf("7. Scoate restrictia unui utilizator de a vota.\n\n\n");
        }
        printf ("[:Meniu principal:] Introduceti un numar: ");
        scanf("%d",&opt);
        switch(opt)
        {
            case 1:
            if (write (sd,&opt,sizeof(int)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            if (read (sd, &lines,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            int song_ids[500], id, votes;
            char strings[10][600], string[600];
            for(int i = 1; i <= lines; ++i)
            {
                if (read (sd, &id, sizeof(int)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                if (read (sd, &votes, sizeof(int)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                song_ids[i] = id;
                if (read (sd, string, sizeof(string)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                strcpy(strings[0],string);
                if (read (sd, string, sizeof(string)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                strcpy(strings[1],string);
                printf("Locul %d. %s - %s. Numar de voturi: %d\n\n",i,strings[1],strings[0],votes);
            }
            afis_din_nou = 0; 
            int viewing_top = 1;
            while(viewing_top)
            {
                int view_option;
                printf("1. Vezi detaliile unei melodii\n");
                printf("2. Voteaza o melodie\n");
                printf("3. Adauga un comentariu la o melodie\n");
                printf("4. Revino la meniul principal\n");
                if(admin) printf("5. Sterge o melodie.\n\n");
                printf("[:Meniu top:] Optiunea aleasa:");
                scanf("%d",&view_option);
                if (write (sd,&view_option,sizeof(int)) <= 0)
                { 
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                switch(view_option)
                {
                    case 1: 
                    printf("[:Informatii melodie:] Scrie indexul unei melodii din top:");
                    int index, votes;
                    scanf("%d",&index);
                    int id_s = song_ids[index];
                    if (write (sd,&id_s,sizeof(int)) <= 0)
                    { 
                        perror ("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    if (read (sd, &index, sizeof(int)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    if (read (sd, &votes, sizeof(int)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    if (read (sd, string, sizeof(string)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    strcpy(name,string);
                    if (read (sd, string, sizeof(string)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    strcpy(genre,string);
                    if (read (sd, string, sizeof(string)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    strcpy(artist,string);
                    if (read (sd, string, sizeof(string)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		            }
                    strcpy(link,string);      
                    printf("--DETALII MELODIE--\n");
                    printf("Numele melodiei: %s\n", name);
                    printf("Artistul: %s\n", artist);
                    printf("Gen muzical: %s\n", genre);
                    printf("Link: %s\n",link);
                    printf("Numar de voturi: %d\n\n", votes);
                    printf("--Comentarii--\n");
                    int no_of_comments;
                    char commentbuf[500],user1[500],comment1[500];
                    if (read (sd, &no_of_comments, sizeof(int)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    for(int i = 1; i <= no_of_comments; ++i)
                    {
                        if (read (sd, user, sizeof(user)) <= 0)
		                {
		                    perror ("[client]Eroare la read() de la server.\n");
                            return errno;
		                }
                        strcpy(user1,user);
                        strtok(user1,"\n");
                        printf("Utilizatorul %s spune: \n",user1);
                        if (read (sd, commentbuf, sizeof(commentbuf)) <= 0)
		                {
		                    perror ("[client]Eroare la read() de la server.\n");
                            return errno;
		                }
                        strcpy(comment1,commentbuf);
                        strtok(comment1,"\n");
                        printf("%s\n",comment1);
                    }
                    viewing_top = 0;
                    break;
                case 2:
                printf("[:Votare:] Scrie indexul unei melodii din topul de mai sus pe care doresti sa o votezi:");
                scanf("%d",&index);
                id_s = song_ids[index];
                if (write (sd,&id_s,sizeof(int)) <= 0)
                { 
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                printf("[:Votare:] Scrie ce vrei sa votezi (1 pentru pozitiv, 2 pentru negativ).\n [:Votare:] Ai grija! Poti vota o singura data pentru o anumita melodie si NU iti poti retrage votul.\n[:Votare:]Optiunea ta?:");
                scanf("%d", &votes);
                if (write (sd,&votes,sizeof(int)) <= 0)
                { 
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                int vote_msg;
                if (read (sd, &vote_msg, sizeof(int)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                if(vote_msg == 1) printf("Imi pare rau. Deja ai votat pentru aceasta melodie!\n\n");
                else if(vote_msg == 2) printf("Eroare la exprimarea votului\n\n");
                else if(vote_msg == 3) printf("Vot exprimat cu succes!\n\n");
                else printf("Un administrator ti-a restrictionat dreptul de votare. Te rugam sa iei legatura cu unul din acestia.");
                break;
            case 3:
                printf("[:Comentariu:] Scrie indexul unei melodii din topul de mai sus:");
                char comment[500];
                scanf("%d",&index);
                id_s = song_ids[index];
                if (write (sd,&id_s,sizeof(int)) <= 0)
                { 
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                printf("[:Comentariu:] Scrie comentariul. Te rugam sa fii politicos!\n");
                read(0,comment,sizeof(comment));
                if (write (sd,comment,sizeof(comment)) <= 0)
                { 
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                int comm_rasp;
                if (read (sd, &comm_rasp, sizeof(int)) <= 0)
		        {
		            perror ("[client]Eroare la read() de la server.\n");
                    return errno;
		        }
                if(comm_rasp == 1) printf("Comentariul tau a fost postat cu succes!\n\n");
                else printf("Eroare la postarea comentariului!\n\n");
                break;
            case 4: viewing_top = 0; break;
            case 5:
                if(admin)
                {
                    int ind,del_mes;
                    printf("[:Stergere melodie:] Scrie indexul melodiei din topul de mai sus care doresti sa fie stearsa:");
                    scanf("%d",&ind);
                    if (write (sd,&ind,sizeof(int)) <= 0)
                    { 
                        perror ("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    if (read (sd, &del_mes, sizeof(int)) <= 0)
		            {
		                perror ("[client]Eroare la read() de la server.\n");
                        return errno;
		            }
                    if(del_mes == 0) printf("Eroare la stergere.\n");
                    else printf("Stergere reusita.\n");

                }
            }
        }
      break;
    case 2:
      printf("[:Top dupa gen:] Scrie genul muzical cautat:");
      scanf("%s",genre);
      if (write (sd,&opt,sizeof(int)) <= 0)
      { 
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }
      if (write (sd,genre,sizeof(genre)) <= 0)
      { 
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }
      if (read (sd, &lines,sizeof(int)) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }
      printf("Se afiseaza melodiile cu genre-ul: %s\n",genre);
      for(int i = 1; i <= lines; ++i)
      {
        if (read (sd, &id, sizeof(int)) <= 0)
		{
		    perror ("[client]Eroare la read() de la server.\n");
            return errno;
		}
        if (read (sd, &votes, sizeof(int)) <= 0)
		{
		    perror ("[client]Eroare la read() de la server.\n");
            return errno;
		}
        song_ids[i] = id;
        if (read (sd, string, sizeof(string)) <= 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        strcpy(strings[0],string);
        if (read (sd, string, sizeof(string)) <= 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        strcpy(strings[1],string);
        printf("Locul %d. %s - %s. Numar de voturi: %d\n\n",i,strings[1],strings[0],votes);
      }
      if(lines == 0) printf("Nu exista melodii care sa aiba acest gen muzical!");
      afis_din_nou = 0; break;
    case 3: 
      if (write (sd,&opt,sizeof(int)) <= 0)
      { 
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
      }
      printf("[:Adaugare melodie:] Scrie, in aceasta ordine, informatiile legate de melodie\n");
      printf("[:Adaugare melodie:] Nume, artist, gen, link\n");
      printf("[:Adaugare melodie:] Te rog sa scrii fiecare informatie de mai sus, urmata de cate un enter. Fii cat mai precis si corect in scrierea informatiilor.\n");
      read(0, name, sizeof(name));
      strtok(name,"\n");
      read(0, artist, sizeof(artist));
      strtok(artist,"\n");
      read(0, genre, sizeof(genre));
      strtok(genre,"\n");
      read(0, link, sizeof(link));
      strtok(link,"\n");
      write(sd, name, sizeof(name));
      write(sd, artist, sizeof(artist));
      write(sd, genre, sizeof(genre));
      write(sd, link, sizeof(link));
      if (read (sd, &msg,sizeof(int)) < 0)
      {
         perror ("[client]Eroare la read() de la server.\n");
         return errno;
      }
      if(msg == 1)
      {
          printf("[:Adaugare melodie:] Melodia ta deja exista!\n");
      }
      else if(msg == 2)
      {
          printf("[:Adaugare melodie:] Melodie adaugata la top cu succes! \n");
      }
      afis_din_nou = 0; break;
      case 4: afis_din_nou = 1; break;
      case 5:  
       if (write (sd,&opt,sizeof(int)) <= 0)
       { 
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
       } 
       logged_in = 0; printf("La revedere! \n"); close(sd); break;
      case 6:
        if (write (sd,&opt,sizeof(int)) <= 0)
        { 
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if(admin)
        {
            afis_din_nou = 0;
            char ban_user[500];
            printf("[:Restrictionare:] Scrie numele utilizatorului pe care doriti sa il restrictionati:");
            scanf("%s",ban_user);
            if (write (sd,ban_user,sizeof(ban_user)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            } 
            int ban_msg;
            if (read (sd, &ban_msg,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if(ban_msg) printf("Daca utilizatorul exista, ar trebui sa fie acum restrictionat\n\n");
            }
        else printf("Comanda necunoscuta\n");
        break;
      case 7:
        if (write (sd,&opt,sizeof(int)) <= 0)
        { 
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if(admin)
        {
            afis_din_nou = 0;
            char ban_user[500];
            printf("[:Anularea restrictionarii:] Scrieti numele utilizatorului caruia doriti sa ii scoateti restrictia:");
            scanf("%s",ban_user);
            if (write (sd,ban_user,sizeof(ban_user)) <= 0)
            { 
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            } 
            int ban_msg;
            if (read (sd, &ban_msg,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if(ban_msg) printf("Daca utilizatorul exista, ar trebui sa nu mai fie restrictionat\n\n");
        }
        else printf("Comanda necunoscuta\n");
        break;
     }
    }
  }
  
}