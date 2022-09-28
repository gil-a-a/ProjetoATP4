#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_livro {
    char isbn[14];
    char titulo[50];
    char autor[50];
    char ano[5];
}LIVRO;

LIVRO* carregaInsercao();
char** carregaRemocao();
void printaRegistro(LIVRO *vet);
int tamanhoRegistro(LIVRO reg);
void removeRegistro();
void insereRegistro();
void compactacao();

int main() {
	int opcao;
	
	/*
	=============================
	= Coisas pra fazer dps:		=
	=>Fazer um menu bonito    	=
	=>Tirar os prints de teste  =
	=>Retornar o ponteiro dos ar=
	= quivos de entrada, pq aí n=
	= precisa ficar abrindo e fe=
	= chando os arqs			=
	=>Ler os comentários e resol=
	=ver oq for preciso			=
	=>Terminar					=
	=>Apresentar				=
	=============================
	*/
	
	do {
		printf("Opcao: ");
		scanf("%d", &opcao);

		switch (opcao)
		{
		case 1:
			insereRegistro();
			break;

		case 2:
			removeRegistro();
			break;

		case 3:
			compactacao();
			break;

		case 0:
			printf("Encerrando programa...");
			break;
		
		default:
			printf("Opcao invalida!\n");
			break;
		}
	}while(opcao != 0);
	
	return 0;
}

/*
char fragmentacao[sizeof(LIVRO) - tamanhoRegistro(reg)];
	for (i = 0; i < sizeof(LIVRO) - tamanhoRegistro(reg)-1; i++) {
		fragmentacao[i] = '@';
	}
*/

LIVRO* carregaInsercao() {
	FILE* in = fopen("insere.bin", "rb");

	if (in == NULL) {
		printf("Falha ao abrir o arquivo!");
		return NULL;
	}
	
	LIVRO* vet = malloc(sizeof(LIVRO) * 8); //8 é fixo pois ja conhecemos o arquivo de entrada
	fread(vet, sizeof(LIVRO), 8, in);

	fclose(in);

	return vet;
}

char** carregaRemocao() {
	FILE* in = fopen("remove.bin", "rb");

	if (in == NULL) {
		printf("Falha ao abrir o arquivo!");
		return NULL;
	}

	char **vet;
    vet = malloc(4 * sizeof(char*));
    int i;
	for(i = 0; i < 4; i++) {
        vet[i] = malloc(14 * sizeof(char)); //aloca cada uma das linhas
    }

	for(i = 0; i < 4; i++) {
		fread(vet[i], sizeof(char), 14, in);
		//printf("%s\n", vet[i]);
	}

	fclose(in);

	return vet;
}

void printaRegistro(LIVRO *vet) {
	int i;
	
	for(i = 0; i < 8; i++) {
		printf("%s %s %s %s", vet[i].ano, vet[i].autor, vet[i].isbn, vet[i].titulo);
	}
}

int tamanhoRegistro(LIVRO reg) {
	return 13 + strlen(reg.autor) + strlen(reg.titulo) + 4;
}

void removeRegistro() {
	FILE* in = fopen("biblioteca.bin", "r+b");
	if (in == NULL){
		printf("Falha ao abrir o arquivo!\n");
		printf("Nenhum registro inserido!\n");
		return;
	}
	
	char** dados = carregaRemocao();	//dps tem q mudar essa linha lá pra main e passar dados por referência, pq aí consome menos memorya
	
	int aux, tam_percorrido, header, i;
	char* isbn;
		isbn = malloc(13 * sizeof(char));
	
	for (i = 0; i < 4; i++){
		fseek(in, sizeof(int), SEEK_SET);	//pula a quantidade de registros inseridos
		fread(&header, sizeof(int), 1, in);	//le o endereço do próximo registro vazio
		printf("i: %d\n", i);
		while(fread(&aux, sizeof(int), 1, in)){	//le o tamanho de cada registro
			printf("aux: %d\n", aux);
			
			printf("dados: %s#abc\n", dados[i]);
			fread(isbn, sizeof(char), 13, in);
			printf("isbn: %s#abc\n", isbn);
			
			if (strcmp(dados[i], isbn) == 0){
				
				fseek(in, -13, SEEK_CUR);
				tam_percorrido = ftell(in);
				fwrite("@", sizeof(char), 1, in);
				fwrite(&header, sizeof(int), 1, in);	//primeiro escreve o endereço pro próximo registro vazio
				
				fseek(in, sizeof(int), SEEK_SET);	//dps pula a quantidade de registros inseridos
				fwrite(&tam_percorrido, sizeof(int), 1, in);	//e escreve o novo endereço pro primeiro elemento da lista
				
				printf("removido\n");
				fclose(in);	//tive q por esse fclose aq, pq ele só tava colocando o @ no biblioteca.bin dps q encerrava o programa
				free(isbn);
				
				return;
			}
			
			fseek(in, aux - 13, SEEK_CUR);
		}
		rewind(in);
	}
	
	//dps dar free em tudo q precisar e já era
	
	printf("ISBN nao encontrado!\n");

	return;
}

