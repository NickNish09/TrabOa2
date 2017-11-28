/****************************************************************************
 * Trabalho de Organização de Arquivos - Estrutura do HD                    *
 *                                                                          *
 *                                                                          *
 * Grupo: Alexandre Aragão - 13/0004880                                     *
 *        Nicholas Nishimoto Marques - 150019343                            *
 *        Thalbert Barbosa de Miranda - 14/0057056                          *
 *                                                                          *
 * Documentação feita com Doxygen,                                          *
 * presente no arquivo "_trab_o_a_v3_8c" na pasta "Documentação"            *
 *                                                                          *
 ****************************************************************************/

 /**
 * @file Trab0AV3.c
 * @author Nicholas Marques, Alexandre Aragão, Thalbert Miranda
 * @date Novembro de 2017
 * @brief Documentação do trabalho de Organização de Arquivos.
 * Trabalho referente à simulação em memória principal de um disco magnético, utilizando-se de conceitos
 * de endereçamento por tabela FAT, setorização de disco, clusters, tempos de seek e de busca. Implementação baseada em
 * vetores na memória principal que simulam cilindros do disco magnético.
 *
 * Versão do código de controle de HD - personalizada para ignorar quebras de linha escondidas.
 * A informação de "Trilhas por superfície" do roteiro foi ignorada, uma vez que ela apenas limita o número de cilindros, e não há limite.
 *
 */

#include<stdio.h>
#include<stdlib.h>



enum{
AddSetor = 1, AddTrilha = 60, AddCilindro = 300,
FOUND = 1, NOT_FOUND = 0,
LATENCY = 6000,TRANSFER_C = 800,TRANSFER_S = 200,SEEK=4000
};

/**
 * @brief Structs utilizadas para simular a tabela fat na memória.
 *
 * Fatent: utilizada para mapear cada setor do HD, indicando se o mesmo está sendo utilizado e o próximo setor relacionado.
 * Fatlist: utilizada para mostrar o nome do arquivo e o primeiro setor por ele ocupado no HD.
 */

typedef struct fatent_s{
    unsigned int    used; //!< De acordo com o valor indica se o setor está sendo usado (valor 1 indica que está sendo usado, 0 caso contrário).
    unsigned int    eof;  //!< Indica se é o último setor utilizado por um arquivo (1 caso seja).
    unsigned int    next; //!< Indica o valor do próximo setor ocupado pelo arquivo que utiliza esse setor.
}fatent;

typedef struct fatlist_s{
    char            nome[100]; //!< String que contém o nome do arquivo ao qual aquele setor está ocupado.
    unsigned int    first_sector; //!< Indica o primeiro setor do arquivo com nome definido na struct.
}fatlist;

typedef struct inttemplist{
    int                    valor;
    struct templistint*    seguinte;
}templistint;


typedef struct block{unsigned char bytes_s[512];}block;/*Setor*/
typedef struct sector_array{ block sector[60];}sector_array;/*Trilha*/
typedef struct track_array{sector_array track[5];}track_array;/*Cilindro*/


void NumtoSring(int num,char* string){
    int aux,temp;
    int i=0,j=0;

    aux = num;
    temp = num;
    while (aux > 0){
        for(j=i;j>0;j--){
            string[j] = string[j-1];
        }
        temp = aux%10;
        string[0] = (temp + 48);
        aux = aux/10;
        i++;
    }
}

/**
 * @brief Recebe as componentes do endereço e gera o endereço.
 *
 *
 *
 */

