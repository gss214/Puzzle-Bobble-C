/*
###################################################################################################################
# Universidade de Brasilia                                                                                        #
# Instituto de Ciencias Exatas                                                                                    #
# Departamento de Ciencia da Computacao                                                                           #
#                                                                                                                 #
# Algoritmos e Programação de Computadores – 1/2019                                                               #
#                                                                                                                 #
# Aluno(a): Guilherme Silva Souza                                                                                            #
# Turma: A                                                                                                        #
# Versão do compilador: gcc (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0                                                 #
#                                                                                                                 #
# Descricao: O programa eh um jogo em um grid 2D representado por uma matriz similar ao jogo                      #
# Puzzle Bobble ​(também conhecido como Bust-a-Move). O jogo possui 5 peças aleatórias, que aparecerão uma por vez #
# em um ponto fixo no meio da base do tabuleiro, e após a ação do jogador, esta deve seguir para o                #
# lugar indicado. O possível lugar final que a peça ficará deve ser visível para que o jogador tome               #
# a ação de dispará-la, e este caminho também deve ser controlado pelo jogador. O jogador tem                     #
# como objetivo conseguir o maior número de pontos antes do fim do jogo, que acaba no momento                     #
# em que as peças alcançam a base do tabuleiro. Conforme o passar do tempo de jogo, ele se torna                  #
# mais difícil, de forma com que a linha pertencente ao topo do tabuleiro jogável vai “descendo”                  #
# aos poucos.                                                                                                     #
###################################################################################################################
*/

#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <termios.h> caso esteja no linux pode descomentar essa linha
#include <time.h>
#include <unistd.h>

/*
##########
# Macros #
##########
*/

#define BLUE "\x1b[34m"
#define CYAN "\x1b[36m"
#define GREEN "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

/*
###################################################################
# Definindo as funcoes kbhit() e getch() para ser usadas em linux #
###################################################################
*/

