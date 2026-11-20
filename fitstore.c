#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define USAR_COR       1
#define MAX_TENTATIVAS 3

#if USAR_COR
    #define RESET    "\033[0m"
    #define NEGRITO  "\033[1m"
    #define VERMELHO "\033[91m"
    #define VERDE    "\033[92m"
    #define AMARELO  "\033[93m"
    #define AZUL     "\033[94m"
    #define CIANO    "\033[96m"
    #define CINZA    "\033[90m"
#else
    #define RESET    ""
    #define NEGRITO  ""
    #define VERMELHO ""
    #define VERDE    ""
    #define AMARELO  ""
    #define AZUL     ""
    #define CIANO    ""
    #define CINZA    ""
#endif

#define MAX_ALUNOS      200
#define MAX_PRODUTOS    100
#define MAX_VENDAS      300
#define MAX_PAGAMENTOS  600
#define MAX_USUARIOS    20
#define MAX_ITENS       10
#define NUM_PLANOS      3
#define DESCONTO_ALUNO  0.10f

const char *NOME_PLANO[NUM_PLANOS]  = {"Basic", "Plus", "Premium"};
const float VALOR_PLANO[NUM_PLANOS] = {89.90f, 129.90f, 179.90f};
const char *NOME_FORMA[4]           = {"", "Dinheiro", "Cartao", "Pix"};

typedef struct {
    char usuario[30];
    char senha[30];
    int  ativo;
    int  isAdmin;
} Login;

typedef struct {
    int  id;
    char nome[50];
    char cpf[12];
    int  plano;
    int  mesPago;
    int  totalCheckins;
    char ultimoCheckin[11];
    int  ativo;
} Aluno;

typedef struct {
    int   id;
    char  nome[40];
    float preco;
    int   estoque;
    int   estoqueMinimo;
} Produto;

typedef struct {
    int   idProduto;
    char  nome[40];
    int   qtd;
    float precoUnit;
} ItemVenda;

typedef struct {
    int       id;
    int       idAluno;
    int       numItens;
    ItemVenda itens[MAX_ITENS];
    float     subtotal;
    float     desconto;
    float     total;
    int       formaPgto;
    char      data[11];
} Venda;

typedef struct {
    int   id;
    int   idAluno;
    int   competencia;
    float valor;
    char  data[11];
} Pagamento;

typedef struct {
    int dia;
    int mes;
    int ano;
} DataSistema;

Login     usuarios[MAX_USUARIOS];
int       totalUsuarios = 0;
Login     usuarioLogado;
Aluno     alunos[MAX_ALUNOS];
int       totalAlunos = 0;
Produto   produtos[MAX_PRODUTOS];
int       totalProdutos = 0;
Venda     vendas[MAX_VENDAS];
int       totalVendas = 0;
Pagamento pagamentos[MAX_PAGAMENTOS];
int       totalPagamentos = 0;

DataSistema dataAtual;
int         competenciaAtual = 0;
int         encerrarSistema  = 0;

/* AGENTE 2 - INFRAESTRUTURA */
void  lerTexto(const char *prompt, char *dest, int tam);
int   lerInteiro(const char *prompt, int min, int max);
float lerDecimal(const char *prompt, float min, float max);
void  pausar(void);
int   ehBissexto(int ano);
int   diasNoMes(int mes, int ano);
void  formatarData(int dia, int mes, int ano, char *dest);
int   calcularCompetencia(int mes, int ano);
void  formatarCompetencia(int comp, char *dest);
void  carregarDataSistema(void);
void  salvarDataSistema(void);
void  configurarDataInicial(void);
int   cpfValido(const char *cpf);
int   buscarAluno(int id);
int   buscarProduto(int id);
int   alunoEmDia(const Aluno *a);
int   carregarLista(const char *arquivo, void *dados, size_t tamItem, int maximo);
void  salvarLista(const char *arquivo, const void *dados, size_t tamItem, int qtd);
void  carregarUsuarios(void);
void  salvarUsuarios(void);
void  carregarAlunos(void);
void  salvarAlunos(void);
void  carregarProdutos(void);
void  salvarProdutos(void);
void  carregarVendas(void);
void  salvarVendas(void);
void  carregarPagamentos(void);
void  salvarPagamentos(void);
void  carregarTudo(void);

/* AGENTE 3 - VISUAL */
void  linha(char c, int n);
void  banner(void);
void  titulo(const char *texto);
void  barra(int valor, int maximo, int largura, const char *cor);
void  mostrarDashboard(void);

/* AGENTE 4 - ACESSO */
int   buscarUsuarioPorNome(const char *nome);
int   fazerLogin(void);
void  cadastrarUsuario(void);
void  listarUsuarios(void);
void  alternarStatusUsuario(void);
void  menuUsuarios(void);
void  avancarMes(void);

/* AGENTE 5 - ACADEMIA */
void  listarPlanos(void);
void  registrarPagamento(int idx);
void  receberMensalidade(void);
int   cpfJaCadastrado(const char *cpf);
void  cadastrarAluno(void);
void  listarAlunos(void);
void  alterarPlano(void);
void  alternarSituacao(void);
void  fazerCheckin(void);
void  menuAcademia(void);

/* AGENTE 6 - LOJA */
int   centavos(float valor);
void  cadastrarProduto(void);
void  listarProdutos(void);
void  alterarPreco(void);
void  reporEstoque(void);
int   estoqueLivre(int idxProduto, const Venda *v);
void  adicionarAoCarrinho(Venda *v);
void  removerUltimoItem(Venda *v);
void  calcularTotais(Venda *v, int comDesconto);
void  mostrarCarrinho(Venda *v, int comDesconto);
void  imprimirCupom(const Venda *v);
void  finalizarVenda(Venda *v, int comDesconto);
void  novaVenda(void);
void  menuLoja(void);

/* AGENTE 7 - RELATORIOS */
void  relInadimplentes(void);
void  relRankingFrequencia(void);
void  relEstoqueCritico(void);
void  relVendasDoDia(void);
void  relMaisVendidos(void);
void  relFaturamento(void);
void  menuRelatorios(void);

void  menuPrincipal(void);

/* AGENTE 2 - INFRAESTRUTURA */
void lerTexto(const char *prompt, char *dest, int tam) {
    int c, i = 0, estourou = 0;
    if (tam <= 0) return;
    printf("%s", prompt);
    while (1) {
        c = getchar();
        if (c == EOF) {
            if (i == 0) {
                encerrarSistema = 1;
            }
            break;
        }
        if (c == '\n') break;
        if (i < tam - 1) {
            dest[i] = (char)c;
            i++;
        } else {
            estourou = 1;
        }
    }
    dest[i] = '\0';
    if (estourou) {
        while (c != '\n' && c != EOF) c = getchar();
    }
}

