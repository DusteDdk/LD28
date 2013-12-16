/*
 * game.c
 *
 *  Created on: 14/12/2013
 *      Author: chrisji
 */
#include "libeo/eng.h"
#include "game.h"

vboModel* spaceMdl;
engObj_s* spaceObj;

vboModel* rocket;
sprite_base* laser;

GLfloat rotDir=1;
GLfloat maxRot=30;

#define MAXMOVES  128
#define MOVETIME 200;
#define COOLDOWN 500;

engObj_s* player;
sprite_base* heartSpriteBase;
char moves[MAXMOVES];

guiContext* hud;
int currentLevel = 0;

vboModel* tireMdl;
vboModel* astAMdl;
vboModel* astBMdl;
vboModel* bossMdl;

bool femGoat=0;

particleEmitter_s* nmeDieEmit;
particleEmitter_s* playerDieEmit;
particleEmitter_s* rocketExplEmit;
particleEmitter_s* laserExplEmit;

sound_s* sndLaserFire,*sndRocketFire,*sndNmeDie,*sndNmeShoot,*sndPlayerDie,*sndPlayerWin;

int bossVis=0;

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

typedef struct
{
    int health;
    GLfloat rxsp;
    GLfloat rysp;
    int fireCoolDown;
    int moveCoolDown;
    int fireRound;
} nmeProp;

void outOfScreenfunc(engObj_s* o);

