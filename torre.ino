#include <Wire.h>

#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
//#include <unistd.h>

#define SIZE 6
#define UP 1
#define DOWN 6
#define LEFT 2
#define RIGHT 5
#define FRONT 3
#define BACK 4
/*#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define FRONT 4
#define BACK 5
*/
struct SnakeBody 
{
  int led;
  struct SnakeBody *next;
};

struct SnakeHead
{
  int x;
  int y;
  int z;
  int direction;
};

struct SnakeBody leds[SIZE][SIZE][SIZE];
struct SnakeHead snake_head;
struct SnakeBody *snake_tail;

void changeDirection(int direction)
{
 /*Serial.begin(9600);
    Serial.print("\n");
  Serial.print(snake_head.direction);
  Serial.print("\n");
  Serial.print(direction);*/
  if ((snake_head.direction == LEFT || snake_head.direction == RIGHT) && (direction != RIGHT && direction != LEFT) ||
      (snake_head.direction == UP || snake_head.direction == DOWN) && (direction != UP && direction != DOWN) ||
      (snake_head.direction == FRONT || snake_head.direction == BACK) && (direction != FRONT && direction != BACK))
  {
    snake_head.direction = direction;
  }    
}
void createFood()
{
  int i, j, k;
  do 
  {
    srand(millis());
    i = rand() % SIZE;
    j = rand() % SIZE;
    k = rand() % SIZE;
  } while (leds[i][j][k].led);
  leds[i][j][k].led = 2;
  Serial.begin(9600);
  Serial.print("\n");
  Serial.print(i);
  Serial.print("\n");
  Serial.print(j);
  Serial.print("\n");
  Serial.print(k);
  Serial.print("\n");
}

void initializeGame()
{

  //initial position
  leds[3][2][0].led = 1;
  leds[3][3][0].led = 1;
  leds[3][3][0].next = NULL;
  leds[3][2][0].next = &leds[3][3][0];
  //head
  snake_head.direction = FRONT;
  snake_head.x = 3;
  snake_head.y = 3;
  snake_head.z = 0;
  //tail
  snake_tail = &leds[3][2][0];
  //food
  createFood();
}

