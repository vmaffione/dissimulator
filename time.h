/*
 *  DisSimulator - Discrete Systems simulator
 *
 *  Copyright (C) 2010  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