int lerInteiro(const char *prompt, int min, int max) {
    char linhaDigitada[40];
    int  valor;
    char extra;
    while (!encerrarSistema) {
        lerTexto(prompt, linhaDigitada, sizeof(linhaDigitada));
        if (encerrarSistema) return min;
        if (sscanf(linhaDigitada, "%d %c", &valor, &extra) == 1
            && valor >= min && valor <= max) {
            return valor;
        }
        printf(VERMELHO "  Valor invalido! Digite um numero de %d a %d." RESET "\n", min, max);
    }
    return min;
}

float lerDecimal(const char *prompt, float min, float max) {
    char  linhaDigitada[40];
    int   i;
    float valor;
    char  extra;
    while (!encerrarSistema) {
        lerTexto(prompt, linhaDigitada, sizeof(linhaDigitada));
        if (encerrarSistema) return min;
        for (i = 0; linhaDigitada[i] != '\0'; i++) {
            if (linhaDigitada[i] == ',') linhaDigitada[i] = '.';
        }
        if (sscanf(linhaDigitada, "%f %c", &valor, &extra) == 1
            && valor >= min && valor <= max) {
            return valor;
        }
        printf(VERMELHO "  Valor invalido! Digite de %.2f a %.2f." RESET "\n", min, max);
    }
    return min;
}

void pausar(void) {
    char descarte[8];
    if (encerrarSistema) return;
    lerTexto("\n  Pressione ENTER para continuar...", descarte, sizeof(descarte));
}

int ehBissexto(int ano) {
    return (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
}

int diasNoMes(int mes, int ano) {
    if (mes == 2) return ehBissexto(ano) ? 29 : 28;
    if (mes == 4 || mes == 6 || mes == 9 || mes == 11) return 30;
    return 31;
}

void formatarData(int dia, int mes, int ano, char *dest) {
    sprintf(dest, "%02d/%02d/%04d", dia, mes, ano);
}

int calcularCompetencia(int mes, int ano) {
    return ano * 12 + mes;
}

void formatarCompetencia(int comp, char *dest) {
    sprintf(dest, "%02d/%04d", (comp - 1) % 12 + 1, (comp - 1) / 12);
}

void carregarDataSistema(void) {
    FILE *f = fopen("data.dat", "rb");
    DataSistema d;
    if (f == NULL) {
        dataAtual.dia = 0;
        dataAtual.mes = 0;
        dataAtual.ano = 0;
        competenciaAtual = 0;
        return;
    }
    if (fread(&d, sizeof(DataSistema), 1, f) == 1
        && d.ano >= 1900 && d.ano <= 2999
        && d.mes >= 1 && d.mes <= 12
        && d.dia >= 1 && d.dia <= diasNoMes(d.mes, d.ano)) {
        dataAtual = d;
        competenciaAtual = calcularCompetencia(dataAtual.mes, dataAtual.ano);
    } else {
        dataAtual.dia = 0;
        dataAtual.mes = 0;
        dataAtual.ano = 0;
        competenciaAtual = 0;
    }
    fclose(f);
}

void salvarDataSistema(void) {
    FILE *f = fopen("data.dat", "wb");
    if (f == NULL) {
        printf(VERMELHO "  Erro ao gravar data.dat!" RESET "\n");
        return;
    }
    fwrite(&dataAtual, sizeof(DataSistema), 1, f);
    fclose(f);
}

void configurarDataInicial(void) {
    int ano, mes, dia, maxDia;
    if (dataAtual.ano >= 1900 && dataAtual.mes >= 1 && dataAtual.dia >= 1) {
        competenciaAtual = calcularCompetencia(dataAtual.mes, dataAtual.ano);
        return;
    }
    titulo("CONFIGURAR DATA DO SISTEMA");
    ano = lerInteiro("  Ano : ", 1900, 2999);
    if (encerrarSistema) return;
    mes = lerInteiro("  Mes (1 a 12): ", 1, 12);
    if (encerrarSistema) return;
    maxDia = diasNoMes(mes, ano);
    dia = lerInteiro("  Dia: ", 1, maxDia);
    if (encerrarSistema) return;

    dataAtual.dia = dia;
    dataAtual.mes = mes;
    dataAtual.ano = ano;
    competenciaAtual = calcularCompetencia(mes, ano);
    salvarDataSistema();
}

int cpfValido(const char *cpf) {
    int i;
    if (strlen(cpf) != 11) return 0;
    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) return 0;
    }
    return 1;
}

int buscarAluno(int id) {
    int i;
    for (i = 0; i < totalAlunos; i++) {
        if (alunos[i].id == id) return i;
    }
    return -1;
}

int buscarProduto(int id) {
    int i;
    for (i = 0; i < totalProdutos; i++) {
        if (produtos[i].id == id) return i;
    }
    return -1;
}

int alunoEmDia(const Aluno *a) {
    return a->mesPago >= competenciaAtual;
}

int carregarLista(const char *arquivo, void *dados, size_t tamItem, int maximo) {
    FILE *f = fopen(arquivo, "rb");
    int   qtd = 0;
    if (f == NULL) return 0;
    if (fread(&qtd, sizeof(int), 1, f) != 1 || qtd < 0 || qtd > maximo) {
        fclose(f);
        return 0;
    }
    if (fread(dados, tamItem, qtd, f) != (size_t)qtd) qtd = 0;
    fclose(f);
    return qtd;
}

void salvarLista(const char *arquivo, const void *dados, size_t tamItem, int qtd) {
    FILE *f = fopen(arquivo, "wb");
    if (f == NULL) {
        printf(VERMELHO "  Erro ao gravar %s!" RESET "\n", arquivo);
        return;
    }
    fwrite(&qtd, sizeof(int), 1, f);
    fwrite(dados, tamItem, qtd, f);
    fclose(f);
}

void carregarUsuarios(void) {
    totalUsuarios = carregarLista("usuarios.dat", usuarios, sizeof(Login), MAX_USUARIOS);
    if (totalUsuarios == 0) {
        memset(&usuarios[0], 0, sizeof(Login));
        strcpy(usuarios[0].usuario, "admin");
        strcpy(usuarios[0].senha,   "fit123");
        usuarios[0].ativo   = 1;
        usuarios[0].isAdmin = 1;
        totalUsuarios = 1;
        salvarUsuarios();
    }
}

void salvarUsuarios(void) {
    salvarLista("usuarios.dat", usuarios, sizeof(Login), totalUsuarios);
}

void carregarAlunos(void) {
    totalAlunos = carregarLista("alunos.dat", alunos, sizeof(Aluno), MAX_ALUNOS);
}

void salvarAlunos(void) {
    salvarLista("alunos.dat", alunos, sizeof(Aluno), totalAlunos);
}

void carregarProdutos(void) {
    totalProdutos = carregarLista("produtos.dat", produtos, sizeof(Produto), MAX_PRODUTOS);
}

void salvarProdutos(void) {
    salvarLista("produtos.dat", produtos, sizeof(Produto), totalProdutos);
}