void runSnake()
{
  digitalWrite(5, HIGH);
  
  struct SnakeHead sb;
  if (snake_head.direction == FRONT) 
  {
    sb.y = (snake_head.y + 1) % SIZE;
    sb.x = snake_head.x;
    sb.z = snake_head.z;
  }
  else if (snake_head.direction == BACK) 
  {
    sb.y = (snake_head.y - 1 + SIZE) % SIZE;
    sb.x = snake_head.x;
    sb.z = snake_head.z;
  }
  else if (snake_head.direction == LEFT) 
  {
    sb.x = (snake_head.x - 1 + SIZE) % SIZE;
    sb.y = snake_head.y;
    sb.z = snake_head.z;
  }
  else if (snake_head.direction == RIGHT) 
  {
    sb.x = (snake_head.x + 1) % SIZE;
    sb.y = snake_head.y;
    sb.z = snake_head.z;
  }
  else if (snake_head.direction == UP) 
  {
    sb.z = (snake_head.z + 1) % SIZE;
    sb.y = snake_head.y;
    sb.x = snake_head.x;
  }
  else if (snake_head.direction == DOWN) 
  {
    sb.z = (snake_head.z - 1 + SIZE) % SIZE;
    sb.y = snake_head.y;
    sb.x = snake_head.x;
  }
  
  leds[snake_head.x][snake_head.y][snake_head.z].next = &leds[sb.x][sb.y][sb.z];

  if (leds[sb.x][sb.y][sb.z].led == 0)
  {
    snake_tail->led = 0;
    struct SnakeBody *aux = snake_tail;
    snake_tail = snake_tail->next;
    aux->next = NULL;
    /*
    struct SnakeBody *aux = leds[snake_head.x][snake_head.y][snake_head.z].ant;
    struct SnakeBody *aux_ant = &leds[snake_head.x][snake_head.y][snake_head.z];
    while (aux != NULL && aux->ant != NULL)
    {
      aux = aux->ant;
      aux_ant = aux_ant->ant;
    }
    aux->led = 0;
    aux_ant->ant = NULL;*/
  }
  else if (leds[sb.x][sb.y][sb.z].led == 1)
  {
    Serial.begin(9600);
  Serial.print("\nQuebrou");
    int i,j,k;
    for(i=0;i<6;i++){
      for(j=0;j<6;j++){
        for(k=0;k<6;k++){
          leds[i][j][k].led = 1;
        }
      }
    }
  }
  else
  {
    createFood();
  }

  leds[sb.x][sb.y][sb.z].led = 1;
  snake_head.x = sb.x;
  snake_head.y = sb.y;
  snake_head.z = sb.z;
  digitalWrite(5, LOW);
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
typedef struct {
  /* period in ticks */
  int period;
  /* ticks until next activation */
  int delay;
  /* function pointer */
  void (*func)(void);
  /* activation counter */
  int exec;
} Sched_Task_t;

Sched_Task_t Tasks[20];
int cur_task = 20;

void Sched_Schedule(void){
  int x;
  for(x=0; x<20; x++) {
    if((Tasks[x].func) && (Tasks[x].delay)){
      Tasks[x].delay--;
      if(!Tasks[x].delay){
        /* Schedule Task */
        Tasks[x].exec++;
        Tasks[x].delay = Tasks[x].period;
      }
    }
  }
}

int Sched_Dispatch(void){
  int prev_task = cur_task;
  int x;
  for(x=0; x<20; x++) {
    if((Tasks[x].func) && (Tasks[x].exec)) {
      Tasks[x].exec--;
      cur_task = x;
      interrupts();
      Tasks[x].func();
      noInterrupts();
      cur_task = prev_task;
      // Delete task if one-shot
      if(!Tasks[x].period)
        Tasks[x].func = 0;
      return 0;
    }
  }
}

void int_handler(void){
  noInterrupts();
  Sched_Schedule();
  Sched_Dispatch();
  interrupts();
};

int Sched_Init(void){
  int x;
  for(x=0; x<20; x++){
    Tasks[x].func = 0;
  }

  //Configure interrupts
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
 
  //OCR1A = 31250; // compare match register 16MHz/256/2Hz
  OCR1A = 31;    // compare match register 16MHz/256/2kHz
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS12); // 256 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts(); // enable all interrupts

  int_handler();
}

