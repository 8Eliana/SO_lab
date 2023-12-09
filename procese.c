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

// Am declarat global variabiele in care se citeste lungimea si inaltimea fisierului,count care numara liniile scrise in fisier
// vectorul "text" pe care il folosesc pentru formatarea textului care urmeaza sa fie scris in fisier "statistica.txt"
int lungime = 0, inaltime = 0;
uint16_t bitCount;
char text[SIZE];
int count = 0;

// Functie folosita pentru a inchide procesul fiului si pentru a se executa si procesul parintelui.Folosesc wait() pentru a incheia de tot procesul
void terminareProces(int status, int wait_pid)
{
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

// Functie in care verific daca intrarea este un fisier obisnuit si/sau BMP si scriu date despre el intr-un fisier de statistica
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

            // Incrementez count-ul pentru fisierele BMP
            count = count + 2;
        }
        // Scriem in fisierul creat dimensiunea fisierului,identificatorul utilizatorului (user id),timpul ultimei modificari,controlul de legaturi
        sprintf(text, "Dimensiunea fisierului este : %ld\nIdentificatorul utilizatorului fisierului este : %d\nTimpul ultimei modificari a fisierului este : %sControul de legaturi a fisierului este : %ld\n", stat_data.st_size, stat_data.st_uid, ctime(&stat_data.st_mtime), stat_data.st_nlink);
        write(scriere, text, strlen(text));

        // Scriem drepurile de acces user,grup si altii
        mode_t userPermissions = stat_data.st_mode & S_IRWXU;
        mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
        mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
        sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
        write(scriere, text, strlen(text));

        // Incrementez count-ul
        count = count + 8;
    }
}

// Functie in care verific daca intrarea este o legatura simbolica si scriu date despre ea intr-un fisier de statistica
void esteLegaturaSimbolica(int fd, int scriere, struct stat stat_data, struct dirent *entry)
{
    if (S_ISLNK(stat_data.st_mode))
    {
        // Scriem numele si dimeniunea legaturii
        sprintf(text, "Nume fisier : %s\nDimensiune legatura : %ld\n", entry->d_name, stat_data.st_size);
        write(scriere, text, strlen(text));

        // Scriem drepurile de acces pentru user,grup si altii
        mode_t userPermissions = stat_data.st_mode & S_IRWXU;
        mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
        mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
        sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
        write(scriere, text, strlen(text));

        // Folosind fstat preluam dimeniunea fisierului target si o sriem in fisierul de statistica
        if (fstat(fd, &stat_data) == -1)
        {
            perror("Eroare\n");
            exit(-1);
        }
        sprintf(text, "Dimensiune fisier target: %ld\n", stat_data.st_size);
        write(scriere, text, strlen(text));

        // Incrementez count-ul
        count = count + 6;
    }
}

// Functie in care verific daca intrarea este un director si scriu date despre el intr-un fisier de statistica
void esteDirector(int scriere, struct stat stat_data, struct dirent *entry)
{
    if (S_ISDIR(stat_data.st_mode))
    {
        // Scriem numele si identificatorul utilizatorului(user id)
        sprintf(text, "Nume director : %s\nIdentificatorul utilizatorului fisierului este : %d\n", entry->d_name, stat_data.st_uid);
        write(scriere, text, strlen(text));
        // Scriem drepurile de acces user,grup si altii
        mode_t userPermissions = stat_data.st_mode & S_IRWXU;
        mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
        mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
        sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
        write(scriere, text, strlen(text));

        // Incrementez count-ul
        count = count + 5;
    }
}

// Functie pe care o utilizez ca sa convertesc o imagine BMP de culoare obisnuita intr-una  de culoare gri
void imageToGray(int fd, int lungime, int inaltime, uint16_t bitCount)
{
    ssize_t citire, scriereBMP; // variabile utilizate la citirea si scrierea in fisierul BMP
    unsigned char pixel[3];     // vectorul pentru pixel
    int colors, dimensiune_bmp; // prima variabila imi spune numarul culorii iar cea de-a doua variabila este declarata dimensiunea imaginii BMP

    // Verific daca bitCount-ul e <= 8 si ii atribui lui colors(echivalent din tabel e NumColors)valoarea care e data in tabelul aferent pentru fisiere BMP
    if (bitCount <= 8)
    {
        if (bitCount == 1)
        {
            colors = 1;
        }
        if (bitCount == 4)
        {
            colors = 16;
        }
        if (bitCount == 8)
        {
            colors = 256;
        }
        dimensiune_bmp = colors; // dimeniunea fisierului BMP va fi data de colors
    }
    if (bitCount > 8) // Daca bitCount-ul e mai mare decat 8,dimeniunea fisierului BMP va fi data de lungime*inaltime
    {
        dimensiune_bmp = lungime * inaltime;
    }

    // Intr-un while citesc un set de 3 bytes pe care trebuie sa ii modific
    while (dimensiune_bmp)
    {
        citire = read(fd, pixel, sizeof(pixel));
        if (citire == -1)
        {
            perror("Erroare la citirea din fisier\n");
            close(fd);
            exit(-1);
        }

        // Formula utilizata pentru convertirea in gri
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

        // Verific bitCount-ul si mai sar o pozitie prin citire
        if (bitCount <= 8)
        {
            uint8_t padding = read(fd, &padding, sizeof(padding));
        }

        dimensiune_bmp--; // Scad din dimensiunea fisierului BMP
    }
}

