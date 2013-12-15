#include "libeo/eng.h"
#include "game.h"

guiContext* startMenuContext;

vboModel* oneMdl;
engObj_s* oneObj;

void oneThink( engObj_s* o )
{
	o->rot.y += 1;
}

void startClicked()
{
	eoObjDel(oneObj);
	startNewGame();
	initLevel(0);
}

void initMenu()
{

	startMenuContext = eoGuiContextCreate();
	guiWindow_s* win = eoGuiAddWindow(startMenuContext, eoSetting()->res.x/2.0-60, eoSetting()->res.y/2-50, 120, 100, "Choose One", NULL);

	eoGuiAddButton(win, 7, 8, 100, 50, "One", startClicked);
	oneMdl = eoModelLoad( "/data/obj/", "one.obj" );

  win = eoGuiAddWindow(startMenuContext, eoSetting()->res.x/2.0-120, eoSetting()->res.y/2-250, 240, 124, "The Law", NULL);
  eoGuiAddLabel(win, 0,0, "One hit = You die");
  eoGuiAddLabel(win, 0,16, "One Object behind you = You die");
  eoGuiAddLabel(win, 0,32, "You can reverse too (a bit)");
  eoGuiAddLabel(win, 0,48, "Space Shoots, Arrows Move");
  eoGuiAddLabel(win, 0,64, "You go off-screen? You die.");
  eoGuiAddLabel(win, 0,80, "Find love, she is pink, don't kill her");

}

void setMenu()
{
	vec3 p;
	eoGuiContextSet(startMenuContext);
	eoGuiShow();
	eoGuiShowCursor(1);

	oneObj = eoObjCreate(ENGOBJ_MODEL);
	oneObj->model = oneMdl;
	oneObj->thinkFunc = oneThink;
	eoObjBake(oneObj);

	eoObjAdd(oneObj);

	p.x = 1.777113;
	p.y = 0.640843;
	p.z = -10.885491;
	eoCamPosSet(p);

	p.x = -2.082835;
	p.y = -1.957727;
	p.z = 13.677655;
	eoCamTargetSet(p);



}