int Sched_AddT(void (*f)(void),int d, int p){
  int x;
  for(x=0; x<20; x++)
    if (!Tasks[x].func) {
      Tasks[x].period = p;
      Tasks[x].delay = d;
      Tasks[x].exec = 0;
      Tasks[x].func = f;
      return x;
    }
  return -1;
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt
  int_handler();
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

int clock = 8;
int reset = 9;

void shift_bit(){
  digitalWrite(clock, HIGH);
  digitalWrite(clock, LOW);
}

void reset_shift(){
  digitalWrite(reset, HIGH);
  shift_bit();
  digitalWrite(reset, LOW);
}

void reset_matrix(){
  //registo A integrado da esquerda
  Wire.beginTransmission(0x24);
  Wire.write(0x12); // address port A
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();
  
  //registo B integrado da esquerda
  Wire.beginTransmission(0x24);
  Wire.write(0x13); // address port B
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();

  //registo A integrado do meio
  Wire.beginTransmission(0x26);
  Wire.write(0x12); // address port A
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();

  //registo B integrado do meio
  Wire.beginTransmission(0x26);
  Wire.write(0x13); // address port B
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();

  //registo A integrado do direita
  Wire.beginTransmission(0x27);
  Wire.write(0x12); // address port A
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();

  //registo B integrado da direita
  Wire.beginTransmission(0x27);
  Wire.write(0x13); // address port B
  Wire.write(0xFF);  // desliga saidas
  Wire.endTransmission();
}

void setup_cubo(){
  Wire.begin();

  //integrado da esquerda
  Wire.beginTransmission(0x24);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x03); // registos 0 e 1 como saídas e resto como entradas
  Wire.endTransmission();
  
  Wire.beginTransmission(0x24);
  Wire.write(0x01); // IODIRB register
  Wire.write(0xC0); // registos 7 e 6 como saídas e resto como entradas
  Wire.endTransmission();


  //integrado da meio
  Wire.beginTransmission(0x26);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x03); // registos 0 e 1 como saídas e resto como entradas
  Wire.endTransmission();

  //resistencias de pull_up
  Wire.beginTransmission(0x26);
  Wire.write(0x0C); // address port A
  Wire.write(0x03);  // value to send
  Wire.endTransmission();

  Wire.beginTransmission(0x26);
  Wire.write(0x01); // IODIRB register
  Wire.write(0xC0); // registos 7 e 6 como saídas e resto como entradas
  Wire.endTransmission();

  //integrado da direita
  Wire.beginTransmission(0x27);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x03); // registos 0 e 1 como saídas e resto como entradas
  Wire.endTransmission();

  //resistencias de pull_up
  Wire.beginTransmission(0x27);
  Wire.write(0x0C); // address port A
  Wire.write(0x03);  // value to send
  Wire.endTransmission();
  
  Wire.beginTransmission(0x27);
  Wire.write(0x01); // IODIRB register
  Wire.write(0xC0); // registos 7 e 6 como saídas e resto como entradas
  Wire.endTransmission();

  //resistencias de pull_up
  Wire.beginTransmission(0x27);
  Wire.write(0x0D); // address port A
  Wire.write(0xC0);  // value to send
  Wire.endTransmission();
  

  //Configurar pinos do shift register
  pinMode(clock, OUTPUT); //clock
  pinMode(reset, OUTPUT); //reset

  reset_matrix();
}

void percorre_matrix(){ 
  digitalWrite(4, HIGH);
  int i,j,k;
  //reset_matrix();
  for(i=0;i<6;i++){
    for(j=0;j<6;j++){
      for(k=0;k<6;k++){
        if(leds[i][j][k].led != 0){
            set_led(i,j,k);
          }
      }
    }
  }
  digitalWrite(4, LOW);
}

void set_led(int x, int y, int z){
  int i;
  //noInterrupts();
  reset_matrix();
  //interrupts();
  for(i=5;i>=0;i--){
    if(i==z){
      reset_shift();
    }else{
      shift_bit();
      shift_bit();
    }
  }
  //noInterrupts();
  set_port_expander(x,y);
  //interrupts();
}