#ifndef _WIN32
int kbhit()
{
    int ch, oldf;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
int getch()
{
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#else
#include <conio.h>
#endif

/*
##################################################
# Struct para guardar as informacoes do jogador. #
##################################################
*/

typedef struct
{
    char nome[11];
    int pontos;
} DadosJogador;

/*
#####################
# Variaveis globais #
#####################
*/

char tabuleiro[10][17];
char peca;
int altura = 10, largura = 17;
float coeficientemira = 0;
int gameover;
int tempo, auxtempo, tempodedescida; //variaveis de controle de tempo
char logotxt, sairtxt, gameovertxt;  //variaveis para controle dos arquivos de textos
int oldj, newj;
float oldcoeficiente;                                                   //variaveis para controle de mira (explicado menlhor na funcao mira)
int criarreplay, jogarreplay, contvetreplay, modoranking, naoranqueado; //as variaveis para controle dos modos de jogo
FILE *arquivoreplay;                                                    //arquivo para salvar as pecas do modo replay
FILE *arquivoconfig;                                                    //arquivo para salvar as configuracoes
FILE *arquivoranking;                                                   //arquvo para salvar o ranking
char vetreplay[200];                                                    //vetor que vai ser carregado com as pecas lidas do arquivo do modo replay
char vetalfabeto[52];                                                   //vetor para a selecao de nick
DadosJogador jogador;
int vetranking[9];

/*
########################
# Cabecalho de funcoes #
########################
*/

int menuMain();
int verificaArquivos();
void carregarLogo();
int menuJogar();
void jogar();
void instrucoes();
int configuracoes();
int configuracoesPecas();
int ranking();
int inserirNick();
void limpaNick();
void printaInformacoesNick();
int sair();
int verificaGameover();
int gameoverMenu();
int registerMatch();
void inicializaRanking();
void inicializaJogo();
void printarTabuleiro();
void desceTabuleiro();
char criarPeca();
void lancarPeca();
void animacaoPeca();
void mira();
int equacaoMira(float m, int x);
int contaTempo();

/*
###########
# Funcoes #
###########
*/

void main()
{

    int arquivos = 0;

    //verificacao para ver se o arquido de ranking existe, se nao existir eh chamada a funcao para inicializar o ranking
    arquivoranking = fopen("ranking.bin", "rb");
    if (arquivoranking == NULL)
    {
        inicializaRanking();
        fclose(arquivoranking);
    }

    //verificacao para ver se o arquido de config existe, se nao existir vai ser criado com valores default
    arquivoconfig = fopen("config.txt", "r");
    if (arquivoconfig == NULL)
    {
        arquivoconfig = fopen("config.txt", "w");
        tempodedescida = 20;
        modoranking = 0;
        fprintf(arquivoconfig, "%d %d", tempodedescida, modoranking);
        fclose(arquivoconfig);
    }

    system(CLEAR);
    arquivos = verificaArquivos();

    if (arquivos == 0)
    {
        menuMain();
    }
    else
    {
        exit(0);
    }
}

/*
#############################################################################
# A funcao abaixo controla o menu e tambem controla o loop da funcao jogar. #
#############################################################################
*/

int menuMain()
{
    int input;

    system(CLEAR);
    //linha para gerar um novo seed de aleatoriedade
    srand(time(NULL));
    carregarLogo();
    printf("\n");

    printf("Selecione uma opcao: \n\n");
    printf("1 - Jogar\n2 - Instrucoes\n3 - Configuracoes\n4 - Ranking\n5 - Sair\n");

    input = getch();

    while (1)
    {
        switch (input)
        {
        //caso ele aperte 1
        case 49:
            system(CLEAR);
            inicializaJogo();
            //se o modo ranqueado tiver desativado
            if (modoranking == 0)
            {
                //ele pega abre o arquivo de config e pega as config definida pelo usuario
                arquivoconfig = fopen("config.txt", "r");
                fscanf(arquivoconfig, "%d %d", &tempodedescida, &modoranking);
                fclose(arquivoconfig);
                //se o modo ranqueado tiver desativado vai ser chamada a funcao menuJogar
                int r = menuJogar();
                //loop para verificar o return, se for 0 eh porque o jogador nao escolheu nenhum modo, se for 1 ele vai sair do loop
                while (r == 0)
                {
                    r = menuJogar();
                }
            }
            //se o modo ranqueado tiver ativo
            else if (modoranking == 1)
            {
                //vai para a tela de selecao de nick
                inserirNick();
                tempodedescida = 20;
            }
            criarPeca();
            printarTabuleiro();
            //inicia a contagem do tempo
            tempo = time(NULL);
            // loop da funcao jogar
            while (gameover == 0)
            {
                jogar();
                printarTabuleiro();
                auxtempo = contaTempo(&tempo);
                if (auxtempo == tempodedescida)
                {
                    auxtempo = 0;
                    tempo = time(NULL);
                    desceTabuleiro();
                }
                gameover = verificaGameover();
            }
            //se sair do while acima eh porque deu gameover
            auxtempo = 0;
            gameoverMenu();
            if (criarreplay == 1)
            {
                fclose(arquivoreplay);
            }
            menuMain();
            break;
        case 50:
            system(CLEAR);
            instrucoes();
            menuMain();
            break;
        case 51:
            configuracoes();
            menuMain();
            break;
        case 52:
            ranking();
            menuMain();
            break;
        case 53:
            sair();
            break;
        default:
            printf(RED "\nOpcao invalida. Digite novamente: " RESET);
            break;
        }
        input = getch();
    }
    return 0;
}

/*
######################################################################################################
# A funcao abaixo verifica se os arquvos de textos usados nesse programa estão na pasta do programa. #
# Se faltar algum arquivo ele diz qual arquivo esta faltando e encerra o programa.                   #
######################################################################################################
*/

int verificaArquivos()
{
    int arquivos;

    FILE *file1 = fopen("arte/logo.txt", "r");
    if (file1 == NULL)
    {
        printf(RED "Erro: " RESET);
        printf("Nao foi possivel abrir o arquivo logo.txt. Verifique se o arquivo esta na pasta desse programa.\n");
        arquivos = 1;
    }

    FILE *file2 = fopen("arte/sair.txt", "r");
    if (file2 == NULL)
    {
        printf(RED "Erro: " RESET);
        printf("Nao foi possivel abrir o arquivo sair.txt. Verifique se o arquivo esta na pasta desse programa.\n");
        arquivos = 1;
    }

    FILE *file3 = fopen("arte/gameover.txt", "r");
    if (file3 == NULL)
    {
        printf(RED "Erro: " RESET);
        printf("Nao foi possivel abrir o arquivo gamerover.txt. Verifique se o arquivo esta na pasta desse programa.\n");
        arquivos = 1;
    }

    FILE *file4 = fopen("arte/ranking.txt", "r");
    if (file4 == NULL)
    {
        printf(RED "Erro: " RESET);
        printf("Nao foi possivel abrir o arquivo rankng.txt. Verifique se o arquivo esta na pasta \"arte\" desse programa.\n");
        arquivos = 1;
    }
    if (arquivos == 1)
    {
        printf("Aperte qualquer tecla para sair do programa: ");
        fclose(file1);
        fclose(file2);
        fclose(file3);
        fclose(file4);
        getchar();
        return 1;
    }
    else
    {
        fclose(file1);
        fclose(file2);
        fclose(file3);
        fclose(file4);
        arquivos = 0;
        return 0;
    }
    return 0;
}

/*
###############################################################
# A funcao abaixo eh responsavel por carregar o logo do game. #
###############################################################
*/

void carregarLogo()
{
    FILE *file = fopen("arte/logo.txt", "r");
    while ((logotxt = fgetc(file)) != EOF)
    {
        printf(BLUE "%c" RESET, logotxt);
    }
    fclose(file);
}

/*
#########################################################################
# A funcao abaixo eh responsavel pelo menu de selecao de modos de jogo. #
#########################################################################
*/

int menuJogar()
{
    int input, i = 0, flag = 0;
    char nomearquivo[50];

    //menu de modos de jogo
    carregarLogo();
    system(CLEAR);
    printf("\nSelecione o modo de jogo:\n\n");
    printf("1: Criar arquivo de replay\n");
    printf("2: Jogar no modo replay\n");
    printf("3: Jogo Nao-Ranqueado\n");
    printf("4: Voltar ao menu\n");

    input = getch();

    while (flag == 0)
    {
        if (input == 49)
        {
            criarreplay = 1;
            jogarreplay = 0;
            flag = 1;
        }
        else if (input == 50)
        {
            jogarreplay = 1;
            criarreplay = 0;
            flag = 1;
        }
        else if (input == 51)
        {
            naoranqueado = 1;
            jogarreplay = 0;
            criarreplay = 0;
            return 1;
        }
        else if (input == 52)
        {
            menuMain();
        }
        else
        {
            printf(RED "Opcao invalida. Digite novamente\n" RESET);
            input = getch();
        }
    }
    //se o mdo escolhido for criar replay, eh aberto um arquivo de texto para salvar as pecas
    if (criarreplay == 1)
    {
        flag = 0;
        system(CLEAR);
        printf("Digite 1 para criar o arquivo replay\n");
        printf("Digite 2 para voltar\n");
        input = getch();
        while (flag == 0)
        {
            if (input == 49)
            {
                printf("\nDigite o nome do arquivo-replay que deseja criar (insira o .txt no final do nome):\n");
                scanf("%[^\n]", nomearquivo);
                flag = 1;
            }
            else if (input == 50)
            {
                return 0;
            }
        }
        arquivoreplay = fopen(nomearquivo, "w");
        return 1;
    }
    //se o modo escolhido for jogar replay, eh aberto o mesmo arquivo de texto, lido e passado para um vetor as pecas
    if (jogarreplay == 1)
    {
        flag = 0;
        system(CLEAR);
        printf("Digite o nome do arquivo-replay que deseja jogar (insira o .txt no final do nome):\n");
        scanf("%[^\n]", nomearquivo);
        arquivoreplay = fopen(nomearquivo, "r");
        //loop para verificar se o arquivo digitado existe
        while (arquivoreplay == NULL)
        {
            printf(RED "\nOpa! Esse arquivo nao existe." RESET);
            printf("\nDigite 1 para digitar novamente o nome do arquivo-replay que deseja jogar\n");
            printf("Digite 2 para voltar\n");
            getchar();
            flag = 0;
            while (flag == 0)
            {
                input = getch();
                if (input == 49)
                {
                    printf("\nDigite o nome do arquivo-replay que deseja jogar (insira o .txt no final):\n");
                    scanf("%[^\n]", nomearquivo);
                    flag = 1;
                }
                else if (input == 50)
                {
                    return 0;
                }
            }
            arquivoreplay = fopen(nomearquivo, "r");
        }
        while (fscanf(arquivoreplay, "%c", &vetreplay[i]) == 1)
        {
            i++;
        }
        return 1;
    }
    return 0;
}

/*
##########################################################################################
# Funcao abaixo eh a jogar, ela é o loop do jogo. A funcao espera uma tecla ser apertada #
# e conforme a tecla o usuario apertou eh realizado uma acao.                            #
##########################################################################################
*/

void jogar()
{
    int i, j, input;

    oldcoeficiente = coeficientemira;

    if (kbhit() == 1)
    {
        //input do usuario eh pego
        input = getch();

        //caso o input for "a", "d", "A" ou "D" (direira ou esquerda), altera-se o coeficiente angular da mira
        if (input == 97 || input == 100 || input == 65 || input == 68)
        {
            if (input == 97 || input == 65)
            {
                coeficientemira -= 0.1;
                if (coeficientemira <= -1)
                {
                    coeficientemira = -1;
                }
            }
            else if (input == 100 || input == 68)
            {
                coeficientemira += 0.1;
                if (coeficientemira >= 1)
                {
                    coeficientemira = 1;
                }
            }
            mira();
        }

        //e caso o input for o espaco eh chamada a funcao de lancar peca
        else if (input == 32)
        {
            animacaoPeca();
            lancarPeca();
            contvetreplay += 2;
            criarPeca();
        }
    }
}

/*
################################################################333#####################################################################
# A funcao abaixo eh reponsavel pelas instrucoes. Ela eh chamad na funcao de menu caso o usuario selecione a opcao relacionada a ela. #
# Ela diz ao usuario instruncoes basicas para jogar o jogo e esapera uma tecla para sair e volta ao menu.                               #
########################################################################################################################################
*/

void instrucoes()
{
    carregarLogo();
    printf("\nComandos:\n");
    printf("\nInclinar mira para a esquerda - a\nInclinar mira para a direita - d\nLancar letra - Barra de Espaço\n");
    printf("\nAo formar um conjunto de 4 ou mais pecas iguais conectadas o jogador ganha a quantidade de pontos equivalente a quantidade de pecas eliminadas, caso um conjunto de pecas eliminidas resulte em um conjunto desconectado as pecas desconectadas caem");
    printf("\n\nAperte qualquer tecla para voltar ao menu: ");
    getch();
}

/*
###############################################################
# A funcao abaixo eh responsavel pelas configuracoes do jogo. #
###############################################################
*/

int configuracoes()
{
    int input;

    //menu de configuracoes
    system(CLEAR);
    carregarLogo();
    printf("\nSelecione uma opcao: \n\n");
    printf("1: Pecas\n");
    printf("2: Ativar/Desativar Modo Ranqueado\n");
    printf("3: Voltar\n");

    //condicional para saber o estado do modo ranqueado
    if (modoranking == 1)
    {
        printf(GREEN "\nModo Ranqueado Ativado\n" RESET);
    }
    else if (modoranking == 0)
    {
        printf(GREEN "\nModo Ranqueado Desativado\n" RESET);
    }

    input = getch();

    while (1)
    {
        switch (input)
        {
        case 49:
            //caso o usuario apertar '1' vai para eh chamado o submenu configuracoesPecas
            configuracoesPecas();
            //assim que voltar o menu configuracoes eh printado novamente na tela
            system(CLEAR);
            carregarLogo();
            printf("\nSelecione uma opcao: \n\n");
            printf("1: Pecas\n");
            printf("2: Ativar/Desativar Modo Ranqueado\n");
            printf("3: Voltar\n");

            if (modoranking == 1)
            {
                printf(GREEN "\nModo Ranqueado Ativado\n" RESET);
            }
            else if (modoranking == 0)
            {
                printf(GREEN "\nModo Ranqueado Desativado\n" RESET);
            }
            break;
        case 50:
            //caso o usario aperte '2' eh verificado o estado do modo ranqueado e trocado (se for 0 vira 1 e se for 1 vira 0)
            if (modoranking == 1)
            {
                modoranking = 0;
                printf(GREEN "Modo Ranqueado Desativado\n" RESET);
            }
            else if (modoranking == 0)
            {
                modoranking = 1;
                printf(GREEN "Modo Ranqueado Ativado\n" RESET);
            }
            break;
        case 51:
            //caso o usuario aperte '3' eh salvado no arquivo config.txt o tempo de descida e a situacao do modo ranqueado
            arquivoconfig = fopen("config.txt", "w");
            fprintf(arquivoconfig, "%d %d", tempodedescida, modoranking);
            fclose(arquivoconfig);
            return 1;
            break;
        default:
            printf(RED "Opcao invalida. Digite novamente\n" RESET);
            break;
        }
        input = getch();
    }
    return 0;
}

/*
########################################################################
# A funcao abaixo eh responsavel pelo submenu configuracoes das pecas. #
########################################################################
*/

int configuracoesPecas()
{
    int flag = 0;
    int input;

    system(CLEAR);
    carregarLogo();
    //menu configuracoes pecas
    printf("\nSelecione uma opcao: \n\n");
    printf("1 - Alterar a Velocidade de Descida do Tabuleiro (MIN: 10s e MAX: 35s)\n");
    printf("2 - Voltar\n");

    //pega uma tecla do usuario
    input = getch();
    while (1)
    {
        system(CLEAR);
        carregarLogo();
        //submenu configuracoes pecas
        printf("\nSelecione uma opcao: \n\n");
        printf("1 - Alterar a Velocidade de Descida do Tabuleiro (MIN: 10s e MAX: 35s)\n");
        printf("2 - Voltar\n");
        switch (input)
        {
        //se a tecla apertada for 1, o usuario vai digitar um novo tempo de descida
        case 49:
            system(CLEAR);
            carregarLogo();
            printf("\nDigite a nova velocidade de descida: ");
            scanf("%d", &tempodedescida);
            //loop para verificacao
            while (tempodedescida > 35 || tempodedescida < 10)
            {
                printf(RED "Velocidade de descida invalida, digite uma velocidade entre 10s e 35s: " RESET);
                scanf("%d", &tempodedescida);
            }
            printf(GREEN "O novo tempo de descida eh: %ds\n" RESET, tempodedescida);
            fflush(stdout);
            sleep(2);
            break;
        //caso o usuario aperte 2, ele volta oara o menu configuracoe pecas
        case 50:
            return 1;
            break;
        default:
            printf(RED "Opcao invalida. Digite novamente\n" RESET);
            break;
        }
        input = getch();
    }
    return 0;
}

/*
#################################################################
# A funcao abaixo eh responsavel por mostrar o ranking na tela. #
#################################################################
*/

int ranking()
{

    int i, cont = 0, input;
    char rankingtxt;

    system(CLEAR);

    //esse arquivo ranking.txt eh so a imagem que fica no topo da tela
    FILE *file = fopen("arte/ranking.txt", "r");
    while ((rankingtxt = fgetc(file)) != EOF)
    {
        printf(BLUE "%c" RESET, rankingtxt);
    }
    fclose(file);

    printf("Pos  Nome          Pontos\n");
    while (cont < 10)
    {
        fread(&jogador, sizeof(DadosJogador), 1, arquivoranking);
        if (cont <= 8)
        {
            printf("%d    %s             %d\n", cont + 1, jogador.nome, jogador.pontos);
            cont++;
        }
        else
        {
            printf("%d   %s             %d\n", cont + 1, jogador.nome, jogador.pontos);
            cont++;
        }
    }

    printf("\nAperte 1 para voltar ao menu\n");
    input = getch();
    while (1)
    {
        if (input == 49)
        {
            return 1;
        }
        input = getch();
    }
}

/*
#########################################################################################################
# A funcao abaixo eh responsavel pelo sistema de insercao de nick. Sistema baseado nos jogos classicos. #
#########################################################################################################
*/

int inserirNick()
{
    int i, contacaracter = 0, flag = 0;
    int ascii = 65;
    int input;

    limpaNick();

    vetalfabeto[0] = '>'; //a primeira posicao do vetor alfabeto eh iniciada com '>'

    //loop que inicializa o vetor alfabeto
    for (i = 1; i < 52; i++)
    {
        //se o indice for impar eh adicionado uma letra (comecando pela letra 'A' que corresponde a 69 na tabela ascii)
        if (i % 2 != 0)
        {
            vetalfabeto[i] = ascii;
            ascii++; //eh incrementado em 1 o int ascii para que seja a proxima letra do alfebeto
        }
        //se o indice for par eh adicionado o espaco
        else
        {
            vetalfabeto[i] = ' '; //na pocisao i vai ser colocado o espaco (dando a sensacao que a seta se moveu)
        }
    }

    //loop que inicializa o nome //na pocisao i vai ser colocado o espaco (dando a sensacao que a seta se moveu)om espacos
    for (i = 0; i < 9; i++)
    {
        jogador.nome[i] = ' ';
    }

    ascii = 65;              //o int ascii eh resetado para 65 (letra 'A')
    printaInformacoesNick(); //chamada a funcao para printar o alfabeto na tela
    input = getch();
    i = 0; //o int i eh zerado para ser usado como controle
    while (1)
    {
        flag = 0;
        //se o usuario apertou 'd'
        if (input == 'd')
        {
            //se o i for menor do que 50 (ultima letra do vetor ('Z))
            if (i < 50)
            {
                vetalfabeto[i + 2] = '>'; //vai ser adicionado '>' na posicao i+2 (que vai sempre ter um espaco)
                vetalfabeto[i] = ' ';     //na pocisao i vai ser colocado o espaco (dando a sensacao que a seta se moveu)
                i += 2;                   //o int i eh acrecido em 2
                ascii++;                  //o int ascii eh acrescido em 1 (proxima letra)
                printaInformacoesNick();  //reprinta o alfabeto na tela com a seta na nova pocisao
            }
            else
            //se o i for maior que o vetor alfabeto so eh printado o alfabeto na tela sem modificacoes
            {
                printaInformacoesNick();
            }
        }
        //se o usuario apertou 'a' eh a mesma logica so com o sentido da movimentacao da seta trocada
        else if (input == 'a')
        {
            if (i >= 1)
            {
                vetalfabeto[i - 2] = '>';
                vetalfabeto[i] = ' ';
                i -= 2;
                ascii--;
                printaInformacoesNick();
            }
            else
            {
                printaInformacoesNick();
            }
        }
        //se o usuario apertou 'z' vai ser selecionado a letra baseado no valor do int ascii
        else if (input == 'z')
        {
            //se o nome tiver menos que 10 caracteres vai ser adicionada a letra ao nome do jogador
            if (contacaracter < 9)
            {
                jogador.nome[contacaracter] = ascii;
                contacaracter++; //vai ser adicionado 1 a variavel contacaracter para controlar quantos caracteres ja foram colocados
                printaInformacoesNick();
            }
            //se eh atigindo o limite de caracteres do nome
            else
            {
                //eh perguntado ao usuario se ele deseja confirmar ou refazer o nick
                printf("%d\n", contacaracter);
                printf("\nSeu nick eh: " GREEN "%s\n" RESET "Aperte X para confirma o nick ou Z para refazer o nick\n", jogador.nome);
                input = getch();
                while (flag == 0)
                {
                    //se confirmar o nick a funcao retorna
                    if (input == 'x')
                    {
                        printf("%d\n", contacaracter);
                        jogador.nome[contacaracter + 1] = '\0';
                        return 1;
                    }
                    //se refazer o nick vai chamar uma funcao para limpar o nick, o contacaracter vai zerar e sai do while com a flag = 1
                    else if (input == 'z')
                    {
                        limpaNick();
                        printaInformacoesNick();
                        flag = 1;
                        contacaracter = 0;
                    }
                    input = getch();
                }
            }
        }
        //se o usuario apertar 'x' ele confirma o nick
        else if (input == 'x')
        {
            //eh feita uma verificacao para ver se o nick tem no minimo 3 caracteres
            if (jogador.nome[0] == ' ' || jogador.nome[2] == ' ')
            {
                printf(RED "\nSeu nick precisa ter no minimo 3 caracteres, aperte qualquer tecla para digitar seu nick novamente\n" RESET);
                getch();
                limpaNick();
                printaInformacoesNick();
                contacaracter = 0;
            }
            //se tiver mais de 3 caracteres volta eh perguntado ao usuario se ele deseja confirmar ou refazer o nick
            else
            {
                printf("\nSeu nick eh: " GREEN "%s\n" RESET "Aperte X para confirma o nick ou Z para refazer o nick\n", jogador.nome);
                input = getch();
                while (flag == 0)
                {
                    if (input == 'x')
                    {
                        jogador.nome[contacaracter + 1] = '\0';
                        return 1;
                    }
                    else if (input == 'z')
                    {
                        limpaNick();
                        printaInformacoesNick();
                        flag = 1;
                        contacaracter = 0;
                    }
                    input = getch();
                }
            }
        }
        else if (input == 49)
        {
            menuMain();
        }
        input = getch();
    }
    return 0;
}

/*
#########################################################################################
# A funcao abaixo eh responsavel por printar na tela as informações do sistema do nick. #
#########################################################################################
*/

void printaInformacoesNick()
{
    int i;
    system(CLEAR);
    printf(GREEN "Modo Ranqueado Ativado\n" RESET);
    printf("Insira seu nick: %s\n", jogador.nome);
    //loop para printar o alfabeto
    for (i = 0; i < 52; i++)
    {
        printf("%c ", vetalfabeto[i]);
        //quando o i for igual a um desses valores (isto eh a cada 5 letras), eh dada a quebra de linha
        if (i == 9 || i == 19 || i == 29 || i == 39 || i == 49)
        {
            printf("\n");
        }
    }
    printf("\n");
    printf("Digite A ou D para selecionar a letra\nDigite Z para confirmar a letra\nDigite X para confirmar o nick\n");
    printf("Digite 1 para voltar ao menu\n");
}

/*
################################################################
# A funcao abaixo eh responsavel por limpar o nick do usuario. #
################################################################
*/

void limpaNick()
{
    int k;
    for (k = 0; k < 10; k++)
    {
        if (k == 9)
        {
            jogador.nome[k] = '\0';
        }
        else
        {
            jogador.nome[k] = ' ';
        }
    }
}

/*
###################################################################################################################
# A funcao abaixo eh a sair. Ela eh chamada na funcao de menu caso o usuario selecione a opcao relacionada a ela. #
# Ela finaliza o programa e mostra as informacoes desse programa para o usuario.                                  #
###################################################################################################################
*/

int sair()
{
    system(CLEAR);
    printf(BLUE "Obrigado por jogar!\n\n" RESET);
    //eh usado um arquivo de texto chamado "sair.txt" que contem as informacoes do programa
    FILE *file = fopen("arte/sair.txt", "r");
    while ((sairtxt = fgetc(file)) != EOF)
    {
        printf(GREEN "%c" RESET, sairtxt);
    }
    fclose(file);
    exit(0);
}

/*
########################################################
# A funcao abaixo verifica as condicoes para gameover. #
########################################################
*/

int verificaGameover()
{
    int i;

    //se o modo de jogo for o replay e acaba as pecas GAMEOVER
    if (jogarreplay == 1)
    {
        if (vetreplay[contvetreplay] == ' ')
        {
            return 1;
        }
    }

    for (i = 1; i < 15 && i != 8; i++)
    {
        //se a linha de "#" atingir a base do tabuleiro GAMEOVER
        if (tabuleiro[1][i] == 'A' || tabuleiro[1][i] == 'B' || tabuleiro[1][i] == 'C' || tabuleiro[1][i] == 'D' || tabuleiro[1][i] == 'E' || tabuleiro[1][i] == '#')
        {
            return 1;
        }
        //se alguma peca aatingir a base do tabuleiro GAMEOVER
        if (tabuleiro[2][largura / 2] == 'A' || tabuleiro[2][largura / 2] == 'B' || tabuleiro[2][largura / 2] == 'C' || tabuleiro[2][largura / 2] == 'D' || tabuleiro[2][largura / 2] == 'E')
        {
            return 1;
        }
    }
    return 0;
}

/*
######################################################################
# A funcao abaixo eh responsavel por mostrar o gameover por jogador, #
#informa a quantidade de pontos e zera a variavel  do gamerover.     #
######################################################################
*/

int gameoverMenu()
{
    char gameovertxt;
    int i;

    system(CLEAR);

    //eh usado um arquivo de texto chamado "gameover.txt" que contem a palavra gameover que vai ser mostrada na tela
    FILE *file = fopen("arte/gameover.txt", "r");
    while ((gameovertxt = fgetc(file)) != EOF)
    {
        printf(BLUE "%c" RESET, gameovertxt);
    }
    fclose(file);
    //eh salva a pontuacao do jogador caso o modo ranqueado tiver ativado
    printf("Pontuacao final: %d\n", jogador.pontos);
    printf("Aperte qualquer tecla para volta ao menu: ");
    if (modoranking == 1)
    {
        registerMatch();
    }
    getch();
    system(CLEAR);
    return 0;
}

/*
############################################################################################
# A funcao abaixo eh responsavel por registrar a partida se o modo ranquead tiver ativado, #
############################################################################################
*/

int registerMatch()
{
    int i = 0, j, p;
    DadosJogador ranking[10];

    //abro o arquivo do ranking para passar os dados para a variavel ranking que eh um vetor de 10 do tipo DadosJogador
    arquivoranking = fopen("ranking.bin", "rb");
    fread(&ranking, sizeof(DadosJogador), 10, arquivoranking);
    fclose(arquivoranking);

    //verificacao para ver se os pontos do jogador atual eh maior do que o ultimo do ranking. Se for, o jogador eh colocado na ultima posicao
    if (jogador.pontos > ranking[9].pontos)
    {
        ranking[9].pontos = jogador.pontos;
        for (i = 0; i < 11; i++)
        {
            ranking[9].nome[i] = jogador.nome[i];
        }
    }

    //eh feito um bubble sort para ordenar o ranking
    DadosJogador aux;
    for (p = 9; p > 0; p--)
    {
        if (ranking[p].pontos > ranking[p - 1].pontos)
        {
            aux = ranking[p - 1];
            ranking[p - 1].pontos = ranking[p].pontos;
            for (i = 0; i < 11; i++)
            {
                ranking[p - 1].nome[i] = ranking[p].nome[i];
            }
            ranking[p].pontos = aux.pontos;
            for (i = 0; i < 11; i++)
            {
                ranking[p].nome[i] = aux.nome[i];
            }
        }
    }

    arquivoranking = fopen("ranking.bin", "wb");
    fwrite(&ranking, sizeof(DadosJogador), 10, arquivoranking);
    fclose(arquivoranking);
    return 0;
}

/*
#############################################################
# A funcao abaixo eh responsavel por inicializar o ranking. #
#############################################################
*/

void inicializaRanking()
{
    int i = 0;

    arquivoranking = fopen("ranking.bin", "wb");

    for (i = 0; i < 10; i++)
    {
        jogador.nome[0] = 'G';
        jogador.nome[1] = 'U';
        jogador.nome[2] = 'I';
        jogador.nome[3] = '\0';
        jogador.pontos = 0;
        fwrite(&jogador, sizeof(DadosJogador), 1, arquivoranking);
    }
}

/*
#############################################$##############
# A funcao abaixo inicializa as coisas do jogo.            #
# E tambem zera as variaveis principais para um novo jogo. #
###########################################################
*/

void inicializaJogo()
{
    int i, j;
    int n = largura / 2; //meio do tabuleiro

    //loop para iniciar o tabuleiro
    for (i = altura - 1; i >= 0; i--)
    {
        for (j = 0; j < largura; j++)
        {
            if (i == 0 || i == altura - 1 || j == 0 || j == largura - 1)
            {
                tabuleiro[i][j] = '#';
            }
            else if (j == n && i >= 2 && i <= 8)
            {
                tabuleiro[i][j] = '-';
            }
            else if (i == 1 && j == 8)
            {
                tabuleiro[i][j] = peca;
            }
            else
            {
                tabuleiro[i][j] = ' ';
            }
        }
    }

    //loop para iniciar o vetor replay, ele ja fica pronto para receber as pecas do arquivo caso o usuario jogue o modo replay
    for (i = 0; i < 200; i++)
    {
        vetreplay[i] = ' ';
    }

    limpaNick();
    //variaveis importantes zeradas
    jogador.pontos = 0;
    gameover = 0;
    criarreplay = 0;
    jogarreplay = 0;
    naoranqueado = 0;
    contvetreplay = 0;
}

/*
##################################################################################################################
# A funcao abaixo printa a matriz na tela e printa as pecas coloridas. Antes eh usada a funcao usleep() para dar #
# a sensacao mais fluida ao jogo e nao ficar aquela parada piscando cada vez que printa.                         #
##################################################################################################################
*/

void printarTabuleiro()
{
    int i, j;

    usleep(60000);
    system(CLEAR);

    if (criarreplay == 1)
    {
        printf("Modo de jogo: Criar Replay\n");
    }
    else if (jogarreplay == 1)
    {
        printf("Modo de jogo: Replay\n");
    }
    else if (modoranking == 1)
    {
        printf("Modo de jogo: Ranqueado\n");
        printf("Jogador: %s\n", jogador.nome);
    }
    else if (modoranking == 0)
    {
        printf("Modo de jogo: Não-Ranqueado\n");
    }

    printf("Pontos: %d\n\n", jogador.pontos);

    for (i = altura - 1; i >= 0; i--)
    {
        for (j = 0; j < largura; j++)
        {
            if (tabuleiro[i][j] == 'A')
            {
                printf(BLUE "A" RESET);
            }
            else if (tabuleiro[i][j] == 'B')
            {
                printf(YELLOW "B" RESET);
            }
            else if (tabuleiro[i][j] == 'C')
            {
                printf(GREEN "C" RESET);
            }
            else if (tabuleiro[i][j] == 'D')
            {
                printf(RED "D" RESET);
            }
            else if (tabuleiro[i][j] == 'E')
            {
                printf(MAGENTA "E" RESET);
            }
            else
            {
                printf("%c", tabuleiro[i][j]);
            }
        }
        printf("\n");
    }
    if (jogarreplay == 1)
    {
        printf("Proximas pecas: %c %c %c\n", vetreplay[contvetreplay + 2], vetreplay[contvetreplay + 4], vetreplay[contvetreplay + 6]);
    }
}

/*
#############################################################################
# A funcao abaixo eh responsavel por descer a matriz. A cada 20 segundos eh #
# chamada a funcao e ela verifica todos os pontos da linha antes da ultima  #
# linha de "#" e desce essa linha e cria a linha de "#" no lugar.           #
#############################################################################
*/

void desceTabuleiro()
{
    int i, j;

    for (i = 2; i < 9; i++)
    {
        for (j = 1; j < 16; j++)
        {
            if (tabuleiro[i][j] == ' ' || tabuleiro[i][j] == 'A' || tabuleiro[i][j] == 'B' || tabuleiro[i][j] == 'C' || tabuleiro[i][j] == 'D' || tabuleiro[i][j] || 'E')
            {
                tabuleiro[i - 1][j] = tabuleiro[i][j];
                tabuleiro[i][j] = '#';
                if (jogarreplay == 0)
                {
                    tabuleiro[1][largura / 2] = peca;
                }
                else
                {
                    tabuleiro[1][largura / 2] = vetreplay[contvetreplay];
                }
            }
        }
    }
}

/*
#######################################################################
# A funcao abaixo cria uma peca (letra) aleatoria e poe no tabuleiro. #
#######################################################################
*/

char criarPeca()
{

    if (jogarreplay == 0)
    {
        peca = 'A' + rand() % 5;
        tabuleiro[1][largura / 2] = peca;
    }
    else if (jogarreplay == 1)
    {
        tabuleiro[1][largura / 2] = vetreplay[contvetreplay];
    }

    if (criarreplay == 1)
    {
        fprintf(arquivoreplay, "%c\n", peca);
    }
}

/*
#####################################################################################################
# A Funcao abaixo lanca a peca no tabuleiro ja detectando se tem alguma peca no caminho e se tiver, #
# a peca vai parar logo embaixo da peca dedectada.                                                  #
#####################################################################################################
*/

void lancarPeca()
{
    int i, j, flag = 0;
    for (i = 8; i >= 2 && flag == 0; i--)
    {
        for (j = 1; j < largura && flag == 0; j++)
        {
            if (tabuleiro[i][j] == '-')
            {
                if (jogarreplay == 0)
                {
                    tabuleiro[i][j] = peca;
                    int posicoesi[10];
                    int posicoesj[10];
                    for (int k = 0; k < 10; k++){
                        posicoesi[k] = 99;
                        posicoesj[k] == 99;
                    } 
                    int contador = 0;
                    if (tabuleiro[i + 1][j] == peca)
                    {
                        contador++;
                        posicoesi[0] = i+1;
                        posicoesj[0] = j;
                    }
                    if (tabuleiro[i - 1][j] == peca)
                    {
                        contador++;
                        posicoesi[1] = i-1;
                        posicoesj[1] = j;
                    }
                    if (tabuleiro[i][j + 1] == peca)
                    {
                        contador++;
                        posicoesi[2] = i;
                        posicoesj[2] = j+1;
                    }
                    if (tabuleiro[i][j - 1] == peca)
                    {
                        contador++;
                        posicoesi[3] = i;
                        posicoesj[3] = j-1;
                    }
                    if (tabuleiro[i - 1][j - 1] == peca)
                    {
                        contador++;
                        posicoesi[4] = i-1;
                        posicoesj[4] = j-1;
                    }

                    if (tabuleiro[i - 1][j + 1] == peca)
                    {
                        contador++;
                        posicoesi[5] = i-1;
                        posicoesj[5] = j+1;
                    }

                    if (tabuleiro[i + 1][j - 1] == peca)
                    {
                        contador++;
                        
                        posicoesi[6] = i+1;
                        posicoesj[6] = j-1;
                    }

                    if (tabuleiro[i + 1][j + 1] == peca)
                    {
                        contador++;
                        
                        posicoesi[7] = i+1;
                        posicoesj[7] = j+1;
                    }
                    if (contador >= 2)
                    {
                        jogador.pontos += 100;
                        for (int k = 0; k < 10; k++){
                            if (posicoesi[k] != 99 && posicoesj[k] != 99){
                                tabuleiro[posicoesi[k]][posicoesj[k]] = ' ';                    
                                tabuleiro[i][j] = ' ';
                            }
                        } 
                    }
                }
                else
                {
                    tabuleiro[i][j] = vetreplay[contvetreplay];
                }
                flag = 1;
            }
        }
    }
}

/*
################################################################################################
# A Funcao abaixo eh responsavel pela animacao da peca no tabuleiro quando ela eh lancada.     #
# Eh usado um loop para ir apagando a peca e printando de novo ate chegar na posicao definida. #
################################################################################################
*/

void animacaoPeca()
{
    int i, j;
    tabuleiro[1][8] = ' ';
    for (i = 0; i <= 10; i++)
    {
        for (j = 0; j <= 16; j++)
        {
            if (tabuleiro[i][j] == '-')
            {
                if (jogarreplay == 0)
                {
                    tabuleiro[i][j] = peca;
                }
                else
                {
                    tabuleiro[i][j] = vetreplay[contvetreplay];
                }
                printarTabuleiro();
                tabuleiro[i][j] = '-';
            }
        }
    }
}

/*
#########################################################################################################
# A funcao abaixo controla a mira do jogo. Eh usado um loop de repeticao para a mira nao apagar         #
# as pecas no tabuleiro ele vai calcular uma nova equacao (calcular um novo y) e substituir pelo antigo #
# assim ele "grava" a posicao da mira sem apagar as pecas do tabuleiro                                  #
#########################################################################################################
*/

void mira()
{
    int i, j;

    for (i = 2; i <= 8; i++)
    {
        oldj = equacaoMira(oldcoeficiente, i);
        newj = equacaoMira(coeficientemira, i);
        if (tabuleiro[i][oldj] == '-')
        {
            tabuleiro[i][oldj] = ' ';
        }
        if (tabuleiro[i][newj] == ' ')
        {
            tabuleiro[i][newj] = '-';
        }
    }
}

/*
###################################################################################################
# A funcao abaixo calcula o "y" da mira. Ela recebe dois parametros, um float "m" que é o valor   #
# do coeficiente da mira (valor da inclinacao) e um int "x" que eh a posicao do x da mira.        #
# Ela pode retornar 3 valores. Retorna 1 se o y calculado for menor do que 1, retorna 15          #
# se o y calculado for maior ou igual a 15 ou retorna a pocisao do y se ele tiver entre 1 e 15.   #
###################################################################################################
*/

int equacaoMira(float m, int x)
{
    int n = largura / 2; //meio do tabuleiro
    int y;
    y = (int)(m * x + n);
    if (y < 1)
    {
        return 1;
    }
    if (y >= 15)
    {
        return 15;
    }
    if (y > 1 && y < 15)
    {
        return y;
    }
}

/*
#############################################################################
# A funcao abaixo conta o tempo em segundos recebendo por referencia um int #
# inicialido com o "time(NULL)"                                             #
#############################################################################
*/

int contaTempo()
{
    int aux = 0;
    aux = (time(NULL) - tempo) + aux;
    return aux;
}
