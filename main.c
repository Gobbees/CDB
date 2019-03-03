//
//  main.c
//  Prog_Algoritmi
//
//  Created by Giovanni Gobbi on 27/06/2018.
//  Copyright © 2018 Giovanni Gobbi. All rights reserved.
//

#include <stdio.h>
#include "lib1718.h"

int main(int argc, const char * argv[]) {
    char * query = "INSERT INTO Scarpe (Marca,Taglia,Costo) VALUES (Nike,47,140)";
    bool ifOk = executeQuery(query);
    printf("%d ", ifOk);
    return 0;
}

