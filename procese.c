// Să se modifice programul din săptămâna anterioară astfel încât acesta va primi un al
// doilea argument ce va reprezenta calea către un director și prin urmare se va apela
// astfel: ./program <director_intrare> <director_iesire>
// Pentru fiecare intrare (fișier obișnuit, fișier.bmp, director, legătură simbolică ce indică
// spre un fișier obișnuit) din directorul de intrare (primul argument), procesul părinte va
// crea câte un nou proces care va scrie informațiile referitoare la statistică într-un fișier din
// directorul de ieșire (al doilea argument), cu numele
// <nume_intrare>_statistica.txt, unde <nume_intrare> reprezinta numele
// intrarii curente din directorul de intrare.
// La încheierea fiecărui proces fiu, acesta va trimite părintelui numărul de linii scrise în
// fișierul statistica.txt.
// În plus, pentru fiecare intrare ce reprezinta o imagine .bmp procesul părinte va crea un
// al doilea proces (pe langa cel responsabil de scrierea de informații în fișierul de
// statistică), care sa citească întreg conținutul fișierului și care va converti imaginea în
// tonuri de gri. Știind ca în cadrul imaginilor.bmp fiecare pixel este caracterizat de 3 valori
// cuprinse între 0 și 255 (reprezentate pe 1 octet), puteți folosind formula de mai jos
// pentru conversia cerută:
// P_gri = 0.299 * P_rosu + 0.587 * P_verde + 0.114 * P_albastru
// unde P_rosu, P_verde, respectiv P_albastru, reprezinta intensitatile celor trei culori.
// La încheierea fiecărui proces fiu, părintele îi va prelua starea și va afișa un mesaj de
// forma “S-a încheiat procesul cu pid-ul <PID> și codul <cod>”, unde
// <PID> reprezinta PID-ul procesului fiu care s-a incheiat, iar <cod> reprezinta codul cu
// care acesta s-a incheiat.

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
#include <sys/wait.h>

#define SIZE 4096

// Am declarat global variabiele in care se citeste lungimea si inaltimea fisierului,count care numara liniile scrise in fisie
// vectorul "text" pe care il folosesc pentru formatarea textului care urmeaza sa fie scris in fisier "statistica.txt"
int lungime = 0, inaltime = 0;
char text[SIZE];
int count = 0;

void terminareProces(int status, int wait_pid)
{
    // Inchid procesul fiului pentru a se executa si procesul parintelui
    // Folosec wait() pentru a incheia de tot procesul
    while ((wait_pid = wait(&status)) > 0)
    {
        if (wait_pid == -1)
        {
            perror("Erroare la asteptarea procesului\n");
            exit(-1);
        }
        else if (WIFEXITED(status))
        {
            printf("Procesul s-a terminat cu pid-ul %d si cu status-ul %d\n", wait_pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Procesul cu pid-ul %d a fost omorat de semnalul %d\n", wait_pid, WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("Procesul cu pid-ul %d a fost oprit de semnalul %d\n", wait_pid, WSTOPSIG(status));
        }
        else if (WIFCONTINUED(status))
        {
            printf("Procesul continua\n");
        }
    }
}

void esteFisier(int fd, int scriere, struct stat stat_data, struct dirent *entry, const char *path)
{
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

            // Incrementez count-il pentru fisierele BMP
            count = count + 2;
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

        // Incrementez count-ul
        count = count + 8;
    }
}

void esteLegaturaSimbolica(int fd, int scriere, struct stat stat_data, struct dirent *entry)
{
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

        // Incrementez count-ul
        count = count + 6;
    }
}

void esteDirector(int scriere, struct stat stat_data, struct dirent *entry)
{
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

        // Incrementez count-ul
        count = count + 5;
    }
}

