#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

void finish_with_error(MYSQL *con)
{
    fprintf(stderr,"%s\n",mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char **argv)
{
    MYSQL *con = mysql_init(NULL);
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


   if (mysql_query(con, "INSERT INTO songs VALUES(6,'Billie Jean','dance','Michael Jackson','https://www.youtube.com/watch?v=A9pd3M1Sxe8', 0)")) {
      finish_with_error(con);
  }
  
    exit(0);
}