int Adress(int j,int k,int l){
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

/**
 * @brief Recebe as componentes do endereço por referência e um inteiro, atualizando as componentes para apontares para aquele endereço
 *
 *
 *
 */

void RevAdress(int endereco,int* j,int* k,int* l){
    *l = endereco/AddCilindro;
    *k = (endereco%AddCilindro)/AddTrilha;
    *j = (endereco%AddCilindro)%AddTrilha;
}

/**
 * @brief Retorna a primeira posição do primeiro arquivo com dado nome
 *
 *
 *
 */

int procura_registro(char* nome,fatlist* Afatlist,int* pos,int fim){
    int i = 0;

    while(i<fim){
        if(comparastring(nome,Afatlist[i].nome)){
            *pos = i;
            printf("\nArquivo de nome %s de numero %d encontrado.\n",Afatlist[i].nome,i);
            printf("\nArquivo de nome %s inicia em %d encontrado.\n",Afatlist[i].nome,Afatlist[i].first_sector);
            return(Afatlist[i].first_sector);
        }
        i++;
    }
    return(-1);
}

/**
 * @brief Imprime a tabela FAT
 *
 * Função responsável por implementar a funcionalidade 4 do trabalho, onde ele mostra a tabela FAT
 * com nome dos arquivos, setores por ele ocupado e o tamanho em disco (em bytes).
 *
 */

void imprime_fat(fatlist* Afatlist,fatent* Afatent,int fat_num){
    int loc=0;
    int i=0,j=0,k=0,n=0;
    int tam = 0;
    int atraso = 0;/*Inicializado com o valor de seek do primeiro cilindro*/
    char StrNumber[20];
    int* stops;
    int* aux;

    //printf("|fat_num = %d|",fat_num);
    if (fat_num > 0){//!*Caso hajam arquivos gravados ...*/
            printf("\nNome:               Tamanho Em Disco:   Localizacao:");
        while(i < fat_num){/*Enquanto houverem arquivos gravados não mostrados na tabela...*/
            j = 0;
            k = 0;
            loc = Afatlist[i].first_sector;
            atraso = atraso + SEEK;
            stops = (int*)malloc(2*sizeof(int));
            stops[j] = loc;//!*Salva o inicio de uma sequencia contínua de dados...*/
            //printf("%d",loc);
            tam = 512*loc;
            loc = Afatent[loc].next;
            //printf(" -> ");
            while(Afatent[loc].eof != 1){
                if((Afatent[loc].next - loc) > 1){/*E caso haja uma "quebra" de continuidade...*/
                    j++;
                    aux = (int*)malloc((j*2+2)*sizeof(int));
                    for(k=0;k<(j*2-1);k++){
                        aux[k] = stops[k];
                    }
                    free(stops);
                    aux[j*2-1] = loc; /*Salva o endereco dessa quebra*/
                    //printf(" %d, ", loc);
                    tam = tam - 512*(1+loc);/*Bem como o tamanho do arquivo nesse intervalo*/
                    loc = Afatent[loc].next;
                    aux[j*2] = loc;/*Salvando tambem o inicio da proxima sequencia continua de dados...*/
                    stops = aux;
                    //printf(" %d -> ", loc);
                    tam = tam + 512*loc;
                }else{
                    loc = Afatent[loc].next;/*Nao havendo interrupcoes, ele segue para o proximo setor*/
                }
            }
            //printf("%d.\n\n", loc);
            stops[j*2+1] = loc;
            tam = tam - 512*(1+loc);
            tam = -tam;
            //NumtoSring(tam,StrNumber);/*Converte um inteiro para uma string que representa tal número*/
            n = printf("\n%s",Afatlist[i].nome);/*Mostra o Nome do n-esimo arquivo...*/
            for (k=0;k<=(20 - n);k++){/*Preenche espacos vazios, para alinhar os dados*/
                printf(" ");
            }
            n = printf("%d bytes",tam);/*Preenche espacos vazios, para alinhar os dados*/
            for (k=0;k<(20 - n);k++){
                printf(" ");
            }
            for(k=0;k<j;k++){//!*Mostra os setores que um arquivo ocupa, de uma forma mais legível que a sugerida no roteiro*/
                printf("%d -> %d, ",stops[k*2],stops[k*2+1]);
            }
            printf("%d -> %d.",stops[k*2],stops[k*2+1]);
            i++;
            free(stops);
        }
    }else{
        printf("\nNenhum arquivo\n");
    }
    printf("\n\n");
}

/**
 * @brief Apaga um arquivo do HD
 *
 * Função responsável por implementar a função 3 do trabalho, que apaga um registro setando os campos de "used" para 0 novamente e
 * ajustando o setor inicial do arquivo.
 *
 */

int apagar_registro(fatent* Afatent, fatlist* Afatlist,int fatnum){
    char nome[100];
    int i=0,k=0;;
    int loc = -1;
    fatent Bfatent;

    printf("\nDigite o nome do registro a ser apagado\n");/*Lê o nome do arquivo a ser removido*/
    scanf("%s",nome);
    getchar();
    while(nome[i]!='\0'&&nome[i]!='.'){/*Põe .txt no final do arquivo, para o caso de o usuario nao ter digitado ou diditar a extensao errada*/
        i++;
    }
    nome[i]= '.';
    nome[i+1]= 't';
    nome[i+2]= 'x';
    nome[i+3]= 't';
    nome[i+4]= '\0';

    loc = procura_registro(nome,Afatlist,&i,fatnum);/*Procura tal arquivo na FAT*/
    if (loc >= 0){/*Se encontrar o arquivo na tabela ...*/
        fatnum = fatnum - 1;/*...reduzir o número de arquivos salvos em 1...*/
        while(Afatent[loc].eof == 0){/*...zerar todos os campos usados pelo arquivo...*/
            Afatent[loc].used = 0;
            loc = Afatent[loc].next;
        }
        Afatent[loc].used = 0;
        Afatent[loc].eof = 0;/*...incluindo o marcador de fim de arquivo...*/
        printf("\nArquivo %s Apagado, inicio em %d\n",nome,Afatlist[i].first_sector);
        printf("\nArquivo %s Ocupando o final, inicio em %d\n",Afatlist[fatnum].nome,Afatlist[fatnum].first_sector);
        Afatlist[i].first_sector = Afatlist[fatnum].first_sector;/*...e copia o ultimo elemento da tabela para a posicao vazia*/
        while(Afatlist[fatnum].nome[k]!='\0'){
            Afatlist[i].nome[k] = Afatlist[fatnum].nome[k];
            k++;
        }
        Afatlist[i].nome[k]='\0';
        printf("\nArquivo %s substituido por %s, inicio em %d\n",nome,Afatlist[i].nome,Afatlist[i].first_sector);
    }else{
        printf("\nArquivo %s não Encontrado\n",nome);
    }
return(fatnum);/*retorna o novo número de arquivos, por comodidade ao programador*/
}

/**
 * @brief Função que escreve no HD o registro passado como argumento.
 *
 * Função que recebe o número de cilindros alocados, a tabela FAT, o número de arquivos gerados atualmente e o nome do arquivo que contém os registros,
 * retornando os registros gravados no HD e atualizando a tabela FAT com os setores utilizados nessa gravação.
 *
 */


fatent* escrever_registro(char* nome,int* l/*l = número de cilindros alocados*/,track_array*** start,int* FAT_NUM,fatlist* fatlistp,fatent* fatentp){
    FILE* fp;
    char linha[513];
    track_array** cilindro;
    track_array** new_start;
    fatent* new_fatentp;
    int i=0,j=0,k=0,n=0,c=0,t=0;
    int aj=0,ak=0,an=0;
    char aux;
    int temp = 0;
    int end = 0;
    int loc = 0;
    int last_loc = 0;
    int save = 0;
    int atraso = SEEK;


    while(nome[i]!='\0'&&nome[i]!='.'){/*Poe a extensao do arquivo como .txt*/
        i++;
    }
    nome[i]= '.';
    nome[i+1]= 't';
    nome[i+2]= 'x';
    nome[i+3]= 't';
    nome[i+4]= '\0';
    new_start = start[0];/*isso garante que se a função não alocar mais cilindros, nada mude*/
    cilindro = start[0];
    fp = fopen(nome,"r");
    loc = Adress(0,0,0);
    while(end == 0 && fp != NULL){/*Enquanto o arquivo não chegar no final, tentar escrever*/
        //printf("Verificando ... \n");
        loc = Adress(j,k,n);
        while(end == 0 && n<(*l)){/*repete até chegar no número de cilindros alocado*/
            //printf("Movendo para cilindro %d \n",n);
            loc = Adress(j,k,n);
            k=0;
            while(end == 0 && k<5){/*Repete até chegar no final do cilindro*/
                //printf("Movendo para trilha %d \n",k);
                loc = Adress(j,k,n);
                j=0;
                while(end == 0 && j<60){/*Repete até chegar no final da trilha*/
                    loc = Adress(j,k,n);
                    //printf("Movendo para setor %d -> endereco |%d|\n",j,loc);
                    c = j;
                    if(fatentp[loc].used!=1){
                        while(c<4+j){/*Anda em clusters de 4 setores, reservando 4 setores sempre que encontra 1 disponível*/
                            i=0;
                            loc = Adress(c,k,n);
                            fatentp[loc].used = 1;
                            //printf("Checando elemento em %d \n",loc);
                            if(save == 0){/*Salva o inicio do arquivo na tabela*/
                                fatlistp[*FAT_NUM].first_sector = loc;
                                while(nome[i]!='\0'){/*um while que copia a string, mais nada*/
                                    fatlistp[*FAT_NUM].nome[i] = nome[i];
                                    i++;
                                }
                                fatlistp[*FAT_NUM].nome[i] = '\0';
                                last_loc = loc;/*Guarda o endereço de início*/
                                save = 1;
                            }else{
                                fatentp[last_loc].next = loc;
                                RevAdress(last_loc,&aj,&ak,&an);
                                if((loc - last_loc) > 1){/*Houve um salto na escrita*/
                                    if(an != n){/*Mudou de cilindro => SEEK + transferencia do setor*/
                                        atraso = atraso + SEEK + TRANSFER_S;
                                    }else{/*Não mudou de cilindro, rodou o disco + transferencia do setor*/
                                        atraso = atraso + LATENCY + TRANSFER_S;
                                    }
                                }else{/*Se nao houve salto, tempo de transferencia*/
                                    atraso = atraso + TRANSFER_S;
                                }
                                last_loc = loc;
                            }
                            //printf("fatent[%d].next = %d\n",last_loc,loc);
                            i=0;
                            //linha[i] = '\0';
                            while(end == 0 && i < 512){/*Escreve em um setor até chegar no fim do arquivo ou o setor acabar*/
                                temp = fscanf(fp,"%c",&cilindro[n]->track[k].sector[c].bytes_s[i]);
                                //if(cilindro[n]->track[k].sector[c].bytes_s[i] != '\n' || cilindro[n]->track[k].sector[c].bytes_s[i-1] != '.'){/*esse IF se livra daquelas quebras de linha escondidas*/
                                i++;
                                //}
                                if(temp == EOF){
                                    //printf("End Of File\n");
                                    end = 1;
                                    if(i <= 512){
                                        i--;
                                        cilindro[n]->track[k].sector[c].bytes_s[i] = '\0';
                                    }
                                }
                            }
                            //getchar();
                            //system("cls");
                            c++;
                            //printf("Preparando para mover(Setor)\n");
                            //printf("Last_loc = %d \n",last_loc);
                        }
                        if(end == 1){
                            fatentp[loc].eof = 1;
                        }
                    }
                    //printf("Preparando para mover(Cluster)\n");
                    j = j+4;
                }
                //printf("Preparando para mover(Trilha)\n");
                k++;
            }
            //printf("Preparando para mover(Cilindro)\n");
            //system("cls");
            n++;
        }
        if(end == 0 && temp != EOF){/*Se o arquivo não tiver terminado de ser escrito ... */
            printf("Alocando cilindro %d \n",n);
            c = 0;
            new_start = (track_array**)malloc((*l+1)*sizeof(track_array*));/*Então aloca um vetor de ponteiros de cilindros com +1 casa...*/
            if(new_start != NULL){/*se new_start for NULL, ago deu ruim na alocação de espaço*/
                while(c<(*l)){/*Copia o vetor antigo no vetor novo*/
                    new_start[c] = start[0][c];
                    c++;
                }
                new_start[c] = (track_array*)malloc(sizeof(track_array));/*Aloca o novo cilindro no novo vetor*/
                start[0] = new_start;
                cilindro = start[0];
            }else{
                printf("Erro na alocacao do novo cilindro\n");
                end = 1;/*isto sairá do loop externo e então da função*/
            }
            printf("Alocando tabela %d - %d \n",(*l)*300,((*l)+1)*300);
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
                //getchar();
            }else{
                printf("Erro na alocacao da tabela\n");
                end = 1;/*isto sairá do loop externo e então da função*/
            }
        }
    }
    if(fp != NULL){/*se fp for NULL aqui, algo deu errado na hora de abrir o arquivo e vai dar ruim se tentar fechá-lo*/
        fclose(fp);
        printf("\nArquivo %s escrito em %d.%d milisegundos\n",nome,atraso/1000,atraso - (atraso/1000)*1000);
        *FAT_NUM = *FAT_NUM + 1;
    }else{
        printf("Arquivo nao encontrado\n");
    }
    //printf("Retornando de |Escrever regitro|\n\n");
    return(fatentp);
}