void citireDirectorScriereFisier(const char *dirpath, const char *dirOutPath)
{
    pid_t pid, pid2; // pid-uri folosite pentru cele doua procese
    struct stat stat_data;
    struct dirent *entry;
    int fd = 0, scriere = 0; // file descriptor pentru deschidere si scriere
    char output_path[SIZE];  // variabila folosita pentru a formata un director in care o sa scriu fisierele de statistica

    // Pentru inceput deschidem directorul
    DIR *directory = opendir(dirpath);
    // Verificare
    if (directory == NULL)
    {
        perror("Eroare la deschiderea directorului");
        exit(-1);
    }

    // Parcurgem directorul si efectuam verificarile necesare
    while ((entry = readdir(directory)) != NULL)
    {
        // Imi formatez entry-ul pe care il primesc din director ca si un path
        char path[SIZE];
        sprintf(path, "%s/%s", dirpath, entry->d_name);
        printf("%s\n", entry->d_name);

        // Formatez directorul in care o sa scriu fisiere de de statistica
        sprintf(output_path, "%s/%s_statistica.txt", dirOutPath, entry->d_name);

        // Imi creez de fiecare data fisierul in care o sa imi scriu datele
        scriere = creat(output_path, S_IWUSR | S_IRUSR);
        if (scriere == -1)
        {
            perror("Eraoare la crearea fisierului de scriere\n");
        }

        // Efectuam verificarile pentru fisiere
        if (lstat(path, &stat_data) == -1)
        {
            perror("Functia lstat a esuat");
            exit(-1);
        }

        // Incep procesul
        if ((pid = fork()) < 0)
        {
            perror("Eroare");
            exit(1);
        }
        if (pid == 0)
        {
            // Deschid fisierul
            fd = open(path, O_RDONLY);
            if (fd == -1)
            {
                perror("Fisierul nu poate fi deschis");
                exit(-1);
            }

            // Verificam daca este un fisier bmp sau fisier normal
            esteFisier(fd, scriere, stat_data, entry, path);

            // Verificam daca este legatura simbolica ce indica sper un fisier obisnuit
            esteLegaturaSimbolica(fd, scriere, stat_data, entry);

            // Verificam daca este director
            esteDirector(scriere, stat_data, entry);

            // Inchidem dupa ficare iteratie fisierul deschid pentru preluarea datelor si cel pentru scrierea datelor
            if (close(fd) == -1)
            {
                perror("Eroare la inchidere\n");
                exit(-1);
            }
            if (close(scriere) == -1)
            {
                perror("Eroare la inchidere\n");
                exit(-1);
            }
            exit(count);
        }

        // Terminam primul proces
        int wait_status = 0, wait_pid = 0;
        terminareProces(wait_status, wait_pid);

        // Initiem al doilea proces in care vom converi imaginea de tip bmp in tonuri de gri
        if (S_ISREG(stat_data.st_mode))
        {
            if (strstr(path, ".bmp") != NULL)
            {
                // Deschidem fisierul atat pentru scriere cat si pentru citire
                fd = open(path, O_RDWR);
                if (fd == -1)
                {
                    perror("Fisierul nu poate fi deschis");
                    exit(-1);
                }
                if ((pid2 = fork()) < 0)
                {
                    perror("Eroare");
                    exit(1);
                }
                if (pid2 == 0)
                {
                    unsigned char pixel[3];
                    ssize_t citire, scriereBMP;
                    lseek(fd, 54, SEEK_SET);
                    while ((citire = read(fd, pixel, sizeof(pixel))))
                    {
                        // Convertim la nivel de gri dupa formula data
                        unsigned char gri = (0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
                        // Ne întoarcem la pozitia curenta
                        lseek(fd, -3, SEEK_CUR);
                        // Scriem pixelul gri inapoi in fisierul de imagine
                        unsigned char pixelGri[3];
                        pixelGri[0] = pixelGri[1] = pixelGri[2] = gri;
                        scriereBMP = write(fd, pixelGri, sizeof(pixelGri));
                        if (scriereBMP == -1)
                        {
                            perror("Erroare la scrierea in fisier\n");
                            close(fd);
                            exit(-1);
                        }
                    }
                    // Inchidem fisierul
                    if (close(fd) == -1)
                    {
                        perror("Eroare la inchidere\n");
                        exit(-1);
                    }
                    // Inchid procesul fiului pentru a se executa si procesul parintelui
                    exit(0);
                }
                int wait_status = 0, wait_pid = 0;
                terminareProces(wait_status, wait_pid);
            }
        }
    }
}

int main(int argc, char **argv)
{
    struct stat stat_dir1;
    struct stat stat_dir2;
    if (argc > 3)
    {
        perror("Usage ./program <director_intrare> <director_iesire>");
        exit(-1);
    }

    // Verificare pentru primul Argument
    if (stat(argv[1], &stat_dir1) == -1)
    {
        perror("Functia stat a esuat");
        exit(-1);
    }
    if (S_ISDIR(stat_dir1.st_mode))
    {
        printf("Primul argument e un director!\n");
    }
    else
    {
        printf("Nu este un director!\n");
    }

    // Verificare pentru al doilea argument
    if (stat(argv[2], &stat_dir2) == -1)
    {
        perror("Functia stat a esuat");
        exit(-1);
    }
    if (S_ISDIR(stat_dir2.st_mode))
    {
        printf("Al doilea argument e un director!\n");
    }
    else
    {
        printf("Nu este un director!\n");
    }
    citireDirectorScriereFisier(argv[1], argv[2]);

    return 0;
}