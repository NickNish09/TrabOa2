#include<stdio.h>
#include<stdlib.h>

#define TAMANHO_CLUSTER 4096
enum{
AddSetor = 1, AddTrilha = 60, AddCilindro = 300,
FOUND = 1, NOT_FOUND = 0
};

typedef struct fatent_s{
    unsigned int    used;
    unsigned int    eof;
    unsigned int    next;
}fatent;

typedef struct fatlist_s{
    char            nome[100];
    unsigned int    first_sector;
}fatlist;

typedef struct inttemplist{
    int                    valor;
    struct templistint*    seguinte;
}templistint;


typedef struct block{unsigned char bytes_s[512];}block;/*Setor*/
typedef struct sector_array{ block sector[60];}sector_array;/*Trilha*/
typedef struct track_array{sector_array track[5];}track_array;/*Cilindro*/

fatlist FAT_ARQUIVOS[300];
fatent FAT_SETORES[600]; // 2 cilindros

int Adress(int j,int k,int l){/*Recebe as componentes do endereço e gera o endereço*/
    int R;

    R = j*AddSetor + k*AddTrilha + l*AddCilindro;
    return(R);
}

int comparastring(char* A, char* B){
    int i=0;

    while(A[i]==B[i] && A[i]!='\0' && B[i]!='\0'){
        i++;
    }
    if(A[i]==B[i]){
        return(1);
    }else{
        return(0);
    }
}

void RevAdress(int endereco,int* j,int* k,int* l){/*Recebe as componentes do endereço por referência e o inteiro, atualizando as componentes para apontares para aquele endereço*/
    *l = endereco/AddCilindro;
    *k = (endereco%AddCilindro)/AddTrilha;
    *j = (endereco%AddCilindro)%AddTrilha;
}



fatent* escrever_registro(char* nome,int* l/*l = número de cilindros alocados*/,track_array** start,int FAT_NUM,fatlist* fatlistp,fatent* fatentp){
    FILE* fp;
    track_array* cilindro;
    track_array** new_start;
    fatent* new_fatentp;
    int i=0,j=0,k=0,n=0,c=0,t=0;
    char string;
    int temp = 0;
    int end = 0;
    int loc = 0;
    int last_loc = 0;
    int save = 0;


    while(nome[i]!='\0'&&nome[i]!='.'){
        i++;
    }
    nome[i]= '.';
    nome[i+1]= 't';
    nome[i+2]= 'x';
    nome[i+3]= 't';
    nome[i+4]= '\0';
    new_start = start;/*isso garante que se a função não alocar mais cilindros, nada mude*/
    fp = fopen(nome,"r");
    i=0;
    loc = Adress(0,0,0);
    while(end != 1 && fp != NULL){/*Enquanto o arquivo não chegar no final, tentar escrever*/
        printf("Verificando ... \n");
        loc = Adress(j,k,n);
        while(temp != EOF && n<(*l)){/*repete até chegar no número de cilindros alocado*/
            printf("Movendo para cilindro %d \n",n);
            loc = Adress(j,k,n);
            k=0;
            while(temp != EOF && k<5){/*Repete até chegar no final do cilindro*/
                printf("Movendo para trilha %d \n",k);
                loc = Adress(j,k,n);
                j=0;
                while(temp != EOF && j<60){/*Repete até chegar no final da trilha*/
                    loc = Adress(j,k,n);
                    printf("Movendo para setor %d -> endereco |%d|\n",j,loc);
                    c = j;
                    if(fatentp[loc].used!=1){
                        while(c<4+j){/*Anda em clusters de 4 setores, reservando 4 setores sempre que encontra 1 disponível*/
                            i=0;
                            loc = Adress(c,k,n);
                            //printf("Checando elemento em %d asda\n",loc);
                            fatentp[loc].used = 1;
                            printf("Checando elemento em %d \n",loc);
                            if(save <= 0){/*Salva o inicio do arquivo na tabela*/
                                t = 0;
                                fatlistp[FAT_NUM].first_sector = loc;
                                while(nome[t]!='\0'){/*um while que copia a string, mais nada*/
                                    fatlistp[FAT_NUM].nome[t] = nome[t];
                                    t++;
                                }
                                fatlistp[FAT_NUM].nome[t] = '\0';
                                save = 1;
                            }
                            //printf("L104\n");
                            printf("fatent[%d].next = %d\n",last_loc,loc);
                            fatentp[last_loc].next = loc;
                            //printf("PL104\n");
                            while(temp != EOF && i<512){/*Escreve em um setor até chegar no fim do arquivo ou o setor acabar*/
                                //printf("Lendo byte %d \n",i);
                                temp = fscanf(fp,"%c",&string);
                                //printf("Lido byte %d, char %c, temp = |%d| \n",i,string,temp);
                                if(temp == EOF){
                                    printf("End Of File\n");
                                    getchar();
                                    fatentp[loc].eof = 1;
                                    end = 1;
                                }else{
                                    //printf("Escrevendo byte %d |%c| no endereco %d \n",i,string,loc);
                                    //printf("L118\n");
                                    new_start[n]->track[k].sector[c].bytes_s[i] = string;
                                    //printf("L118b\n");
                                    fatentp[loc].used = 1;
                                    //printf("PL118\n");
                                }
                                i++;
                            }
                            //getchar();
                            //system("cls");
                            c++;
                            printf("Preparando para mover(Setor)\n");
                            printf("Last_loc = %d \n",last_loc);
                            last_loc = loc;
                        }
                    }
                    printf("Preparando para mover(Cluster)\n");
                    j = j+4;
                }
                printf("Preparando para mover(Trilha)\n");
                k++;
            }
            printf("Preparando para mover(Cilindro)\n");
            //system("cls");
            n++;
        }
        if(end == 0 && temp != EOF){/*Se o arquivo não tiver terminado de ser escrito ... */
            printf("Alocando cilindro %d \n",n);
            c = 0;
            new_start = (track_array**)calloc(*l+1,sizeof(track_array*));/*Então aloca um vetor de ponteiros de cilindros com +1 casa...*/
            if(new_start != NULL){/*se new_start for NULL, ago deu ruim na alocação de espaço*/
                while(c<(*l)){/*Copia o vetor antigo no vetor novo*/
                    new_start[c] = start[c];
                    c++;
                }
                new_start[c] = (track_array*)malloc(sizeof(track_array));/*Aloca o novo cilindro no novo vetor*/
                start = new_start;
            }else{
                printf("Erro na alocacao do novo cilindro\n");
                end = 1;/*isto sairá do loop externo e então da função*/
            }
            printf("Alocando tabela %d - %d \n",(*l)*300,((*l)+1)*300);
            getchar();
            c = 0;
            new_fatentp = (fatent*)calloc((*l+1)*300,sizeof(fatent));/*Então aloca um vetor de ponteiros de fatent com +1 casa...*/
            if(fatentp != NULL){/*se new_fatentp for NULL, ago deu ruim na alocação de espaço*/
                while(c<(*l)*300){/*Copia o vetor antigo no vetor novo*/
                    //printf("Copiando elemento %d na tabela (de %d)\n",c,*l*300);
                    new_fatentp[c].eof = fatentp[c].eof;
                    new_fatentp[c].next = fatentp[c].next;
                    new_fatentp[c].used = fatentp[c].used;
                    c++;
                }
                //printf("Liberando vetor antigo\n");
                //free(fatentp);
                //printf("Liberando vetor antigo\n");
                //fatentp = NULL;
                //printf("Liberando vetor antigo\n");
                fatentp = new_fatentp;
                //printf("Liberando vetor antigo\n");
                (*l)++;
            }else{
                printf("Erro na alocacao da tabela\n");
                end = 1;/*isto sairá do loop externo e então da função*/
            }
        }
    }
    if(fp != NULL){/*se fp for NULL aqui, algo deu errado na hora de abrir o arquivo e vai dar ruim se tentar fechá-lo*/
        fclose(fp);
    }else{
        printf("Arquivo nao encontrado\n");
    }
    printf("Retornando de |Escrever regitro|\n\n");
    return(fatentp);
}