// Functia in care citesc din director,creez procesele si fac ceea ce e necesar
void citireDirectorScriereFisier(const char *dirpath, const char *dirOutPath)
{
    pid_t pid, pid2;         // pid-uri folosite pentru cele doua procese
    struct stat stat_data;   // structura stat utilizata pentru fisiere
    struct dirent *entry;    // structura utilizata pentru intrarea in director
    int fd = 0, scriere = 0; // file descriptor pentru deschidere si scriere
    char output_path[SIZE];  // variabila folosita pentru a formata un director in care o sa scriu fisierele de statistica

    // Pentru inceput deschidem directorul
    DIR *directory = opendir(dirpath);

    // Verificare pentru directorul deschis
    if (directory == NULL)
    {
        perror("Eroare la deschiderea directorului");
        exit(-1);
    }

    // Parcurgem directorul si efectuam verificarile necesare,cat si procesele cerute
    while ((entry = readdir(directory)) != NULL)
    {
        // Imi formatez entry-ul pe care il primesc din director ca si un path
        char path[SIZE];
        sprintf(path, "%s/%s", dirpath, entry->d_name);

        // Formatez directorul in care o sa scriu fisiere de de statistica
        sprintf(output_path, "%s/%s_statistica.txt", dirOutPath, entry->d_name);

        // Sar peste directoarele de tipul : . si ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

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

        // Incep primul procesul
        if ((pid = fork()) < 0)
        {
            perror("Eroare");
            exit(1);
        }
        if (pid == 0)
        {
            // Deschid fisierul pe care il primesc la intrare
            fd = open(path, O_RDONLY);
            if (fd == -1)
            {
                perror("Fisierul nu poate fi deschis");
                exit(-1);
            }

            // Verificam daca este un fisier BMP sau fisier normal
            esteFisier(fd, scriere, stat_data, entry, path);

            // Verificam daca este legatura simbolica ce indica sper un fisier obisnuit
            esteLegaturaSimbolica(fd, scriere, stat_data, entry);

            // Verificam daca este director
            esteDirector(scriere, stat_data, entry);

            // Inchidem dupa ficare iteratie fisierul deschis pentru preluarea datelor si cel pentru scrierea datelor
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

            // Exit-ul primeste numarul de linii scrise in fisier de statistica pentru ficare tip de fisier sau director in parte
            exit(count);
        }
        else
        {
            // Terminam primul proces
            int wait_status = 0, wait_pid = 0;
            terminareProces(wait_status, wait_pid);
        }

        // Initiem al doilea proces in care vom converti imaginea de tip BMP in tonuri de gri
        if (S_ISREG(stat_data.st_mode)) // Verific daca este fisier regular
        {
            if (strstr(path, ".bmp") != NULL) // Verific daca contine extensia BMP
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
                    uint16_t bitCount; // Variabila declarata prentru citirea bitCount-ului din tabel
                    lseek(fd, 18, SEEK_CUR);
                    read(fd, &lungime, sizeof(lungime));   // Citesc lungimea
                    read(fd, &inaltime, sizeof(inaltime)); // Citesc inaltimea
                    lseek(fd, 2, SEEK_CUR);                // Sar peste inca doi bytes ca sa ajung la bitCount
                    read(fd, &bitCount, sizeof(bitCount)); // Citesc bitCount-ul
                    lseek(fd, 24, SEEK_CUR);               // Sar cu inca 24 bytes ca sa ajung la Color Table

                    // Functia care imi transforma imaginea in una gri
                    imageToGray(fd, lungime, inaltime, bitCount);

                    // Inchidem fisierul
                    if (close(fd) == -1)
                    {
                        perror("Eroare la inchidere\n");
                        exit(-1);
                    }

                    // Pun exit code-ul pe 0
                    exit(0);
                }
                else
                {
                    int wait_status = 0, wait_pid = 0;
                    terminareProces(wait_status, wait_pid);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    // Structuri pentru verificarea daca argumentele din linia de comanda sunt directoare
    struct stat stat_dir1;
    struct stat stat_dir2;

    // Verific daca numarul argumentelor este corect
    if (argc > 3)
    {
        perror("Usage ./program <director_intrare> <director_iesire>");
        exit(-1);
    }

    // Verificare pentru primul argument
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

    // Apelez functia care imi creaza procesele si ce e necesar
    citireDirectorScriereFisier(argv[1], argv[2]);

    return 0;
}
