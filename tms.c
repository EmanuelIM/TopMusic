 /* servTcpConc.c - Exemplu de server TCP concurent
    Asteapta un nume de la clienti; intoarce clientului sirul
    "Hello nume".
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
#include <string.h>
#include <mysql.h>
#include <stdlib.h>

/* portul folosit */
#define PORT 2021

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void finish_with_error(MYSQL *con)
{
    printf("%s\n",mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    int sd,votes,posneg,id_piesa,admin = 0,index;	//descriptorul de socket, alte variabile care sunt declarate pentru citirea unor informatii de la client
    char genre[100],name[100], link[500],artist[100];
    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }
    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }
    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }
    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	int length = sizeof (from);
    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);
    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);
    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}
    	int pid;
    	if ((pid = fork()) == -1) 
        {
    		close(client);
    		continue;
    	} 
        else if (pid > 0) 
        {
    		// parinte
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} 
        else if (pid == 0) 
        {
    		// copil
    		close(sd);
            int nr, msg = 0, log_in = 0, user_id = 0;
            char user[250], pass[250];
             while(log_in == 0)
             {
                MYSQL *con = mysql_init(NULL); //obiect de tip MySQL pentru efectuarea operatiilor cu baza de date
	            if (read (client, &nr,sizeof(int)) <= 0)
	            {
			        perror ("Eroare la read() de la client.\n");
	            }
	            switch(nr) // switch pentru tratarea cazurilor in functie de optiunea aleasa de client
                {
                    case 1:
                        if (read (client, user,sizeof(user)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if (read (client, pass,sizeof(pass)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if(con == NULL)
                        {
                            fprintf(stderr, "%s\n", mysql_error(con));
                            exit(1);
                        }
                        if(mysql_real_connect(con, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                        //Ne conectam la baza de date TopMusic cu utilizatorul marceliacob si parola sqlpass
                        //Acestea au fost create folosind comanda CREATE USER marceliacob@localhost IDENTIFIED BY 'sqlpass';
                        {
                            finish_with_error(con);
                        }
                        if(mysql_query(con,"SELECT * FROM users")) //utilizand mysql_query putem scrie apeluri de tip select ca sa comunicam cu baza de date
                        {
                            finish_with_error(con);
                        }
                        MYSQL_RES *result = mysql_store_result(con);
                        if(result == NULL)
                        {
                            finish_with_error(con);
                        }
                        int num_fields = mysql_num_fields(result); // variabila ce pastreaza numarul de variabile dintr-o tabela
                        int ok = 0 , found = 0;
                        MYSQL_ROW row; // obiect care preia fiecare rand returnat de inteogare
                        while ((row = mysql_fetch_row(result))&& ok == 0 && found == 0)
                        {
                            if(strcmp(row[1],user)==0 && strcmp(row[2],pass) != 0)
                            {      
                                msg = 3;
                                ok = 1;
                            }
                            else if(strcmp(row[1],user)==0 && strcmp(row[2],pass) == 0)
                            {
                                user_id = atoi(row[0]);
                                if(strcmp(row[3],"Administrator")==0)
                                {
                                    msg = 1;
                                    admin = 1;
                                }
                                else msg = 2;
                                log_in = 1;
                                ok = 1;
                                found = 1;
                            }
                        }
                        if(ok == 0) msg = 4;
                        mysql_close(con);
	                    if (write (client, &msg, sizeof(int)) <= 0)
		                {
		                    perror ("Eroare la write() catre client.\n");
		                }
                        break;
                    case 2:
                        if (read (client, user,sizeof(user)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if (read (client, pass,sizeof(pass)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        MYSQL *con = mysql_init(NULL);
                        if(con == NULL)
                        {
                            fprintf(stderr, "%s\n", mysql_error(con));
                            exit(1);
                        }
                        if(mysql_real_connect(con, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                        {
                            finish_with_error(con);
                        }
                        if(mysql_query(con,"SELECT * FROM users"))
                        {
                            finish_with_error(con);
                        }
                        result = mysql_store_result(con);
                        if(result == NULL)
                        {
                            finish_with_error(con);
                        }
                        num_fields = mysql_num_fields(result);
                        ok = 0;
                        while ((row = mysql_fetch_row(result)) && ok == 0)
                        {
                            if(strcmp(row[1], user)==0)   
                                ok = 1;
                        }
                        if(ok == 0)
                        {
                            msg = 2;
                            if(mysql_query(con,"SELECT * FROM users ORDER BY user_id DESC"))
                            {
                                finish_with_error(con);
                            }
                            result = mysql_store_result(con);
                            row = mysql_fetch_row(result);
                            int new_id = atoi(row[0])+1; // Se preia cel mai mare id din tabela si se adauga 1 ca sa primim id-ul noii inregistrari din tabela
                            char query[300];
                            sprintf(query, "INSERT INTO users VALUES('%d','%s','%s','%s','%d')", new_id,user,pass,"User",0); 
                            //Utilizand sprintf, putem folosi variabile din C in interogare.
                            if (mysql_query(con,query)) 
                            {
                                finish_with_error(con);
                            }
                        }
                        else msg = 1;
                        mysql_close(con);
	                    if (write (client, &msg, sizeof(int)) <= 0)
		                {
		                    perror ("Eroare la write() catre client.\n");
		                }
                        break;
                    case 3: close(client); exit(0);  break;
                }
             }
            while(log_in == 1) //Am iesit din meniul de logare, intram in meniul principal al aplicatiei
            {
                MYSQL *con1 = mysql_init(NULL);
	            if (read (client, &nr,sizeof(int)) <= 0)
	            {
		            perror ("Eroare la read() de la client.\n");
	            }
                switch(nr)
                {
                    case 1:
                        if(mysql_real_connect(con1, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                        {
                            finish_with_error(con1);
                        }
                        if(mysql_query(con1,"SELECT * FROM songs ORDER BY votes DESC"))
                        {
                            printf("%s\n",mysql_error(con1));
                            finish_with_error(con1);
                        }
                        MYSQL_RES *result = mysql_store_result(con1);
                        int num_fields = mysql_num_fields(result);
                        MYSQL_ROW row;
                        int count = 0;
                        char string[600];
                        while ((row = mysql_fetch_row(result)))
                        {
                            count ++; //Aflam astfel numarul de linii returnate de interogare, pentru a transmite acest numar la client.
                        }
                        if(mysql_query(con1,"SELECT * FROM songs ORDER BY votes DESC"))
                        {
                            printf("%s\n",mysql_error(con1));
                            finish_with_error(con1);
                        }
                        result = mysql_store_result(con1);
                        if (write (client, &count, sizeof(int)) <= 0)
		                {
		                    perror ("Eroare la write() catre client.\n");
		                }
                        for(int i = 1; i <= count; ++i)
                        {
                            row = mysql_fetch_row(result);
                            int x, y; x = atoi(row[0]); y = atoi(row[5]);
                            if (write (client, &x, sizeof(int)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                            if (write (client, &y, sizeof(int)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                            strcpy(string, row[1]);
                            if (write (client, string, sizeof(string)) <= 0)
		                    {
                                perror ("Eroare la write() catre client.\n");
		                    }
                            strcpy(string, row[3]);
                            if (write (client, string, sizeof(string)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                        }
                        int viewing_top = 1;
                        while(viewing_top)
                        {
                            int ind;
                            int new_opt;
                            if (read (client, &new_opt,sizeof(int)) <= 0)
	                        {
			                    perror ("Eroare la read() de la client.\n");
	                        }
                            switch(new_opt)
                            {
                                case 1:
                                    if (read (client, &ind,sizeof(int)) <= 0)
	                                {
			                            perror ("Eroare la read() de la client.\n");
	                                }
                                    char query[500];
                                    sprintf(query, "SELECT * FROM songs WHERE id = '%d'", ind);
                                    if(mysql_query(con1,query))
                                    {
                                    printf("%s\n",mysql_error(con1));
                                    finish_with_error(con1);
                                    }
                                    result = mysql_store_result(con1);
                                    while ((row = mysql_fetch_row(result)))
                                    {
                                        count ++;
                                    }
                                    if(mysql_query(con1,query))
                                    {
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    }
                                    result = mysql_store_result(con1);
                                    row = mysql_fetch_row(result);
                                    int x,y; x = atoi(row[0]); y = atoi(row[5]);
                                    if (write (client, &x, sizeof(int)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }
                                    if (write (client, &y, sizeof(int)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }
                                    strcpy(string, row[1]);
                                    if (write (client, string, sizeof(string)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }
                                    strcpy(string, row[2]);
                                    if (write (client, string, sizeof(string)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }  
                                    strcpy(string, row[3]);
                                    if (write (client, string, sizeof(string)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }     
                                    strcpy(string, row[4]);
                                    if (write (client, string, sizeof(string)) <= 0)
		                            {
		                                perror ("Eroare la write() catre client.\n");
		                            }
                                    sprintf(query, "SELECT * FROM comments WHERE song_id = '%d'", x);
                                    if(mysql_query(con1,query))
                                    {
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    } 
                                    result = mysql_store_result(con1);
                                    count = 0;
                                    while ((row = mysql_fetch_row(result)))
                                    {
                                        count ++;
                                    }
                                    sprintf(query, "SELECT * FROM comments WHERE song_id = '%d'", x);
                                     if(mysql_query(con1,query))
                                    {
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    } 
                                    result = mysql_store_result(con1);
                                    if (write (client, &count, sizeof(int)) <= 0)
		                            {
		                                perror ("[Thread]Eroare la write() catre client.\n");
		                            }
                                    char query1[500];
                                    for(int i = 1; i <= count; ++i)
                                    {
                                        char comment1[500];
                                        row = mysql_fetch_row(result);
                                        bzero(comment1,500);
                                        strcpy(comment1,row[3]);
                                        sprintf(query1, "SELECT * FROM users WHERE user_id = '%d'", atoi(row[1]));
                                    if(mysql_query(con1,query1))
                                    {
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    }
                                    MYSQL_RES *result1 = mysql_store_result(con1);
                                    row = mysql_fetch_row(result1);
                                    bzero(user,250);
                                    strcpy(user,row[1]);
                                    write(client, user, sizeof(user));
                                    write(client, comment1, sizeof(comment1));
                                    }
                                    viewing_top = 0;    
                                    break;
                                case 2:
                                if (read (client, &id_piesa,sizeof(int)) <= 0)
	                            {
			                         perror ("Eroare la read() de la client.\n");
	                            }
                                if (read (client, &posneg,sizeof(int)) <= 0)
	                            {
			                         perror ("Eroare la read() de la client.\n");
	                            }
                                if(posneg == 1) sprintf(query, "UPDATE songs SET votes = votes + 1 WHERE id = %d", id_piesa);
                                else if(posneg == 2) sprintf(query, "UPDATE songs SET votes = votes - 1 WHERE id = %d AND votes > 0", id_piesa);
                                int vote_msg;
                                int can_vote;
                                char userquery[500];
                                sprintf(userquery, "SELECT * FROM users WHERE user_id = '%d'",user_id);
                                if(mysql_query(con1,userquery))
                                {
                                    printf("%s\n",mysql_error(con1));
                                    finish_with_error(con1);
                                }
                                result = mysql_store_result(con1);
                                row = mysql_fetch_row(result);
                                if(atoi(row[4]) == 1) vote_msg = 4;
                                else
                                {
                                    sprintf(query1,"SELECT * FROM votes WHERE song_id = %d AND user_id = %d",id_piesa,user_id);
                                    if(mysql_query(con1,query1))
                                    {
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    }
                                    result = mysql_store_result(con1);
                                    count = 0;
                                    while ((row = mysql_fetch_row(result)))
                                    {
                                        count ++;
                                    }
                                    if(count > 0) vote_msg = 1;
                                    else 
                                    {
                                        if(mysql_query(con1,query))
                                        {
                                            vote_msg = 2;
                                            printf("%s\n",mysql_error(con1));
                                            finish_with_error(con1);
                                        }
                                        else
                                        {
                                            if(mysql_query(con1,"SELECT * FROM votes ORDER BY id DESC"))
                                            {
                                                finish_with_error(con1);
                                            }
                                            result = mysql_store_result(con1);
                                            row = mysql_fetch_row(result);
                                            int new_id = atoi(row[0])+1;
                                            sprintf(query, "INSERT INTO votes VALUES('%d','%d','%d')", new_id, user_id, id_piesa);
                                            if(mysql_query(con1,query))
                                            {
                                                vote_msg = 2;
                                                printf("%s\n",mysql_error(con1));
                                                finish_with_error(con1);
                                            }
                                        vote_msg = 3;
                                        }
                                    }
                                }
                                if (write (client, &vote_msg, sizeof(int)) <= 0)
		                        {
		                            perror ("Eroare la write() catre client.\n");
		                        }
                                break;
                            case 3:
                                if (read (client, &index,sizeof(int)) <= 0)
	                            {
			                        perror ("Eroare la read() de la client.\n");
	                            }
                                char comment[500];
                                if (read (client, comment,sizeof(comment)) <= 0)
	                            {
			                        perror ("Eroare la read() de la client.\n");
	                            }
                                int comm_query[500];
                                if(mysql_query(con1,"SELECT * FROM comments ORDER BY id DESC"))
                                {
                                    finish_with_error(con1);
                                }
                                result = mysql_store_result(con1);
                                row = mysql_fetch_row(result);
                                int new_id = atoi(row[0])+1;
                                sprintf(comm_query,"INSERT INTO comments VALUES('%d','%d','%d','%s')",new_id, user_id, index, comment);
                                int comm_post;
                                if(mysql_query(con1,comm_query))
                                {
                                    comm_post = 0;
                                    finish_with_error(con1);
                                }
                                else comm_post = 1;
                                if (write (client, &comm_post, sizeof(int)) <= 0)
		                        {
		                            perror ("[Thread]Eroare la write() catre client.\n");
		                        }
                                break;
                            case 4: viewing_top = 0; break;
                            case 5:
                                if(admin)
                                {
                                    if (read (client, &id_piesa,sizeof(int)) <= 0)
	                                {
			                            perror ("Eroare la read() de la client.\n");
	                                }
                                    char query2[500];
                                    sprintf(query2, "DELETE FROM comments WHERE song_id = '%d'", id_piesa);
                                    int del_mes;
                                    if(mysql_query(con1,query2))
                                    {
                                        del_mes = 0;
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    }
                                    sprintf(query2, "DELETE FROM votes WHERE song_id = '%d'", id_piesa);
                                    if(mysql_query(con1,query2))
                                    {
                                        del_mes = 0;
                                        printf("%s\n",mysql_error(con1));
                                        finish_with_error(con1);
                                    }
                                    else del_mes = 1;
                                    if (write(client,&del_mes,sizeof(int))<=0)
                                    {
                                        perror ("Eroare la write() catre client.\n");
                                    }
                                }

                            }
                        } 
                        break;
                    case 2:
                        if(mysql_real_connect(con1, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                        {
                            finish_with_error(con1);
                        }
                        char query[500],genre[100];
                        if (read (client, genre,sizeof(genre)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        strtok(genre,"\n");
                        sprintf(query, "SELECT * FROM songs WHERE genre = '%s'", genre);
                        if(mysql_query(con1,query))
                        {
                            printf("%s\n",mysql_error(con1));
                            finish_with_error(con1);
                        }
                        result = mysql_store_result(con1);
                        num_fields = mysql_num_fields(result);count = 0;
                        while ((row = mysql_fetch_row(result)))
                        {
                            count ++;
                        }
                        if(mysql_query(con1,query))
                        {
                           printf("%s\n",mysql_error(con1));
                           finish_with_error(con1);
                        }
                        result = mysql_store_result(con1);
                        if (write (client, &count, sizeof(int)) <= 0)
		                {
		                    perror ("Eroare la write() catre client.\n");
		                }
                        for(int i = 1; i <= count; ++i)
                        {
                            row = mysql_fetch_row(result);
                            int x,y; x = atoi(row[0]); y = atoi(row[5]);
                            if (write (client, &x, sizeof(int)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                            if (write (client, &y, sizeof(int)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                            strcpy(string, row[1]);
                            if (write (client, string, sizeof(string)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                            strcpy(string, row[3]);
                            if (write (client, string, sizeof(string)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }
                        }    
                        break;
                    case 3:
                        if (read (client, name,sizeof(name)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if (read (client, artist,sizeof(artist)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if (read (client, genre,sizeof(genre)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        if (read (client, link,sizeof(link)) <= 0)
	                    {
			                perror ("Eroare la read() de la client.\n");
	                    }
                        MYSQL *con = mysql_init(NULL);
                        if(con == NULL)
                        {
                            fprintf(stderr, "%s\n", mysql_error(con));
                            exit(1);
                        }
                        if(mysql_real_connect(con, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                        {
                            finish_with_error(con);
                        }
                        if(mysql_query(con,"SELECT * FROM songs"))
                        {
                            finish_with_error(con);
                        }
                        result = mysql_store_result(con);
                        if(result == NULL)
                        {
                            finish_with_error(con);
                        }
                        num_fields = mysql_num_fields(result);
                        int ok = 0;
                        while ((row = mysql_fetch_row(result))&& ok == 0)
                        {
                            if(strcmp(row[1],name)==0)   
                                ok = 1;
                        }
                        if(ok == 0)
                        {
                            msg = 2;
                            if(mysql_query(con,"SELECT * FROM songs ORDER BY id DESC"))
                            {
                                finish_with_error(con);
                            }
                            result = mysql_store_result(con);
                            row = mysql_fetch_row(result);
                            int new_id = atoi(row[0])+1;
                            char query[300];
                            sprintf(query, "INSERT INTO songs VALUES('%d','%s','%s','%s','%s','%d')", new_id,name,genre,artist,link,0);
                            if (mysql_query(con,query)) 
                            {
                                finish_with_error(con);
                            }
                        }           
                        else msg = 1;
                        mysql_close(con);
	                    if (write (client, &msg, sizeof(int)) <= 0)
	                    {
		                    perror ("Eroare la write() catre client.\n");
	                    }
                        break;
                        case 5: log_in = 0; close(client); break;
                        case 6:
                        if(admin)
                        {
                            if(mysql_real_connect(con1, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                            {
                                finish_with_error(con1);
                            }
                            char ban_user[500],query[500];
                            if (read (client, ban_user,sizeof(ban_user)) <= 0)
	                        {
	                            perror ("Eroare la read() de la client.\n");
	                        }
                            sprintf(query, "UPDATE users SET restriction = 1 WHERE user = '%s'", ban_user);
                            if(mysql_query(con1,query))
                            {
                                finish_with_error(con1);
                            }
                            int ban_msg = 1;
                            if (write (client, &ban_msg, sizeof(int)) <= 0)
		                    {
		                        perror ("Eroare la write() catre client.\n");
		                    }

                        }
                        break;
                      case 7:
                          if(admin)
                          {
                            if(mysql_real_connect(con1, "localhost", "marceliacob", "sqlpass","TopMusic",0,NULL,0) == NULL)
                            {
                                finish_with_error(con1);
                            }
                            char ban_user[500],query[500];
                            if (read (client, ban_user,sizeof(ban_user)) <= 0)
	                        {
			                    perror ("Eroare la read() de la client.\n");
	                        }
                            sprintf(query, "UPDATE users SET restriction = 0 WHERE user = '%s'", ban_user);
                            if(mysql_query(con1,query))
                            {
                                finish_with_error(con1);
                            }
                            int ban_msg = 1;
                            if (write (client, &ban_msg, sizeof(int)) <= 0)
		                    {
		                        perror ("[Thread]Eroare la write() catre client.\n");
                            }
                          }
                          break;
                }
            }
        }
    close(client);
    exit(0);
    }
}