void carregarVendas(void) {
    totalVendas = carregarLista("vendas.dat", vendas, sizeof(Venda), MAX_VENDAS);
}

void salvarVendas(void) {
    salvarLista("vendas.dat", vendas, sizeof(Venda), totalVendas);
}

void carregarPagamentos(void) {
    totalPagamentos = carregarLista("pagamentos.dat", pagamentos, sizeof(Pagamento),
                                    MAX_PAGAMENTOS);
}

void salvarPagamentos(void) {
    salvarLista("pagamentos.dat", pagamentos, sizeof(Pagamento), totalPagamentos);
}

void carregarTudo(void) {
    carregarDataSistema();
    carregarUsuarios();
    carregarAlunos();
    carregarProdutos();
    carregarVendas();
    carregarPagamentos();
}

/* AGENTE 3 - VISUAL */
void linha(char c, int n) {
    int i;
    for (i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

void banner(void) {
    printf("\n" CIANO NEGRITO);
    printf("  +------------------------------------------------+\n");
    printf("  |           F I T S T O R E   v1.0               |\n");
    printf("  |     Academia + Loja  |  Gestao Integrada       |\n");
    printf("  +------------------------------------------------+\n");
    printf(RESET);
}

void titulo(const char *texto) {
    printf("\n" AMARELO NEGRITO "  >> %s" RESET "\n", texto);
    printf(CINZA "  ");
    linha('-', 60);
    printf(RESET);
}

void barra(int valor, int maximo, int largura, const char *cor) {
    int cheios = 0, i;
    if (maximo > 0) cheios = (valor * largura) / maximo;
    if (valor > 0 && cheios == 0) cheios = 1;
    printf("%s[", cor);
    for (i = 0; i < largura; i++) putchar(i < cheios ? '#' : '.');
    printf("]" RESET);
}

void mostrarDashboard(void) {
    char  hoje[11], comp[8];
    int   i, ativos = 0, devedores = 0, checkinsHoje = 0, criticos = 0, vendasHoje = 0;
    float totalHoje = 0;

    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, hoje);
    formatarCompetencia(competenciaAtual, comp);

    for (i = 0; i < totalAlunos; i++) {
        if (alunos[i].ativo) {
            ativos++;
            if (!alunoEmDia(&alunos[i])) devedores++;
            if (strcmp(alunos[i].ultimoCheckin, hoje) == 0) checkinsHoje++;
        }
    }
    for (i = 0; i < totalProdutos; i++) {
        if (produtos[i].estoque <= produtos[i].estoqueMinimo) criticos++;
    }
    for (i = 0; i < totalVendas; i++) {
        if (strcmp(vendas[i].data, hoje) == 0) {
            vendasHoje++;
            totalHoje += vendas[i].total;
        }
    }

    titulo("PAINEL DO DIA");
    printf("  " CINZA "Data: %s   |   Competencia: %s" RESET "\n\n", hoje, comp);
    printf("  %-28s " NEGRITO "%d" RESET "\n", "Alunos ativos", ativos);
    printf("  %-28s %s%d" RESET "\n", "Mensalidades em atraso",
           devedores > 0 ? VERMELHO : VERDE, devedores);
    printf("  %-28s " CIANO "%d" RESET "\n", "Check-ins hoje", checkinsHoje);
    printf("  %-28s %s%d" RESET "\n", "Produtos em estoque critico",
           criticos > 0 ? AMARELO : VERDE, criticos);
    printf("  %-28s " CIANO "%d" RESET " (R$ %.2f)\n", "Vendas hoje", vendasHoje, totalHoje);
}

/* AGENTE 4 - ACESSO */
int buscarUsuarioPorNome(const char *nome) {
    int i;
    for (i = 0; i < totalUsuarios; i++) {
        if (strcmp(usuarios[i].usuario, nome) == 0) return i;
    }
    return -1;
}

int fazerLogin(void) {
    char usuario[30], senha[30];
    int  tentativa, idx;

    banner();
    for (tentativa = 1; tentativa <= MAX_TENTATIVAS && !encerrarSistema; tentativa++) {
        lerTexto("\n  Usuario: ", usuario, sizeof(usuario));
        if (encerrarSistema) return 0;
        lerTexto("  Senha  : ", senha, sizeof(senha));
        if (encerrarSistema) return 0;

        idx = buscarUsuarioPorNome(usuario);
        if (idx >= 0 && strcmp(usuarios[idx].senha, senha) == 0) {
            if (!usuarios[idx].ativo) {
                printf(VERMELHO "\n  Este usuario esta DESATIVADO. Contate o admin." RESET "\n");
                continue;
            }
            usuarioLogado = usuarios[idx];
            printf(VERDE "\n  Acesso liberado! Bem-vindo(a), %s." RESET "\n",
                   usuarioLogado.usuario);
            return 1;
        }
        printf(VERMELHO "\n  Credenciais invalidas (tentativa %d de %d)." RESET "\n",
               tentativa, MAX_TENTATIVAS);
    }
    return 0;
}

void cadastrarUsuario(void) {
    Login novo;

    titulo("CADASTRAR NOVO USUARIO");
    if (totalUsuarios >= MAX_USUARIOS) {
        printf(VERMELHO "  Limite de usuarios atingido." RESET "\n");
        return;
    }

    memset(&novo, 0, sizeof(novo));

    while (!encerrarSistema) {
        lerTexto("  Login do novo usuario: ", novo.usuario, sizeof(novo.usuario));
        if (encerrarSistema) return;
        if (strlen(novo.usuario) < 3) {
            printf(VERMELHO "  O login deve ter pelo menos 3 caracteres." RESET "\n");
        } else if (buscarUsuarioPorNome(novo.usuario) >= 0) {
            printf(VERMELHO "  Esse nome de usuario ja existe!" RESET "\n");
        } else {
            break;
        }
    }

    do {
        lerTexto("  Senha (min. 4 caracteres): ", novo.senha, sizeof(novo.senha));
        if (encerrarSistema) return;
        if (strlen(novo.senha) < 4) {
            printf(VERMELHO "  Senha muito curta!" RESET "\n");
        }
    } while (strlen(novo.senha) < 4 && !encerrarSistema);

    novo.ativo   = 1;
    novo.isAdmin = 0;

    usuarios[totalUsuarios] = novo;
    totalUsuarios++;
    salvarUsuarios();

    printf(VERDE "\n  Usuario '%s' criado e ATIVO!" RESET "\n", novo.usuario);
}

void listarUsuarios(void) {
    int i;
    titulo("USUARIOS DO SISTEMA");
    printf(NEGRITO "  %-4s %-22s %-12s %s" RESET "\n",
           "NUM", "USUARIO", "PERFIL", "STATUS");
    for (i = 0; i < totalUsuarios; i++) {
        printf("  %-4d %-22s %-12s ", i + 1, usuarios[i].usuario,
               usuarios[i].isAdmin ? "ADMIN" : "OPERADOR");
        if (usuarios[i].ativo) printf(VERDE "ATIVO" RESET "\n");
        else                   printf(VERMELHO "DESATIVADO" RESET "\n");
    }
}

void alternarStatusUsuario(void) {
    int num, idx;

    titulo("ATIVAR / DESATIVAR USUARIO");
    listarUsuarios();

    num = lerInteiro("\n  Numero do usuario para alterar (0 = cancelar): ", 0, totalUsuarios);
    if (num == 0 || encerrarSistema) return;

    idx = num - 1;
    if (usuarios[idx].isAdmin) {
        printf(VERMELHO "\n  Nao e permitido desativar a conta do administrador." RESET "\n");
        return;
    }

    usuarios[idx].ativo = !usuarios[idx].ativo;
    salvarUsuarios();

    printf(VERDE "\n  Usuario '%s' agora esta %s." RESET "\n",
           usuarios[idx].usuario,
           usuarios[idx].ativo ? "ATIVO" : "DESATIVADO");
}

void menuUsuarios(void) {
    int opcao;
    if (!usuarioLogado.isAdmin) {
        printf(VERMELHO "\n  Acesso restrito ao administrador." RESET "\n");
        return;
    }
    do {
        titulo("GERENCIAR USUARIOS (ADMIN)");
        printf("  1 - Cadastrar novo usuario\n");
        printf("  2 - Listar usuarios\n");
        printf("  3 - Ativar / Desativar usuario\n");
        printf("  0 - Voltar\n");
        opcao = lerInteiro("\n  Escolha: ", 0, 3);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: cadastrarUsuario();      break;
            case 2: listarUsuarios();        break;
            case 3: alternarStatusUsuario(); break;
        }
        if (opcao != 0) pausar();
    } while (opcao != 0 && !encerrarSistema);
}

