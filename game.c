/*
 * game.c
 *
 *  Created on: 14/12/2013
 *      Author: chrisji
 */
#include "libeo/eng.h"
#include "game.h"

listItem* segments;
engObj_s* playerObj;
vboModel* playerMdl;


guiContext* hud;

int currentLevel = 0;

void gameShowLevel()
{
  eoPrint("ShowLevel");
}

void gameCollectKeys()
{
  eoPrint("gameCollectKeys");
}

void gameRunLevel()
{
  eoPrint("gameRunLevel");
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
  camGet()->target.x += 30;
  camGet()->pos.x = o->pos.x+20;
  o->rot.x +=1;
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
	segments = initList();
	playerMdl = eoModelLoad( "/data/obj/", "goat.obj");



}

void startNewGame()
{
	  eoGuiContextSet(hud);
	  eoGuiShowCursor(0);

}

void objInit(engObj_s* o)
{
	listAddData(segments, (void*)o);
	o->colTeam = 1;
}

void initLevel(int l)
{
	char levelName[256];
	currentLevel = l;
	listItem* it=segments;



	if(listSize(segments))
	{
		while( (it=it->next) )
		{
			engObj_s* o = (engObj_s*)it->data;
			eoObjDel(o);
		}
	}

	sprintf(levelName, "lvl%i.txt", l);
	//eoLoadScene( Data("/data/levels/", levelName), objInit);

  playerObj = eoObjCreate(ENGOBJ_MODEL);
  playerObj->model = playerMdl;
  playerObj->colTeam = 2;
  playerObj->colFunc = playerHit;

  playerObj->pos.x = 0.0;
  playerObj->pos.y = 0.0;
  playerObj->pos.z = 0.0;
  playerObj->rot.y = 180;

  engObj_s* parEmit = eoObjCreate(ENGOBJ_PAREMIT);
  particleEmitter_s* playerEmitter = eoPsysNewEmitter();
  playerEmitter->addictive=1;
  playerEmitter->numParticlesPerEmission = 3;
  playerEmitter->ticksBetweenEmissions = 1;
  playerEmitter->particleLifeMax = 500;
  playerEmitter->particleLifeVariance = 20;
  playerEmitter->shrink=0;
  playerEmitter->fade=0;
  playerEmitter->percentFlicker=70;
  playerEmitter->sizeMax=0.05;
  playerEmitter->sizeVariance=0.03;
  playerEmitter->rotateParticles=0;
  playerEmitter->colorVariance[0]=0.4;
  playerEmitter->colorVariance[1]=0.0;
  playerEmitter->colorVariance[2]=0.3;
  playerEmitter->colorVariance[3]=0.0;
  playerEmitter->color[0]=0.9;
  playerEmitter->color[1]=0.0;
  playerEmitter->color[2]=0.4;
  playerEmitter->emitSpeedMax=1;
  playerEmitter->emitSpeedVariance=0.1;
  eoPsysBake(playerEmitter);
  parEmit->emitter = playerEmitter;
  parEmit->offsetPos.x = -1.4;
  parEmit->offsetPos.y = 0.0;
  eoObjBake(parEmit);

  playerObj->thinkFunc = playerThink;

  eoObjAttach( playerObj, parEmit );


  eoObjBake(playerObj);
  eoObjAdd(playerObj);
  camGet()->pos.z = 50;
  camGet()->target.x = 0;
  camGet()->target.y = 0;
  camGet()->target.z = 0;
}
