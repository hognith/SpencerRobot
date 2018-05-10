#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    pid_t pid;
    pid = fork(); //Býr til nýjan process
 
 	   
    if(pid < 0) {      //Ath hvort það hafi tekist
        perror("Fork failed\n");
        exit(1);
    }
    else if(pid == 0){                                          //Hér erum við inn í nýja processnum.
        printf("In child process.\n");                          //Þessari línu má sleppa, aðeins til að villuprófa
        execlp("mpg123", "mpg123", "-q", "./SuperMario.mp3", NULL);   //mpg123 látinn spila skránna test_mp3
        printf("Done playing, exiting child process.\n");       //Þessari línu má sleppa, aðeins til að villuprófa
        return 0;                                               //Process hættir keyrslu
    }

    //Hér erum við inni í upphaflega forritinu.

    wait(NULL);  //Bíða eftir því að lagið klárist. Það er gott að hafa þessa línu ef þið eruð að prófa kóðann.
                  //Ef þið eruð að nota þennan kóða inn í forritinu ykkar myndi forritið halda áfram keyrslu
                    //meðan lagið væri að klárast og þá sleppa wait(NULL).
}