void avancarMes(void) {
    int maxDia;
    char comp[8], hoje[11];

    if (dataAtual.mes == 12) {
        dataAtual.mes = 1;
        dataAtual.ano++;
    } else {
        dataAtual.mes++;
    }
    maxDia = diasNoMes(dataAtual.mes, dataAtual.ano);
    if (dataAtual.dia > maxDia) {
        dataAtual.dia = maxDia;
    }

    competenciaAtual = calcularCompetencia(dataAtual.mes, dataAtual.ano);
    salvarDataSistema();

    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, hoje);
    formatarCompetencia(competenciaAtual, comp);
    printf(AMARELO "\n  [MODO TESTE] Data agora e %s (competencia %s)." RESET "\n", hoje, comp);
    printf("  Alunos que nao pagaram passam a constar como em atraso.\n");
}

/* AGENTE 5 - ACADEMIA */
void listarPlanos(void) {
    int i;
    printf("\n  Planos disponiveis:\n");
    for (i = 0; i < NUM_PLANOS; i++) {
        printf("   %d - %-8s R$ %7.2f /mes\n", i + 1, NOME_PLANO[i], VALOR_PLANO[i]);
    }
}

void registrarPagamento(int idx) {
    Pagamento p;
    char      comp[8];
    int       devidas = competenciaAtual - alunos[idx].mesPago;

    if (devidas <= 0) {
        printf(VERDE "\n  %s ja esta em dia!" RESET "\n", alunos[idx].nome);
        return;
    }
    if (totalPagamentos >= MAX_PAGAMENTOS) {
        printf(VERMELHO "\n  Limite de pagamentos atingido." RESET "\n");
        return;
    }
    memset(&p, 0, sizeof(p));
    p.id          = totalPagamentos + 1;
    p.idAluno     = alunos[idx].id;
    p.competencia = alunos[idx].mesPago + 1;
    p.valor       = VALOR_PLANO[alunos[idx].plano];
    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, p.data);

    pagamentos[totalPagamentos] = p;
    totalPagamentos++;
    alunos[idx].mesPago = p.competencia;
    salvarPagamentos();
    salvarAlunos();

    formatarCompetencia(p.competencia, comp);
    printf(VERDE "\n  Pagamento registrado: competencia %s - R$ %.2f" RESET "\n", comp, p.valor);
    if (devidas > 1) {
        printf(AMARELO "  Ainda restam %d mensalidade(s) em atraso." RESET "\n", devidas - 1);
    }
}

void receberMensalidade(void) {
    int id, idx;
    titulo("RECEBER MENSALIDADE");
    id = lerInteiro("  ID do aluno (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarAluno(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Aluno nao encontrado." RESET "\n");
        return;
    }
    printf("\n  Aluno: %s | Plano %s (R$ %.2f)\n", alunos[idx].nome,
           NOME_PLANO[alunos[idx].plano], VALOR_PLANO[alunos[idx].plano]);
    registrarPagamento(idx);
}

int cpfJaCadastrado(const char *cpf) {
    int i;
    for (i = 0; i < totalAlunos; i++) {
        if (strcmp(alunos[i].cpf, cpf) == 0) return 1;
    }
    return 0;
}

void cadastrarAluno(void) {
    Aluno a;
    char  cpf[40];
    int   opcao;

    titulo("MATRICULAR NOVO ALUNO");
    if (totalAlunos >= MAX_ALUNOS) {
        printf(VERMELHO "  Limite de alunos atingido." RESET "\n");
        return;
    }
    memset(&a, 0, sizeof(a));
    a.id = totalAlunos + 1;

    do {
        lerTexto("  Nome completo: ", a.nome, sizeof(a.nome));
        if (encerrarSistema) return;
        if (strlen(a.nome) < 3) printf(VERMELHO "  Nome muito curto!" RESET "\n");
    } while (strlen(a.nome) < 3 && !encerrarSistema);

    while (!encerrarSistema) {
        lerTexto("  CPF (somente 11 numeros): ", cpf, sizeof(cpf));
        if (encerrarSistema) return;
        if (!cpfValido(cpf)) {
            printf(VERMELHO "  CPF invalido!" RESET "\n");
        } else if (cpfJaCadastrado(cpf)) {
            printf(VERMELHO "  CPF ja cadastrado!" RESET "\n");
        } else {
            break;
        }
    }
    strcpy(a.cpf, cpf);

    listarPlanos();
    a.plano            = lerInteiro("  Escolha o plano: ", 1, NUM_PLANOS) - 1;
    if (encerrarSistema) return;
    a.mesPago          = competenciaAtual - 1;
    a.totalCheckins    = 0;
    a.ultimoCheckin[0] = '\0';
    a.ativo            = 1;

    alunos[totalAlunos] = a;
    totalAlunos++;
    salvarAlunos();
    printf(VERDE "\n  Aluno matriculado com sucesso! ID %d" RESET "\n", a.id);

    opcao = lerInteiro("  Registrar a 1a mensalidade agora? (1 = sim, 0 = nao): ", 0, 1);
    if (opcao == 1 && !encerrarSistema) registrarPagamento(totalAlunos - 1);
}

