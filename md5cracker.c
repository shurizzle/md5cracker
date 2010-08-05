#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <unistd.h>
#include    "md5.h"
#include    <pthread.h>
#include    <getopt.h>

#define STR_BLOCK   10
#define NTHREADS    10

char  * alphabet        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
int     alphabet_len    = 26;

const   struct option long_options[] = 
{
    {"threads", required_argument, 0, 't'},
    {"alphabet", required_argument, 0, 'a'},
    {"pass", required_argument, 0, 'p'},
    {"hash", required_argument, 0, 'H'},
    {"help", no_argument, 0, 'h'}
};

char    *  password;
int        plen    = 0;
int        threads = NTHREADS;
char    *  rightpass = NULL;
char    *  ihash = NULL;

pthread_mutex_t passres = PTHREAD_MUTEX_INITIALIZER;

#define init_pass()         password = (char *) calloc (STR_BLOCK + 1, sizeof (char));\
                            password[0] = alphabet[0];\
                            plen = 1

void    help            (const char []);
char  * strdup          (char *);
void    inc_string      (void);
void  * test_passwd     (void *);
void  * print_func      (void *);
char  * md5             (char *);
char    test_password   (char *);

int
main (int argc,
      char ** argv)
{
    int  i, c, option_index;
    pthread_t * testers;
    pthread_t printer;

    init_pass ();

    while (1)
    {
        option_index = 0;
        c = getopt_long (argc, argv, "H:a:t:p:h", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'H':
                ihash = strdup (optarg);
            break;
            case 'a':
                alphabet = strdup (optarg);
                alphabet_len = strlen (alphabet);
            break;
            case 't':
                threads = atoi (optarg);
            break;
            case 'p':
                password = strdup (optarg);
                plen = strlen (password);
            break;
            case 'h':
                help (argv[0]);
            break;
            case '?':
                exit (0);
            break;
            default:
                puts ("Options not valid, show helps with '-h' or '--help'");
                exit (0);
            break;
        }
    }

    if (!ihash)
    {
        puts ("The program require an md5 hash in input");
        exit (-1);
    }

    if (strlen (ihash) != 32)
    {
        puts ("The program require a valid md5 hash");
        exit (-1);
    }

    testers = (pthread_t *) calloc (threads, sizeof (pthread_t));

    for (i = 0; i < threads; i++)
        pthread_create (&testers[i], NULL, test_passwd, NULL);
    pthread_create (&printer, NULL, print_func, NULL);

    for (i = 0; i < threads; i++)
        pthread_join (testers[i], NULL);
    pthread_join (printer, NULL);

    printf ("\nLa password giusta è: \"%s\"\n", rightpass);

    free (password);
    free (rightpass);
    pthread_mutex_destroy (&passres);

    return 0;
}

char *
md5 (char * str)
{
    MD5_CTX md5c;
    char * hash = (char *) calloc (33, sizeof (char));
    int i;
    MD5Init (&md5c);
    MD5Update (&md5c, str, strlen (str));
    MD5Final (&md5c);

    for (i = 0; i < 16; i++)
        sprintf (hash, "%s%02x", hash, md5c.digest[i]);

    return hash;
}

char
test_password (char * str)
{
    char res = 0;
    char * hash = md5 (str);

    if (!strcmp (ihash, hash))
        res = 1;

    free (hash);

    return res;
}

void
inc_string ()
{
    int i;
    char end = 1;

    for (i = plen - 1; i >= 0; i--)
    {
        if (password[i] != alphabet[alphabet_len - 1])
        {
            password[i] = * (strchr (alphabet, password[i]) + 1);
            end = 0;
            break;
        }
    }

    if (end)
    {
        if (!(plen % STR_BLOCK))
        {
            password = (char *) realloc (password, (plen + STR_BLOCK + 1) * sizeof (char));
            memset (password + plen, 0, STR_BLOCK);
        }
        password[plen++] = alphabet[0];
        for (i = 0; i < plen; i++)
            password[i] = alphabet[0];
        return;
    }

    for (i += 1; i < plen; i++)
        password[i] = alphabet[0];
}

void *
test_passwd (void * args)
{
    char * testing = calloc (STR_BLOCK + 1, sizeof (char));
    int talloc = STR_BLOCK;
    args = NULL;

    while (rightpass == NULL)
    {
        if (test_password (testing))
        {
            rightpass = strdup (testing);
            return NULL;
        }

        pthread_mutex_lock (&passres);
        inc_string ();
        if (talloc == plen)
            testing = (char *) realloc (testing, ((talloc += STR_BLOCK) + 1) * sizeof (char));
        strcpy (testing, password);
        pthread_mutex_unlock (&passres);
    }

    free (testing);

    return NULL;
}

void *
print_func (void * args)
{
    args = NULL;
    while (rightpass == NULL)
    {
        pthread_mutex_lock (&passres);
        printf ("Last tested: %s\n", password);
        pthread_mutex_unlock (&passres);
        sleep (1);
    }
    return NULL;
}

char *
strdup (char * str)
{
    char * dup = (char *) malloc ((strlen (str) + 1) * sizeof (char));
    strcpy (dup, str);
    return dup;
}

void
help (const char pname[])
{
    puts ("MD5 Cracker, author shura, member of HUF -> https://www.hackers-uf.org/");
    puts ("Licence: GPLv3");
    printf ("\nUSAGE: %s [ARGUMENTS]\n", pname);
    puts ("");
    puts ("    --help | -h        show this help");
    puts ("--alphabet | -a        set alphabet to use");
    puts ("    --hash | -H        set hash to compare with");
    puts ("    --pass | -p        set startin password, default is \"A\"");
    puts (" --threads | -t        set number of threads, default is 10");
    puts ("");
    exit (0);
}
