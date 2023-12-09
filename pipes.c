// Sa se implementeze:
// A) un shell script care se apelează sub forma "bash script <c>", unde <c> este un caracter
// alfanumeric.
// Script-ul va verifica faptul că a primit exact un argument. În caz contrar, va afișa un mesaj de
// utilizare și se va încheia.
// Script-ul va citi continuu linii de la intrarea standard pana la intalnirea end-of-file (atunci când
// intrarea standard e tastatura/terminal, condiția se poate genera prin combinatia de butoane
// CTRL+D) și va contoriza numărul de “propoziții corecte” care conțin caracterul primit ca și
// argument.
// O propoziție se considera corecta dacă respectă următoarele condiții:
// - începe cu litera mare
// - conține doar litere mari, mici, cifre, spații, virgula, punct semnul exclamarii sau
// semnul întrebării
// - se termina cu semnul întrebării, semnul exclamarii sau punct
// - nu contine virgula (,) înainte de și
// La final va afișa acest contor pe ecran (doar valoarea numerică).
// B) Să se modifice programul C din saptamana anterioara astfel incat va primi un al treilea
// argument <c> ce reprezinta un caracter alfanumeric și prin urmare programul se va apela astfel:
// ./program <director_intrare> <director_iesire> <c>
// In saptamana precedenta a fost cerută următoarea functionalitate: La parcurgerea
// directorului, în cazul în care o intrare curentă este un fișier obișnuit, dar nu o imagine BMP,
// procesul părinte creeaza un proces ce va scrie informații despre acest fișier într-un fișier de
// statistica din directorul de ieșire.
// Pe langa acest proces fiu, pentru categoria fișierelor obișnuite care nu au extensia .bmp,
// părintele va mai crea cate un proces fiu.
// Tot pentru categoria fișierelor obișnuite care nu au extensia .bmp, după ce procesul fiu
// responsabil cu fișierele de statistica a scris datele in fișierul corespunzător din directorul de
// ieșire, acest proces va trebui sa genereze folosind o comanda conținutul fișierului obișnuit
// fără extensie .bmp pentru care a extras informațiile statistice și va trimite acest conținut
// celuilalt proces fiu creat de părinte.
// Cel de-al doilea proces fiu creat de părinte pentru fiecare fișier obișnuit fără extensia .bmp
// va calcula, din secvența primită de la procesul fiu responsabil cu generarea conținutului
// fișierului, numărul de “propoziții corecte” care conțin caracterul <c> primit ca și argument de
// program și va transmite acest număr părintelui printr-un pipe. În acest scop, acest proces va
// apela script-ul de la punctul A).
// Părintele va afișa pe ecran suma tuturor acestor numere primite sub forma unui mesaj: ”Au
// fost identificate in total <n> propozitii corecte care contin
// caracterul <c>”,unde <n> reprezinta numarul de propozitii corecte, iar <c> reprezinta
// caracterul primit de program ca argument.
// Toate procesele fiu create, precum și procesul părinte vor rula obligatoriu în paralel.
// La încheierea fiecărui proces fiu, părintele îi va prelua starea și va afișa un mesaj de forma
// ”S-a încheiat procesul cu pid-ul <pid> și codul <cod>”, unde <pid>
// reprezinta PID-ul procesului fiu care s-a incheiat, iar <cod> reprezinta codul cu care acesta s-a incheiat.

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

