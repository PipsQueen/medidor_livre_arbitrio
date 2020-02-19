/* @Ivan: Como ainda nao sei se o reset sera feito com o pressionar de dois botoes simultaneos ou a depender de um tempo sem acao,
 * as implementacoes para ambos os casos estao comentadas. O que compete ao reset por timer estara identificado por ##TIMER##,
 * ja o que sera feito com botoes simultaneos estara identificado por ##DOISBOTOES##
 *
 * Alias, nada daqui foi testado, mas acredito que a implementacao nao deve estar muito longe do necessario.
 */




#define N_HISTORICO 4
#define N_ESTATISTICAS (1<<N_HISTORICO)
#define N_BOTOES 2
// @Ivan: De onde veio o ATRASO_BOUNCE? 100ms eh o ideal? Alias, isso eh em ms? Se for, como parametro: no Beaba operamos com 4 ms (+-) de bounce e 5 ms de 2botoes. 
#define ATRASO_BOUNCE 100 
/* ##DOISBOTOES##
#define ATRASO_2BOTOES = 155*/
/* ##TIMER##
#define MAX_INTERVALO = 5000 // @Ivan: Assumindo que o ATRASO_BOUNCE eh contado em ms, isso daria 5 segundos entre 1 usuario e outro. */
#define intervalo_treinamento 8

int estatisticas[N_ESTATISTICAS];
int historico[N_HISTORICO];
int acertos = 0;
int n_escolhas = 0;
float taxa_acertos = 0.5;
/* ##TIMER##
int timer = 0; 
*/
struct botao {
    int n;
    int estado;
    int atraso;
};
struct botao botoes[N_BOTOES];
enum estado_botao { b_apertado, b_solto, b_ativar, b_desapertado };

const int pinoBotao0 = 2;    // the number of the pushbutton pin
const int pinoBotao1 = 3;
const int pinoVoltimetro = 4;
int pinosBotoes[N_BOTOES] = { pinoBotao0, pinoBotao1 };
void atualiza_botoes ();
void atualiza_displays ();
float f_livre_arbitrio();
void reset_Historico(); // Implementaçao para o reset de estatisticas.

void setup() {

  pinMode(pinoBotao0, INPUT);
  pinMode(pinoBotao1, INPUT);
  pinMode(pinoVoltimetro, OUTPUT);

  //Inicializa estatisticas
  for ( int i = 0; i < N_ESTATISTICAS; i++ ) {
    estatisticas[i] = 0;
  }
  // Inicializa historico
  for (int i = 0; i < N_HISTORICO; i++ ) {
    historico[i] = 0;
  }
  // put your setup code here, to run once:

}

// put your main code here, to run repeatedly
void loop() {

  atualiza_botoes();
  if (/*(*/ botoes[0].estado == b_ativar || botoes[1].estado == b_ativar /*) && botoes[0].estado != botoes[1].estado*/) { //  ##DOISBOTOES##
	//##TIMER## timer = 0;
    n_escolhas++;

    // botao = 0 ou 1
    // 0: esquerda, 1: direita

    // Geracao de previsao
    // Possiveis estados: botao = 0 | historico ou botao = 1 | historico

    // Exemplo
    // historico
    // 0010
    // 10010 - 7 -> previsao
    // 00010 - 4

    int prox_estado0, prox_estado1;
    prox_estado0 = 0;
    prox_estado0 += 0 * 1<<(N_HISTORICO-1);
    for ( i = 0; i < N_HISTORICO-1; i++ ) {
      prox_estado0 += historico[i] * 1<<(N_HISTORICO-1-(i+1));
    }

    prox_estado1 = 0;
    prox_estado1 += 1 * 1<<(N_HISTORICO-1);
    for ( i = 0; i < N_HISTORICO-1; i++ ) {
      prox_estado1 += historico[i] * 1<<(N_HISTORICO-1-(i+1));
    }

    if ( estatistica[prox_estado0] > estatistica[prox_estado1] ) {
      previsao = 0;
    }
    else {
      previsao = 1;
    }

    //
    //
	
	int botao;

	for (botao = 0; botao < N_BOTOES; botao++) {
		if (botoes[botao].estado() == b_ativar) break; // Se os dois estiverem apertados a escolha sera tida como 0
	}

    if ( botao == previsao ) {
      acertos++;
      // taxa_acertos = acertos / n_escolhas;       ***movido abaixo para acomodar o fato de que se a previsao foi errada a taxa de acertos ainda eh atualizada***
    }
	taxa_acertos = (float)acertos / n_escolhas; //conversao para float para garantir que seja feita uma divisao que resulte em float
    // Acomodacao da nova decisao no historico
    for( i = N_HISTORICO-1; i > 0; i-- ) {
      historico[i] = historico[i-1];
    }
    historico[0] = botao; 

    // Estado associado ao novo historico
    int novo_estado = 0;
    for ( i = 0; i < N_HISTORICO; i++ ) {
        #ifdef DEBUG
        printf("historico %d : %d\n" , i, historico[i] );
        #endif // DEBUG
        novo_estado += historico[i] * 1<<(N_HISTORICO-1-i);
    }

    // Incementa estatistica do novo estado
    estatisticas[novo_estado]++;

    atualiza_display();

    Serial.print("Previsibilidade: %f", taxa_acertos);
  }
  else { //Necessario para situacoes de reset.
	
	/*	##DOISBOTOES##
	if(botoes[0].estado == b_ativar && botoes[0].estado == botoes[1].estado){
		reset_estatisticas();
		digitalWrite(pinoVoltimetro, 0.5);
	}
	 */

	/*	##TIMER##
	timer++;
	if(timer > MAX_INTERVALO) {
		reset_estatisticas();
		digitalWrite(pinoVoltimetro, 0.5);
	}
	*/
  }

}