void listarAlunos(void) {
    int i;
    titulo("ALUNOS CADASTRADOS");
    if (totalAlunos == 0) {
        printf("  Nenhum aluno cadastrado.\n");
        return;
    }
    printf(NEGRITO "  %-4s %-26s %-8s %-9s %-9s %s" RESET "\n",
           "ID", "NOME", "PLANO", "SITUACAO", "MENSAL.", "CHECK-INS");
    for (i = 0; i < totalAlunos; i++) {
        printf("  %-4d %-26.26s %-8s ", alunos[i].id, alunos[i].nome,
               NOME_PLANO[alunos[i].plano]);
        if (alunos[i].ativo) printf(VERDE "%-9s" RESET " ", "ATIVO");
        else                 printf(CINZA "%-9s" RESET " ", "INATIVO");
        if (alunoEmDia(&alunos[i])) printf(VERDE "%-9s" RESET " ", "EM DIA");
        else                        printf(VERMELHO "%-9s" RESET " ", "ATRASADA");
        printf("%d\n", alunos[i].totalCheckins);
    }
}

void alterarPlano(void) {
    int id, idx;
    titulo("TROCAR PLANO DO ALUNO");
    id = lerInteiro("  ID do aluno (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarAluno(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Aluno nao encontrado." RESET "\n");
        return;
    }
    printf("\n  %s esta no plano %s.\n", alunos[idx].nome, NOME_PLANO[alunos[idx].plano]);
    listarPlanos();
    alunos[idx].plano = lerInteiro("  Novo plano: ", 1, NUM_PLANOS) - 1;
    if (encerrarSistema) return;
    salvarAlunos();
    printf(VERDE "\n  Plano atualizado para %s." RESET "\n", NOME_PLANO[alunos[idx].plano]);
}

void alternarSituacao(void) {
    int id, idx;
    titulo("ATIVAR / INATIVAR ALUNO");
    id = lerInteiro("  ID do aluno (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarAluno(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Aluno nao encontrado." RESET "\n");
        return;
    }
    alunos[idx].ativo = !alunos[idx].ativo;
    if (alunos[idx].ativo && alunos[idx].mesPago < competenciaAtual - 1) {
        alunos[idx].mesPago = competenciaAtual - 1;
    }
    salvarAlunos();
    printf("\n  %s agora esta %s.\n", alunos[idx].nome,
           alunos[idx].ativo ? "ATIVO" : "INATIVO");
    if (alunos[idx].ativo) {
        printf("  Meses de afastamento nao sao cobrados: deve apenas o mes atual.\n");
    }
}

void fazerCheckin(void) {
    int  id, idx;
    char hoje[11];

    titulo("CHECK-IN NA ACADEMIA");
    id = lerInteiro("  ID do aluno (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarAluno(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Aluno nao encontrado." RESET "\n");
        return;
    }
    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, hoje);
    if (!alunos[idx].ativo) {
        printf(VERMELHO "\n  ACESSO NEGADO: matricula inativa." RESET "\n");
        return;
    }
    if (!alunoEmDia(&alunos[idx])) {
        printf(VERMELHO "\n  ACESSO NEGADO: mensalidade em atraso! Procure a recepcao." RESET "\n");
        return;
    }
    if (strcmp(alunos[idx].ultimoCheckin, hoje) == 0) {
        printf(AMARELO "\n  %s ja fez check-in hoje." RESET "\n", alunos[idx].nome);
        return;
    }
    alunos[idx].totalCheckins++;
    strcpy(alunos[idx].ultimoCheckin, hoje);
    salvarAlunos();
    printf(VERDE "\n  ACESSO LIBERADO! Bem-vindo(a), %s. Treino numero %d." RESET "\n",
           alunos[idx].nome, alunos[idx].totalCheckins);
}

void menuAcademia(void) {
    int opcao;
    do {
        titulo("MODULO ACADEMIA");
        printf("  1 - Matricular aluno\n");
        printf("  2 - Listar alunos\n");
        printf("  3 - Receber mensalidade\n");
        printf("  4 - Check-in\n");
        printf("  5 - Trocar plano\n");
        printf("  6 - Ativar / Inativar aluno\n");
        printf("  0 - Voltar\n");
        opcao = lerInteiro("\n  Escolha: ", 0, 6);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: cadastrarAluno();     break;
            case 2: listarAlunos();       break;
            case 3: receberMensalidade(); break;
            case 4: fazerCheckin();       break;
            case 5: alterarPlano();       break;
            case 6: alternarSituacao();   break;
        }
        if (opcao != 0) pausar();
    } while (opcao != 0 && !encerrarSistema);
}

/* AGENTE 6 - LOJA */
int centavos(float valor) {
    return (int)(valor * 100 + 0.5f);
}

void cadastrarProduto(void) {
    Produto p;

    titulo("CADASTRAR PRODUTO");
    if (totalProdutos >= MAX_PRODUTOS) {
        printf(VERMELHO "  Limite de produtos atingido." RESET "\n");
        return;
    }
    memset(&p, 0, sizeof(p));
    p.id = totalProdutos + 1;
    do {
        lerTexto("  Nome do produto: ", p.nome, sizeof(p.nome));
        if (encerrarSistema) return;
        if (strlen(p.nome) < 2) printf(VERMELHO "  Nome muito curto!" RESET "\n");
    } while (strlen(p.nome) < 2 && !encerrarSistema);
    p.preco         = lerDecimal("  Preco de venda (R$): ", 0.01f, 9999.0f);
    if (encerrarSistema) return;
    p.estoque       = lerInteiro("  Estoque inicial: ", 0, 9999);
    if (encerrarSistema) return;
    p.estoqueMinimo = lerInteiro("  Estoque minimo (alerta): ", 0, 9999);
    if (encerrarSistema) return;

    produtos[totalProdutos] = p;
    totalProdutos++;
    salvarProdutos();
    printf(VERDE "\n  Produto cadastrado! ID %d" RESET "\n", p.id);
}

void listarProdutos(void) {
    int i;
    titulo("PRODUTOS E ESTOQUE");
    if (totalProdutos == 0) {
        printf("  Nenhum produto cadastrado.\n");
        return;
    }
    printf(NEGRITO "  %-4s %-26s %10s %8s %6s  %s" RESET "\n",
           "ID", "PRODUTO", "PRECO", "ESTOQUE", "MIN.", "STATUS");
    for (i = 0; i < totalProdutos; i++) {
        printf("  %-4d %-26.26s %10.2f %8d %6d  ", produtos[i].id, produtos[i].nome,
               produtos[i].preco, produtos[i].estoque, produtos[i].estoqueMinimo);
        if (produtos[i].estoque == 0)
            printf(VERMELHO "ESGOTADO" RESET "\n");
        else if (produtos[i].estoque <= produtos[i].estoqueMinimo)
            printf(AMARELO "CRITICO" RESET "\n");
        else
            printf(VERDE "OK" RESET "\n");
    }
}

