// Sa se modifice programul din saptamana anterioare astfel incat acesta va primi ca și
// parametru calea către un director (în locul unei imagini) și prin urmare se va apela astfel:
// ./program <director_intrare>
// Programul va trebui sa parcurga directorul dat ca parametru și va scrie în cadrul fișierului
// statistica.txt în funcție de tipul intrării curente astfel:
// - dacă este un fișier obișnuit cu extensia .bmp vor scrie informațiile de mai sus
// - dacă este un fișier obișnuit, dar fără extensia .bmp,va scrie aceleași informații
// ca și în cazul anterior, însă fără lungime și înălțime
// - dacă este o legatura simbolica ce indica spre un fișier obișnuit va scrie
// următoarele informații:
// nume legatura: nume
// dimensiune legatura: dimensiunea legaturii
// dimensiune fisier dimensiunea fisierului target
// drepturi de acces user legatura: RWX
// drepturi de acces grup legatura: R–-
// drepturi de acces altii legatura: ---
// - dacă este un director se vor scrie următoarele informații:
// nume director: director
// identificatorul utilizatorului: <user id>
// drepturi de acces user: RWX
// drepturi de acces grup: R–-
// drepturi de acces altii: ---
// - pentru orice alte cazuri nu se va scrie nimic în fișierul statistica.txt

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define SIZE 4096

// Am declarat global variabilele folosite ca si file descriptor,structura pentru stat,variabiele in care se citeste lungimea si inaltimea fisierului,
// un pointer la structura dirent,cat si vectorul "text" pe care il folosesc pentru formatarea textului care urmeaza sa fie scris in fisier "statistica.txt"
struct stat stat_data;
struct dirent *entry;
int fd = 0, scriere = 0;
int lungime = 0, inaltime = 0;
char text[SIZE];

void citireDirectorScriereFisier(const char *filename, const char *dirpath)
{
    // Pentru inceput deschidem directorul
    DIR *directory = opendir(dirpath);
    // Verificare
    if (directory == NULL)
    {
        perror("Eroare la deschiderea directorului");
        exit(-1);
    }
    scriere = creat(filename, S_IWUSR | S_IRUSR);
    if (scriere == -1)
    {
        perror("Eraoare la crearea fisierului de scriere\n");
    }

    // Parcurgem directorul si efectuam verificarile necesare
    while ((entry = readdir(directory)) != NULL)
    {
        // Imi formatez entry-ul pe care il primesc din director ca si un path
        char path[SIZE];
        sprintf(path, "%s/%s", dirpath, entry->d_name);
        printf("%s\n", entry->d_name);

        // Efectuam verificarile pentru fisiere
        if (lstat(path, &stat_data) == -1)
        {
            perror("Functia lstat a esuat");
            exit(-1);
        }

        // Deschid fisierul
        fd = open(path, O_RDONLY);
        if (fd == -1)
        {
            perror("Fisierul nu poate fi deschis");
            exit(-1);
        }

         // Verificam daca este un fisier bmp sau fisier normal
        if (S_ISREG(stat_data.st_mode))
        {
            // Incep sa scriu datele necesare in fisier
            sprintf(text, "Nume fisier : %s\n", entry->d_name);
            write(scriere, text, strlen(text));

            // Verific daca se termina cu extensia ".bmp" si astfel scriu inaltimea si lungimea fisierului
            if (strstr(path, ".bmp") != NULL)
            {
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
            }

            // Scriem in fisierul creat dimensiunea fisieruli,identificatorul utilizatorului (user id),timpul ultimei modificari,controlul de legaturi
            sprintf(text, "Dimensiunea fisierului este : %ld\nIdentificatorul utilizatorului fisierului este : %d\nTimpul ultimei modificari fisierului este : %sControul de legaturi a fisierului este : %ld\n", stat_data.st_size, stat_data.st_uid, ctime(&stat_data.st_mtime), stat_data.st_nlink);
            write(scriere, text, strlen(text));

            // Scriem drepurile de acces user,grup si altii
            mode_t userPermissions = stat_data.st_mode & S_IRWXU;
            mode_t groupPermissions = stat_data.st_mode & S_IRGRP;
            mode_t othersPermissions = stat_data.st_mode & S_IRGRP;
            sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
            write(scriere, text, strlen(text));
        }

        // Verificam daca este legatura simbolica ce indica sper un fisier obisnuit
        if (S_ISLNK(stat_data.st_mode))
        {
            // Scriem nume si dimeniunea legaturii
            sprintf(text, "Nume fisier : %s\nDimensiune legatura : %ld\n", entry->d_name, stat_data.st_size);
            write(scriere, text, strlen(text));

            // Scriem drepurile de acces user,grup si altii
            mode_t userPermissions = stat_data.st_mode & S_IRWXU;
            mode_t groupPermissions = stat_data.st_mode & S_IRGRP;
            mode_t othersPermissions = stat_data.st_mode & S_IRGRP;
            sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
            write(scriere, text, strlen(text));

            // Folosind fstat preluam dimeniunea fisierului target si o sriem
            if (fstat(fd, &stat_data) == -1)
            {
                perror("Eraoare\n");
                exit(-1);
            }
            sprintf(text, "Dimensiune fisier target: %ld\n", stat_data.st_size);
            write(scriere, text, strlen(text));
        }

        // Verificam daca este director
        if (S_ISDIR(stat_data.st_mode))
        {
            // Scriem numele si identificatorul utilizatorului(user id)
            sprintf(text, "Nume director : %s\nIdentificatorul utilizatorului fisierului este : %d\n", entry->d_name, stat_data.st_uid);
            write(scriere, text, strlen(text));
            // Scriem drepurile de acces user,grup si altii
            mode_t userPermissions = stat_data.st_mode & S_IRWXU;
            mode_t groupPermissions = stat_data.st_mode & S_IRGRP;
            mode_t othersPermissions = stat_data.st_mode & S_IRGRP;
            sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
            write(scriere, text, strlen(text));
        }
        // Inchidem dupa ficare iteratie fisierul deschid pentru preluarea datelor
        if (close(fd) == -1)
        {
            perror("Eroare lainchidere\n");
            exit(-1);
        }
    }
    // Inchidem fisierul deschis penru scrierea datelor prelucrate
    if (close(scriere) == -1)
    {
        perror("Eroare lainchidere\n");
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    struct stat stat_dir;
    if (argc > 2)
    {
        perror("Usage ./program <director_intrare>");
        exit(-1);
    }
    if (stat(argv[1], &stat_dir) == -1)
    {
        perror("Functia stat a esuat");
        exit(-1);
    }
    if (S_ISDIR(stat_dir.st_mode))
    {
        printf("E un director!\n");
    }
    else
    {
        printf("Nu este un director!\n");
    }
    citireDirectorScriereFisier("statistica.txt", argv[1]);

    return 0;
}