void atualiza_botoes () {
//Anterior: if (solto) {}
//Aproveitado do @atualiza_botao(botao* b) do codigo do beaba
	
	for (int botao = 0; botao < N_BOTOES; botao++) {
		int estaAtivo = digitalRead(pinosBotoes[botao]);

		switch (botoes[botao].estado){
		case b_apertado:
			botoes[botao].atraso++;
			if (botoes[botao].atraso > ATRASO_BOUNCE && estaAtivo == LOW) {
				botoes[botao].estado = b_solto;
				botoes[botao].atraso = 0;
			}
			break;
		case b_ativar:
			botoes[botao].estado = b_desapertado;
			botoes[botao] = 0;
			break;
		case b_desapertado:
			botoes[botao].atraso++;
			if (botoes[botao].atraso > ATRASO_BOUNCE && estaAtivo == HIGH) {
				botoes[botao].estado = b_apertado;
				botoes[botao].atraso = 0;
			}
			break;
		case b_solto:
			/* ##DOISBOTOES##
			botoes[botao].atraso++;
			if(botoes[botao].atraso > ATRASO_2BOTOES){
				botoes[botao].estado = b_ativar;
				botoes[botao].atraso = 0;
			}
			
			*/

			//##TIMER## nao muda a implementacao abaixo:
			botoes[botao].estado = b_ativar;
			botoes[botao].atraso = 0;
			break;
		default: break;
		}
	}
}

void atualiza_displays () {
  // Display atual: Voltimetro (0-5V)
  // TODO: movimentar agulha de acordo com variancia
  // @Ivan: O analogWrite deveria	 ser capaz de movimentar a agulha

  saida = f_livre_arbitrio();

  analogWrite(pinoVoltimetro, saida);

}

// Use sua habilidade artistico-matematica-filosofica
float f_livre_arbitrio() {
  float livre_arbitrio;
  if ( taxa_acertos < 0.5 ) {
    livre_arbitrio = 1;
  }
  else {
  //Anterior: livre_arbitrio = pow((1-2*(taxa_acertos-0.5)),3); @Ivan: Acredito que seja muito ingreme. Para uma curva menos ingreme mas ainda radical, 2.5 parece bom.
    livre_arbitrio = pow((1-2*(taxa_acertos-0.5)),1.2); // @Ivan: Cria uma curva de livre arbitrio tal que ainda existe algum livre arbitrio ate por volta de taxa_acertos ~= 0.9 (livre arbitrio sendo 0.1 nesse caso)
  }
  return livre_arbitrio;
}


// Serve para permitir que um novo usuario passe a utilizar.
void reset_Estatisticas() {
	// Reinicializa estatisticas
	for (int i = 0; i < N_ESTATISTICAS; i++) {
		estatisticas[i] = 0;
	}
	// Reinicializa historico
	for (int i = 0; i < N_HISTORICO; i++) {
		historico[i] = 0;
	}
	// Reinicializa as variaveis que gerarao o livre arbitrio do usuario.
	n_escolhas = 0;
	taxa_acertos = 0.5;
	acertos = 0;
	// timer = 0; ##TIMER##
}