void geraarquivo(char* In, char* nome){//!*Escreve um arquivo contendo a string In de nome "nome". Não utilizada nesta versão.*/
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

/**
 * @brief Escreve um arquivo contendo a string In de nome "nome". imprime uma string.
 *
 *
 *
 */

void geraarquivo_b(track_array** start,fatent* FatEnt,fatlist* FatList, char* nome, int fatnum){
    FILE* fpo;
    int i = 0;
    int j=0,k=0,l=0;
    int loc = 0;
    int temp = 0;
    int atraso = SEEK;
    int aj = 0, ak = 0, an = 0;
    int aj2 = 0, ak2 = 0, an2 = 0;



    while(nome[i]!='\0'&&nome[i]!='.'){/*Põe .txt como extensão do arquivo*/
        i++;
    }
    nome[i]= '.';
    nome[i+1]= 't';
    nome[i+2]= 'x';
    nome[i+3]= 't';
    nome[i+4]= '\0';
    i=0;

    while(temp!=1 && i<fatnum){/*Procura a string na lista de arquivos salvos*/
        temp = comparastring(FatList[i].nome,nome);
        i++;
    }
    if(temp==1){/*Se encontrar a string na lista de arquivos salvos ...*/
        loc = FatList[i-1].first_sector;/*... pega o endereço de onde começa o arquivo*/
        i=0;
        while(nome[i]!='\0'&&nome[i]!='.'){/*Altera o nome do arquivo de saida, adicionando um "SAIDA.txt" ao final do nome original*/
            i++;
        }
        nome[i]= 'S';
        nome[i+1]= 'A';
        nome[i+2]= 'I';
        nome[i+3]= 'D';
        nome[i+4]= 'A';
        nome[i+5]= '.';
        nome[i+6]= 't';
        nome[i+7]= 'x';
        nome[i+8]= 't';
        nome[i+9]= '\0';
        fpo = fopen(nome,"w");
        while(FatEnt[loc].eof != 1){/*E equanto não chegar no fim do arquivo ...*/
            i=0;
            RevAdress(loc,&j,&k,&l);/*Pega as coordenadas do setor*/
            while(start[l]->track[k].sector[j].bytes_s[i]!= '\0' &&i<512){/*printa o setor*/
                fprintf(fpo,"%c",start[l]->track[k].sector[j].bytes_s[i]);
                i++;
            }
            RevAdress(FatEnt[loc].next,&aj,&ak,&an);
            if((loc - FatEnt[loc].next) > 1){/*Houve um salto na escrita*/
                if(an != l){/*Mudou de cilindro => SEEK*/
                    atraso = atraso + SEEK + TRANSFER_S;
                }else{/*Não mudou de cilindro, rodou o disco*/
                    atraso = atraso + LATENCY + TRANSFER_S;
                }
            }else{/*Se nao houve salto, tempo de transferencia*/
                atraso = atraso + TRANSFER_S;
            }
            loc = FatEnt[loc].next;/*e avança pro próximo*/
        }
        i=0;
        RevAdress(loc,&j,&k,&l);/*Pega as coordenadas do setor*/
        while(start[l]->track[k].sector[j].bytes_s[i]!= '\0' && i<512){/*printa o setor com eof*/
            fprintf(fpo,"%c",start[l]->track[k].sector[j].bytes_s[i]);
            i++;
        }
        fclose(fpo);
        atraso = atraso + LATENCY;/*Rotacao do disco pra chegar ao final*/
        printf("\nArquivo %s Lido em %d.%d milisegundos\n",nome,atraso/1000,atraso - (atraso/1000)*1000);
    }else{
        printf("Arquivo %s Nao Encontrado\n",nome);
    }
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
    int fat_num = 0;
    char nome[100];

    Fatent = (fatent*)calloc(300,sizeof(fatent));
    cylinder = (track_array*)malloc(sizeof(track_array));
    inicio_disco = &cylinder;
    while (option != 5){
        system("cls");
        switch(option){
            case(1):
                printf("Selecionado: Criar novo registro. \n");
                printf("Digite o nome do arquivo que contem os registros\n");
                scanf("%s",nome);
                getchar();
                Fatent = escrever_registro(nome,&l,&inicio_disco,&fat_num,Fatlist,Fatent);
                break;
            case(2):
                printf("Selecionado: Ler do registro \n");
                printf("Digite o nome do arquivo que deseja ler\n");
                scanf("%s",nome);
                getchar();
                geraarquivo_b(inicio_disco,Fatent,Fatlist,nome,fat_num);
                break;
            case(3):
                printf("Selecionado: Apagar um registro \n");
                fat_num = apagar_registro(Fatent,Fatlist,fat_num);
                break;
            case(4):
                printf("Selecionado: Mostrar tabela FAT \n");
                imprime_fat(Fatlist,Fatent,fat_num);
                break;
            case(5):
                printf("Selecionado: Sair \n");
                printf("Tem certeza que deseja sair?\n 5 - sim, 0 - nao \n");
                scanf("%d",&option);
                break;

        }
        printf("\nEstado: %d Cilindros, %d arquivos\n",l,fat_num);
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
