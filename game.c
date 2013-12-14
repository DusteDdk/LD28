/*
 * game.c
 *
 *  Created on: 14/12/2013
 *      Author: chrisji
 */
#include "libeo/eng.h"
#include "game.h"



int cmIdx = 0;
int cmTl = 1000;
int cmCd=0;

#define MAXMOVES  128
#define MOVETIME 250;
#define COOLDOWN 550;



char moves[MAXMOVES];

guiContext* hud;
int currentLevel = 0;

GLfloat fieldStart,fieldStop;

void gameShowLevel()
{

}

void gameCollectKeys()
{
  eoPrint("gameCollectKeys");
}

char playerMove;

void gameRunLevel()
{
  if( moves[cmIdx] )
  {
	  if( cmCd > 0 )
	  {
		  cmCd -= eoTicks();

	  } else {
		  if( cmTl > 0 )
		  {
			  cmTl-= eoTicks();
			  //Execute move
			  playerMove=moves[cmIdx];

		  } else {
			  cmTl = MOVETIME;
			  cmIdx++;
			  cmCd = COOLDOWN;
		  }
	  }
  }
}

void gameEndLevel()
{
  eoPrint("gameEndLevel");
}


int state;
#define GSTATE_NON 0
#define GSTATE_SHOWLEVEL 1
#define GSTATE_GETINPUT 2
#define GSTATE_RUNLEVEL 3
#define GSTATE_ENDLEVEL 4

void frameStart()
{
  	switch(state)
	{
  	case GSTATE_SHOWLEVEL:
	  gameShowLevel();
	break;
	case GSTATE_GETINPUT:
	  gameCollectKeys();
	break;
	case GSTATE_RUNLEVEL:
	  gameRunLevel();
	break;
	case GSTATE_ENDLEVEL:
	  gameEndLevel();
	break;
	}
}

void playerThink( engObj_s* o)
{
  eoCamTargetSet(o->pos);
  camGet()->target.x += 20;
  camGet()->pos.x = o->pos.x+15;
  o->rot.x +=3;
  o->vel.x = 0.6;

  if( playerMove )
  {
	  if( playerMove == 'u' )
	  {
		  o->pos.y += 0.6;
	  } else if( playerMove == 'd' )
	  {
		  o->pos.y -= 0.6;
	  } else if( playerMove == 'f' )
	  {

	  }
	  playerMove = 0;
  }
}


void playerHit(engObj_s* a, engObj_s* b)
{
	eoExec("echo You suck.");
}

void initGame()
{
	hud = eoGuiContextCreate();
	guiWindow_s* win = eoGuiAddWindow(hud,10,10,150,50, "" , NULL);
	win->showTitle=FALSE;
	state = GSTATE_NON;
	eoRegisterStartFrameFunc(frameStart);
}

void startNewGame()
{
	  eoGuiContextSet(hud);
	  eoGuiShowCursor(0);
}

void objInit(engObj_s* o)
{

	if( strcmp(o->className, "obs" ) == 0 )
	{
		o->colTeam = 1;
		//Set z behind camera at first
		o->pos.z = 0;
	} else if( strcmp( o->className, "player" ) == 0 )
	{
		o->colTeam = 2;
		o->thinkFunc = playerThink;
		o->colFunc = playerHit;
		o->rot.y = 180;


		  engObj_s* parEmit = eoObjCreate(ENGOBJ_PAREMIT);
		  particleEmitter_s* playerEmitter = eoPsysNewEmitter();
		  playerEmitter->addictive=1;
		  playerEmitter->numParticlesPerEmission = 5;
		  playerEmitter->ticksBetweenEmissions = 1;
		  playerEmitter->particleLifeMax = 500;
		  playerEmitter->particleLifeVariance = 400;
		  playerEmitter->shrink=1;
		  playerEmitter->fade=0;
		  playerEmitter->percentFlicker=60;
		  playerEmitter->sizeMax=0.10;
		  playerEmitter->sizeVariance=0.07;
		  playerEmitter->rotateParticles=0;
		  playerEmitter->colorVariance[0]=0.1;
		  playerEmitter->colorVariance[1]=0.9;
		  playerEmitter->colorVariance[2]=0.9;
		  playerEmitter->colorVariance[3]=0;
		  playerEmitter->color[0]=1;
		  playerEmitter->color[1]=1;
		  playerEmitter->color[2]=1;
		  playerEmitter->emitSpeedMax=1;
		  playerEmitter->emitSpeedVariance=0.5;
		  eoPsysBake(playerEmitter);
		  parEmit->emitter = playerEmitter;
		  parEmit->offsetPos.x = -1.4;
		  parEmit->offsetPos.y = 0.0;
		  eoObjBake(parEmit);


		  eoObjAttach( o, parEmit );


	}

  eoObjBake(o);
  eoObjAdd(o);


}

void initLevel(int l)
{
	char levelName[256];
	currentLevel = l;

	int i=0;
	memset( moves, 0, MAXMOVES);
	moves[i++] = 'u';
	moves[i++] = 'd';
	moves[i++] = 'd';
	moves[i++] = 'u';
	moves[i++] = 'd';
	moves[i++] = 'u';
	moves[i++] = 'u';


	cmIdx = 0;
	cmTl = MOVETIME;
	cmCd=( cmCd) *5 ;

	fieldStart=0;
	fieldStop=0;

	sprintf(levelName, "level%i.lvl", l);
	eoLoadScene( Data("/", levelName), objInit);



  //eoObjAdd(playerObj);
  camGet()->pos.z = 40;
  camGet()->target.x = 0;
  camGet()->target.y = 0;
  camGet()->target.z = 0;

  state = GSTATE_RUNLEVEL;
}
