#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <time.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define COLOR SSD1306_WHITE
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PIN_PLAYER_1 1
#define PIN_PLAYER_2 2

const int BALL_R = 2;
const int PAD_W = 4;
const int PAD_H = 15;
const int PAD_OFFSET = 3;
const float MAX_SPEED_X = 6;
const float MAX_SPEED_Y = 5;

float i1, i2;
int s1 = 0, s2 = 0;

struct Paddle
{
  float x;
  float y;
  bool left;
};

Paddle p1;
Paddle p2;
float bX;
float bY;
float vX;
float vY;

float getPaddleTop(Paddle p)
{
  return p.y;
}

float getPaddleBottom(Paddle p)
{
  return getPaddleTop(p) + PAD_H;
}

float getPaddleLeft(Paddle p)
{
  return p.x;
}

float getPaddleRight(Paddle p)
{
  return getPaddleLeft(p) + PAD_W;
}

float randF()
{
    return (float)rand() / (float)RAND_MAX ;
}

float randRange(float a, float b)
{
  float d = b - a;
  float r = randF() * d;
  return r + a;
}

void setupLCD()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.cp437(true);
  display.clearDisplay();
  display.display();
}

void setupGame()
{
  
  bX = SCREEN_WIDTH / 2;
  bY = SCREEN_HEIGHT / 2;
  vX = randRange(-MAX_SPEED_X, MAX_SPEED_X);
  vY = randRange(-MAX_SPEED_Y, MAX_SPEED_Y);

  p1.left = true;
  p2.left = false;

  p1.x = PAD_OFFSET;
  p2.x = SCREEN_WIDTH - PAD_W - PAD_OFFSET;
}

float translateInput(int pin)
{
  float raw = analogRead(pin);
  if (raw < 0) raw = 0;
  if (raw > 100) raw = 100;
  raw = raw / 100.0f;
  return raw;
}

float inputToPaddleY(float i, float old)
{
  const float alpha = 0.2f;
  float maxVal = SCREEN_HEIGHT - PAD_H;
  return alpha * (i * maxVal) + (1.0f - alpha) * old;
}

void setup() {
  srand(time(0));
  Serial.begin(9600);
  pinMode(PIN_PLAYER_1, INPUT);
  pinMode(PIN_PLAYER_2, INPUT);
  setupGame();
  setupLCD();
}

void loop() {
  readInput();
  updateGame();
  render();
  delay(10);
}

void readInput()
{
  i1 = translateInput(PIN_PLAYER_1);
  i2 = translateInput(PIN_PLAYER_2);
}

void updatevY(float pY)
{
  float c = pY + PAD_H * 0.5f;
  float a = (bY - c) / (PAD_H * 0.5f);
  vY = MAX_SPEED_Y * a;
}

void updateGame()
{
  // Move Ball
  bX += vX;
  bY += vY;

  // Move Paddles
  p1.y = inputToPaddleY(i1, p1.y);
  p2.y = inputToPaddleY(i2, p2.y);
  
  // Wall Collision Detection 
  /*if (bX < 0)
  {
    bX = 0;
    vX *= -1;
  }
  else if (bX > SCREEN_WIDTH)
  {
    bX = SCREEN_WIDTH;
    vX *= -1;
  }*/

  if (bY < 0)
  {
    bY = 0;
    vY *= -1;
  }
  else if (bY > SCREEN_HEIGHT)
  {
    bY = SCREEN_HEIGHT;
    vY *= -1;
  }
  
  // Player Collision Detection
  if (
    bX >= p1.x && bX <= p1.x + PAD_W &&
    bY >= p1.y && bY <= p1.y + PAD_H)
  {
    bX = p1.x + PAD_W;
    vX = fabs(vX);
    updatevY(p1.y);
  }

  if (
    bX >= p2.x && bX <= p2.x + PAD_W &&
    bY >= p2.y && bY <= p2.y + PAD_W)
  {
    bX = p2.x;
    vX = -fabs(vX);
    updatevY(p2.y);
  }
  
  // Speed Clamp
  if (vX < -MAX_SPEED_X) vX = -MAX_SPEED_X;
  else if (vX > MAX_SPEED_X) vX = MAX_SPEED_X;

  if (vY < -MAX_SPEED_Y) vY = -MAX_SPEED_Y;
  else if (vY > MAX_SPEED_Y) vY = MAX_SPEED_Y;

  // Scoring
  if (bX < 0)
  {
    s2 ++;
    renderScores();
    setupGame();
  }
  else if (bX > SCREEN_WIDTH)
  {
    s1 ++;
    renderScores();
    setupGame();
  }
}

void renderPaddle(Paddle p)
{
  uint64_t x0 = p.x;
  uint64_t y0 = p.y;
  uint64_t w = PAD_W;
  uint64_t h = PAD_H;
  display.fillRect(x0, y0, w, h, COLOR);
}

void renderBall()
{
  display.fillCircle(
    bX,
    bY,
    BALL_R,
    COLOR);
}

void render()
{
  display.clearDisplay();
  /*display.setTextSize(2.f);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  
  display.print(F("P1: "));
  display.println(i1);

  display.print(F("P2: "));
  display.println(i2);
*/
  renderPaddle(p1);
  renderPaddle(p2);
  renderBall();
  display.display();
}

void renderScores()
{
  display.setTextSize(3.f);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print(F(" "));
  display.print(s1);
  display.print(F(":"));
  display.println(s2);
  
  display.display();
  delay(3000);
}
