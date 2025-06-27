#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINHA 100
#define MAX_COL 100
#define MAX_FASES 10
#define ARQUIVO_ESTATS "estatisticas.txt"
#define TAM_PADRAO 11

typedef struct {
    int x, y;
} Posicao;

typedef struct {
    char nome[50];
    int score;
} Jogador;

typedef struct {
    int largura;
    int altura;
    char **mapa;
    Posicao jogador;
    Posicao saida;
} Fase;

void limparBuffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

char **alocarMapa(int altura, int largura) {
    char **mapa = malloc(sizeof(char *) * altura);
    int i;
    for (i = 0; i < altura; i++)
        mapa[i] = malloc(sizeof(char) * (largura + 1));
    return mapa;
}

void liberarMapa(char **mapa, int altura) {
    int j;
    for (j = 0; j < altura; j++)
        free(mapa[j]);
    free(mapa);
}

int dx[4] = {0, 0, -1, 1};
int dy[4] = {-1, 1, 0, 0};

void embaralhar(int *vetor, int n) {
    int i, j, temp;
    for (i = 0; i < n; i++) {
        j = rand() % n;
        temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

void dfsLabirinto(char **mapa, int x, int y, int largura, int altura) {
    mapa[x][y] = ' ';
    int dirs[4] = {0, 1, 2, 3};
    embaralhar(dirs, 4);
    int i, nx, ny;
    for (i = 0; i < 4; i++) {
        nx = x + dx[dirs[i]] * 2;
        ny = y + dy[dirs[i]] * 2;
        if (nx > 0 && nx < altura && ny > 0 && ny < largura && mapa[nx][ny] == '#') {
            mapa[x + dx[dirs[i]]][y + dy[dirs[i]]] = ' ';
            dfsLabirinto(mapa, nx, ny, largura, altura);
        }
    }
}

void gerarFaseAleatoria(Fase *fase) {
    int i, j;
    fase->altura = TAM_PADRAO;
    fase->largura = TAM_PADRAO;
    fase->mapa = alocarMapa(fase->altura, fase->largura);

    for (i = 0; i < fase->altura; i++)
        for (j = 0; j < fase->largura; j++)
            fase->mapa[i][j] = '#';

    dfsLabirinto(fase->mapa, 1, 1, fase->largura, fase->altura);

    fase->mapa[1][1] = '@';
    fase->jogador.x = 1;
    fase->jogador.y = 1;

    fase->mapa[fase->altura - 2][fase->largura - 2] = 'E';
    fase->saida.x = fase->altura - 2;
    fase->saida.y = fase->largura - 2;
}

int carregarFase(Fase *fase, const char *nomeArquivo) {
    FILE *arq = fopen(nomeArquivo, "r");
    if (!arq) return 0;

    fscanf(arq, "%d %d\n", &fase->altura, &fase->largura);
    fase->mapa = alocarMapa(fase->altura, fase->largura);

    int k, l;
    for (k = 0; k < fase->altura; k++) {
        fgets(fase->mapa[k], fase->largura + 2, arq);
        for (l = 0; l < fase->largura; l++) {
            if (fase->mapa[k][l] == '@') {
                fase->jogador.x = k;
                fase->jogador.y = l;
            } else if (fase->mapa[k][l] == 'E') {
                fase->saida.x = k;
                fase->saida.y = l;
            }
        }
    }

    fclose(arq);
    return 1;
}

void imprimirMapa(Fase *fase) {
    int m;
    for (m = 0; m < fase->altura; m++)
        printf("%s", fase->mapa[m]);
}

int moverJogador(Fase *fase, char direcao) {
    int dx_mov = 0, dy_mov = 0;
    if (direcao == 'w') dx_mov = -1;
    else if (direcao == 's') dx_mov = 1;
    else if (direcao == 'a') dy_mov = -1;
    else if (direcao == 'd') dy_mov = 1;
    else return 0;

    int nx = fase->jogador.x + dx_mov;
    int ny = fase->jogador.y + dy_mov;

    if (nx < 0 || ny < 0 || nx >= fase->altura || ny >= fase->largura) return 0;
    if (fase->mapa[nx][ny] == '#') return 0;

    fase->mapa[fase->jogador.x][fase->jogador.y] = ' ';
    fase->jogador.x = nx;
    fase->jogador.y = ny;
    fase->mapa[nx][ny] = '@';
    return 1;
}

int chegouSaida(Fase *fase) {
    return fase->jogador.x == fase->saida.x && fase->jogador.y == fase->saida.y;
}

void salvarEstatistica(Jogador *jog) {
    FILE *arq = fopen(ARQUIVO_ESTATS, "a");
    if (arq) {
        fprintf(arq, "%s %d\n", jog->nome, jog->score);
        fclose(arq);
    } else {
        printf("Erro ao salvar estatísticas.\n");
    }
}

void mostrarEstatisticas() {
    FILE *arq = fopen(ARQUIVO_ESTATS, "r");
    if (!arq) {
        printf("Nenhuma estatística salva ainda.\n");
        return;
    }

    char nome[50];
    int pontos;
    printf("\n=== Estatísticas Salvas ===\n");
    while (fscanf(arq, "%49s %d", nome, &pontos) == 2) {
        printf("Jogador: %-20s Pontuação: %d\n", nome, pontos);
    }
    printf("==========================\n\n");

    fclose(arq);
}

void jogar() {
    Jogador jog;
    srand(time(NULL));

    printf("Digite seu nome: ");
    fgets(jog.nome, sizeof(jog.nome), stdin);
    size_t len = strlen(jog.nome);
    if (len > 0 && jog.nome[len - 1] == '\n')
        jog.nome[len - 1] = '\0';
    else
        limparBuffer();

    jog.score = 0;

    int n;
    for (n = 1; n <= MAX_FASES; n++) {
        Fase fase;
        int sucesso = 0;

        if (n <= 5) {
            char nomeFase[20];
            sprintf(nomeFase, "fase%d.txt", n);
            sucesso = carregarFase(&fase, nomeFase);
        } else {
            printf("Gerando fase aleatória %d...\n", n);
            gerarFaseAleatoria(&fase);
            sucesso = 1;
        }

        if (!sucesso) {
            printf("Erro ao carregar a fase %d\n", n);
            break;
        }

        char comando;
        int terminou_fase = 0;
        limparBuffer();

        while (!terminou_fase) {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            printf("Fase %d - Pontos: %d\n", n, jog.score);
            imprimirMapa(&fase);
            printf("Use W A S D para se mover: ");
            comando = getchar();
            moverJogador(&fase, comando);
            if (chegouSaida(&fase))
                terminou_fase = 1;
            limparBuffer();
        }

        liberarMapa(fase.mapa, fase.altura);
        jog.score += 100;
    }

    salvarEstatistica(&jog);
    printf("\nJogo finalizado! Pontuação: %d\n", jog.score);

#ifdef _WIN32
    system("pause");
#else
    printf("Pressione ENTER para voltar ao menu...\n");
    limparBuffer();
    getchar();
#endif
}

int main() {
    int opcao;
    do {
        printf("\n=== Maze Survivor ===\n");
        printf("1. Iniciar Jogo\n");
        printf("2. Estatísticas\n");
        printf("3. Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);
        limparBuffer();

        switch (opcao) {
            case 1: jogar(); break;
            case 2: mostrarEstatisticas(); break;
            case 3: printf("Saindo...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opcao != 3);

    return 0;
}