void alterarPreco(void) {
    int id, idx;
    titulo("ALTERAR PRECO");
    id = lerInteiro("  ID do produto (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarProduto(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Produto nao encontrado." RESET "\n");
        return;
    }
    printf("\n  %s custa hoje R$ %.2f\n", produtos[idx].nome, produtos[idx].preco);
    produtos[idx].preco = lerDecimal("  Novo preco (R$): ", 0.01f, 9999.0f);
    if (encerrarSistema) return;
    salvarProdutos();
    printf(VERDE "\n  Preco atualizado. Vendas antigas mantem o valor da epoca." RESET "\n");
}

void reporEstoque(void) {
    int id, idx, qtd;
    titulo("REPOR ESTOQUE (ENTRADA DE MERCADORIA)");
    id = lerInteiro("  ID do produto (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarProduto(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Produto nao encontrado." RESET "\n");
        return;
    }
    printf("\n  %s: estoque atual %d\n", produtos[idx].nome, produtos[idx].estoque);
    qtd = lerInteiro("  Quantidade recebida: ", 1, 9999);
    if (encerrarSistema) return;
    produtos[idx].estoque += qtd;
    salvarProdutos();
    printf(VERDE "\n  Estoque atualizado: %d unidade(s)." RESET "\n", produtos[idx].estoque);
}

int estoqueLivre(int idxProduto, const Venda *v) {
    int i, noCarrinho = 0;
    for (i = 0; i < v->numItens; i++) {
        if (v->itens[i].idProduto == produtos[idxProduto].id) {
            noCarrinho += v->itens[i].qtd;
        }
    }
    return produtos[idxProduto].estoque - noCarrinho;
}

void adicionarAoCarrinho(Venda *v) {
    int id, idx, qtd, i, livre;

    if (v->numItens >= MAX_ITENS) {
        printf(VERMELHO "\n  Carrinho cheio (maximo %d itens)." RESET "\n", MAX_ITENS);
        return;
    }
    listarProdutos();
    id = lerInteiro("\n  ID do produto (0 = cancelar): ", 0, 99999);
    if (id == 0 || encerrarSistema) return;
    idx = buscarProduto(id);
    if (idx < 0) {
        printf(VERMELHO "\n  Produto nao encontrado." RESET "\n");
        return;
    }
    livre = estoqueLivre(idx, v);
    if (livre <= 0) {
        printf(VERMELHO "\n  Produto sem estoque disponivel." RESET "\n");
        return;
    }
    qtd = lerInteiro("  Quantidade: ", 1, 9999);
    if (encerrarSistema) return;
    if (qtd > livre) {
        printf(VERMELHO "\n  Estoque insuficiente! Disponivel: %d" RESET "\n", livre);
        return;
    }
    for (i = 0; i < v->numItens; i++) {
        if (v->itens[i].idProduto == id) {
            v->itens[i].qtd += qtd;
            printf(VERDE "\n  Quantidade atualizada: %dx %s" RESET "\n",
                   v->itens[i].qtd, produtos[idx].nome);
            return;
        }
    }
    v->itens[v->numItens].idProduto = id;
    strcpy(v->itens[v->numItens].nome, produtos[idx].nome);
    v->itens[v->numItens].qtd       = qtd;
    v->itens[v->numItens].precoUnit = produtos[idx].preco;
    v->numItens++;
    printf(VERDE "\n  Adicionado: %dx %s" RESET "\n", qtd, produtos[idx].nome);
}

void removerUltimoItem(Venda *v) {
    if (v->numItens == 0) {
        printf(AMARELO "\n  Carrinho ja esta vazio." RESET "\n");
        return;
    }
    v->numItens--;
    printf(AMARELO "\n  Ultimo item removido: %s" RESET "\n", v->itens[v->numItens].nome);
}

void calcularTotais(Venda *v, int comDesconto) {
    int i;
    v->subtotal = 0;
    for (i = 0; i < v->numItens; i++) {
        v->subtotal += v->itens[i].qtd * v->itens[i].precoUnit;
    }
    v->desconto = comDesconto ? v->subtotal * DESCONTO_ALUNO : 0;
    v->total    = v->subtotal - v->desconto;
}

void mostrarCarrinho(Venda *v, int comDesconto) {
    int i;
    calcularTotais(v, comDesconto);
    titulo("CARRINHO");
    if (v->numItens == 0) {
        printf("  (vazio)\n");
    }
    for (i = 0; i < v->numItens; i++) {
        printf("  %2d. %-24.24s %3d x %8.2f = %9.2f\n", i + 1, v->itens[i].nome,
               v->itens[i].qtd, v->itens[i].precoUnit,
               v->itens[i].qtd * v->itens[i].precoUnit);
    }
    printf("  " CINZA);
    linha('-', 52);
    printf(RESET);
    printf("  Subtotal: R$ %.2f\n", v->subtotal);
    if (comDesconto) {
        printf(VERDE "  Desconto aluno (%.0f%%): - R$ %.2f" RESET "\n",
               DESCONTO_ALUNO * 100, v->desconto);
    }
    printf(NEGRITO "  TOTAL: R$ %.2f" RESET "\n", v->total);
}

void imprimirCupom(const Venda *v) {
    int i, idx;
    printf("\n" CIANO "  ======================================================" RESET "\n");
    printf(NEGRITO "   FITSTORE - CUPOM #%04d                   %s" RESET "\n", v->id, v->data);
    printf(CIANO "  ======================================================" RESET "\n");
    if (v->idAluno > 0) {
        idx = buscarAluno(v->idAluno);
        if (idx >= 0) printf("   Aluno: %s\n", alunos[idx].nome);
    }
    for (i = 0; i < v->numItens; i++) {
        printf("   %2dx %-30.30s R$ %9.2f\n", v->itens[i].qtd, v->itens[i].nome,
               v->itens[i].qtd * v->itens[i].precoUnit);
    }
    printf("  ------------------------------------------------------\n");
    printf("   Subtotal ........................ R$ %9.2f\n", v->subtotal);
    printf("   Desconto ........................ R$ %9.2f\n", v->desconto);
    printf(NEGRITO "   TOTAL ........................... R$ %9.2f" RESET "\n", v->total);
    printf("   Pagamento: %s\n", NOME_FORMA[v->formaPgto]);
    printf(CIANO "  ======================================================" RESET "\n");
}

