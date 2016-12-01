
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

//#include <paddleButtons.h>

//The paddle always starts at the middle
#define PADDLE_START_POSITION (screenWidth/2)
#define GREEN_LED BIT6 

unsigned char oldP1SwitchValue, oldP2SwitchValue;
char p1Score = '0';
char p2Score = '0';

AbRect leftPaddle = {abRectGetBounds, abRectCheck, {20,5}};
AbRect rightPaddle = {abRectGetBounds, abRectCheck, {20,5}};

AbRectOutline fieldOutline = {
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2 - 5, screenHeight/2 - 5}
};

//Positon the left paddle and select COLOR_WHITE
Layer leftPaddleLayer = {
  (AbShape *)&leftPaddle,
  {(screenWidth/2), (screenHeight/2)-70},
  {0,0}, {0,0},
  COLOR_WHITE,
  0
};

Layer rightPaddleLayer = {
  (AbShape *)&rightPaddle,
  {(screenWidth/2), (screenHeight/2)+70},
  {0,0}, {0,0},
  COLOR_WHITE,
  &leftPaddleLayer,
};

Layer fieldLayer = {
  (AbShape *)&fieldOutline,
  {screenWidth/2,screenHeight/2},
  {0,0}, {0,0},
  COLOR_BLACK,
  &rightPaddleLayer,
};

Layer ballLayer = {
  (AbShape *)&circle8,
  {(screenWidth/2), (screenHeight/2)},
  {0,0}, {0,0},
  COLOR_VIOLET,
  &fieldLayer,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer ml2 = { &leftPaddleLayer, {0,0}, 0};
MovLayer ml1 = { &rightPaddleLayer, {0,0}, 0};
MovLayer ml0 = { &ballLayer, {6,3}, 0};

//Function provided by Dr. Freudenthal
movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
    \Parameter ml2 checks the left paddle
    \Parameter ml3 checks the right paddle
*/ 
//Function provided by Dr. Freudenthal
//This function has been modified for collision detection
//Two additional parameters have been added for both paddles
void mlAdvance(MovLayer *ml,MovLayer *ml2, MovLayer *ml3, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++)
    {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */

    //check paddle collision
    if((ml->layer->posNext.axes[1] >= 134) && (ml->layer->posNext.axes[0] <= ml2->layer->posNext.axes[0] + 18 && ml->layer->posNext.axes[0] >= ml2->layer->posNext.axes[0] - 18))
      {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	ml->velocity.axes[0] += 1;
	newPos.axes[axis] += (2*velocity);
	drawString5x7(3,20, "Player 1:", COLOR_RED, COLOR_BLACK);
  drawChar5x7(100,20, p1Score, COLOR_WHITE, COLOR_BLACK);
  drawString5x7(3,130, "Player 2:", COLOR_BLUE, COLOR_BLACK);
    drawChar5x7(100,130, p2Score, COLOR_WHITE, COLOR_BLACK);
    buzzer_set_period(1500);
	int redrawscreen = 1;

      }

    else if ((ml->layer->posNext.axes[1] <= 21) && (ml2->layer->posNext.axes[0] <= ml3->layer->posNext.axes[0] + 20 && ml->layer->posNext.axes[0]>= ml3->layer->posNext.axes[0] -20))
      {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	ml->velocity.axes[0] += 1;
	newPos.axes[axis] += (2*velocity);
	drawString5x7(3,20, "Player 1:", COLOR_RED, COLOR_BLACK);
  drawChar5x7(100,20, p1Score, COLOR_WHITE, COLOR_BLACK);
  drawString5x7(3,130, "Player 2:", COLOR_BLUE, COLOR_BLACK);
    drawChar5x7(100,130, p2Score, COLOR_WHITE, COLOR_BLACK);
    buzzer_set_period(3000);
	int redrawScreen = 1;

      }
    //Check boundaries
    if ((ml->layer->posNext.axes[1] <= 20)|| (ml->layer->posNext.axes[1] >=100))
      {
 	p1Score++;
	ml->layer->posNext = newPos;
      }
    if((ml->layer->posNext.axes[1] == 130))
      {
	p2Score++;
	ml->layer->posNext = newPos;
      }
    ml->layer->posNext = newPos;
    drawString5x7(3,20, "Player 1:", COLOR_RED, COLOR_BLACK);
  drawChar5x7(100,20, p1Score, COLOR_WHITE, COLOR_BLACK);
  drawString5x7(3,130, "Player 2:", COLOR_BLUE, COLOR_BLACK);
    drawChar5x7(100,130, p2Score, COLOR_WHITE, COLOR_BLACK);
    }
  }
}

Region fieldFence;
Region leftFence;
Region rightFence;

int redrawScreen = 1;
u_int bgColor = COLOR_BLACK;

void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  buzzer_init();
  p2sw_init(15);

  shapeInit();

  layerInit(&leftPaddleLayer);
  layerDraw(&leftPaddleLayer);

  layerInit(&rightPaddleLayer);
  layerDraw(&rightPaddleLayer);

  layerInit(&ballLayer);
  layerDraw(&ballLayer);

  layerGetBounds(&fieldLayer, &fieldFence);
  //layerGetBounds(&leftPaddleLayer, &leftFence);
  //layerGetBounds(&rightPaddleLayer, &rightFence);
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  drawString5x7(3,20, "Player 1:", COLOR_RED, COLOR_BLACK);
  drawChar5x7(100,20, p1Score, COLOR_WHITE, COLOR_BLACK);
  drawString5x7(3,130, "Player 2:", COLOR_BLUE, COLOR_BLACK);
    drawChar5x7(100,130, p2Score, COLOR_WHITE, COLOR_BLACK);
  
    for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml0, &ballLayer);
    //movLayerDraw(&ml1, &leftPaddleLayer);
    //movLayerDraw(&ml2, &rightPaddleLayer);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &ml1, &ml2, &fieldFence);
    //mlAdvance(&ml0, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