void insereRegistro() {
	LIVRO* dados = carregaInsercao();
	int a = 0; //numero de registros ja inseridos
	int b = -1; //ponteiro pro primeiro registro deletado (-1 significa final da lista)

	//verificar se tem arquivo antes de abrir com w+b
	FILE* out = fopen("biblioteca.bin", "r+b"); //tenta abrir um arquivo existente
	if (out == NULL) {

		out = fopen("biblioteca.bin", "w+b");
		if (out == NULL) {
			printf("Falha ao abrir o arquivo!");
			return;
		}

		fwrite(&a, sizeof(int), 1, out);
		fwrite(&b, sizeof(int), 1, out);
		rewind(out);
	}

	fread(&a, sizeof(int), 1, out); //le o cabecalho
	if (a == 8) { //ja inseriu todos os registros disponiveis
		printf("Todos os registros cadastrados!\n");
		return;
	}
	a++;
	rewind(out);
	fwrite(&a, sizeof(int), 1, out);
	
	LIVRO reg = dados[a-1];

	int i, tam = tamanhoRegistro(reg) + 3;
	char buffer[tam];	//o + 3 é por causa dos 3 #'s
	
	sprintf(buffer, "%s#%s#%s#%s", reg.isbn, reg.titulo, reg.autor, reg.ano);
	
	/*
		Percorrer a lista de espaços disponíveis antes de inserir um novo registro
		Tem q ser na ordem first-fit, ou seja, o primeiro registro a ser escolhido
		vai ser o q tiver tamanho igual
	*/

	fseek(out, 0L, SEEK_END);
	fwrite(&tam, sizeof(int), 1, out);
	fwrite(buffer, sizeof(buffer), 1, out);
	
	printf("Registro inserido\n");
	
	fclose(out);
}

void compactacao() {
	FILE* in = fopen("biblioteca.bin", "rb");
	if (in == NULL) {
		printf("Falha ao abrir o arquivo!\n");
		printf("Nenhum registro inserido!\n");
		return;
	}

	FILE* out = fopen("temp.bin", "wb");
	if (out == NULL) {
		printf("Falha ao abrir o arquivo!\n");
		return;
	}

	/* ********** COPIANDO CABECALHO ********** */
	int aux;
	fread(&aux, sizeof(int), 1, in); //le o numero de registros inseridos
	fwrite(&aux, sizeof(int), 1, out); 
	
	aux = -1;	//tirei a linha de baixo, pq dps de compactar o arquivo, ficava sobrando o index antigo da lista de remoção
	//fread(&aux, sizeof(int), 1, in); //le o index do primeiro registro deletado
	fseek(in, sizeof(int), SEEK_CUR);	//pula o index do in
	fwrite(&aux, sizeof(int), 1, out);
	/* **************************************** */

	char c;
	char *buffer;
	buffer = malloc(sizeof(char)*aux);	//char buffer[aux];
	
	while(fread(&aux, sizeof(int), 1, in)) {
		if (fread(&c, sizeof(char), 1, in) && c != '@') {
			fseek(in, -1, SEEK_CUR);	//essa linha tava como "fseek(in, -sizeof(char), SEEK_CUR);", só mudei o -sizeof(char) pq meu compilador deu warning aí
			fread(buffer, sizeof(char), aux, in);
			fwrite(&aux, sizeof(int), 1, out);
			fwrite(buffer, sizeof(char), aux, out);
		} else {
			fseek(in, aux - 1, SEEK_CUR);
		}
	}
	
	free(buffer);
	fclose(in);
	fclose(out);
	remove("biblioteca.bin");
	rename("temp.bin", "biblioteca.bin");

	return;
}