void finalizarVenda(Venda *v, int comDesconto) {
    int   i, idx;
    float recebido;

    calcularTotais(v, comDesconto);
    mostrarCarrinho(v, comDesconto);

    printf("\n  Formas de pagamento: 1 - Dinheiro | 2 - Cartao | 3 - Pix\n");
    v->formaPgto = lerInteiro("  Escolha: ", 1, 3);
    if (encerrarSistema) return;

    if (v->formaPgto == 1) {
        do {
            recebido = lerDecimal("  Valor recebido (R$): ", 0, 99999);
            if (encerrarSistema) return;
            if (centavos(recebido) < centavos(v->total)) {
                printf(VERMELHO "  Valor insuficiente!" RESET "\n");
            }
        } while (centavos(recebido) < centavos(v->total) && !encerrarSistema);
        printf("  Troco: R$ %.2f\n", recebido - v->total);
    }

    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, v->data);
    for (i = 0; i < v->numItens; i++) {
        idx = buscarProduto(v->itens[i].idProduto);
        produtos[idx].estoque -= v->itens[i].qtd;
    }
    vendas[totalVendas] = *v;
    totalVendas++;
    salvarProdutos();
    salvarVendas();

    imprimirCupom(v);
    for (i = 0; i < v->numItens; i++) {
        idx = buscarProduto(v->itens[i].idProduto);
        if (produtos[idx].estoque <= produtos[idx].estoqueMinimo) {
            printf(AMARELO "  ALERTA: %s com estoque baixo (%d un). Reponha!" RESET "\n",
                   produtos[idx].nome, produtos[idx].estoque);
        }
    }
}

void novaVenda(void) {
    Venda v;
    int   idAluno, idx, comDesconto = 0, opcao, concluida = 0;

    titulo("CAIXA - NOVA VENDA");
    if (totalVendas >= MAX_VENDAS) {
        printf(VERMELHO "  Limite de vendas atingido." RESET "\n");
        return;
    }
    if (totalProdutos == 0) {
        printf(AMARELO "  Cadastre produtos antes de vender." RESET "\n");
        return;
    }
    memset(&v, 0, sizeof(v));
    v.id = totalVendas + 1;

    idAluno = lerInteiro("  ID do aluno para desconto (0 = cliente avulso): ", 0, 99999);
    if (encerrarSistema) return;
    if (idAluno > 0) {
        idx = buscarAluno(idAluno);
        if (idx < 0) {
            printf(AMARELO "  Aluno nao encontrado. A venda sera avulsa." RESET "\n");
        } else {
            v.idAluno = idAluno;
            if (alunos[idx].ativo && alunoEmDia(&alunos[idx])) {
                comDesconto = 1;
                printf(VERDE "  Cliente: %s - desconto de %.0f%% liberado!" RESET "\n",
                       alunos[idx].nome, DESCONTO_ALUNO * 100);
            } else {
                printf(AMARELO "  %s esta inativo ou em atraso: sem desconto." RESET "\n",
                       alunos[idx].nome);
            }
        }
    }

    while (!concluida && !encerrarSistema) {
        mostrarCarrinho(&v, comDesconto);
        printf("\n  1 - Adicionar produto\n");
        printf("  2 - Remover ultimo item\n");
        printf("  3 - Finalizar venda\n");
        printf("  0 - Cancelar venda\n");
        opcao = lerInteiro("\n  Escolha: ", 0, 3);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: adicionarAoCarrinho(&v); break;
            case 2: removerUltimoItem(&v);   break;
            case 3:
                if (v.numItens == 0) {
                    printf(VERMELHO "\n  Carrinho vazio!" RESET "\n");
                } else {
                    finalizarVenda(&v, comDesconto);
                    concluida = 1;
                }
                break;
            case 0:
                printf(AMARELO "\n  Venda cancelada. Estoque preservado." RESET "\n");
                concluida = 1;
                break;
        }
    }
}

void menuLoja(void) {
    int opcao;
    do {
        titulo("MODULO LOJA");
        printf("  1 - Caixa (nova venda)\n");
        printf("  2 - Cadastrar produto\n");
        printf("  3 - Listar produtos e estoque\n");
        printf("  4 - Repor estoque\n");
        printf("  5 - Alterar preco\n");
        printf("  0 - Voltar\n");
        opcao = lerInteiro("\n  Escolha: ", 0, 5);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: novaVenda();        break;
            case 2: cadastrarProduto(); break;
            case 3: listarProdutos();   break;
            case 4: reporEstoque();     break;
            case 5: alterarPreco();     break;
        }
        if (opcao != 0) pausar();
    } while (opcao != 0 && !encerrarSistema);
}

/* AGENTE 7 - RELATORIOS */
void relInadimplentes(void) {
    int   i, qtd = 0, meses;
    float valor, divida = 0;

    titulo("RELATORIO - MENSALIDADES EM ATRASO");
    printf(NEGRITO "  %-4s %-26s %-8s %-7s %s" RESET "\n",
           "ID", "NOME", "PLANO", "MESES", "DEVE (R$)");
    for (i = 0; i < totalAlunos; i++) {
        if (alunos[i].ativo && !alunoEmDia(&alunos[i])) {
            meses = competenciaAtual - alunos[i].mesPago;
            valor = meses * VALOR_PLANO[alunos[i].plano];
            printf(VERMELHO "  %-4d %-26.26s %-8s %-7d %.2f" RESET "\n", alunos[i].id,
                   alunos[i].nome, NOME_PLANO[alunos[i].plano], meses, valor);
            qtd++;
            divida += valor;
        }
    }
    if (qtd == 0) {
        printf(VERDE "  Nenhum aluno em atraso. Parabens!" RESET "\n");
    } else {
        printf("\n  Total: %d aluno(s) | Divida acumulada: R$ %.2f\n", qtd, divida);
    }
}

void relRankingFrequencia(void) {
    int pos[MAX_ALUNOS], n = 0, i, j, aux, maior;

    titulo("RELATORIO - RANKING DE FREQUENCIA (TOP 10)");
    for (i = 0; i < totalAlunos; i++) {
        if (alunos[i].ativo) pos[n++] = i;
    }
    if (n == 0) {
        printf("  Nenhum aluno ativo.\n");
        return;
    }
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (alunos[pos[j]].totalCheckins < alunos[pos[j + 1]].totalCheckins) {
                aux = pos[j]; pos[j] = pos[j + 1]; pos[j + 1] = aux;
            }
        }
    }
    maior = alunos[pos[0]].totalCheckins;
    for (i = 0; i < n && i < 10; i++) {
        printf("  %2d. %-22.22s ", i + 1, alunos[pos[i]].nome);
        barra(alunos[pos[i]].totalCheckins, maior, 24, i == 0 ? AMARELO : VERDE);
        printf(" %d\n", alunos[pos[i]].totalCheckins);
    }
}

