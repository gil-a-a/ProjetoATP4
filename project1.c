#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_livro {
    char isbn[14];
    char titulo[50];
    char autor[50];
    char ano[5];
}LIVRO;

void printaRegistro(LIVRO *vet) {
	int i;
	
	for(i = 0; i < 8; i++) {
		printf("%s %s %s %s", vet[i].ano, vet[i].autor, vet[i].isbn, vet[i].titulo);
	}
}
int tamanhoRegistro(LIVRO reg) {
	return 13 + strlen(reg.autor) + strlen(reg.titulo) + 4;
}

void insereRegistro(LIVRO reg) {
	//verificar se tem arquivo antes de abrir com w+b
	FILE* out = fopen("biblioteca.bin", "w+b");
	
	int i, tam;
	char buffer[tamanhoRegistro(reg)+3];	//o + 3 é por causa dos 3 #'s
	
	sprintf(buffer, "%s#%s#%s#%s", reg.isbn, reg.titulo, reg.autor, reg.ano);
	
	tam = tamanhoRegistro(reg);
	fwrite(&tam, sizeof(int), 1, out);
	fwrite(buffer, sizeof(buffer), 1, out);
}

int main() {
	FILE* in = fopen("insere.bin", "rb");
	if (in == NULL) {
		printf("Falha ao abrir o arquivo!");
	}
	
	LIVRO vet[8];
	fread(vet, sizeof(LIVRO), 8, in);
	
	insereRegistro(vet[1]);
	
	fclose(in);
	return 0;
}

/*
char fragmentacao[sizeof(LIVRO) - tamanhoRegistro(reg)];
	for (i = 0; i < sizeof(LIVRO) - tamanhoRegistro(reg)-1; i++) {
		fragmentacao[i] = '@';
	}
*/
