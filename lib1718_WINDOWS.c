#define _CRT_SECURE_NO_WARNINGS
#define MAX_LEN 128     //lunghezza massima di una stringa in generale
#define MAX_ROW 1024   //lunghezza massima di una riga della tabella

#include "lib1718.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <io.h>
#include <wchar.h>

bool executeQuery(char* query) {    //funzione base che deve essere richiamata dal main

	int columns = 10;   //grandezza base colonne
	char tmpQuery[MAX_LEN]; //crea una copia della stringa poiche strtok modifica la stringa che tokenizza
	char *end_col;  //serve a capire quale stringa tokenizzare in caso di due strtok su stringhe diverse (come end_control in INSERT)
	strcpy(tmpQuery, query); //copia la query in tmpQuery
	char *ptr = NULL;   //puntatore ad ogni elemento splittato
	ptr = strtok_s(tmpQuery, " ", &end_col); //splitta la stringa iniziale per capire che tipo di query deve eseguire

	if (strcmp(ptr, "CREATE") == 0) {  //CREATE TABLE (Tempo O(n colonne))

		char nome[MAX_LEN], **colonne = NULL; //nome = nome tabella, colonne = nomi colonne
		int num_colonne = 0, nome_lenght = 0; //num_colonne indica il numero delle colonne della tabella, nome_lenght indica la lunghezza del nome della tabella

		ptr = strtok_s(NULL, " ", &end_col); //salta a TABLE
		if (strcmp(ptr, "TABLE") != 0) //controlla che sia scritto bene
			return false;

		ptr = strtok_s(NULL, " ", &end_col); //salta al nome della tabella
		strcpy(nome, ptr);  //lo copia in name
		nome_lenght = (int)strlen(ptr); //lunghezza stringa

		ptr = strtok_s(NULL, ",", &end_col); //salta alle colonne
		if (ptr == NULL)
			return false;

		if (ptr[0] != '(') //termina se non è presente la parentesi di inizio colonne
			return false;

		removeCharacter(ptr, 0); //rimuove la parentesi iniziale
		if ((colonne = (char **)calloc(sizeof(char **), columns)) == NULL) //inizializza la matrice di char delle colonne e controlla che sia andata a buon fine
			return false;

		while (ptr != NULL) { //per ogni colonna

			if (num_colonne == columns) {
				columns *= 2;   //raddoppia la dimensione se deve riallocare
				if ((colonne = (char **)realloc(colonne, sizeof(char **) * columns)) == NULL) //se ha finito le colonne rialloca e controlla che sia andata a buon fine
					return false;
			}

			if ((colonne[num_colonne] = (char *)calloc(sizeof(char *), MAX_LEN)) == NULL) //alloca la riga della matrice e controlla che sia andata a buon fine
				return false;

			strcpy(colonne[num_colonne++], ptr); //e copia il nome della colonna
			ptr = strtok_s(NULL, ",", &end_col); //passa alla colonna sucessiva
		}
		if (verifyLastParentesis(colonne[num_colonne - 1]) == 0) //verifica che sia presente l' ultima parentesi
			return false;

		removeCharacter(colonne[num_colonne - 1], 1); //rimuove l' ultima parentesi dell' ultimo campo
		if (colonne[num_colonne - 1][0] == '\0') //se da errore termina
			return false;

		return createTable(nome, colonne, num_colonne, nome_lenght);  //chiama createTable e ritorna il valore (1=success, 0=fail)
	}
	else if (strcmp(ptr, "INSERT") == 0) {      //INSERT (Tempo O(n colonne))

		char nome[MAX_LEN], **dati = NULL;  //nome indica il nome della tabella, dati i dati da inserire
		int nome_lenght = 0;  //nome_lenght = lunghezza nome tabella

		ptr = strtok_s(NULL, " ", &end_col);  //salta a INTO
		if (strcmp(ptr, "INTO") != 0)            //controlla che INTO sia scritto correttamente
			return false;
		ptr = strtok_s(NULL, " ", &end_col);  //salta al nome della tabella

		char *fileName = NULL, *end_control;  //fileName = nome del file da aprire, end_control = utile per strtok_s(vedi end_col)
		if ((fileName = (char *)calloc(sizeof(char), MAX_LEN)) == NULL)  //nome del file da ricercare e controlla che sia andata a buon fine
			return false;

		fileName = aggiungiTxt(ptr, (int)strlen(ptr)); //viene assegnato a filename il nome del file compreso txt
		FILE *file; //ptr al file
		if ((file = fopen(fileName, "r")) == NULL)    //apro il file in lettura
			return false;

		char **colonneFile = NULL, stringaSplit[MAX_LEN]; //colonneFile = colonne lette dal file da confrontare con quelle della query, stringaSplit = riga di dichiarazione tabella del file

		if ((colonneFile = (char **)calloc(sizeof(char **), columns)) == NULL) //inizializzo colonneFile e controllo che sia andata a buon fine
			return false;

		fscanf(file, "%[^\n]", stringaSplit);    //legge la riga di dichiarazione  della tabella dal file
		char *ptrStringa = NULL;    //ptrStringa è il ptr in cui vanno le splittature della stringa

		ptrStringa = strtok_s(stringaSplit, " ", &end_control); //prima splittatura che prende table
		ptrStringa = strtok_s(NULL, " ", &end_control);     //prende il nome della tabella
		ptrStringa = strtok_s(NULL, " ", &end_control);     //passa a columns
		ptrStringa = strtok_s(NULL, ",;", &end_control);    //splitta e arriva ai nomi delle colonne

		int num_campi = 0; //num_campi è il numero delle colonne

		while (ptrStringa != NULL) {     //legge tutte le colonne del file
			colonneFile[num_campi] = (char *)calloc(sizeof(char *), MAX_LEN); //inizializza ogni riga di colonnefile
			if (colonneFile[num_campi] == NULL)
				return false;
			strcpy(colonneFile[num_campi++], ptrStringa); //e le copia in colonnefile
			ptrStringa = strtok_s(NULL, ",;", &end_control); //splitta la sucessiva colonna
		}
		fclose(file); //chiude il file letto

		strcpy(nome, ptr);  //copia in nome il nome della tabella
		nome_lenght = (int)strlen(ptr); //lunghezza del nome della tabella
		ptr = strtok_s(NULL, ",(", &end_col);

		for (int i = 0; i < num_campi; i++) {        //controlla che i nomi della tabella corrispondano a quelli della query
			if (strcmp(colonneFile[i], ptr) != 0)
				return false; //se non corrispondono ritorna errore
			if (i != num_campi - 1)
				ptr = strtok_s(NULL, ",)", &end_col);
		}

		ptr = strtok_s(NULL, " ", &end_col);
		ptr = strtok_s(NULL, ",", &end_col);
		if (ptr == NULL)
			return false;

		if (ptr[0] != '(') //termina se non è presente la parentesi di inizio colonne
			return false;

		removeCharacter(ptr, 0); //rimuove la parentesi iniziale dal primo campo
		if ((dati = (char **)calloc(sizeof(char **), columns)) == NULL) //inizializza la matrice di char delle colonne
			return false;

		for (int i = 0; i < num_campi; i++) { //inserisce i dati da inserire nel file nella tabella dati
			if (i == columns) {
				columns *= 2;
				if ((dati = (char **)realloc(dati, sizeof(char **) * columns)) == NULL) //se ha finito le colonne rialloca
					return false;
			}

			if ((dati[i] = (char *)calloc(sizeof(char*), MAX_LEN)) == NULL) //alloca la riga della matrice
				return false;
			if (ptr == NULL)	//se sono presenti meno dati rispetto al numero delle colonne ritorna errore
				return false;
			strcpy(dati[i], ptr); //inserisce il campo da inserire nella colonna
			ptr = strtok_s(NULL, ",(", &end_col);
		}

		if (verifyLastParentesis(dati[num_campi - 1]) == 0) //controlla che sia presente l'ultima parentesi
			return false;

		removeCharacter(dati[num_campi - 1], 1); //rimuove l ultima parentesi
		if (dati[num_campi - 1][0] == '\0') //controlla che l ultimo non sia null (è null se l ultima splittatura ha splittato solo una parentesi)
			return false;

		return insert(nome, dati, num_campi, nome_lenght); //chiama insert e ritorna il risultato della chiamata
	}
	else if (strcmp(ptr, "SELECT") == 0) {   //SELECT (Tempo O(n colonne))

		int columns = 10; //numero di colonne del vettore colonne
		ptr = strtok_s(NULL, " ,", &end_col);

		char **colonne = NULL, tabella[MAX_LEN]; //colonne indica gli eventuali campi della select (* = NULL), tabella = nome della tabella
		int num_colonne = 0, nome_lenght = 0; //num_colonne = numero colonne della select, nome_lenght = lunghezza del nome della tabella

		if (strcmp(ptr, "*") == 0) { //se SELECT *

			ptr = strtok_s(NULL, " ", &end_col); //splitta FROM
			if (strcmp(ptr, "FROM") != 0)        //controlla che FROM sia scritto correttamente
				return false;
			ptr = strtok_s(NULL, " ", &end_col);
			strcpy(tabella, ptr);           //copia il nome della tabella in tabella
			nome_lenght = (int)strlen(ptr); //e la sua lunghezza
		}
		else { //se SELECT Colonna,Colonna,....

			if ((colonne = (char **)calloc(sizeof(char **), columns)) == NULL) //inizializza la tabella delle colonne della select
				return false;

			while (strcmp(ptr, "FROM") != 0 && ptr != NULL) {     //splitta finche non arriva a from
				if (num_colonne == columns) {        //se è terminato lo spazio rialloca il vettore colonne
					columns *= 2;
					if ((colonne = (char **)realloc(colonne, sizeof(char **) * columns)) == NULL)
						return false;
				}

				if ((colonne[num_colonne] = (char *)calloc(sizeof(char *), MAX_LEN)) == NULL)    //alloca la nuova riga in cui copiare la nuova colonna
					return false;
				strcpy(colonne[num_colonne++], ptr); //copia in colonne la colonna
				ptr = strtok_s(NULL, " ,", &end_col);
			}

			ptr = strtok_s(NULL, " ", &end_col);
			strcpy(tabella, ptr);//copia il nome della tabella in tabella
			nome_lenght = (int)strlen(ptr); //e la sua lunghezza
		}

		ptr = strtok_s(NULL, " ", &end_col); //splitta per vedere quale tipo di filtro bisogna performare
		if (ptr == NULL)        //se non sono presenti filtri
			return selectNoFilter(query, colonne, tabella, nome_lenght, num_colonne);    //chiama la funzione selectNoFilter

		if (strcmp(ptr, "WHERE") == 0) {     //se è presente il filtro where
			char campo[MAX_LEN], operando[MAX_LEN], parametro[MAX_LEN];     //campo = colonna su cui si filtra, operando = > < >= <= == !=, parametro = valore del filtro

			ptr = strtok_s(NULL, " ,", &end_col);
			strcpy(campo, ptr);   //copia campo
			ptr = strtok_s(NULL, " ,", &end_col);
			strcpy(operando, ptr);  //copia operando
			ptr = strtok_s(NULL, " ,", &end_col);
			if (ptr == NULL)
				return false;
			strcpy(parametro, ptr);     //copia parametro

			return selectWhere(query, colonne, tabella, nome_lenght, num_colonne, campo, operando, parametro); //chiama selectWhere
		}
		else if (strcmp(ptr, "ORDER") == 0) {  //se è presente il filtro ORDER BY
			char campo[MAX_LEN], tipo[MAX_LEN];  //campo = campo su cui si ordina, tipo = ASC DESC

			ptr = strtok_s(NULL, " ", &end_col);  //controlla che BY sia scritto bene
			if (strcmp(ptr, "BY") != 0)
				return false;

			ptr = strtok_s(NULL, " ,", &end_col);
			strcpy(campo, ptr);    //copia campo

			ptr = strtok_s(NULL, " ", &end_col);
			if (ptr == NULL)
				return false;
			strcpy(tipo, ptr);     //copia tipo

			if (strcmp(tipo, "ASC") != 0 && strcmp(tipo, "DESC") != 0)       //controlla che non siano presenti piu di un campo da ordinare
				return false;

			return selectOrder(query, colonne, tabella, nome_lenght, num_colonne, campo, tipo);   //chiama selectOrder
		}
		else if (strcmp(ptr, "GROUP") == 0) {    //se è presente il filtro GROUP BY
			if (colonne == NULL)		//deve per forza esserci la colonna e deve essere uguale a quella che si raggruppa
				return false;
			
			char campo[MAX_LEN];    //campo = campo su cui si ordina

			ptr = strtok_s(NULL, " ", &end_col);
			if (strcmp(ptr, "BY") != 0)      //controlla che BY sia scritto bene
				return false;

			ptr = strtok_s(NULL, " ,", &end_col);
			strcpy(campo, ptr);     //copia campo

			ptr = strtok_s(NULL, " ,", &end_col);  //verifica che sia presente soltanto un campo su cui fare GROUP BY

			if (strcmp(colonne[0], campo) != 0 || num_colonne > 1 || ptr != NULL) //se la colonne della SELECT è diversa da quella della GROUP BY, se il numero delle colonne della select è piu di 1 o se i campi della GROUP BY sono piu di 1
				return false;

			return selectGroup(query, colonne, tabella, nome_lenght, num_colonne, campo);        //chiama selectGroup
		}
	}
	return false; //se la prima parola non è CREATE INSERT o SELECT
}

