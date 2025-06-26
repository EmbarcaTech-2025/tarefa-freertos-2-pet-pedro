
# Tarefa: Roteiro de FreeRTOS #2 - EmbarcaTech 2025

Autor: Pedro Henrique Oliveira e Petersson Matos

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Campinas, Junho de 2025

---

Chaos Climb - Um Jogo de Plataforma para Raspberry Pi Pico e OLED


🎮 Sobre o Jogo
Chaos Climb é um jogo de plataforma minimalista desenvolvido para a Raspberry Pi Pico utilizando o sistema operacional em tempo real FreeRTOS e um display OLED. O objetivo do jogo é atravessar níveis passando entre plataformas móveis e fixas, com cada novo nível gerando um desafio único e aleatório.

O jogo apresenta um design visual simples, com um personagem quadrado e plataformas finas, focando na jogabilidade e na imprevisibilidade do ambiente.

✨ Recursos Principais
Progressão de Níveis: Conclua um nível alcançando a plataforma fixa à direita. Ao fazer isso, um novo nível será gerado.

Dificuldade Dinâmica:

A altura das plataformas fixa inicial e final é aleatória a cada nova partida/nível, criando layouts únicos.

As plataformas móveis possuem velocidades e direções (para cima/baixo) aleatórias a cada novo nível.

Controles Simples: Mova o personagem para a esquerda e direita usando dois botões físicos.

Plataformas Duplas: Duas plataformas móveis se movimentam em sincronia vertical em cada coluna, adicionando complexidade aos saltos.

Menu e Configurações:

Tela inicial com o nome do jogo.

Tela de configurações para ajustar a velocidade de movimento do personagem.

Navegação intuitiva por cliques curtos e long-press de botões.

Ciclo Visual Contínuo: As plataformas móveis realizam um movimento cíclico suave, aparecendo em uma extremidade da tela e reaparecendo na outra, sem interrupções abruptas.

Feedback Visual: Tela de "GAME OVER" ao cair e "NÍVEL COMPLETO!" ao passar de fase.

---

## 📜 Licença
GNU GPL-3.0.
