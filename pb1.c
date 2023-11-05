// Se va scrie un program in limbajul C ce va prelucra un fisier de intrare si va realiza o serie de statistici pe cel fisier.
// Programul va primit 3 parametri si se va apela dupa cum urmeaza:
// ./program <fisier_intrare> <fisier_statistica> <ch>
// Primul argument reprezinta un fisier text de intrare aflat pe disc ce contine in principal text.
// Al doilea argument reprezinta fisierul de iesire al programului ce va contine informatii statistice realizate de program.
// Al treilea parametru reprezinta un caracter
// Programul trebuie sa parcurga fisierul text dat ca prim parametru si sa realizeze urmatoarele statistici:
// numar litere mici
// numar litere mari
// numar cifre
// numar de aparitii ale caracterului <ch> dat ca si al treilea parametru
// dimensiune fisier
// Un exemplu de fisier de statistica (de iesire) ar putea fi urmatorul:
// numar litere mici: 20
// numar litere mari: 35
// numar cifre: 76
// numar aparitii caracter: 12
// dimensiune fisier: 100
// Se va respecta intocmai structura fisierului de iesire.
// Se vor folosi doar apeluri sistem pentru lucrul cu fisierele (open, read, write, close, stat, fstat, lstat... etc).
// Nu se permite folosirea functiilor din biblioteca standard stdio pentru lucrul cu fisiere (fopen, fread, fwrite, fclose... etc).
// Se permite folosirea functiei sprintf pentru a obtine un string formatat pentru a putea fi scris in fisier folosind apelul sistem "write".

// Pentru a obtine dimensiunea fisierului se vor folosi apeluri sistem din familia stat.

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SIZE 4096

int main(int argc, char **argv)
{
    char buffer[SIZE];
    char text[1000];
    int deschidere, citire, scriere;
    int litere_mici = 0;
    int litere_mari = 0;
    int count_ch = 0;
    int cifre = 0;
    struct stat fileStat;

    if (argc > 4)
    {
        perror("Numarul de argumente este prea mare");
        exit(-1);
    }
    deschidere = open(argv[1], O_RDONLY);
    if (deschidere == -1)
    {
        perror("Fisierul nu poate fi deschis");
        exit(-1);
    }
    else
    {
        while ((citire = read(deschidere, buffer, SIZE)))
        {
            for (int i = 0; i < citire; i++)
            {
                if (islower(buffer[i]))
                {
                    litere_mici++;
                }
                if (isupper(buffer[i]))
                {
                    litere_mari++;
                }
                if (isdigit(buffer[i]))
                {
                    cifre++;
                }
                if (buffer[i] == argv[3][0])
                {
                    count_ch++;
                }
            }
        }
        close(deschidere);
        scriere = open(argv[2], O_WRONLY);
        if (scriere == -1)
        {
            perror("Fisierul nu poate fi deschis");
            exit(-1);
        }
        else
        {
            sprintf(text, "Numarul de litere mici este : %d\n", litere_mici);
            write(scriere, text, strlen(text));
            sprintf(text, "Numarul de litere mari este : %d\n", litere_mari);
            write(scriere, text, strlen(text));
            sprintf(text, "Numarul de cifre este : %d\n", cifre);
            write(scriere, text, strlen(text));
            sprintf(text, "Numarul de aparitii al caracterului dat ca si parametru este : %d\n", count_ch);
            write(scriere, text, strlen(text));
            if (stat(argv[1], &fileStat) == -1)
            {
                perror("Functia stat a esuat");
                exit(-1);
            }
            else
            {
                sprintf(text, "Dimensiunea fisierului este : %ld bytes\n", fileStat.st_size);
                write(scriere, text, strlen(text));
            }
            close(scriere);
        }
    }

    return 0;
}
