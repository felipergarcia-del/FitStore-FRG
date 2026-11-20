# FitStore - Academia + Loja (PDV)

Sistema de gestão em **linguagem C** para atender uma academia que também opera uma loja na recepção. O software integra o **controle de alunos, mensalidades e acesso (check-in)** a um **ponto de venda (PDV)** com controle de estoque, carrinho, desconto automático e emissão de cupom.

Projeto Integrador desenvolvido para a disciplina de **Lógica de Programação com Linguagem C** — **IESB**. Utiliza exclusivamente as bibliotecas padrão `<stdio.h>`, `<string.h>` e `<ctype.h>`.

> **Executar online (sem instalar nada):** Cole o conteúdo de `src/fitstore.c` no [OnlineGDB](https://www.onlinegdb.com) (linguagem C) e clique em **Run**.
>
> **Credenciais iniciais de Administrador:** Usuário `admin` | Senha `fit123`

## Sumário

1. O que é o sistema e para quem foi feito

2. Como executar

3. Guia de uso

4. Regras de negócio

5. Como o projeto foi construído

6. Estrutura de pastas e arquivos de dados

7. Limitações conhecidas e melhorias futuras

8. Autor e turma

## 1. O que é o sistema e para quem foi feito

O **FitStore** foi projetado sob medida para o **Marcão**, proprietário da **Academia Ritmo Forte**, que precisava substituir controles manuais por um sistema único onde a recepção e a loja conversassem entre si.

| Área | Funcionalidades Principais | 
| ----- | ----- | 
| **Controle de Acesso** | Login com até 3 tentativas, gerenciamento multiusuário pelo administrador (`usuarios.dat`) e painel diário com indicadores coloridos. | 
| **Módulo Academia** | Matrícula com validação de CPF único, 3 planos disponíveis, recebimento de mensalidade mês a mês, troca de plano, ativação/inativação e **check-in** com bloqueio automático de inadimplentes. | 
| **Módulo Loja (PDV)** | Cadastro de produtos, alerta de estoque mínimo, reposição, alteração de preço, **caixa com carrinho**, desconto automático para alunos adimplentes, cálculo de troco e impressão de cupom. | 
| **Relatórios Gerenciais** | Inadimplentes e dívida acumulada, ranking de frequência com gráfico em barras, estoque crítico com sugestão de compra, vendas do dia, top 5 produtos mais vendidos e faturamento consolidado. | 
| **Persistência e Data** | Pergunta a data inicial (`ano`, `mês` e `dia`) quando não há `data.dat` válido e grava a `DataSistema` e competência em `data.dat` ao avançar o mês, mantendo o estado entre execuções. | 

## 2. Como executar

### Opção A — Compilador Online (Recomendada)

1. Acesse o [OnlineGDB](https://www.onlinegdb.com) ou o [Replit](https://replit.com) e selecione a linguagem **C**.

2. Substitua o código padrão pelo conteúdo de `src/fitstore.c`.

3. Clique em **Run**.

> **Observações:**
>
> * Caso o console exiba códigos como `[92m` no lugar das cores, altere `#define USAR_COR 1` para `#define USAR_COR 0` no início do código.
>
> * Compiladores online mantêm os arquivos `.dat` apenas durante a sessão ativa.

### Opção B — VS Code / Terminal com GCC

```
git clone https://github.com/SEU-USUARIO/fitstore-felipe.git
cd fitstore-felipe
gcc -Wall -Wextra -pedantic -std=c99 src/fitstore.c -o fitstore
./fitstore            # No Windows: fitstore.exe

```

O código compila com **zero warnings**.

### Opção C — GitHub Codespaces

No repositório, clique em **Code > Codespaces > Create** e use os comandos da Opção B no terminal.

### Credenciais de Acesso

| Perfil | Usuário | Senha | Permissões | 
| ----- | ----- | ----- | ----- | 
| **Administrador** | `admin` | `fit123` | Acesso total + cadastro e ativação/desativação de operadores | 
| **Operador** | *(criado pelo admin)* | *(definida pelo admin)* | Academia, Loja, Relatórios e Modo Teste | 

## 3. Guia de uso

### 3.1 Configuração de Data, Login e Painel do Dia

Quando o arquivo `data.dat` ainda não existe (ou não contém uma data válida), o sistema solicita **Ano** (`1900` a `2999`), **Mês** (`1` a `12`) e **Dia** (ajustado automaticamente caso ultrapasse o último dia do mês, inclusive anos bissextos) e salva em `data.dat`. Na primeira execução, também cria automaticamente a conta `admin` (`fit123`) em `usuarios.dat`. Após 3 tentativas incorretas de login, o programa encerra.

```
  >> PAINEL DO DIA
  ------------------------------------------------------------
  Data: 20/11/2026   |   Competencia: 11/2026

  Alunos ativos                3
  Mensalidades em atraso       1
  Check-ins hoje               1
  Produtos em estoque critico  0
  Vendas hoje                  3 (R$ 660.12)

  >> MENU PRINCIPAL
  ------------------------------------------------------------
  Logado como: admin

  1 - Academia  (alunos, mensalidades, check-in)
  2 - Loja      (produtos, estoque, caixa)
  3 - Relatorios gerenciais
  4 - Modo teste: avancar o mes
  5 - Gerenciar usuarios (Admin)
  0 - Sair

```

* A opção **4 (Modo teste: avancar o mes)** avança `dataAtual` em 1 mês (ajustando o dia para o máximo do novo mês quando necessário), recalcula `competenciaAtual = ano * 12 + mes` e **salva em `data.dat`**, mantendo o mês avançado mesmo se o programa for encerrado e aberto novamente.

* A opção **5 (Gerenciar usuarios)** aparece exclusivamente quando o usuário logado tem `isAdmin == 1`.

### 3.2 Módulo Academia

Gerencia o ciclo de vida dos alunos e a liberação da entrada:

| Opção | Descrição | 
| ----- | ----- | 
| **1 - Matricular aluno** | Valida nome, CPF (11 dígitos numéricos, sem duplicidade) e plano. O aluno inicia devendo o mês vigente, com opção de quitar a 1ª mensalidade no ato. | 
| **2 - Listar alunos** | Exibe ID, nome, plano, situação (`ATIVO`/`INATIVO`), status financeiro (`EM DIA`/`ATRASADA`) e total de check-ins. | 
| **3 - Receber mensalidade** | Quita uma mensalidade por vez, sempre começando pela mais antiga em aberto. | 
| **4 - Check-in** | Libera a entrada apenas para alunos ativos, em dia e que ainda não fizeram check-in na `dataAtual`. | 
| **5 - Trocar plano** | Atualiza o plano do aluno para as próximas cobranças. | 
| **6 - Ativar / Inativar aluno** | Alterna o status sem excluir histórico; ao reativar, meses anteriores ao mês passado são ajustados para não cobrar períodos afastados. | 

```
  >> ALUNOS CADASTRADOS
  ------------------------------------------------------------
  ID   NOME                       PLANO    SITUACAO  MENSAL.   CHECK-INS
  1    Ana Souza                  Plus     ATIVO     EM DIA    1
  2    Bruno Lima                 Basic    ATIVO     ATRASADA  0
  3    Carla Dias                 Premium  ATIVO     EM DIA    0

```

### 3.3 Módulo Loja (PDV)

Controla o estoque e realiza vendas integradas ao cadastro de alunos:

| Opção | Descrição | 
| ----- | ----- | 
| **1 - Caixa (nova venda)** | Identifica o aluno (ou `0` para avulso), aplica 10% de desconto se elegível, gerencia o carrinho, calcula troco em dinheiro, dá baixa no estoque e imprime o cupom. | 
| **2 - Cadastrar produto** | Registra nome, preço unitário, quantidade inicial e estoque mínimo para alerta. | 
| **3 - Listar produtos e estoque** | Exibe o catálogo com status colorido (`OK`, `CRITICO` ou `ESGOTADO`). | 
| **4 - Repor estoque** | Adiciona unidades recebidas ao estoque atual do produto. | 
| **5 - Alterar preço** | Atualiza o valor de venda sem alterar os registros de vendas já realizadas. | 

```
  ======================================================
   FITSTORE - CUPOM #0001                   20/11/2026
  ======================================================
   Aluno: Ana Souza
    2x Whey Protein 900g              R$    299.80
    4x Barra de Proteina              R$     34.00
  ------------------------------------------------------
   Subtotal ........................ R$    333.80
   Desconto ........................ R$     33.38
   TOTAL ........................... R$    300.42
   Pagamento: Dinheiro
  ======================================================

```

### 3.4 Módulo Relatórios Gerenciais

Oferece indicadores visuais com gráficos de barras no terminal:

1. **Mensalidades em atraso:** lista alunos ativos inadimplentes, meses em aberto e dívida total.

2. **Ranking de frequência:** Top 10 alunos mais assíduos com barras proporcionais.

3. **Estoque crítico:** produtos com `estoque <= estoqueMinimo` e sugestão automática de compra.

4. **Vendas de hoje:** resumo das vendas realizadas na `dataAtual` e total de descontos concedidos.

5. **Produtos mais vendidos:** Top 5 itens por quantidade vendida.

6. **Faturamento geral:** comparativo percentual entre receitas de Mensalidades e Loja (PDV).

### 3.5 Gerenciar Usuários (Exclusivo Admin)

Permite ao administrador (`admin`) criar contas operacionais para funcionários, listar usuários cadastrados e ativar ou desativar o acesso de qualquer operador sem excluir seu registro.

## 4. Regras de negócio

| Regra | Definição no Sistema | 
| ----- | ----- | 
| **Planos e Valores** | **Basic:** R\$ 89,90/mês | **Plus:** R\$ 129,90/mês | **Premium:** R\$ 179,90/mês | 
| **Inadimplência** | Quando `mesPago < competenciaAtual`, o aluno consta em atraso (sem período de tolerância). | 
| **Quitação** | Pagamento integral de 1 mensalidade por operação, quitando sempre a competência mais antiga. | 
| **Controle de Acesso** | Bloqueia alunos inativos, inadimplentes ou que já realizaram check-in na data atual. | 
| **Desconto na Loja** | **10%** aplicado automaticamente apenas para clientes identificados como **alunos ativos e em dia**. | 
| **Controle de Estoque** | Impede vender além do saldo disponível (considerando itens já no carrinho); baixa ocorre somente ao finalizar a venda. | 
| **Sugestão de Compra** | Calculada pela fórmula `(estoqueMinimo * 2) - estoque`. | 
| **Limite do Carrinho** | Máximo de **10 itens diferentes** por venda. | 
| **Integridade de Dados** | Alunos nunca são excluídos fisicamente, apenas alternados entre `ATIVO` e `INATIVO`. | 
| **Acesso Administrativo** | Apenas usuários com `isAdmin = 1` acessam o menu de usuários; a conta `admin` não pode ser desativada. | 

## 5. Como o projeto foi construído

O desenvolvimento seguiu uma arquitetura modular dividida em **Agentes** dentro de `fitstore.c` e evoluída por **etapas incrementais**, utilizando exclusivamente `<stdio.h>`, `<string.h>` e `<ctype.h>`:

| Agente | Responsabilidade | Funções Principais | 
| ----- | ----- | ----- | 
| **Agente 2 — Infraestrutura** | Entrada segura, calendário/competência, buscas e arquivos `.dat` | `lerTexto`, `lerInteiro`, `lerDecimal`, `carregarDataSistema`, `salvarDataSistema`, `configurarDataInicial`, `carregarLista`, `salvarLista` | 
| **Agente 3 — Visual** | Interface de console | `linha`, `banner`, `titulo`, `barra`, `mostrarDashboard` | 
| **Agente 4 — Acesso** | Autenticação, gestão de usuários e avanço de mês | `buscarUsuarioPorNome`, `fazerLogin`, `cadastrarUsuario`, `listarUsuarios`, `alternarStatusUsuario`, `menuUsuarios`, `avancarMes` | 
| **Agente 5 — Academia** | Planos, matrículas, mensalidades e check-in | `listarPlanos`, `registrarPagamento`, `receberMensalidade`, `cadastrarAluno`, `listarAlunos`, `alterarPlano`, `alternarSituacao`, `fazerCheckin`, `menuAcademia` | 
| **Agente 6 — Loja** | Produtos, estoque, carrinho, cupom e caixa | `centavos`, `cadastrarProduto`, `listarProdutos`, `alterarPreco`, `reporEstoque`, `estoqueLivre`, `adicionarAoCarrinho`, `removerUltimoItem`, `calcularTotais`, `mostrarCarrinho`, `imprimirCupom`, `finalizarVenda`, `novaVenda`, `menuLoja` | 
| **Agente 7 — Relatórios** | Indicadores gerenciais e gráficos em texto | `relInadimplentes`, `relRankingFrequencia`, `relEstoqueCritico`, `relVendasDoDia`, `relMaisVendidos`, `relFaturamento`, `menuRelatorios` | 

### Decisões Técnicas

* **Apenas 3 bibliotecas padrão (`<stdio.h>`, `<string.h>`, `<ctype.h>`):** A data e a competência são controladas manualmente via `DataSistema` (`dia`, `mes`, `ano`, com cálculo de anos bissextos e dias por mês) e persistidas em `data.dat`, sem depender de `time.h` ou `stdlib.h`.

* **Entrada segura com `getchar` + `sscanf`:** `lerTexto` lê caractere a caractere até `\n` ou `EOF` (tratando encerramento limpo via `encerrarSistema`), enquanto `lerInteiro` e `lerDecimal` validam formatos e faixas numéricas.

* **Competência numérica:** Calculada como `ano * 12 + mes`, facilitando comparações de mensalidades em dia ou em atraso.

* **Dinheiro em centavos:** A função `centavos()` converte valores para inteiro arredondado na conferência de pagamento em dinheiro, evitando imprecisões de ponto flutuante.

* **Histórico imutável de vendas:** Cada `ItemVenda` armazena cópia do nome e do preço unitário no momento da venda.

## 6. Estrutura de pastas e arquivos de dados

```
fitstore/
├── src/
│   └── fitstore.c              # Código-fonte completo do sistema
├── docs/
│   ├── Guia_do_Aluno_FitStore.docx
│   └── Etapa0_Briefing.docx
├── relatorio/
│   └── Relatorio_Tecnico.pdf   # Registro de erros, soluções e uso de IA
├── prints/                     # Capturas de tela utilizadas neste README
├── .gitignore                  # Ignora executáveis e arquivos .dat locais
└── README.md                   # Documentação do projeto

```

### Arquivos de Dados (`.dat`)

Gerados automaticamente na pasta de execução por meio de rotinas binárias (`carregarLista`/`salvarLista` e `carregarDataSistema`/`salvarDataSistema`):

* `data.dat` — Estrutura `DataSistema` (`dia`, `mes`, `ano`) gravada na configuração inicial e atualizada ao avançar o mês.

* `usuarios.dat` — Credenciais, status (`ativo`) e perfil (`isAdmin`) dos usuários.

* `alunos.dat` — Cadastro, plano, última competência paga e histórico de check-ins dos alunos.

* `produtos.dat` — Catálogo, preços, estoque atual e estoque mínimo.

* `vendas.dat` — Histórico de vendas, itens vendidos, descontos, totais e formas de pagamento.

* `pagamentos.dat` — Registro financeiro das mensalidades pagas.

> **Nota:** Caso alguma `struct` seja alterada no código-fonte, remova os arquivos `.dat` antigos antes de executar novamente para evitar incompatibilidade de leitura binária.

## 7. Limitações conhecidas e melhorias futuras

### Limitações Conhecidas

* **Capacidade estática:** Os vetores em memória possuem limites fixos (`MAX_ALUNOS = 200`, `MAX_PRODUTOS = 100`, `MAX_VENDAS = 300`, `MAX_PAGAMENTOS = 600`, `MAX_USUARIOS = 20`).

* **Armazenamento local em binário:** Os arquivos `.dat` ficam na pasta de execução e não são portáveis entre arquiteturas incompatíveis.

* **Senhas sem hash:** As senhas são armazenadas como texto na `struct Login` dentro de `usuarios.dat`.

* **Operações não contempladas:** Não há estorno/devolução de vendas nem emissão de nota fiscal eletrônica.

### Melhorias Futuras

* Implementar **pagamento antecipado** de várias mensalidades em uma única operação.

* Adicionar **relatório financeiro filtrado por mês/ano** e **exportação para CSV**.

* Implementar **devolução de produto** com reposição automática ao estoque.

* Adicionar opção para usuários **alterarem a própria senha**.

## 8. Autor e turma

* **Autor:** Felipe Ribeiro Garcia

* **Instituição:** Centro Universitário IESB

* **Disciplina:** Lógica de Programação com Linguagem C

* **Turma:** *ENGIADM1A*

* **Professor:** Prof. Me. Bruno Assunção Dias