// Functie folosita pentru a inchide procesul fiului si pentru a se executa si procesul parintelui.Folosesc wait() pentru a incheia de tot procesul
void terminareProces(int status, int wait_pid, int pid)
{
    while ((wait_pid = waitpid(pid, &status, 0)) > 0)
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

// Functie in care verific daca intrarea este un fisier BMP si scriu date despre el intr-un fisier de statistica
void esteFisierBMP(int fd, int scriere, struct stat stat_data, struct dirent *entry, const char *path)
{
    if (S_ISREG(stat_data.st_mode)) // Verific daca e regular file
    {
        // Verific daca se termina cu extensia ".bmp"
        if (strstr(path, ".bmp") != NULL)
        {
            // Incep sa scriu datele necesare in fisier
            sprintf(text, "Nume fisier : %s\n", entry->d_name);
            write(scriere, text, strlen(text));

            // Cu ajutoul functiei lseek,setam indicatorul pozitiei cu un anumit numar de octeti pentru a citi din fisier exact datele de care am nevoie
            // Trebuie sa extragem lungimea fisierului asa ca setez indicatorul pozitiei la 18 bytes si ajung la pozitia dorita
            lseek(fd, 18, SEEK_SET);
            read(fd, &lungime, sizeof(lungime));
            sprintf(text, "Inaltimea fisierului BMP este : %d\n", lungime);
            write(scriere, text, strlen(text));
            // Setez indicatorul pentru a citi lungimea fisierului
            lseek(fd, 22, SEEK_SET);
            read(fd, &inaltime, sizeof(inaltime));
            sprintf(text, "Lungimea fisierului BMP este : %d\n", inaltime);
            write(scriere, text, strlen(text));

            // Scriem in fisierul creat dimensiunea fisierului,identificatorul utilizatorului (user id),timpul ultimei modificari,controlul de legaturi
            sprintf(text, "Dimensiunea fisierului este : %ld\nIdentificatorul utilizatorului fisierului este : %d\nTimpul ultimei modificari fisierului este : %sControul de legaturi a fisierului este : %ld\n", stat_data.st_size, stat_data.st_uid, ctime(&stat_data.st_mtime), stat_data.st_nlink);
            write(scriere, text, strlen(text));

            // Scriem drepurile de acces pentru user,grup si altii
            mode_t userPermissions = stat_data.st_mode & S_IRWXU;
            mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
            mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
            sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
            write(scriere, text, strlen(text));

            // Incrementez count-ul
            count = count + 10;
        }
    }
}

// Functie pe care o aplic pentru un fisier simplu,fara extensia ".bmp", si scriu datele despre el in fisierul de statistica
void esteFisier(int fd, int scriere, struct stat stat_data, struct dirent *entry, const char *path)
{
    // Incep sa scriu datele necesare in fisier
    sprintf(text, "Nume fisier : %s\n", entry->d_name);
    write(scriere, text, strlen(text));

    // Scriem in fisierul creat dimensiunea fisierului,identificatorul utilizatorului (user id),timpul ultimei modificari,controlul de legaturi
    sprintf(text, "Dimensiunea fisierului este : %ld\nIdentificatorul utilizatorului fisierului este : %d\nTimpul ultimei modificari fisierului este : %sControul de legaturi a fisierului este : %ld\n", stat_data.st_size, stat_data.st_uid, ctime(&stat_data.st_mtime), stat_data.st_nlink);
    write(scriere, text, strlen(text));

    // Scriem drepurile de acces pentru user,grup si altii
    mode_t userPermissions = stat_data.st_mode & S_IRWXU;
    mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
    mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
    sprintf(text, "Drepurile de acces ale user-ului fisierului sunt : %c%c%c\nDrepurile de acces ale grupuli in fisierul sunt : %c%c%c\nDrepurile de acces ale altora in fisierul sunt : %c%c%c\n", (userPermissions & S_IRUSR) ? 'R' : '-', (userPermissions & S_IWUSR) ? 'W' : '-', (userPermissions & S_IXUSR) ? 'X' : '-', (groupPermissions & S_IRGRP) ? 'R' : '-', (groupPermissions & S_IWGRP) ? 'W' : '-', (groupPermissions & S_IXGRP) ? 'X' : '-', (othersPermissions & S_IROTH) ? 'R' : '-', (othersPermissions & S_IWOTH) ? 'W' : '-', (othersPermissions & S_IXOTH) ? 'X' : '-');
    write(scriere, text, strlen(text));

    // Incrementez count-ul
    count = count + 8;
}

// Functie in care verific daca intrarea este o legatura simbolica si scriu date despre ea intr-un fisier de statistica
void esteLegaturaSimbolica(int fd, int scriere, struct stat stat_data, struct dirent *entry)
{
    if (S_ISLNK(stat_data.st_mode))
    {
        // Scriem nume si dimeniunea legaturii
        sprintf(text, "Nume fisier : %s\nDimensiune legatura : %ld\n", entry->d_name, stat_data.st_size);
        write(scriere, text, strlen(text));

        // Scriem drepurile de acces pentru user,grup si altii
        mode_t userPermissions = stat_data.st_mode & S_IRWXU;
        mode_t groupPermissions = stat_data.st_mode & S_IRWXG;
        mode_t othersPermissions = stat_data.st_mode & S_IRWXO;
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

// Functie in care verific daca intrarea este un director si scriu date despre el intr-un fisier de statistica
void esteDirector(int scriere, struct stat stat_data, struct dirent *entry)
{
    if (S_ISDIR(stat_data.st_mode))
    {
        // Scriem numele si identificatorul utilizatorului(user id)
        sprintf(text, "Nume director : %s\nIdentificatorul utilizatorului fisierului este : %d\n", entry->d_name, stat_data.st_uid);
        write(scriere, text, strlen(text));
        // Scriem drepurile de acces pentru user,grup si altii
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
    int colors, dimensiune_bmp; // prima variabila imi spune numarul culorii iar cea de-a doua variabila este declarata dimensiunea imaginii bmp

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
void citireDirectorScriereFisier(const char *dirpath, const char *dirOutPath, char *argument)
{
    pid_t pid, pid2;              // pid-uri folosite pentru cele doua procese
    pid_t pid_file1, pid_file2;   // pid-uri folosite pentru cele doua procese de la fisiere obisnuite
    int pipe_fd1[2], pipe_fd2[2]; // Cele doua pipe-uri pe care le folosec pentru comunicarea intre procese
    struct stat stat_data;        // structura stat utilizata pentru fisiere
    struct dirent *entry;         // structura utilizata pentru intrarea in director
    int fd = 0, scriere = 0;      // file descriptor pentru deschidere si scriere
    char output_path[SIZE];       // variabila folosita pentru a formata un director in care o sa scriu fisierele de statistica

    // Pentru inceput deschidem directorul
    DIR *directory = opendir(dirpath);

    // Verificare pentru director
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

        // Formatez directorul in care o sa scriu fisiere de de statistica
        sprintf(output_path, "%s/%s_statistica.txt", dirOutPath, entry->d_name);

        // Sar peste directoarele de tipul : . si ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Deschid fisierul de intrare
        fd = open(path, O_RDONLY);
        if (fd == -1)
        {
            perror("Fisierul nu poate fi deschis");
            exit(-1);
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

        // Creez primul pipe
        if (pipe(pipe_fd1) == -1)
        {
            perror("Eroare la crearea pipe-ului!\n");
            exit(-1);
        }

        // Creez al doilea pipe
        if (pipe(pipe_fd2) == -1)
        {
            perror("Eroare la crearea pipe-ului!\n");
            exit(-1);
        }

        // Verific daca fisierul de intare(path-ul) este un fisier regular si are extensia BMP,daca etse un link sau daca este un director
        if ((S_ISREG(stat_data.st_mode) && strstr(path, ".bmp") != NULL) || (S_ISDIR(stat_data.st_mode)) || (S_ISLNK(stat_data.st_mode)))
        {
            // Incep primul proces
            if ((pid = fork()) < 0)
            {
                perror("Eroare");
                exit(1);
            }
            if (pid == 0)
            {
                // Inchid capetele nefolosite din pipe-uri
                close(pipe_fd1[0]);
                close(pipe_fd1[1]);
                close(pipe_fd2[1]);
                close(pipe_fd2[0]);

                // Verificam daca este un fisier BMP
                esteFisierBMP(fd, scriere, stat_data, entry, path);

                // Verificam daca este legatura simbolica ce indica sper un fisier obisnuit
                esteLegaturaSimbolica(fd, scriere, stat_data, entry);

                // Verificam daca este director
                esteDirector(scriere, stat_data, entry);

                // Exit-ul primeste numarul de linii scrise in fisier de statistica pentru ficare tip de fisier sau director in parte
                exit(count);
            }
            else
            {
                // Terminam primul proces
                int wait_status = 0, wait_pid = 0;
                terminareProces(wait_status, wait_pid, pid);
            }
        }

        // Initiem al doilea proces in care vom converi imaginea de tip bmp in tonuri de gri
        if (S_ISREG(stat_data.st_mode))
        {
            if (strstr(path, ".bmp") != NULL)
            {
                close(fd); // Inchid fisierul care este deschis doar pentru citire

                // Deschidem fisierul atat pentru scriere cat si pentru citire
                fd = open(path, O_RDWR);
                if (fd == -1)
                {
                    perror("Fisierul nu poate fi deschis");
                    exit(-1);
                }

                // Incep procesul
                if ((pid2 = fork()) < 0)
                {
                    perror("Eroare");
                    exit(1);
                }
                if (pid2 == 0)
                {
                    // Inchid capetele nefolosite din pipe-uri
                    close(pipe_fd1[0]);
                    close(pipe_fd1[1]);
                    close(pipe_fd2[1]);
                    close(pipe_fd2[0]);

                    uint16_t bitCount; // Variabila declarata prentru citirea bitCount-ului din tabel
                    lseek(fd, 18, SEEK_CUR);
                    read(fd, &lungime, sizeof(lungime));   // Citesc lungimea
                    read(fd, &inaltime, sizeof(inaltime)); // Citesc inaltimea
                    lseek(fd, 2, SEEK_CUR);                // Sar peste inca doi bytes ca sa ajung la bitCount
                    read(fd, &bitCount, sizeof(bitCount)); // Citec bitCount-ul
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
                    int wait_status1 = 0, wait_pid1 = 0;
                    terminareProces(wait_status1, wait_pid1, pid2);
                }
            }
        }

        // Verific daca este fisier obisnuit si nu are extensia .bmp si fac doua procese pentru ele
        if (S_ISREG(stat_data.st_mode))
        {
            if (strstr(path, ".bmp") == NULL)
            {
                // Incep primul proces
                if ((pid_file1 = fork()) < 0)
                {
                    perror("Eroare");
                    exit(1);
                }
                if (pid_file1 == 0)
                {
                    // Inchid capetele nefolosite din pipe-uri
                    close(pipe_fd1[0]);
                    close(pipe_fd2[1]);
                    close(pipe_fd2[0]);

                    // Apelez functia care imi scrie datele fisierului obisnuit in statistica
                    esteFisier(fd, scriere, stat_data, entry, path);

                    // Redirectionez stdout pentru a scrie din pipe
                    dup2(pipe_fd1[1], 1);
                    close(pipe_fd1[1]); // Inchid capat-ul

                    // Ma folosesc de execpl pentru a prelua continutul din fisier
                    execlp("cat", "cat", path, NULL);
                    perror("Erroare execlp");

                    // Pun exit code-ul pe 0
                    exit(0);
                }

                // Urmatorul proces pentru fisiere simple
                if ((pid_file2 = fork()) < 0)
                {
                    perror("Eroare");
                    exit(1);
                }
                if (pid_file2 == 0)
                {
                    // Inchid capetele nefolosite din pipe-uri
                    close(pipe_fd1[1]);
                    close(pipe_fd2[0]);

                    dup2(pipe_fd1[0], 0); // Redirectionez intrarea standard pentru capatul de citire din primul pipe
                    close(pipe_fd1[0]);   // Inchid capatul
                    dup2(pipe_fd2[1], 1); // Redirectionez iesirea standard pentru capatul de scriere din al doilea pipe
                    close(pipe_fd2[1]);   // Inchid capatul

                    // Execut scriptul pentru a verifica datele primite
                    execlp("bash", "bash", "script_proiect.sh", argument, NULL);

                    // Pun exit code-ul pe 0
                    exit(0);
                }

                // Inchid capetele nefolosite din pipe-uri
                close(pipe_fd1[0]);
                close(pipe_fd1[1]);
                close(pipe_fd2[1]);

                // Redirectez intrarea standard din al doilea pipe pentru a citi numarul de propozitii corecte dat in urma rularii scriptului
                dup2(pipe_fd2[0], 0);

                char buff[SIZE]; // Buffer utilizat pentru citirea din pipe

                int nr_prop = read(pipe_fd2[0], buff, sizeof(buff)); // Citesc numarul de propozitii din pipe
                if (nr_prop == -1)                                   // Verificare pentru citire
                {
                    perror("Eroare la citire\n");
                    exit(-1);
                }
                close(pipe_fd2[0]); // Inchid capatul

                // Afisaare
                printf("Au fost identificate in total %d propozitii corecte care contin caracterul %s\n", atoi(buff), argument);

                // Terminam cele doua procese
                int wait_status2 = 0, wait_pid2 = 0;
                int wait_status3 = 0, wait_pid3 = 0;
                terminareProces(wait_status2, wait_pid2, pid_file1);
                terminareProces(wait_status3, wait_pid3, pid_file2);
            }
        }

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
    }
}

int main(int argc, char **argv)
{
    // Structuri pentru verificarea daca argumentele din linia de comanda sunt directoare
    struct stat stat_dir1;
    struct stat stat_dir2;

    // Verific daca numarul argumentelor este corect
    if (argc > 4)
    {
        perror("Usage ./program <director_intrare> <director_iesire> <c>");
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

    // Apelez functia care primeste ca parametru cele trei argumnte din linia de comanda si imi creaza procesele si ceea ce e necesar
    citireDirectorScriereFisier(argv[1], argv[2], argv[3]);

    return 0;
}