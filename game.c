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
GLfloat rot=0;
GLfloat rotDir=1;
GLfloat maxRot=30;

#define MAXMOVES  128
#define MOVETIME 200;
#define COOLDOWN 500;

engObj_s* player;
sprite_base* heartSpriteBase, *starSprBase;
char moves[MAXMOVES];

guiContext* hud;
int currentLevel = 0;

GLfloat fieldStart,fieldStop;

int state;
#define GSTATE_NON 0
#define GSTATE_SHOWLEVEL 1
#define GSTATE_GETINPUT 2
#define GSTATE_RUNLEVEL 3
#define GSTATE_ENDLEVEL 4

void gameShowLevel()
{

	//Progress to collectKeys
	state = GSTATE_GETINPUT;
}

void gameCollectKeys()
{
	//Progress to flyThrough
	engObj_s* rocket = eoObjCreate(ENGOBJ_MODEL);
	rocket->model = eoModelLoad( "/data/obj/rocket/", "rocket.obj");
	eoObjBake(rocket);
	eoObjAttach(player, rocket);

	engObj_s* parEmit = eoObjCreate(ENGOBJ_PAREMIT);
	particleEmitter_s* playerEmitter = eoPsysNewEmitter();
	playerEmitter->addictive=1;
	playerEmitter->numParticlesPerEmission = 2;
	playerEmitter->ticksBetweenEmissions = 1;
	playerEmitter->particleLifeMax = 500;
	playerEmitter->particleLifeVariance = 400;
	playerEmitter->shrink=0;
	playerEmitter->fade=0;
	playerEmitter->percentFlicker=60;
	playerEmitter->sizeMax=0.010;
	playerEmitter->sizeVariance=0.005;
	playerEmitter->rotateParticles=0;
	playerEmitter->colorVariance[0]=0.1;
	playerEmitter->colorVariance[1]=1;
	playerEmitter->colorVariance[2]=1;
	playerEmitter->colorVariance[3]=0;
	playerEmitter->color[0]=1;
	playerEmitter->color[1]=1;
	playerEmitter->color[2]=1;
	playerEmitter->emitSpeedMax=3;
	playerEmitter->emitSpeedVariance=0.5;
	playerEmitter->sprBase=heartSpriteBase;
	eoPsysBake(playerEmitter);
	parEmit->emitter = playerEmitter;
	parEmit->offsetPos.x = -2.5;
	parEmit->offsetPos.y = 0.5;
	eoObjBake(parEmit);

	eoObjAttach( player, parEmit );
	state = GSTATE_RUNLEVEL;

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


  camGet()->target.x =o->pos.x+ 10;
  camGet()->pos.x = o->pos.x+5;
  //o->rot.x +=3;
  o->vel.x = 0.5;

  o->rot.x+= 1*rotDir;
  if( o->rot.x > maxRot || o->rot.x < -maxRot )
  {
	  rotDir *= -1;
  }

  GLfloat rot=0;
  int rotDir=0;
  GLfloat maxRot=15;


  if( playerMove )
  {
	  if( playerMove == 'u' )
	  {
		  o->pos.y += 0.8;
	  } else if( playerMove == 'd' )
	  {
		  o->pos.y -= 0.8;
	  } else if( playerMove == 'f' )
	  {

	  }
	  playerMove = 0;
  }
}


void playerHit(engObj_s* a, engObj_s* b)
{
	//eoObjDel(player);
//	eoExec("echo You suck.");

}

void scFunc(inputEvent* e)
{
	eoGfxScreenshot(NULL);
}


void initGame()
{
	hud = eoGuiContextCreate();
	state = GSTATE_NON;
	eoRegisterStartFrameFunc(frameStart);

	heartSpriteBase = eoSpriteBaseLoad( Data("/data/gfx/", "heartparticle.spr"));
	starSprBase = eoSpriteBaseLoad( Data("/data/gfx/","defaultparticle.spr") );

	eoInpAddFunc("screenshot", "Save a screenshot.", scFunc, INPUT_FLAG_UP);
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
		player = o;
		o->colTeam = 2;
		o->thinkFunc = playerThink;
		o->colFunc = playerHit;
		o->rot.y = 180;

	}

  eoObjBake(o);
  eoObjAdd(o);
}

void initLevel(int l)
{
	char levelName[256];
	currentLevel = l;

	int i=0;
	memset( moves, 0, MAXMOVES );
	moves[i++] = 'd';
	moves[i++] = 'd';
	moves[i++] = 'u';
	moves[i++] = 'd';
	moves[i++] = 'u';
	moves[i++] = 'u';
	moves[i++] = 'd';

	cmIdx = 0;
	cmTl = MOVETIME;
	cmCd=( cmCd) *5 ;

	fieldStart=0;
	fieldStop=0;

	sprintf(levelName, "level%i.lvl", l);
	eoLoadScene( Data("/data/lvl/", levelName), objInit);


	for(int i=0; i < 1500; i++)
	{
		engObj_s* star = eoObjCreate(ENGOBJ_SPRITE);

		star->sprite=eoSpriteNew(starSprBase,1,1);
		star->pos.x=-1000+eoRandFloat(4000);
		star->pos.y=-1000+eoRandFloat(2000);
		star->pos.z= -2000 + eoRandFloat(4000);
		eoObjBake(star);
		eoObjAdd(star);
		star->sprite->scale.x = eoRandFloat(200)/1000.0f;
		star->sprite->scale.y = star->sprite->scale.x;


	}

  //eoObjAdd(playerObj);
  camGet()->pos.z = 30;
  camGet()->target.x = 0;
  camGet()->target.y = 0;
  camGet()->target.z = 0;

  state = GSTATE_SHOWLEVEL;
}