//RIMUOVE L'ULTIMA PARENTESI DALL ULTIMA COLONNA/CAMPO (Tempo O(lunghezza stringa))
void removeCharacter(char *string, int firstLast) {  //string è la stringa da controllare, firstLast è il tipo di cancellazione che si vuole fare
	if (string == NULL) //se la stringa è nulla termina
		return;
	int i = 0;
	if (firstLast == 1) {    //se firstlast è 1 rimuove l ultimo
		while (string[i] != '\0')
			i++;
		string[i - 1] = '\0';
	}
	else {
		while (string[i] != '\0') { //altrimenti rimuove il primo
			string[i] = string[i + 1];
			i++;
		}
	}
}

//VERIFICA CHE SIA PRESENTE L ULTIMA PARENTESI (Tempo O(lunghezza stringa))
bool verifyLastParentesis(char *string) {       //string è la stringa da controllare
	if (string == NULL)  //se la stringa è nulla ritorna 0
		return false;

	int i = 0;
	while (string[i + 1] != '\0') {
		i++;
	}
	return string[i] == ')'; //se l ultimo carattere è ) ritorna true altrimenti false
}

//AGGIUNGE .TXT A STRINGA E LA RITORNA IN OUTPUT (Tempo O(1))
char * aggiungiTxt(char *string, int lenght) { //string è il nome della tabella, lenght la lunghezza
	char *fileName;
	if ((fileName = (char *)calloc(sizeof(char), lenght + 4)) == NULL) //il +4 Ë per scrivere .txt
		return NULL;

	strcpy(fileName, string);  //copia il nome della tabella
	strcpy(fileName + lenght, ".txt");   //e .txt alla fine
	return fileName;   //e ritorna il nome completo
}

