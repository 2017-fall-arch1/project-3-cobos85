/** \file pong.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "noise.h"

#define GREEN_LED BIT6

int buzzer = 0;


AbRect rect10 = {abRectGetBounds, abRectCheck, {10,10}}; /* 10x10 rectangle */
AbRect pad = {abRectGetBounds, abRectCheck, {2,20}};     /* 2x20 paddle */

AbRectOutline fieldOutline = {	                         /* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 10, screenHeight/2 - 10}
};


/* ball layer */
Layer layer3 = {		                         /* Layer with a white circle */
  (AbShape *)&circle8,
  {(screenWidth/2)+10, (screenHeight/2)+5},              /* bit below & right of center */
  {0,0}, {0,0},				                 /* last & next pos */
  COLOR_WHITE,
  0
};

Layer fieldLayer = {		                         /* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},                       /* center */
  {0,0}, {0,0},				                 /* last & next pos */
  COLOR_WHITE,
  &layer3
};

/* paddle 1 layer */
Layer layer1 = {		                         /* Layer with a white rectangle */
  (AbShape *)&pad,
  {((screenWidth/4)-20), screenHeight/2},                /* left edge center */
  {0,0}, {0,0},				                 /* last & next pos */
  COLOR_WHITE,
  &fieldLayer,
};

/* paddle 2 layer */
Layer layer0 = {		                         /* Layer with a white rectangle */
  (AbShape *)&pad,
  {(screenWidth/2)+52, screenHeight/2},                  /* right edge center */
  {0,0}, {0,0},				                 /* last & next pos */
  COLOR_WHITE,
  &layer1,
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

/* initial value of {0,0} will be overwritten */
MovLayer ml3 = { &layer3, {1,1}, 0 };            /* updates ball movements */
MovLayer ml1 = { &layer1, {0,1}, &ml3 };         /* updates paddle 1 horizontal */
MovLayer ml0 = { &layer0, {0,1}, &ml1 };         /* updates paddle 2 horizontal */

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			            /* disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			            /* disable interrupts (GIE on) */


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


/* 2 strings to store each score */
char p1[10] = "P1";
char p2[10] = "P2";
char score1 = 0;
char score2 = 0;

/* draws scores on screen */
void drawScore(char *score, char size)
{
  drawString5x7(size, 14, score, COLOR_BLACK, COLOR_WHITE);
}


/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */

    /* to update score when hitting fence */
    if(shapeBoundary.topLeft.axes[0] < fence->topLeft.axes[0])
      {
	score1++;
	p1[2] = '0' + score1;
	drawScore(p1,1);
      }
    else if(shapeBoundary.botRight.axes[0] > fence->botRight.axes[0])
      {
	score2++;
	p2[2] = '0' + score2;
	drawScore(p2,90);
      }
    
    ml->layer->posNext = newPos;
  } /**< for ml */
}

/* to detect collision against paddle 1 */
void p1collision()
{
  if((layer3.pos.axes[0] - (circle8.radius) <= (layer1.pos.axes[0] + 1)) && (layer3.pos.axes[1] >= layer1.pos.axes[1] - 12) && (layer3.pos.axes[1] <= layer1.pos.axes[1] + 12))
    {
      layer3.posNext.axes[0] += 3;
      ml3.velocity.axes[0] = -ml3.velocity.axes[0];
      set_pd(2000);
      buzzer = 1;
    }
}

/* to detect collision against paddle 2 */
void p2collision()
{
  if((layer3.pos.axes[0] + (circle8.radius) >= (layer0.pos.axes[0] - 3)) && (layer3.pos.axes[1] >= layer0.pos.axes[1] - 12) && (layer3.pos.axes[1] <= layer0.pos.axes[1] + 12))
  {
    layer3.posNext.axes[0] -= 3;
    ml3.velocity.axes[0] = -ml3.velocity.axes[0];
    set_pd(2000);
    buzzer = 1;
  }
}

/* switch update for player 1 paddle */
void p1sw(u_int sw)
{
  if(!(sw & (1<<0)))
    {
      ml1.velocity.axes[1] = 5;
    }
  else if(!(sw & (1<<1)))
    {
      ml1.velocity.axes[1] = -5;
    }
  else
    {
      ml1.velocity.axes[1] = 0;
    }
}

/* switch update for player 2 paddle */
void p2sw(u_int sw)
{
  if(!(sw & (1<<2)))
    {
      ml0.velocity.axes[1] = 5;
    }
  else if(!(sw & (1<<3)))
    {
      ml0.velocity.axes[1] = -5;
    }
  else
    {
      ml0.velocity.axes[1] = 0;
    }
}

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;            /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  init_noise();               /* initialize buzzer for sound */

  layerInit(&layer0);
  layerDraw(&layer0);


  layerGetBounds(&fieldLayer, &fieldFence);
  drawScore(p1, 1);
  drawScore(p2, 90);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  u_int swt;

  for(;;) {

    if(score1 == 10 || score2 == 10)
      {
	continue;
      }

    swt = p2sw_read();
    
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }

    if(buzzer)
      {
	set_pd(0);
	buzzer = 0;
      }
    
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;

    p1sw(swt);               /* update paddle 1 */
    p2sw(swt);               /* update paddle 2 */

    p1collision();           /* checking for paddle 1 collison with ball */
    p2collision();           /* checking for paddle 2 collison with ball */
    
    movLayerDraw(&ml0, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
