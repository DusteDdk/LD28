/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libeo/eng.h"

#include "menu.h"

#ifndef DATADIR
  #define DATADIR "."
#endif

#define STATE_MAINMENU 1
int state=STATE_MAINMENU;



void inpQuitFunc(inputEvent* e )
{
  eoPrint("Exiting.");
  eoQuit();
}




void objInitFunc( engObj_s* o)
{
  eoPrint("initFunc called for obj %p (%s)",o,o->className);
}

int main(int argc, char *argv[])
{
	//All must bow for the migthy vector of DOOOOM!
  vec3 p;

  eoInitAll(argc, argv, DATADIR);
  
  //Enable mouse-selection
  //eoGameEnableMouseSelection(0.2);

 
  //Un-Pause simulation
  eoPauseSet(FALSE);


  //Set some GL light stuffs
 /* GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 2.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

  GLfloat pos[] = { 10,10,10,0 }; //Last pos: 0 = dir, 1=omni
  glLightfv( GL_LIGHT0, GL_POSITION, pos );

  GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);*/


  eoInpAddFunc( "exit", "Exit the game.", inpQuitFunc, INPUT_FLAG_DOWN);
  eoExec("bind esc exit");

  eoExec("echo You only get one.!");

  eoPrint("InitGame");
  initGame();
  eoPrint("InitMenu");
  initMenu();
  eoPrint("SetMenu");
  setMenu();



  eoMainLoop();

  return 0;
}
