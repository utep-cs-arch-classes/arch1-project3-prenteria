#include <msp430.h>
#include <lcdutils.h>
#include <drawPaddle.h>
#include <lcddraw.h>
#include <paddleButtons.h>

short score;

void resetScore(void);
void updateScore(void);

unsigned char previousP1Score, previousP2Score;

#define P1ScoreMask SW1
#define P2ScoeMask (Up, Down, Left, Right)

void initSwitchInterrupts()
{



}

