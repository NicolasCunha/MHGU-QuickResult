# Quick Result

Plugin [Forge](https://github.com/Fexty12573/forge) para o [MHGU Recomp](https://github.com/morerokk/MHGURecomp)
(Monster Hunter Generations Ultimate 1.4.0) que **pula a tela de "Quest Complete"** no fim da quest e leva direto
para as recompensas.

[Read in English](README.md)

- A tela de Quest Complete normalmente prende você por uns 30 segundos. Com o Quick Result ela some assim que começa.
- Opcional: pular também a contagem de "retorno à vila" logo depois de concluir a quest.
- As duas opções ficam no menu do Forge (aperte **Insert** no jogo e abra **Quick Result**). Pular o Quest Complete
  vem ligado e pular a contagem vem desligado. As opções voltam ao padrão toda vez que o jogo inicia.

## Instalação

1. Baixe o `QuickResult-<versão>.zip` em [Releases](../../releases), ou compile (veja abaixo).
2. Extraia na pasta `mods` ao lado do `mhgurecomp.exe`, para ficar
   `mods/QuickResult/romfs/nativeNX/plugins/quickresult.nro`.
3. Abra o jogo. O `log.txt` deve ter a linha `[QuickResult] 1.1.0 ready: ...` e, depois de cada quest,
   `[QuickResult] Quest Complete screen skipped (...)`.

Para desinstalar, apague `mods/QuickResult`.

**Requisitos:** MHGU Recomp com Forge (o Forge 1.5.1 já vem no recomp) e o jogo na versão **1.4.0**.

**Limitações:** testado só em single player (offline) e no Windows. Se der problema no multiplayer online, desligue a
opção no menu do Forge.

## Como funciona

Depois de concluir a quest, o jogo passa pelo estado 3 (contagem de retorno) e pelo estado 4 (a tela de Quest
Complete). Os dois terminam quando **a mesma contagem** chega a zero; no estado 4 ela recomeça em 899. O Quick Result
zera essa contagem no estado 4, que é a saída que o próprio jogo usaria. A tela de resultados, onde o jogo salva o clear
da quest, roda normalmente. Mais detalhes no [README em inglês](README.md#how-it-works).

## Compilar

Dependências:

| Dependência | Windows | Linux | macOS |
|---|---|---|---|
| Shell POSIX | [Git for Windows](https://git-scm.com/download/win) (Git Bash) | já vem | já vem |
| Arm GNU Toolchain (`arm-none-eabi-gcc`) | `winget install Arm.ArmGnuToolchain` | `sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi` | `brew install arm-none-eabi-gcc` |
| Python 3 | `winget install Python.Python.3.12` | `sudo apt install python3` | `brew install python` |

O devkitPro **não** é necessário.

Na pasta do repositório (no Windows, pelo Git Bash):

```sh
sh build.sh                                # gera out/quickresult.nro
sh build.sh release                        # gera dist/QuickResult-<versão>.zip, pronto para publicar
RECOMP=/d/MHGURecomp sh build.sh install   # compila e copia para <recomp>/mods/QuickResult
sh build.sh diag                           # gera out/quickresult_diag.nro, a versão de pesquisa (só escreve no log)
```

No Windows o script encontra o compilador ARM e o Python nas pastas padrão de instalação, mesmo fora do `PATH`.

## Créditos e licença

Licença MIT ([LICENSE](LICENSE)). Agradecimentos ao [Forge](https://github.com/Fexty12573/forge), ao
[MHGU Recomp](https://github.com/morerokk/MHGURecomp), ao [mhgu-gathering-map](https://github.com/lick-mee/mhgu-gathering-map),
ao [mhgu-field-qol](https://github.com/mhvuze/mhgu-field-qol) (arquivos de build e `forge.h`, MIT, ver
[THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES)) e ao SkipEndQuestTime.