void geraarquivo(char* In, char* nome){/*Escreve um arquivo contendo a string In de nome "nome". imprime uma string*/
    FILE* fpo;
    int i = 0;


    fpo = fopen(nome,"a");
    i=0;
    while(In[i]!='\0'&&i<512){
        fprintf(fpo,"%c",In[i]);
        i++;
    }
    fclose(fpo);
    printf("Arquivo criado com o nome %s \n",nome);
}

void geraarquivo_b(track_array** start,fatent* FatEnt,fatlist* FatList, char* nome){/*Escreve um arquivo contendo a string In de nome "nome". imprime uma string*/
    FILE* fpo;
    int i = 0;
    int j=0,k=0,l=0;
    int loc = 0;


    while(comparastring(FatList[i].nome,nome)!=1 && i<300){/*Procuro a string na lista de arquivos salvos*/
        i++;
    }
    if(comparastring(FatList[i].nome,nome)==1){/*Se encontrar a string na lista de arquivos salvos ...*/
        loc = FatList[i].first_sector;/*... pega o endereço de onde começa o arquivo*/
        i=0;
        while(nome[i]!='\0'&&nome[i]!='.'){/*Alterei o nome do arquivo, adicionando um "O.txt" ao final*/
                    i++;
        }
        nome[i]= 'O';
        nome[i+1]= '.';
        nome[i+2]= 't';
        nome[i+3]= 'x';
        nome[i+4]= 't';
        nome[i+5]= '\0';
        fpo = fopen(nome,"w");
        while(FatEnt[loc].eof != 1){/*E equanto não chegar no fim do arquivo ...*/
            i=0;
            RevAdress(loc,&j,&k,&l);/*Pega as coordenadas do setor*/
            while(start[l]->track[k].sector[j].bytes_s[i]!='\0'&&i<512){/*printa o setor*/
                fprintf(fpo,"%c",start[l]->track[k].sector[j].bytes_s[i]);
                i++;
            }
            loc = FatEnt[loc].next;/*e avança pro próximo*/
        }
        i=0;
        RevAdress(loc,&j,&k,&l);/*Pega as coordenadas do setor*/
        while(start[l]->track[k].sector[j].bytes_s[i]!='\0'&&i<512){/*printa o setor com eof*/
            fprintf(fpo,"%c",start[l]->track[k].sector[j].bytes_s[i]);
            i++;
        }
        fclose(fpo);
        printf("Arquivo criado com o nome %s \n",nome);
    }else{
        printf("Arquivo Nao Encontrado\n");
    }
}