//CONTROLLA CHE STRINGA SIA PRESENTE IN COLONNE Tempo O(n colonne)
bool isInColonne(char *string, char **colonne, int num_colonne) {  //string è la colonna da ricercare, colonne il vettore delle colonne e num_colonne il numero delle colonne
	for (int i = 0; i < num_colonne; i++) {
		if (strcmp(string, colonne[i]) == 0)    //se la colonna è presente ritorna true
			return true;
	}
	return false;       //altrimenti false
}

//FUNZIONE CHE LEGGE TUTTE LE RIGHE DAL FILE (Tempo O(n righe * n colonne))
char*** readFromFile(char *nome_tabella, int *righe, int *colonne) {    //nome tabella = nome della tabella da cui leggere i dati, righe = variabile passata by reference in cui viene copiato il numero delle righe, colonne = uguale a righe ma numero colonne
	int rows = -1, columns = 0; //rows parte da -1 perche la prima riga è la dichiarazione della tabella
	int dimColonne = 10; //numero di colonne per ogni riga
	char ***dati;   //dati è la stuttura dati che conterrà tutte le righe della tabella

	FILE *file;
	if ((file = fopen(nome_tabella, "r")) == NULL)   //apre il file e controlla che sia andato a buon fine
		return NULL;

	char temp[MAX_ROW];     //temp contiene la riga che viene letta ogni volta dal file
	while (fgets(temp, MAX_ROW, file) != NULL)   //conta quante righe sono presenti nel file in modo da allocare le righe di dati una sola volta
		rows++;
	rewind(file);

	if ((dati = (char ***)calloc(sizeof(char ***), rows)) == NULL)   //inizializza la struttura dati e controlla che sia andata a buon fine
		return NULL;

	fgets(temp, MAX_ROW, file); //fa una prima lettura per saltare la riga di inizializzazione della tabella

	for (int i = 0; i < rows; i++) { //cicla per il numero di righe
		fgets(temp, MAX_ROW, file);   //legge la nuova riga
		columns = 0;        //columns contiene il numero delle colonne
		if ((dati[i] = (char **)calloc(sizeof(char **), dimColonne)) == NULL)     //inizializza la nuova riga della struttura dati
			return NULL;
		char *ptr = NULL;   //puntatore ad ogni colonna splittato
		ptr = strtok(temp, " "); //splitta la stringa per saltare ROW

		while ((ptr = strtok(NULL, ",;")) != NULL && strcmp(ptr, "\n") != 0) { //legge colonna per colonna finche non terminano
			if (columns == dimColonne) { //rialloca la riga se le colonne non bastano
				dimColonne *= 2;
				if ((dati[i] = (char **)realloc(dati[i], sizeof(char **) * dimColonne)) == NULL)
					return NULL;
			}
			if ((dati[i][columns] = (char *)calloc(sizeof(char *), MAX_LEN)) == NULL)  //inizializza la cella della nuova colonna
				return NULL;
			strcpy(dati[i][columns++], ptr);    //copia la colonna nella cella di dati
		}
	}

	*colonne = columns; //infine passa al chiamante il numero delle colonne
	*righe = rows;      //e il numero di righe

	return dati;  //e ritorna dati
}

