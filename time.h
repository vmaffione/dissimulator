#ifndef TIME__H__
#define TIME__H__


/* Questa classe è stata introdotta allo scopo di permettere solo a Console di modificare la variabile
   che conta gli istanti di esecuzione. In particolare si è voluto impedire al modulo Block di accedervi
   in scrittura, in quanto chi scrive un nuovo blocco può al massimo avere bisogno di guardarlo in sola
   lettura. */
class Time
{
    static int K; // conta gli istanti della simulazione
    friend class Console; // permette l'accesso completo alla funzione "start" di Console
    Time();
    public:
    static unsigned int getTime() { return K; }
};

#endif
