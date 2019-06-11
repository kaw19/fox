#define SUPORTE    		false   // quando a luminária estiver no suporte (RS closed = LOW)
#define LIGA	   	    HIGH
#define DESL	   	    LOW

// Pinos do uC ----------------------------------------------------------------------------
#define RS            3   // Indica o local da luminária: no suporte ou não (Reed Switch)
#define LED           4   // pino para ligar/desligar lanterna
#define IRED          5   // InfraVermelho: liga/desl. lanterna com ela no suporte

// Estados --------------------------------------------------------------------------------
#define ESTADO_A      1   // Lanterna DESL./FORA
#define ESTADO_B      2   // Lanterna DESL./SUPORTE
#define ESTADO_C      3   // Lanterna LIGA./FORA

// Eventos --------------------------------------------------------------------------------
#define NADA			    0		// nenhum evento
#define COMANDO_IV 		1		// evento: comando infravermelho
#define RETIRADA_SUP	2		// evento: retirada da lanterna do suporte
#define RETORNO_SUP		3   // evento: retorno da lanterna ao suporte

// Var.s Globais --------------------------------------------------------------------------
long tempo;
int  contComandos;
int  estado; //, estado_ant;
bool sup_ant, sup_atual;        // lant. no suporte (true) ou fora (false)

// Protótipos ------------------------------------------------------------------------------
void lanterna(bool acao);
int verifica_evento(void);
bool leRS(void);                // retorna valor da média das leituras do Reed Switch
bool leIR(void);

void setup() {
  pinMode(LED, OUTPUT); pinMode(13, OUTPUT);
  pinMode(RS, INPUT_PULLUP);    // pullup interno de 20 kOhm. RS: open => HIGH, closed => LOW 
  pinMode(IRED, INPUT);         // liga/desl. lanterna com ela no suporte
  // Serial.begin(9600);  
  sup_ant = sup_atual = (leRS() == SUPORTE); 
  if (sup_atual) {              // lanterna no suporte
    digitalWrite(LED, LOW);     // desliga lanterna
    estado = ESTADO_A; }
  else {                        // lanterna fora do suporte
    digitalWrite(LED, HIGH);    // liga lanterna
    estado = ESTADO_C; }
}

void loop() {
  int evento;
  evento = verifica_evento();
  if (evento != NADA)
    switch(estado) {
      case ESTADO_A:            // no suporte, lanterna desligada
        switch(evento) {
          case COMANDO_IV:
            estado = ESTADO_B; break;
          case RETIRADA_SUP:
            estado = ESTADO_C; break; }
        lanterna(LIGA); break;
      case ESTADO_B:            // no suporte, lanterna ligada
        switch(evento) {
          case COMANDO_IV:
            lanterna(DESL); estado = ESTADO_A; break;
          case RETIRADA_SUP:
            lanterna(LIGA); estado = ESTADO_C; break; }
        break;
      case ESTADO_C:            // fora do suporte, lanterna ligada
        if(evento == RETORNO_SUP) {
          estado = ESTADO_B; }
    }
}

// Rotinas & Funções ------------------------------------------------------------------------
void lanterna(bool acao) {
  digitalWrite(LED, acao); 		// liga/desl. lanterna
}    
      
int verifica_evento(void) {
  int ev;
  bool comando;
  
  sup_ant = sup_atual;
  sup_atual = (leRS() == SUPORTE);
  if (sup_atual == sup_ant)
    ev = NADA;
  else if (sup_atual && (!sup_ant))    // voltou para o suporte
	  ev = RETORNO_SUP;
  else if (sup_ant && (!sup_atual))    // tirou do suporte
    ev = RETIRADA_SUP;
  else
    ev = NADA;
  
  comando = leIR();
  if (sup_atual && comando) {	         // Está no suporte e tem comando p/ ligar?
    //Serial.print("Comando IV: "); Serial.println(comando);
    ev = COMANDO_IV; delay(500); }
  return ev;
}

bool leRS(void) {                // lê o estado do Reed Switch - lanterna no suporte?
  unsigned i=0, alto = 0;
  for(; i < 1000; i++)
    if (digitalRead(RS))
      alto++;
  if (alto > 500) {
    digitalWrite(13, HIGH);
    return true;                // lanterna fora do suporte
  }
  else {
    digitalWrite(13, LOW);
    return false;               // lanterna no suporte
  }
}

bool leIR(void) {
  unsigned i=0, alto = 0;
  for(; i < 1000; i++)
    if (digitalRead(IRED))
      alto++;
  if (alto > 500)
    return false;               // obstáculo detectado
  else
    return true;                // sem obstáculo
}