//FUNZIONE CREATE TABLE Tempo O(n colonne)
bool createTable(char *nome_tabella, char **colonne, int num_colonne, int nome_lenght) {  //funzione che prende in input il nome della tabella, le colonne, il numero delle colonne e la lunghezza del nome della tabella e crea il file con la prima riga "TABLE ******** COLUMNS ***************"

	FILE *file;     //puntatore al file
	char *fileName; //nome del file

	if ((fileName = aggiungiTxt(nome_tabella, nome_lenght)) == NULL)   //viene aggiunto .txt al nome del file tramite la funzione aggiungiTxt
		return false;
	if ((file = fopen(fileName, "w")) == NULL)  //viene aperto il file in scrittura e si controlla che sia stato aperto correttamente
		return false;
	else {
		fprintf(file, "TABLE %s COLUMNS ", nome_tabella); //si stampa nel file TABLE ***** COLUMNS
		if (num_colonne == 0)   //controlla che le colonne non siano NULL (per sicurezza ma credo sia superfluo)
			return false;

		for (int i = 0; i < num_colonne; i++) {  //stampa sul file tutte le colonne
			fprintf(file, "%s", colonne[i]);
			if (i != num_colonne - 1)
				fprintf(file, ",");
		}
		fprintf(file, ";\n");
		fclose(file);   //chiude il file
		for(int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di colonne
			free(colonne[i]);
		free(colonne);
		return true;    //ritorna true se tutto è andato correttamente
	}
}


//FUNZIONE INSERT Tempo O(n colonne)
bool insert(char *tabella, char **dati, int num_colonne, int nome_lenght) {    //funzione che prende in input il nome della tabella, i dati, il numero dei dati e la lunghezza del nome della tabella e crea il file con la prima riga "TABLE ******** COLUMNS ***************"

	FILE *file; //puntatore al file
	char *fileName;     //stringa che conterrà il nome del file
	if ((fileName = aggiungiTxt(tabella, nome_lenght)) == NULL)  //aggiunge txt al file e controlla che sia andata a buon fine
		return false;

	if ((file = fopen(fileName, "a")) == NULL)  //apre il file in append e controlla che sia corretto
		return false;

	fprintf(file, "ROW ");      //stampa manualmente ROW nel file

	for (int i = 0; i < num_colonne; i++) { //stampa i dati nel file
		fprintf(file, "%s", dati[i]);
		if (i != num_colonne - 1)
			fprintf(file, ",");
	}
	fprintf(file, ";\n");
	fclose(file);       //chiude il file
	for (int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di dati
		free(dati[i]);
	free(dati);
	return true;        //ritorna true se tutto è andato correttamente
}

//FUNZIONE SELECT SENZA NESSUN FILTRO APPLICATO (Tempo O(n righe * n colonne))  //costo determinato da readFromFile
bool selectNoFilter(char *query, char **colonne, char *tabella, int nome_lenght, int num_colonne) {    //query = query testuale, colonne = lista colonne della select (se NULL la select ha *), tabella = nome della tabella su cui si fa la select, nome_lenght = lunghezza nome tabella, num_colonne = numero di colonne della select

	FILE *file_input, *file_output; //file_input = tabella, file_output = query_result
	char *fileName = aggiungiTxt(tabella, nome_lenght);  //aggiunge txt al nome della tabella per aprire il file
	if ((file_input = fopen(fileName, "r")) == NULL)        //apre il file della tabella
		return false;

	if ((file_output = fopen("query_results.txt", "a")) == NULL)    //apre query result
		return false;

	fprintf(file_output, "%s;\n", query);   //stampa la query nel file

	char temp[MAX_ROW]; //lunghezza massima di una riga
	fgets(temp, MAX_ROW, file_input); //legge la prima riga dal file
	fprintf(file_output, "%s", temp); //stampa TABLE ******* COLUMNS ******,******,****;

	int rows = 0, columns = 0;
	char ***dati = readFromFile(fileName, &rows, &columns); //legge tutti i dati dalla tabella
	if (dati == NULL)    //controlla che tutto sia andato bene
		return false;

	int *toPrint;   //vettore utile a capire quali colonne andranno stampate nel file
	if ((toPrint = (int *)calloc(sizeof(int *), columns)) == NULL)
		return false;

	char *pnt = NULL;   //splitta la riga TABLE ******* COLUMNS ******,******,****;
	pnt = strtok(temp, " ");
	pnt = strtok(NULL, " ");
	pnt = strtok(NULL, " ");//splitta finche legge le colonne

	if (colonne == NULL) {
		for (int i = 0; i < columns; i++)    //mettendo tutte le colonne a 1 stamperà tutte le colonne
			toPrint[i] = 1;
	}
	else {
		for (int i = 0; i < columns; i++) {//legge le colonne della riga TABLE ******* COLUMNS ******,******,****;
			if (isInColonne(pnt = strtok(NULL, ",;\n"), colonne, num_colonne))   //cosi stamperà solo le colonne richieste dalla Select (se presenti)
				toPrint[i] = 1;
		}
	}

	off_t position; //variabile per togliere l ultima virgola dal file
	for (int i = 0; i < rows; i++) { //scrive nel file ogni riga
		fprintf(file_output, "ROW "); //scrive ROW nel file
		for (int j = 0; j < columns; j++) {
			if (toPrint[j] == 1) {   //scrive solamente le colonne richieste
				fprintf(file_output, "%s,", dati[i][j]);
			}
		}
		_fseeki64(file_output, -1, SEEK_END);
		position = (off_t) _ftelli64(file_output);
		_chsize(_fileno(file_output), position);  //cancella l ultima virgola
		fprintf(file_output, ";\n"); //aggiunge il ;
	}
	fprintf(file_output, "\n");
	fclose(file_input); //chiude i file
	fclose(file_output);
	for (int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di colonne
		free(colonne[i]);
	free(colonne);

	for (int i = 0; i < rows; i++) {		//libera lo spazio di memoria di dati
		for (int j = 0; j < columns; j++) {
			free(dati[i][j]);
		}
	}
	free(dati);
	return true;    //se arriva qui è terminato correttamente
}

//FUNZIONE SELECT CON FILTRO WHERE (Tempo O(n righe * n colonne))  //costo determinato da readFromFile
bool selectWhere(char *query, char **colonne, char *tabella, int nome_lenght, int num_colonne, char *campo, char *operando, char *parametro) {  //query = query testuale, colonne = lista colonne della select (se NULL la select ha *), tabella = nome della tabella su cui si fa la select, nome_lenght = lunghezza nome tabella, num_colonne = numero di colonne della select, campo = campo su cui si fa la where, operando = > < ....., parametro = valore del filtro

	FILE *file_input, *file_output; //file_input = tabella, file_output = query_result
	char *fileName = aggiungiTxt(tabella, nome_lenght);  //aggiunge txt al nome della tabella per aprire il file
	if ((file_input = fopen(fileName, "r")) == NULL)        //apre il file della tabella
		return false;

	if ((file_output = fopen("query_results.txt", "a")) == NULL)    //apre query result
		return false;

	fprintf(file_output, "%s;\n", query);   //stampa la query nel file

	char temp[MAX_ROW]; //lunghezza massima di una riga
	fgets(temp, MAX_ROW, file_input); //legge la prima riga dal file
	fprintf(file_output, "%s", temp); //stampa TABLE ******* COLUMNS ******,******,****;

	int rows = 0, columns = 0;
	char ***dati = readFromFile(fileName, &rows, &columns); //legge tutti i dati dalla tabella
	if (dati == NULL)    //controlla che tutto sia andato bene
		return false;

	int *toPrint;   //vettore utile a capire quali colonne andranno stampate nel file
	if ((toPrint = (int *)calloc(sizeof(int *), columns)) == NULL)
		return false;

	char *pnt = NULL;   //splitta la riga TABLE ******* COLUMNS ******,******,****;
	pnt = strtok(temp, " ");
	pnt = strtok(NULL, " ");
	pnt = strtok(NULL, " ");//splitta finche legge le colonne

	if (colonne == NULL) {
		for (int i = 0; i < columns; i++)    //mettendo tutte le colonne a 1 stamperà tutte le colonne
			if (strcmp(campo, pnt = strtok(NULL, ",;\n")) == 0)
				toPrint[i] = 2;
			else
				toPrint[i] = 1;
	}
	else {
		for (int i = 0; i < columns; i++) {//legge le colonne della riga TABLE ******* COLUMNS ******,******,****;
			if (isInColonne(pnt = strtok(NULL, ",;\n"), colonne, num_colonne)) {  //cosi stamperà solo le colonne richieste dalla Select (se presenti)
				toPrint[i] = 1;
				if (strcmp(campo, pnt) == 0) //mette 3 per differenziare il fatto che vada stampata
					toPrint[i] = 3;         //se 1 colonna non where ma va stampata, se 3 la colonna è quella del where e va stampata, se 2 colonna del where ma non da stampare
			}
			else if (strcmp(campo, pnt) == 0) {
				toPrint[i] = 2;
			}
		}
	}
	int ifOk = 0;   //utile a vedere se la riga va stampata o no
	off_t position; //variabile per togliere l ultima virgola dal file
	for (int i = 0; i < rows; i++) { //scrive nel file ogni riga che soddisfa la condizione

		for (int j = 0; j < columns; j++) {
			if (toPrint[j] == 2 || toPrint[j] == 3) {//campo = colonna su cui si filtra, operando = > < >= <= == !=, parametro = valore del filtro
				if (strcmp(operando, "==") == 0) {
					ifOk = !(strcmp(dati[i][j], parametro));
				}
				else if (strcmp(operando, "!=") == 0) {
					ifOk = strcmp(dati[i][j], parametro);
				}
				else if (strcmp(operando, ">") == 0) {
					int data = atoi(dati[i][j]), comparer = atoi(parametro);
					ifOk = data > comparer;
				}
				else if (strcmp(operando, ">=") == 0) {
					int data = atoi(dati[i][j]), comparer = atoi(parametro);
					ifOk = data >= comparer;
				}
				else if (strcmp(operando, "<") == 0) {
					int data = atoi(dati[i][j]), comparer = atoi(parametro);
					ifOk = data < comparer;
				}
				else if (strcmp(operando, "<=") == 0) {
					int data = atoi(dati[i][j]), comparer = atoi(parametro);
					ifOk = data <= comparer;
				}
				else    //se operando è errato esce dal ciclo e ritorna errore
					return false;
			}
		}

		if (ifOk == 1) {
			fprintf(file_output, "ROW "); //scrive ROW nel file

			for (int j = 0; j < columns; j++) {
				if (toPrint[j] == 1 || toPrint[j] == 3) //se la parola è una parola target
					fprintf(file_output, "%s,", dati[i][j]); //la scrivo
			}
			_fseeki64(file_output, -1, SEEK_END);
			position = (off_t) _ftelli64(file_output);
			_chsize(_fileno(file_output), position);  //cancella l ultima virgola
			fprintf(file_output, ";\n"); //aggiunge il ;
		}

	}

	fprintf(file_output, "\n");
	fclose(file_input); //chiude i file
	fclose(file_output);
	for (int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di colonne
		free(colonne[i]);
	free(colonne);

	for (int i = 0; i < rows; i++) {		//libera lo spazio di memoria di dati
		for (int j = 0; j < columns; j++) {
			free(dati[i][j]);
		}
	}
	free(dati);

	return true;    //se arriva qui è terminato correttamente
}

//FUNZIONE SELECT CON FILTRO ORDER BY (Tempo O(n righe ^ 2 + n righe * n colonne))  //Bubble sort costa n righe ^ 2 e readFromFile nr * nc
bool selectOrder(char *query, char **colonne, char *tabella, int nome_lenght, int num_colonne, char *campo, char *tipo) { //query = query testuale, colonne = lista colonne della select (se NULL la select ha *), tabella = nome della tabella su cui si fa la select, nome_lenght = lunghezza nome tabella, num_colonne = numero di colonne della select, campo = campo su cui si ordina, tipo = ASC DESC
	FILE *file_input, *file_output; //file_input = tabella, file_output = query_result
	char *fileName = aggiungiTxt(tabella, nome_lenght);  //aggiunge txt al nome della tabella per aprire il file
	if ((file_input = fopen(fileName, "r")) == NULL)        //apre il file della tabella
		return false;

	int rows = 0, columns = 0;
	char ***dati = readFromFile(fileName, &rows, &columns); //legge tutti i dati dalla tabella
	if (dati == NULL)    //controlla che tutto sia andato bene
		return false;

	char temp[MAX_ROW]; //lunghezza massima di una riga
	fgets(temp, MAX_ROW, file_input); //legge la prima riga dal file


	char *pnt = NULL;   //splitta la riga TABLE ******* COLUMNS ******,******,****;
	pnt = strtok(temp, " ");
	pnt = strtok(NULL, " ");
	pnt = strtok(NULL, " ");//splitta finche legge le colonne

	int colonnaDaOrd = 0, ifFound = 0;   //indice che indica la colonna su cui si deve ordinare
	while ((pnt = strtok(NULL, " ,;\n")) != NULL) { //legge le colonne
		if (strcmp(pnt, campo) == 0) { //continua finche non trova la colonna su cui bisogna ordinare
			ifFound = 1;
			break;
		}
		colonnaDaOrd++;
	}
	if (ifFound == 0) //se non ha trovato la colonna
		return false;

	char **rigaTemp;
	if ((rigaTemp = (char **)calloc(sizeof(char **), columns)) == NULL)
		return false;

	if (strcmp(tipo, "ASC") == 0) {      //ordinamento bubble sort in tempo O(n righe ^ 2). Fare un merge sort o un heap sort su righe di una matrice non so se sia possibile
										 //ordinamento crescente
		for (int j = 0; j < rows - 1; j++) {
			for (int i = j + 1; i < rows; i++) {
				if (atoi(dati[j][colonnaDaOrd]) == 0) {     //se deve ordinare stringhe
															//se il campo è 0 la funzione non produce i risultati attesi dato che si basa sulla funzione atoi della libreria string.h
					if (strcmp(dati[j][colonnaDaOrd], dati[i][colonnaDaOrd]) > 0) {
						rigaTemp = dati[j];
						dati[j] = dati[i];
						dati[i] = rigaTemp;
					}
				}
				else {
					if (atoi(dati[j][colonnaDaOrd]) > atoi(dati[i][colonnaDaOrd])) {     //se deve ordinare numeri
						rigaTemp = dati[j];
						dati[j] = dati[i];
						dati[i] = rigaTemp;
					}
				}
			}
		}
	}
	else {
		//ordinamento decrescente
		for (int j = 0; j < rows - 1; j++) {
			for (int i = j + 1; i<rows; i++) {
				if (atoi(dati[j][colonnaDaOrd]) == 0) {     //se deve ordinare stringhe
															//se il campo è 0 la funzione non produce i risultati attesi dato che si basa sulla funzione atoi della libreria string.h
					if (strcmp(dati[j][colonnaDaOrd], dati[i][colonnaDaOrd]) < 0)
					{
						rigaTemp = dati[j];
						dati[j] = dati[i];
						dati[i] = rigaTemp;
					}
				}
				else {
					if (atoi(dati[j][colonnaDaOrd]) < atoi(dati[i][colonnaDaOrd])) { //se deve ordinare numeri
						rigaTemp = dati[j];
						dati[j] = dati[i];
						dati[i] = rigaTemp;
					}
				}
			}
		}
	}

	if ((file_output = fopen("query_results.txt", "a")) == NULL)    //apre il file di output
		return false;
	fprintf(file_output, "%s;\n", query);    //scrive la query sul file

	rewind(file_input);
	fgets(temp, MAX_ROW, file_input);
	fprintf(file_output, "%s", temp); //stampa TABLE ******* COLUMNS ******,******,****;

	pnt = NULL;
	pnt = strtok(temp, " ");
	pnt = strtok(NULL, " ");
	pnt = strtok(NULL, " "); //cosi ora legge la prima colonna

	int *toPrint;   //vettore utile a capire quali colonne andranno stampate nel file
	if ((toPrint = (int *)calloc(sizeof(int *), columns)) == NULL)
		return false;

	if (colonne == NULL) {
		for (int i = 0; i < columns; i++)    //mettendo tutte le colonne a 1 stamperà tutte le colonne
			toPrint[i] = 1;
	}
	else {
		for (int i = 0; i < columns; i++) {//legge le colonne della riga TABLE ******* COLUMNS ******,******,****;
			if (isInColonne(pnt = strtok(NULL, ",;\n"), colonne, num_colonne))   //cosi stamperà solo le colonne richieste dalla Select (se presenti)
				toPrint[i] = 1;
		}
	}
	off_t position;
	for (int i = 0; i < rows; i++) { //scrive nel file ogni riga
		fprintf(file_output, "ROW "); //scrive ROW nel file
		for (int j = 0; j < columns; j++) {
			if (toPrint[j] == 1) {   //scrive solamente le colonne richieste
				fprintf(file_output, "%s,", dati[i][j]);
			}
		}
		_fseeki64(file_output, -1, SEEK_END);
		position =(off_t) _ftelli64(file_output);
		_chsize(_fileno(file_output), position);  //cancella l ultima virgola
		fprintf(file_output, ";\n"); //aggiunge il ;
	}
	fprintf(file_output, "\n");
	fclose(file_input); //chiude i file
	fclose(file_output);
	
	for (int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di colonne
		free(colonne[i]);
	free(colonne);
	
	for (int i = 0; i < rows; i++) {		//libera lo spazio di memoria di dati
		for (int j = 0; j < columns; j++) {
			free(dati[i][j]);
		}
	}
	free(dati);
	return true;    //se arriva qui è terminato correttamente
}

//FUNZIONE SELECT CON FILTRO GROUP BY (Tempo O(n righe * n colonne + n righe * n occorrenze))  //nr * nc costo readFromFile nr * no costo ciclo per raggruppare
bool selectGroup(char *query, char **colonne, char *tabella, int nome_lenght, int num_colonne, char *campo) {   //query = query testuale, colonne = lista colonne della select (se NULL la select ha *), tabella = nome della tabella su cui si fa la select, nome_lenght = lunghezza nome tabella, num_colonne = numero di colonne della select, campo = campo su cui si ordina, tipo = ASC DESC

	int dimVettGroup = 10, indexGruppi = 0; //dimVettGroup = dimensione vettore gruppi, indexGruppi = indice ultimo gruppo memorizzato
	FILE *file_input, *file_output;

	char *fileName = aggiungiTxt(tabella, nome_lenght);  //aggiunge txt al nome della tabella per aprire il file
	if ((file_input = fopen(fileName, "r")) == NULL)        //apre il file della tabella
		return false;

	int rows = 0, columns = 0;
	char ***dati = readFromFile(fileName, &rows, &columns);
	if (dati == NULL)
		return false;

	char temp[MAX_ROW]; //lunghezza massima di una riga
	fgets(temp, MAX_ROW, file_input); //legge la prima riga dal file

	char *pnt = NULL;   //splitta la riga TABLE ******* COLUMNS ******,******,****;
	pnt = strtok(temp, " ");
	pnt = strtok(NULL, " ");
	pnt = strtok(NULL, " ");//splitta finche legge le colonne

	int colonnaDaRagg = 0, ifFound = 0;   //indice che indica la colonna su cui si deve raggruppare
	while ((pnt = strtok(NULL, " ,;\n")) != NULL) { //legge le colonne
		if (strcmp(pnt, campo) == 0) { //continua finche non trova la colonna su cui bisogna raggruppare
			ifFound = 1;
			break;
		}
		colonnaDaRagg++;
	}
	if (ifFound == 0) //se non ha trovato la colonna
		return false;

	char ***gruppi = NULL;      //struttura dati che conterra i gruppi
	if ((gruppi = (char ***)calloc(sizeof(char ***), dimVettGroup)) == NULL)
		return false;

	for (int i = 0; i < dimVettGroup; i++) { //istanzia la struttura dati, che per ogni riga ha due colonne, una col nome e una col numero di occorrenze
		if ((gruppi[i] = (char **)calloc(sizeof(char **), 2)) == NULL)
			return false;
		if ((gruppi[i][0] = (char *)calloc(sizeof(char *), MAX_LEN)) == NULL || (gruppi[i][1] = (char *)calloc(sizeof(char *), 5)) == NULL)
			return false;
	}

	ifFound = 0;    //utile a capire se la riga è gia presente in gruppi o è da aggiungere
	for (int i = 0; i < rows; i++) { //cicla tutte le righe di dati Costo O(n righe * n occorrenze)
		ifFound = 0;
		for (int j = 0; j < indexGruppi; j++) {  //cerca in gruppi se la riga è gia stata trovata prima
			if (strcmp(dati[i][colonnaDaRagg], gruppi[j][0]) == 0) {     //se è gia stata trovata aumenta di 1 il COUNT
				sprintf(gruppi[j][1], "%d", atoi(gruppi[j][1]) + 1);
				ifFound = 1;
			}
		}

		if (ifFound == 0) {  //se non è stata trovata
			if (indexGruppi == dimVettGroup) {       //se è terminato lo spazio rialloca
				dimVettGroup *= 2;
				if ((gruppi = (char ***)realloc(gruppi, sizeof(char ***) * dimVettGroup)) == NULL)
					return false;
				for (int i = dimVettGroup / 2; i < dimVettGroup; i++) {
					if ((gruppi[i] = (char **)calloc(sizeof(char **), 2)) == NULL)
						return false;
					if ((gruppi[i][0] = (char *)calloc(sizeof(char *), MAX_LEN)) == NULL || (gruppi[i][1] = (char *)calloc(sizeof(char *), 5)) == NULL)
						return false;
				}
			}
			strcpy(gruppi[indexGruppi][0], dati[i][colonnaDaRagg]);     //aggiunge la nuova riga con COUNT a 1
			strcpy(gruppi[indexGruppi++][1], "1");
		}
	}

	if ((file_output = fopen("query_results.txt", "a")) == NULL)    //apre query result
		return false;

	fprintf(file_output, "%s;\n", query);   //stampa la query nel file

	fprintf(file_output, "TABLE %s COLUMNS %s,COUNT;\n", tabella, campo);   //stampa la dichiarazione della table come da consegna

	for (int i = 0; i < indexGruppi; i++) {  //stampa le righe raggruppate
		fprintf(file_output, "ROW %s,%s;\n", gruppi[i][0], gruppi[i][1]);
	}
	fprintf(file_output, "\n");
	fclose(file_input); //chiude i file
	fclose(file_output);
	
	for (int i = 0; i < num_colonne; i++)	//libera lo spazio di memoria di colonne
		free(colonne[i]);
	free(colonne);
	
	for (int i = 0; i < rows; i++) {		//libera lo spazio di memoria di dati
		for (int j = 0; j < columns; j++) {
			free(dati[i][j]);
		}
	}
	free(dati);

	for (int i = 0; i < rows; i++) {		//libera lo spazio di memoria di dati
		for (int j = 0; j < 2; j++) {
			free(gruppi[i][j]);
		}
	}
	free(gruppi);
	return true;    //se arriva qui è terminato correttamente
}