void set_port_expander(int i,int j){
  switch (i) {
    case 0:
      //registo B integrado da esquerda
      Wire.beginTransmission(0x24);
      Wire.write(0x13); // address port B
      break;
    case 1:
      //registo A integrado da esquerda
      Wire.beginTransmission(0x24);
      Wire.write(0x12); // address port A
      break;
    case 2:
      //registo B integrado do meio
      Wire.beginTransmission(0x26);
      Wire.write(0x13); // address port B
      break;
    case 3:
      //registo A integrado do meio
      Wire.beginTransmission(0x26);
      Wire.write(0x12); // address port A
      break;
    case 4:
      //registo B integrado da direita
      Wire.beginTransmission(0x27);
      Wire.write(0x13); // address port B
      break;
    case 5:
      //registo A integrado do direita
      Wire.beginTransmission(0x27);
      Wire.write(0x12); // address port A
      break;
  }

  if((i==0) || (i==2) || (i==4)) { //registo B
    switch (j) {
      case 0:
        Wire.write(0b11011111);
        break;
      case 1:
        Wire.write(0b11101111);
        break;
      case 2:
        Wire.write(0b11110111);
        break;
      case 3:
        Wire.write(0b11111011);
        break;
      case 4:
        Wire.write(0b11111101);
        break;
      case 5:
        Wire.write(0b11111110);
        break;
    }
  }else if((i==1) || (i==3) || (i==5)) { //registo A
    switch (j) {
      case 0:
        Wire.write(0b01111111);
        break;
      case 1:
        Wire.write(0b10111111);
        break;
      case 2:
        Wire.write(0b11011111);
        break;
      case 3:
        Wire.write(0b11101111);
        break;
      case 4:
        Wire.write(0b11110111);
        break;
      case 5:
        Wire.write(0b11111011);
        break;
    }
  }
  Wire.endTransmission();
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void ler_botao(){
  
  digitalWrite(3, HIGH);
  byte inA,inB,inC;
  //noInterrupts();
  Serial.begin(9600);
  Wire.beginTransmission(0x27);
  Wire.write(0x12); // address port A
  Wire.endTransmission();
  Wire.requestFrom(0x27, 2);

  inA = Wire.read();
  inB = Wire.read();
  inA = inA & 0x03;
  inB = inB & 0xC0;
  //Serial.print(inB);
  //Serial.print("\n");
  //botao cima 1 para right
  if(inA == 1){
      //Serial.print("cima\n");
      //digitalWrite(3, HIGH);
      changeDirection(UP); 
  }
  //botao baixo 2 left
  if(inA == 2){
      //Serial.print("baixo\n");
      //digitalWrite(3, LOW);
      changeDirection(DOWN);
  }
  if(inB == 128){
      //Serial.print("direita\n");
      //digitalWrite(3, HIGH);
      changeDirection(RIGHT); 
  }
  //botao baixo 2 left
  if(inB == 64){
      //Serial.print("esquerda\n");
      //digitalWrite(3, LOW);
      changeDirection(LEFT);
  }

  Wire.endTransmission();

  Serial.begin(9600);
  Wire.beginTransmission(0x26);
  Wire.write(0x12); // address port A
  Wire.endTransmission();
  Wire.requestFrom(0x26, 1);

  inC = Wire.read();
  inC = inC & 0x03;
  //Serial.print(inC);
  //Serial.print("\n");
  
  if(inC == 1){
      //Serial.print("frente\n");
      //digitalWrite(3, HIGH);
      changeDirection(FRONT); 
  }
  //botao baixo 2 left
  if(inC == 2){
      //Serial.print("atras\n");
      //digitalWrite(3, LOW);
      changeDirection(BACK);
  }

  digitalWrite(3, LOW);
  //interrupts();
  //percorre_matrix();
  
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void setup() {
  pinMode(3, OUTPUT); //clock
  pinMode(4, OUTPUT); //clock
  pinMode(5, OUTPUT); //clock
  Sched_Init();
  setup_cubo();
  initializeGame();
  Sched_AddT(percorre_matrix, 1, 40);
  Sched_AddT(ler_botao, 21, 150);
  Sched_AddT(runSnake, 80, 750);
}

void loop() {
  
  /*
  byte inA,inB;
  Serial.begin(9600);
  //Serial.print("I received: ");
  //Wire.beginTransmission(0x27);
  //Wire.write(0x13);

  Wire.requestFrom(0x27, 2);

  while(Wire.available())    // slave may send less than requested
  {
    inA = Wire.read();
    inB = Wire.read();
    inA = inA & 0x03;
    inB = inB & 0xC0;
    delay(5);
    //botao cima 1 para right
    if(inA == 1){
        //Serial.print("Carregou no botao de cima\n");
        snake_head.direction = UP; 
    }
    //botao baixo 2 left
    if(inA == 2){
        //Serial.print("Carregou no botao de baixo\n");
        snake_head.direction = DOWN;
    }
    
    
  }*/
  
 
}