void relEstoqueCritico(void) {
    int i, qtd = 0, sugestao;

    titulo("RELATORIO - ESTOQUE CRITICO");
    printf(NEGRITO "  %-4s %-26s %8s %6s %10s" RESET "\n",
           "ID", "PRODUTO", "ESTOQUE", "MIN.", "COMPRAR");
    for (i = 0; i < totalProdutos; i++) {
        if (produtos[i].estoque <= produtos[i].estoqueMinimo) {
            sugestao = produtos[i].estoqueMinimo * 2 - produtos[i].estoque;
            printf("%s  %-4d %-26.26s %8d %6d %10d" RESET "\n",
                   produtos[i].estoque == 0 ? VERMELHO : AMARELO, produtos[i].id,
                   produtos[i].nome, produtos[i].estoque, produtos[i].estoqueMinimo,
                   sugestao);
            qtd++;
        }
    }
    if (qtd == 0) printf(VERDE "  Todos os produtos acima do estoque minimo." RESET "\n");
}

void relVendasDoDia(void) {
    int   i, qtd = 0;
    float total = 0, descontos = 0;
    char  hoje[11];

    titulo("RELATORIO - VENDAS DE HOJE");
    formatarData(dataAtual.dia, dataAtual.mes, dataAtual.ano, hoje);
    printf(NEGRITO "  %-6s %-6s %-9s %-10s %10s" RESET "\n",
           "CUPOM", "ITENS", "ALUNO", "PAGTO", "TOTAL");
    for (i = 0; i < totalVendas; i++) {
        if (strcmp(vendas[i].data, hoje) == 0) {
            printf("  %-6d %-6d %-9d %-10s %10.2f\n", vendas[i].id, vendas[i].numItens,
                   vendas[i].idAluno, NOME_FORMA[vendas[i].formaPgto], vendas[i].total);
            qtd++;
            total += vendas[i].total;
            descontos += vendas[i].desconto;
        }
    }
    if (qtd == 0) {
        printf("  Nenhuma venda registrada hoje.\n");
    } else {
        printf("\n  %d venda(s) | Total: R$ %.2f | Descontos concedidos: R$ %.2f\n",
               qtd, total, descontos);
    }
}

void relMaisVendidos(void) {
    int qtdVendida[MAX_PRODUTOS] = {0};
    int pos[MAX_PRODUTOS], n = 0, i, j, k, idx, aux, maior;

    titulo("RELATORIO - PRODUTOS MAIS VENDIDOS (TOP 5)");
    for (i = 0; i < totalVendas; i++) {
        for (k = 0; k < vendas[i].numItens; k++) {
            idx = buscarProduto(vendas[i].itens[k].idProduto);
            if (idx >= 0) qtdVendida[idx] += vendas[i].itens[k].qtd;
        }
    }
    for (i = 0; i < totalProdutos; i++) {
        if (qtdVendida[i] > 0) pos[n++] = i;
    }
    if (n == 0) {
        printf("  Ainda nao ha vendas registradas.\n");
        return;
    }
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (qtdVendida[pos[j]] < qtdVendida[pos[j + 1]]) {
                aux = pos[j]; pos[j] = pos[j + 1]; pos[j + 1] = aux;
            }
        }
    }
    maior = qtdVendida[pos[0]];
    for (i = 0; i < n && i < 5; i++) {
        printf("  %d. %-24.24s ", i + 1, produtos[pos[i]].nome);
        barra(qtdVendida[pos[i]], maior, 20, CIANO);
        printf(" %d un\n", qtdVendida[pos[i]]);
    }
}

void relFaturamento(void) {
    int   i, percM, percL;
    float mensal = 0, loja = 0, descontos = 0, total;

    titulo("RELATORIO - FATURAMENTO GERAL");
    for (i = 0; i < totalPagamentos; i++) mensal += pagamentos[i].valor;
    for (i = 0; i < totalVendas; i++) {
        loja      += vendas[i].total;
        descontos += vendas[i].desconto;
    }
    total = mensal + loja;
    percM = total > 0 ? (int)(mensal * 100 / total + 0.5f) : 0;
    percL = total > 0 ? 100 - percM : 0;

    printf("  Mensalidades  R$ %10.2f  ", mensal);
    barra(percM, 100, 30, CIANO);
    printf(" %d%%\n", percM);
    printf("  Loja (PDV)    R$ %10.2f  ", loja);
    barra(percL, 100, 30, VERDE);
    printf(" %d%%\n", percL);
    printf("  " CINZA);
    linha('-', 56);
    printf(RESET);
    printf(NEGRITO "  FATURAMENTO TOTAL  R$ %.2f" RESET "\n", total);
    printf("  Descontos concedidos a alunos: R$ %.2f\n", descontos);
}

void menuRelatorios(void) {
    int opcao;
    do {
        titulo("MODULO RELATORIOS");
        printf("  1 - Mensalidades em atraso\n");
        printf("  2 - Ranking de frequencia\n");
        printf("  3 - Estoque critico\n");
        printf("  4 - Vendas de hoje\n");
        printf("  5 - Produtos mais vendidos\n");
        printf("  6 - Faturamento geral\n");
        printf("  0 - Voltar\n");
        opcao = lerInteiro("\n  Escolha: ", 0, 6);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: relInadimplentes();     break;
            case 2: relRankingFrequencia(); break;
            case 3: relEstoqueCritico();    break;
            case 4: relVendasDoDia();       break;
            case 5: relMaisVendidos();      break;
            case 6: relFaturamento();       break;
        }
        if (opcao != 0) pausar();
    } while (opcao != 0 && !encerrarSistema);
}

void menuPrincipal(void) {
    int opcao, maxOpcao;
    do {
        banner();
        mostrarDashboard();
        titulo("MENU PRINCIPAL");
        printf("  " CINZA "Logado como: %s" RESET "\n\n", usuarioLogado.usuario);
        printf("  1 - Academia  (alunos, mensalidades, check-in)\n");
        printf("  2 - Loja      (produtos, estoque, caixa)\n");
        printf("  3 - Relatorios gerenciais\n");
        printf("  4 - Modo teste: avancar o mes\n");
        if (usuarioLogado.isAdmin) {
            printf(CIANO "  5 - Gerenciar usuarios (Admin)" RESET "\n");
            maxOpcao = 5;
        } else {
            maxOpcao = 4;
        }
        printf("  0 - Sair\n");
        opcao = lerInteiro("\n  Escolha: ", 0, maxOpcao);
        if (encerrarSistema) return;
        switch (opcao) {
            case 1: menuAcademia();   break;
            case 2: menuLoja();       break;
            case 3: menuRelatorios(); break;
            case 4: avancarMes(); pausar(); break;
            case 5: menuUsuarios();   break;
        }
    } while (opcao != 0 && !encerrarSistema);
}

int main(void) {
    carregarTudo();
    configurarDataInicial();
    if (encerrarSistema) return 0;
    if (!fazerLogin()) {
        if (!encerrarSistema) {
            printf(VERMELHO "\n  Tentativas esgotadas. Sistema encerrado." RESET "\n");
        }
        return 1;
    }
    menuPrincipal();
    printf("\n  Ate logo!\n");
    return 0;
}