void nmeThink(engObj_s* o)
{
  if( o->deleteMe) return;
  nmeProp* dat = (nmeProp*)o->gameData;

  if( o->pos.x < player->pos.x-10 )
  {
    o->pos =player->pos;
  }

  o->rot.x += dat->rxsp;
  o->rot.y += dat->rysp;
  if(o->model==bossMdl)
  {
    bossVis=1;
    //move towards player?
    vec3 d = eoVec3FromPoints(o->pos, player->pos);
    if( eoVec3Len( d ) >  40 )
    {
      o->vel = eoVec3Scale(d, 0.0105);
    } else {
      o->vel.x=0;
    }

    //Do something randomish about movements
    dat->moveCoolDown -= eoTicks();
    if( dat->moveCoolDown < 1 )
    {
      o->vel.y = -0.3+eoRandFloat(0.6);
      dat->moveCoolDown=rand()%1000;
    }

    if( o->pos.y > 10 )
    {
      o->pos.y = 10;
    } else if( o->pos.y < -10)
    {
      o->pos.y = -10;
    }

    dat->fireCoolDown -= eoTicks();
    if(dat->fireCoolDown<1)
    {
      dat->fireCoolDown = 500 + rand()%700;
      dat->fireRound+=20;

      int i;
      engObj_s* b;
      for(i=0; i < 9; i++ )
      {
        b = eoObjCreate(ENGOBJ_SPRITE);
        b->sprite = eoSpriteNew( laser,1,1 );
        b->vel.z=0;

        b->vel.y = sin( (float)((i*40)+dat->fireRound)*0.0174532925 )*0.35;
        b->vel.x = cos( (float)((i*40)+dat->fireRound)*0.0174532925 )*0.35;

        b->pos = o->pos;

        b->colTeam = o->colTeam;
        b->thinkFunc=outOfScreenfunc;
        eoObjBake(b);
        b->gameData = malloc(sizeof(nmeProp));
        ((nmeProp*)b->gameData)->health=50;
        eoSpriteScale(b->sprite, 0.07,0.07);
        eoObjAdd(b);
      }
      eoSamplePlay(sndNmeShoot, 128);


    }
  }
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

int lastTime = 0;

engObj_s* addGenericNME()
{
  engObj_s* o = eoObjCreate(ENGOBJ_MODEL);
  o->colTeam = 1;
  //Set z behind camera at first
  o->pos.z = 0;
  o->gameData = malloc( sizeof(nmeProp));
  ((nmeProp*)(o->gameData))->health = 100;


  memset(((nmeProp*)(o->gameData)), 0, sizeof(nmeProp));
  ((nmeProp*)(o->gameData))->rxsp = eoRandFloat(1);
  ((nmeProp*)(o->gameData))->rysp = -1.0+eoRandFloat(2);
  o->pos.y = -11.0 + eoRandFloat(22.0);
  eoPrint("ypos: %f", o->pos.y);
  o->pos.x = eoCamPosGet().x + 80;
  o->pos.z = 0;
  o->thinkFunc = nmeThink;
  return(o);
}

void spawnTire()
{
  engObj_s* o = addGenericNME();
  ((nmeProp*)(o->gameData))->health = 100;
  o->model = tireMdl;
  eoObjBake(o);
  eoObjAdd(o);
}

void spawnAstA()
{
  engObj_s* o = addGenericNME();
  ((nmeProp*)(o->gameData))->health = 200;
  o->model = astAMdl;
  eoObjBake(o);
  eoObjAdd(o);

}

void spawnAstB()
{
  engObj_s* o = addGenericNME();
  ((nmeProp*)(o->gameData))->health = 400;
  o->model = astBMdl;
  eoObjBake(o);
  eoObjAdd(o);


}

void spawnBoss()
{
  engObj_s* o = addGenericNME();
  ((nmeProp*)(o->gameData))->health = 2000;
  o->model = bossMdl;
  eoObjBake(o);
  eoObjAdd(o);
}

void fThink( engObj_s* o)
{
  o->rot.x +=1;
}

void gameRunLevel()
{


//spawn something?
  if(bossVis) return;

  if( player->pos.x > 1200 )
  {
    if(!femGoat)
    {
      femGoat=1;
      engObj_s* f = eoObjCreate(ENGOBJ_MODEL);
      f->colTeam=3;
      f->model=eoModelLoad( "/data/obj/","fgoat.obj");
      f->pos.z =0;
      f->pos.y =0;
      f->pos.x = player->pos.x + 40;
      f->thinkFunc = fThink;
      f->gameData = malloc(sizeof(nmeProp));
      ((nmeProp*)f->gameData)->health = 50000;
      eoObjBake(f);
      eoObjAdd(f);

      eoPsysEmit(playerDieEmit, f->pos);
    }
  } else {

    lastTime -= eoTicks();
    if( lastTime < 1)
    {
      lastTime = rand()%1000;

      int i =rand()%1000;
      i/=10;
      if( i < 25 )
      {
        spawnTire();
      } else if (i < 50)
      {
        spawnAstA();
      } else if( i < 97)
      {
        spawnAstB();
      } else if( i < 101)
      {
        spawnBoss();
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

bool playerMoveUp, playerMoveDown,playerMoveLeft, playerMoveRight, playerShoot;
void inpMoveUp(inputEvent* e)
{
  playerMoveUp=TRUE;
}

void inpMoveDn(inputEvent* e)
{
  playerMoveDown=TRUE;
}

void inpMoveRight(inputEvent* e)
{
  playerMoveRight=TRUE;
}

void inpMoveLeft(inputEvent* e)
{
  playerMoveLeft=TRUE;
}

void inpShoot(inputEvent* e)
{
  playerShoot=TRUE;
}

void rocketCol( engObj_s* a, engObj_s* b)
{
  vec3 p = a->pos;
  eoPsysEmit(rocketExplEmit, p);
  eoObjDel(a);
  if( b->deleteMe )
    return;
  ((nmeProp*)(b->gameData))->health-=80;

  if(   ((nmeProp*)(b->gameData))->health < 1 )
  {
    eoSamplePlay(sndNmeDie,128);
    eoPsysEmit(nmeDieEmit, b->pos);
    free(b->gameData);
    b->gameData=0;
    eoObjDel(b);
  }
}

void laserCol( engObj_s* a, engObj_s* b)
{
  vec3 p = a->pos;
  p.x-=2;
  if( b->deleteMe )
    return;

  eoPsysEmit(laserExplEmit, p);
  eoObjDel(a);
  ((nmeProp*)(b->gameData))->health-=40;
  if(   ((nmeProp*)(b->gameData))->health < 1 )
  {
    eoSamplePlay(sndNmeDie,128);
    eoPsysEmit(nmeDieEmit, b->pos);
    free(b->gameData);
    b->gameData=0;
    eoObjDel(b);
  }

}


void outOfScreenfunc( engObj_s* o )
{
  if(o->pos.x > eoCamPosGet().x+80 || o->pos.x < eoCamPosGet().x-80 || o->pos.y > 30|| o->pos.y < -30 )
  {
    if( o->gameData )
    {
      free(o->gameData);
      o->gameData=NULL;
    }
    eoObjDel(o);
  }
}

int rcd=0;
int lcd=0;
void playerThink( engObj_s* o)
{

  if( !bossVis && !femGoat )
  {
    o->vel.x = 0.3;
  } else {
    bossVis=0;
    o->vel.x = 0.01;
  }
  camGet()->target.x =o->pos.x+ 10;
  camGet()->pos.x = o->pos.x+5;
  //o->rot.x +=3;
  spaceObj->pos = eoCamPosGet();
  spaceObj->rot.y +=0.001;
  spaceObj->rot.x +=0.0012;
  o->rot.x+= 1*rotDir;
  if( o->rot.x > maxRot || o->rot.x < -maxRot )
  {
	  rotDir *= -1;
  }


  if( playerMoveUp )
  {
    playerMoveUp=FALSE;
    o->pos.y += 0.4;
  } else if(playerMoveDown)
  {
    playerMoveDown=FALSE;
    o->pos.y -= 0.4;
  }

  if( playerMoveRight)
  {
    playerMoveRight=FALSE;
    o->pos.x +=0.4;
  } else if( playerMoveLeft)
  {
    playerMoveLeft=FALSE;
    o->pos.x -=0.2;
  }

  if(rcd > 0 )
  {
    rcd -= eoTicks();
  }
  if(lcd > 0)
  {
    lcd -= eoTicks();
  }
  if(playerShoot)
  {
    playerShoot=FALSE;

    if(rcd < 1)
    {
      engObj_s* r = eoObjCreate(ENGOBJ_MODEL);
      r->model=rocket;
      r->colTeam=o->colTeam;
      r->colFunc=rocketCol;
      r->vel.x = o->vel.x + 0.6;
      r->pos = o->pos;
      r->pos.x += 3;
      engObj_s* parEmit = eoObjCreate(ENGOBJ_PAREMIT);
      particleEmitter_s* emitter = eoPsysNewEmitter();
      emitter->addictive=1;
      emitter->numParticlesPerEmission = 20;
      emitter->ticksBetweenEmissions = 1;
      emitter->particleLifeMax = 100;
      emitter->particleLifeVariance = 50;
      emitter->shrink=1;
      emitter->fade=0;
      emitter->percentFlicker=60;
      emitter->sizeMax=0.04;
      emitter->sizeVariance=0.02;
      emitter->rotateParticles=0;
      emitter->colorVariance[0]=0.2;
      emitter->colorVariance[1]=0.5;
      emitter->colorVariance[2]=0.5;
      emitter->colorVariance[3]=0;
      emitter->color[0]=1;
      emitter->color[1]=0.5;
      emitter->color[2]=0.5;
      emitter->emitSpeedMax=1;
      emitter->emitSpeedVariance=0.5;

      eoPsysBake(emitter);
      parEmit->emitter = emitter;
      parEmit->offsetPos.x = -2.5;
      parEmit->offsetPos.y = 0.0;
      eoObjBake(parEmit);

      eoObjAttach( r, parEmit );

      eoObjBake(r);
      r->thinkFunc = outOfScreenfunc;
      eoObjAdd(r);
      rcd = 500;
      eoSamplePlay(sndRocketFire,128);
    }

    if(lcd<1)
    {
      eoSamplePlay(sndLaserFire, 128);
      engObj_s* l = eoObjCreate(ENGOBJ_SPRITE);
      l->sprite = eoSpriteNew( laser,1,1 );
      l->vel.x = o->vel.x + 1.5;
      l->pos = o->pos;
      l->pos.x += 3;
      l->colTeam = o->colTeam;
      l->colFunc = laserCol;
      l->thinkFunc=outOfScreenfunc;
      eoObjBake(l);
      eoSpriteScale(l->sprite, 0.04,0.04);
      eoObjAdd(l);
      l->_hitBox.x=0.6;
      l->_hitBox.y=0.6;
      l->_hitBox.z=0.6;
      lcd = 60;
    }

  }



}


void playerHit(engObj_s* a, engObj_s* b)
{

  if( b->colTeam == 3 )
  {
    guiWindow_s* win = eoGuiAddWindow(hud, eoSetting()->res.x/2-100, eoSetting()->res.y/2-50, 200,100,"Thanks.", NULL);
    eoGuiAddLabel(win, 0,0, "You won!");
    eoGuiAddLabel(win, 0,16, "Press ESC, you only got one life.");
    eoGuiAddLabel(win, 0,32, "Don't waste it...");
    eoPauseSet(TRUE);

    eoSamplePlay(sndPlayerWin, 128);
  } else {
    eoSamplePlay(sndPlayerDie, 128);
    eoPsysEmit(playerDieEmit, a->pos);
    eoPsysEmit(nmeDieEmit, b->pos);
    free(b->gameData);
    b->gameData=NULL;
    eoObjDel(a);
    eoObjDel(b);

    guiWindow_s* win = eoGuiAddWindow(hud, eoSetting()->res.x/2-100, eoSetting()->res.y/2-50, 200,100,"Thanks.", NULL);
    eoGuiAddLabel(win, 0,0, "Game Over Dude.");
    eoGuiAddLabel(win, 0,16, "Press ESC, you only got one life.");
    eoGuiAddLabel(win, 0,32, "Don't waste it...");
  }
}

void scFunc(inputEvent* e)
{
	eoGfxScreenshot(NULL);
}


void initGame()
{
  spaceMdl = eoModelLoad( "/data/obj/","space.obj");
  rocket = eoModelLoad( "/data/obj/","rocket.obj");

  tireMdl= eoModelLoad("/data/obj/Tire/","tire.obj");
  astAMdl= eoModelLoad("/data/obj/","asteroid1.obj");
  astBMdl= eoModelLoad("/data/obj/","asteroid2.obj");
  bossMdl= eoModelLoad("/data/obj/Crate/","crate.obj");

	hud = eoGuiContextCreate();
	state = GSTATE_NON;
	eoRegisterStartFrameFunc(frameStart);

	heartSpriteBase = eoSpriteBaseLoad( Data("/data/gfx/", "heartparticle.spr"));

	eoInpAddFunc("screenshot", "Save a screenshot.", scFunc, INPUT_FLAG_UP);

	eoInpAddFunc("moveUp", "duh", inpMoveUp, INPUT_FLAG_HOLD);
	eoInpAddFunc("moveDown", "guess",inpMoveDn, INPUT_FLAG_HOLD);
	eoInpAddFunc("moveRight", "well", inpMoveRight, INPUT_FLAG_HOLD);
	eoInpAddFunc("moveLeft", "obvious", inpMoveLeft, INPUT_FLAG_HOLD);
	eoInpAddFunc("fire", "shootie boom boom", inpShoot, INPUT_FLAG_HOLD);

	eoExec( "exec /data/autoexec.cfg" );



	nmeDieEmit =  eoPsysNewEmitter();
	nmeDieEmit->addictive=1;
	nmeDieEmit->numParticlesPerEmission = 30;
	nmeDieEmit->ticksBetweenEmissions = 0;
	nmeDieEmit->particleLifeMax = 500;
	nmeDieEmit->particleLifeVariance = 100;
	nmeDieEmit->shrink=1;
	nmeDieEmit->fade=1;
	nmeDieEmit->percentFlicker=70;
	nmeDieEmit->sizeMax=0.040;
	nmeDieEmit->sizeVariance=0.025;
	nmeDieEmit->rotateParticles=0;
	nmeDieEmit->colorVariance[0]=0.1;
	nmeDieEmit->colorVariance[1]=0.5;
	nmeDieEmit->colorVariance[2]=0.5;
	nmeDieEmit->colorVariance[3]=0;
	nmeDieEmit->color[0]=1;
	nmeDieEmit->color[1]=1;
	nmeDieEmit->color[2]=1;
	nmeDieEmit->emitSpeedMax=6;
	nmeDieEmit->emitSpeedVariance=6;
	nmeDieEmit->sprBase=heartSpriteBase;
  eoPsysBake(nmeDieEmit);

  playerDieEmit =  eoPsysNewEmitter();
  playerDieEmit->addictive=1;
  playerDieEmit->numParticlesPerEmission = 25;
  playerDieEmit->ticksBetweenEmissions = 0;
  playerDieEmit->particleLifeMax = 2500;
  playerDieEmit->particleLifeVariance = 1000;
  playerDieEmit->shrink=1;
  playerDieEmit->fade=1;
  playerDieEmit->percentFlicker=50;
  playerDieEmit->sizeMax=0.10;
  playerDieEmit->sizeVariance=0.025;
  playerDieEmit->rotateParticles=0;
  playerDieEmit->colorVariance[0]=0.0;
  playerDieEmit->colorVariance[1]=0.25;
  playerDieEmit->colorVariance[2]=0.25;
  playerDieEmit->colorVariance[3]=0;
  playerDieEmit->color[0]=1;
  playerDieEmit->color[1]=0.6;
  playerDieEmit->color[2]=0.6;
  playerDieEmit->emitSpeedMax=7;
  playerDieEmit->emitSpeedVariance=2;
  playerDieEmit->sprBase=heartSpriteBase;
  eoPsysBake(playerDieEmit);

	laserExplEmit=  eoPsysNewEmitter();
	laserExplEmit->addictive=1;
	laserExplEmit->numParticlesPerEmission = 5;
	laserExplEmit->ticksBetweenEmissions = 0;
	laserExplEmit->particleLifeMax = 250;
	laserExplEmit->particleLifeVariance = 0;
	laserExplEmit->shrink=0;
	laserExplEmit->fade=1;
	laserExplEmit->percentFlicker=90;
	laserExplEmit->sizeMax=0.030;
	laserExplEmit->sizeVariance=0.0;
	laserExplEmit->rotateParticles=0;
	laserExplEmit->colorVariance[0]=0.5;
	laserExplEmit->colorVariance[1]=0.0;
	laserExplEmit->colorVariance[2]=0.5;
	laserExplEmit->colorVariance[3]=0;
	laserExplEmit->color[0]=1;
	laserExplEmit->color[1]=1;
	laserExplEmit->color[2]=1;
	laserExplEmit->emitSpeedMax=4;
	laserExplEmit->emitSpeedVariance=3;
  eoPsysBake(laserExplEmit);


  rocketExplEmit=  eoPsysNewEmitter();
  rocketExplEmit->addictive=1;
  rocketExplEmit->numParticlesPerEmission = 10;
  rocketExplEmit->ticksBetweenEmissions = 0;
  rocketExplEmit->particleLifeMax = 350;
  rocketExplEmit->particleLifeVariance = 0;
  rocketExplEmit->shrink=0;
  rocketExplEmit->fade=1;
  rocketExplEmit->percentFlicker=90;
  rocketExplEmit->sizeMax=0.040;
  rocketExplEmit->sizeVariance=0.0;
  rocketExplEmit->rotateParticles=0;
  rocketExplEmit->colorVariance[0]=0.0;
  rocketExplEmit->colorVariance[1]=0.6;
  rocketExplEmit->colorVariance[2]=0.6;
  rocketExplEmit->colorVariance[3]=0;
  rocketExplEmit->color[0]=1;
  rocketExplEmit->color[1]=1.0;
  rocketExplEmit->color[2]=1.0;
  rocketExplEmit->emitSpeedMax=3;
  rocketExplEmit->emitSpeedVariance=3;
  eoPsysBake(rocketExplEmit);

  sndLaserFire = eoSampleLoad( Data("/data/sound/", "laser.wav") );
  sndRocketFire = eoSampleLoad( Data("/data/sound/", "rocket.wav") );
  sndNmeDie = eoSampleLoad( Data("/data/sound/", "nmedie.wav") );
  sndNmeShoot = eoSampleLoad( Data("/data/sound/", "nmeshoot.wav") );
  sndPlayerDie = eoSampleLoad( Data("/data/sound/", "playerdie.wav") );
  sndPlayerWin = eoSampleLoad( Data("/data/sound/", "win.wav") );

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



	sprintf(levelName, "level%i.lvl", l);
	eoLoadScene( Data("/data/lvl/", levelName), objInit);
	laser = eoSpriteBaseLoad( Data("/data/gfx/","defaultparticle.spr") );

  //eoObjAdd(playerObj);
  camGet()->pos.z = 30;
  camGet()->target.x = 0;
  camGet()->target.y = 0;
  camGet()->target.z = 0;

  spaceObj = eoObjCreate(ENGOBJ_MODEL);
  spaceObj->model = spaceMdl;
  spaceObj->fullBright = TRUE;
  eoObjBake(spaceObj);
  eoObjAdd(spaceObj);

  state = GSTATE_SHOWLEVEL;
  rcd=0;


}