void mostrar_fat(fatlist* FatList){
    //scanf("%s",FAT_ARQUIVOS[0].nome);
    //scanf("%d",&FAT_ARQUIVOS[0].first_sector);
    /*FAT_ARQUIVOS[0].first_sector = 1;
    FAT_SETORES[1].eof = 0;
    FAT_SETORES[1].next = 2;
    FAT_SETORES[1].eof = 0;
    FAT_SETORES[2].next = 6;
    FAT_SETORES[2].eof = 0;
    FAT_SETORES[6].eof = 1;*/
    int i,cont;
    int next;
    printf("Nome              Setores              Tamanho \n");
    for(i=0;i<1;i++){
        printf("%s              ",FatList[i].nome);
        /*next = FatList[i].first_sector;
        printf("%d, ",next);
        cont = 0;
        while(FAT_SETORES[next].eof != 1){
            cont++;
            next = FAT_SETORES[next].next;
            printf("%d, ",next);
        }
        printf("           ");
        printf("%d",(cont+1)*TAMANHO_CLUSTER);
        printf(" B \n");*/
    }
}

void apagar_registro(){
    char nome[50];
    int primeiro_setor;
    int next,i;

    scanf("%s",nome);
    FAT_ARQUIVOS[0].nome[0] = 'o';
    FAT_ARQUIVOS[0].nome[1] = 'l';
    FAT_ARQUIVOS[0].nome[2] = 'a';
    FAT_ARQUIVOS[0].nome[3] = '\0';

    FAT_ARQUIVOS[0].first_sector = 1;
    FAT_SETORES[1].eof = 0;
    FAT_SETORES[1].next = 2;
    FAT_SETORES[1].eof = 0;
    FAT_SETORES[2].next = 6;
    FAT_SETORES[2].eof = 0;
    FAT_SETORES[6].eof = 1;
    for(i=0;i<1;i++){
        if(strcmp(FAT_ARQUIVOS[i].nome,nome) == 0){
            primeiro_setor = FAT_ARQUIVOS[i].first_sector;
        }
    }

    next = primeiro_setor;
    while(FAT_SETORES[next].eof != 1){
        FAT_SETORES[next].used = 0;
        next = FAT_SETORES[next].next;
    }
    printf("%d\n",FAT_SETORES[1].used);
    printf("%d\n",FAT_SETORES[2].used);
    printf("%d\n",FAT_SETORES[6].used);
    // Testes
    //Testes
}

int main(){
    int option=0;
    track_array** inicio_disco;
    track_array*  cylinder;
    char hundred[200];
    char arquivo[20];
    fatlist Fatlist[300];
    fatent* Fatent;
    int l=1,i=0,j=0,k=0,n=0;
    int fat_num = -1;
    char nome[100];

    Fatent = (fatent*)calloc(300,sizeof(fatent));
    cylinder = (track_array*)calloc(1,sizeof(track_array));
    inicio_disco = &cylinder;
    while (option != 5){
        system("cls");
        switch(option){
            case(1):
                printf("Selecionado: Criar novo registro. \n");
                printf("Digite o nome do arquivo que contem os registros\n");
                scanf("%s",nome);
                getchar();
                fat_num++;
                Fatent = escrever_registro(nome,&l,inicio_disco,fat_num,Fatlist,Fatent);
                geraarquivo_b(inicio_disco,Fatent,Fatlist,nome);
                break;
            case(2):
                printf("Selecionado: Ler do registro \n");
                break;
            case(3):
                printf("Selecionado: Apagar um registro \n");
                apagar_registro();
                break;
            case(4):
                printf("Selecionado: Mostrar tabela FAT \n");
                mostrar_fat(Fatlist);
                break;
            case(5):
                printf("Selecionado: Sair \n");
                printf("Tem certeza que deseja sair?\n 5 - sim, 0 - nao \n");
                scanf("%d",&option);
                break;

        }
        printf("Selecione uma opcao: \n");
        printf("1 - Criar novo registro \n");
        printf("2 - Ler um registro \n");
        printf("3 - Apagar um registro \n");
        printf("4 - Mostrar tabela FAT \n");
        printf("5 - Sair\n");
        scanf("%d",&option);
    }

return (0);
}
