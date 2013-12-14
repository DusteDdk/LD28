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

int state;
#define GSTATE_NON 0
#define GSTATE_SHOWLEVEL 1
#define GSTATE_GETINPUT 2
#define GSTATE_RUNLEVEL 3
#define GSTATE_ENDLEVEL 4

void frameStart()
{
/*	switch(state)
	{
	case GSTATE_SHOWLEVEL:
		gameShowLevel();
	break;
	case GSTATE_GETINPUT:
		gameGetInput();
	break;
	case GSTATE_RUNLEVEL:
		gameRunLevel();
	break;
	case GSTATE_ENDLEVEL:
		gameEndLevel();
	break;
	}*/
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

	playerObj = eoObjCreate(ENGOBJ_MODEL);
	playerObj->model = playerMdl;
	playerObj->colTeam = 2;
	playerObj->colFunc = playerHit;
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

	eoLoadScene( Data("/data/levels/", levelName), objInit);
}
