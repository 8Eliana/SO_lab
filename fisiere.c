// Se va scrie un program în limbajul C ce va prelucra un fișier de intrare ce reprezinta o
// imagine in format BMP și va realiza o serie de statistici pe baza acestui fișier. Programul
// va primi un parametru și se va apela după cum urmează:
// ./program <fisier_intrare>
// Programul va verifica faptul că a primit un singur argument, precum și tipul acestuia, iar
// în caz contrar va afișa un mesaj de eroare ”Usage ./program
// <fisier_intrare>”.
// Programul trebuie sa citeasca header-ul fisierului BMP și sa extraga valoarea înălțimii,
// respectiv a lungimii pentru imaginea data.
// Programul va crea un fișier cu numele statistica.txt în care vor fi scrise
// următoarele informații:
// nume fisier: poza.bmp
// inaltime: 1920
// lungime: 1280
// dimensiune: <dimensiune in octeti>
// identificatorul utilizatorului: <user id>
// timpul ultimei modificari: 28.10.2023
// contorul de legaturi: <numar legaturi>
// drepturi de acces user: RWX
// drepturi de acces grup: R–-
// drepturi de acces altii: ---
// Se vor folosi doar apeluri sistem pentru lucrul cu fișierele (open, read, write, close, stat,
// fstat, lstat... etc). Nu se permite folosirea funcțiilor din biblioteca standard stdio pentru
// lucrul cu fisiere (fopen, fread, fwrite, fclose... etc). Se permite folosirea funcției sprintf
// pentru a obține un string formatat pentru a putea fi scris în fișier folosind apelul sistem
// "write".

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define SIZE 4096

// Am declarat global variabilele folosite ca si file descriptor,structura pentru stat,variabiele in care se citeste lungimea si inaltimea fisierului,
// cat si vectorul "text" pe care il folosesc pentru formatarea textului care urmeaza sa fie scris in fisier "statistica.txt"
struct stat stat_data;
int fd = 0, scriere = 0;
int lungime = 0, inaltime = 0;
char text[SIZE];

void verificareDeschidereFisier(const char *pathname)
{
    if (stat(pathname, &stat_data) == -1)
    {
        perror("Functia stat a esuat");
        exit(-1);
    }
    if (S_ISREG(stat_data.st_mode))
    {
        printf("E un fisier!\n");
    }
    else
    {
        printf("Nu este fisier!\n");
    }
    fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        perror("Fisierul nu poate fi deschis");
        exit(-1);
    }
}

void citireScriereFisier(const char *pathname, const char *filein)
{
    // Se creaza fisierul de scriere si se face o verificare
    scriere = creat(pathname, S_IWUSR | S_IRUSR);
    if (scriere == -1)
    {
        perror("Eraoare la crearea fisierului de scriere\n");
    }

    // Incep sa scriu datele necesare in fisier
    sprintf(text, "Nume fisier : %s\n", filein);
    write(scriere, text, strlen(text));

    // Cu ajutoul functiei lseek,setam indicatorul pozitiei cu un anumit numar de octeti pentru a citi din fisier exact datele de care am nevoie
    // Trebuie sa extragem lungimea fisierului asa ca setez indicatorul pozitiei la 18 bytes si ajung la pozitia dorita
    lseek(fd, 18, SEEK_SET);
    read(fd, &inaltime, SIZE);
    sprintf(text, "Inaltimea fisierului BMP este : %d\n", inaltime);
    write(scriere, text, strlen(text));

    // Setez indicatorul pentru a citi lungimea fisierului
    lseek(fd, 22, SEEK_SET);
    read(fd, &lungime, SIZE);
    sprintf(text, "Lungimea fisierului BMP este : %d\n", lungime);
    write(scriere, text, strlen(text));

    // Scriem in fisierul creat dimensiunea fisieruli citit
    sprintf(text, "Dimensiunea fisierului BMP este : %ld\n", stat_data.st_size);
    write(scriere, text, strlen(text));

    // Scriem identificatorul utilizatorului fisierului citit(user id)
    sprintf(text, "Identificatorul utilizatorului fisierului BMP este : %d\n", stat_data.st_uid);
    write(scriere, text, strlen(text));

    // Scriem timpul ultumei modificari
    sprintf(text, "Timpul ultimei modificari fisierului BMP este : %s", ctime(&stat_data.st_mtime));
    write(scriere, text, strlen(text));

    // Scriem in fisier controlul de legaturi
    sprintf(text, "Controul de legaturi a fisierului BMP este : %ld\n", stat_data.st_nlink);
    write(scriere, text, strlen(text));

    // Scriem drepurile de acces user
    mode_t userPermissions = stat_data.st_mode & S_IRWXU;
    sprintf(text, "Drepurile de acces ale user-ului fisierului BMP sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-');
    write(scriere, text, strlen(text));

    // Scriem drepurile de acces grup
    mode_t groupPermissions = stat_data.st_mode & S_IRGRP;
    sprintf(text, "Drepurile de acces ale user-ului fisierului BMP sunt : %c%c%c\n", (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-');
    write(scriere, text, strlen(text));

    // Scriem drepturi de acces altii
    mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
    sprintf(text, "Drepurile de acces ale user-ului fisierului BMP sunt : %c%c%c\n", (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
    write(scriere, text, strlen(text));

    // Inchidem fisierele deschise
    if (close(fd) == -1)
    {
        perror("Eroare de inchidere\n");
        exit(-1);
    }
    if (close(scriere) == -1)
    {
        perror("Eroare de inchidere\n");
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        perror("Usage ./program <fisier_intrare>");
        exit(-1);
    }
    verificareDeschidereFisier(argv[1]);
    citireScriereFisier("statistica.txt", argv[1]);

    return 0;
}
