#include <Application.h>
#include <Screen.h>
#include <Window.h>
#include <Font.h>
#include <ScrollView.h>
#include <ListView.h>
#include <OS.h>
#include <TextControl.h>
#include <Button.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

enum {
	VOID				= 0,
	APP_SERVER			= 1,
	DESKBAR				= 2,
	KERNEL				= 3,
	NET_SERVER			= 4
};

enum {
	STANDARD_AREA		= 0,
	STACK_AREA			= 1,
	LOCKED_AREA			= 2
};

enum {
	ARROW_DOWN			= 0,
	ARROW_UP			= 1,
	ARROW_LEFT			= 2,
	ARROW_RIGHT			= 3,
	NO_ARROW			= 4
};

enum {
	WIDTH 				= 240,
	HEIGHT 				= 180,
	CTRL_HEIGHT 		= 60,
	BUTTON_HEIGHT 		= 6,
	ITEM_HEIGHT 		= 12,
	LINE_HEIGHT 		= 14,
	ITEM_MIN_COUNT 		= 15,
	MEMO_COUNT 			= 8,
	SHARED_LIB_COUNT 	= 16,
	COLUMN_COUNT 		= 8,
	TAB_COUNT 			= 27
};

enum {
	BUTTON_TOP 			= 10000,
	BUTTON_BOTTOM 		= 10001,
	STOP_BUTTON			= 10100,
	RESET_BUTTON		= 10110,
	ERASE_BUTTON		= 10120,
	DUMP_BUTTON			= 10130,
	AUTOSAVE_BUTTON		= 10131,
	SCROLL_BUTTON		= 10200,
	TEAM_SELECTION		= 10300,
	CHECK_SIZE			= 10400,
	REDRAW				= 10500
};

enum {
	ITEM_ENABLE			= 0x01,
	ITEM_MODIFIED		= 0x02,
	ITEM_TOUCHED		= 0x04,
	ITEM_VISIBLE		= 0x08
};

enum {
	USED_INDEX			= 0,
	RSVD_INDEX			= 1,
	MAX_INDEX			= 2,
	SWAP_INDEX			= 3,
	COPY_INDEX			= 4,
	IN_INDEX			= 5,
	OUT_INDEX			= 6,
	ID_INDEX			= 7
};

enum {
	NAME_TAB			= 0,
	NAME_FULL_TAB		= 1,
	USED_ENABLE_TAB		= 2,
	USED_TAB			= 3,
	USED_CARE_TAB		= 4,
	RSVD_ENABLE_TAB		= 5,
	RSVD_TAB			= 6,
	RSVD_CARE_TAB		= 7,
	MAX_ENABLE_TAB		= 8,
	MAX_TAB				= 9,
	MAX_CARE_TAB		= 10,
	SWAP_ENABLE_TAB		= 11,
	SWAP_TAB			= 12,
	SWAP_CARE_TAB		= 13,
	COPY_ENABLE_TAB		= 14,
	COPY_TAB			= 15,
	COPY_CARE_TAB		= 16,
	IN_ENABLE_TAB		= 17,
	IN_TAB				= 18,
	IN_CARE_TAB			= 19,
	OUT_ENABLE_TAB		= 20,
	OUT_TAB				= 21,
	OUT_CARE_TAB		= 22,
	ID_ENABLE_TAB		= 23,
	ID_TAB				= 24,
	EXPAND_TAB			= 25,
	TERMINATOR_TAB		= 26
};

static uint8 TabSelectable[TAB_COUNT] = {
	0, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1,
	1,
	0
};

static uint8 TabBlocIndex[TAB_COUNT] = {
	0, 0,
	1, 1, 1,
	2, 2, 2,
	3, 3, 3,
	4, 4, 4,
	5, 5, 5,
	6, 6, 6,
	7, 7, 7,
	8, 8,
	9,
	10
};

static uint8 TabType[TAB_COUNT] = {
	0, 1,
	2, 0, 3,
	2, 0, 3,
	2, 0, 3,
	2, 0, 3,
	2, 0, 3,
	2, 0, 3,
	2, 0, 3,
	2, 0,
	4,
	5
};

static char TabNames[COLUMN_COUNT+2][5] = {
	"Name",
	"Phys",
	"Rsvd",
	"Used",
	"Swap",
	"Copy",
	"In",
	"Out",
	"Id",
	""
};

static char short_lib_names[SHARED_LIB_COUNT][31] = {
	"libGL.so_seg",
	"libatalk.so_seg",
	"libbe.so_seg",
	"libbe.patch.so_seg",
	"libdevice.so_seg",
	"libgame.so_seg",
	"libmail.so_seg",
	"libmedia.so_seg",
	"libmidi.so_seg",
	"libnet.so_seg",
	"libnetdev.so_seg",
	"libroot.so_seg",
	"libstdc++.r4.so_seg",
	"libtextencoding.so_seg",
	"libtracker.so_seg",
	"libtranslation.so_seg"
};

static char real_lib_names[SHARED_LIB_COUNT][16] = {
	"libGL",
	"libatalk",
	"libbe",
	"libbe.patch",
	"libdevice",
	"libgame",
	"libmail",
	"libmedia",
	"libmidi",
	"libnet",
	"libnetdev",
	"libroot",
	"libstdc++.r4",
	"libtextencoding",
	"libtracker",
	"libtranslation"
};

struct TNode {
	char 		name[32];
	TNode 		*prev;
	TNode 		*next;
	TNode		*areas;
	int8		type;
	int8		level;
	uint8		status;
	char		_reserved_;
	union {
		int32	selector[1];
		int32	used;
	};
	int32		rsvd;
	int32		max;
	int32		swap;
	int32		copy;
	int32		in;
	int32		out;
	union {
		area_id	aid;
		team_id	tid;
	};

	void		Remove(TNode **root);
	void		TouchAll();
	bool		CleanUpAll(TNode **root);
	bool		Sort(TNode **root);
	TNode		*Copy(TNode **root, int32 sign = 1);
	void		Merge(TNode **root, int32 sign);
	void		SortAll(TNode **root);
	
	bool		Enable();
	bool		Modified();
	bool		Touched();
	bool		Visible();
	void		SetEnable(bool enable);
	void		SetModified(bool modified);
	void		SetTouched(bool touched);
	void		SetVisible(bool visible);
};

class TItem : public BListItem {
public :
	TItem(TNode *n);
	
	virtual void 	DrawItem(BView *owner, BRect itemRect, bool complete);
	virtual void 	Update(BView *owner, const BFont *font);
	TNode			*Node() { return n; }
	void			SetNode(TNode *node) { n = node; }

private :
	void 			ConvertToString(char *str, int32 val, int32 index);
	TNode			*n;
};

class BarView : public BView {
public :
	BarView(BRect frame, int32 arrow, BMessage *click_message, uint32 resize_mode);
	~BarView();
	void SetArrow(int32 arrow);
	virtual	void Draw(BRect updateRect);
	virtual	void MouseDown(BPoint where);
	virtual	void MouseUp(BPoint where);
	virtual	void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);

private :
	bool		pressed, click_session;
	int32		cached_arrow;
	BMessage 	*msg;
};

class ControlView : public BView {
public :
	ControlView(BRect frame, BFont *font);
	
	virtual void	Draw(BRect updateRect);
	void			Update();
	
	BButton			*reset, *erase, *stop, *dump, *autosave;
	BarView			*scroll[4];
	BTextView		*tv;
	BTextControl	*tc;
};

class LineView : public BView {
public :
	LineView(BRect frame, BFont *font);

	virtual	void	Draw(BRect updateRect);
	virtual	void	MouseDown(BPoint where);
	virtual	void	MouseUp(BPoint where);
	virtual	void	MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
	float			NthTabWidth(int32 index);
	int32			TabIndex(BPoint where, BRect *update);

private :
	int32			index_item;
	bool			pressed;
	bool			click_session;
	bool			expanded;
	bool			display[TAB_COUNT];
	BRect			last_update;
};

class TWindow : public BWindow {
public :
	TWindow(BRect frame);
	~TWindow();
	virtual	bool	QuitRequested();
	virtual void	WorkspaceActivated(int32 ws, bool state);
	virtual	void	MessageReceived(BMessage *message);
	void			CheckVisibility();
	void			UpdateWindow();
	void			Redraw();
	void			DumpStates();
	void			UpdateDisplay(bool area, TNode *root, bool modified, bool touched);

	BView			*mv;
	BarView			*b1, *b2;
	LineView		*lnv;
	BListView		*lv, *lv2;
	ControlView		*cv;
	
	bool			show_control, show_area, auto_dump, dump_save_all;
	int32			dump_delay, dump_index, dump_delay_count;
	BScrollView 	*sv, *sv2;
};

class TApp : public BApplication {
public :
	TApp();

	TWindow *tw;
};

/*----------------------------------------------------------------*/

TNode		*root;
TNode		*kernel;
TNode		*libraries;
TNode		*loader_cache;
TNode		*root_display;
TNode		*memos[MEMO_COUNT];
char		memo_names[MEMO_COUNT][25];
bool		area_modified;
bool		team_modified;
BLocker		*lock;
team_id		FootPrint_team;
int32		PositiveState;
int32		NegativeState;
bool		UpdateCurrentState;

int32		sorting_index;
bool		display_fullname;
bool		display_options[TAB_COUNT];
bool		care_options[TAB_COUNT-1];

void register_area(area_info *ainfo, TNode *n, bool library, int32 type) {
	TNode		*a;
	int32		max;

	a = n->areas;
	while (a != NULL) {
		if (a->aid == ainfo->area)
			goto area_found;
		a = a->next;
	}
	// create a new entry
	a = new TNode();
	memcpy(a->name, ainfo->name, 32);
	a->name[31] = 0;
	if ((ainfo->lock == B_FULL_LOCK) || (ainfo->lock == B_CONTIGUOUS))
		type = LOCKED_AREA;
	a->type = type;
	a->level = 0;
	a->aid = ainfo->area;
	a->SetEnable(true);
	a->prev = NULL;
	a->next = n->areas;
	a->areas = NULL;
	if (n->areas != NULL)
		n->areas->prev = a;
	n->areas = a;
	n->SetModified(true);
	if (library) {
		a->used = 0;
		a->rsvd = 0;
		a->copy = 0;
		a->in = 0;
		a->out = 0;
	}
	a->max = 0;
	a->swap = 0;	// for locked areas.
	
area_found :
	a->SetEnable(true);
	if (library) {
		if (a->used < ainfo->ram_size)
			a->used = ainfo->ram_size;
		if (a->rsvd < ainfo->size)
			a->rsvd = ainfo->size;
		max = a->used+B_PAGE_SIZE*(ainfo->out_count-ainfo->in_count);
		if (max > a->rsvd)
			max = a->rsvd;
		if (a->max < max)
			a->max = max;
		if (a->type == STANDARD_AREA) {
			if (a->swap < a->rsvd)
				a->swap = a->rsvd;
		} else if (a->type == STACK_AREA) {
			if (a->swap < max)
				a->swap = max;
		}
		if (a->copy < ainfo->copy_count)
			a->copy = ainfo->copy_count;
		if (a->in < ainfo->in_count)
			a->in = ainfo->in_count;
		if (a->out < ainfo->out_count)
			a->out = ainfo->out_count;
		if (a->Sort(&n->areas))
			n->SetModified(true);
	}
	else {
		if (((a->rsvd ^ ainfo->size) | (a->used ^ ainfo->ram_size) |
			 (a->copy ^ ainfo->copy_count) | (a->in ^ ainfo->in_count) |
			 (a->out ^ ainfo->out_count)) != 0) {
			n->SetTouched(true);
			a->used = ainfo->ram_size;
			a->rsvd = ainfo->size;
			max = a->used+B_PAGE_SIZE*(ainfo->out_count-ainfo->in_count);
			if (max > a->rsvd)
				max = a->rsvd;
			a->max = max;
			if (a->type == STANDARD_AREA)
				a->swap = a->rsvd;
			else if (a->type == STACK_AREA)
				a->swap = max;
			a->copy = ainfo->copy_count;
			a->in = ainfo->in_count;
			a->out = ainfo->out_count;
			if (a->Sort(&n->areas))
				n->SetModified(true);
		}
		// include in the team stats
		n->used += a->used;
		n->rsvd += a->rsvd;
		n->max += a->max;
		n->swap += a->swap;
		n->copy += a->copy;
		n->in += a->in;
		n->out += a->out;
	}
}

void update_areas(TNode *n) {
	bool		belong;
	int32		i, j, cookie, len, seg, type, old_max;
	uint32		adr;
	area_info	ainfo;

	// Mark all areas of the team disabled
	n->areas->TouchAll();
	// reset the team-wide stats.
	old_max = n->max;
	n->used = 0;
	n->rsvd = 0;
	n->max = 0;
	n->swap = 0;
	n->copy = 0;
	n->in = 0;
	n->out = 0;
	// go through the area list of the team
	cookie = 0;
	while (get_next_area_info(n->tid, &cookie, &ainfo) == B_OK) {
		// preload area information
		belong = true;
		type = STANDARD_AREA;
		adr = (uint32)ainfo.address;
		// apply special area filtering
		if (adr == 0xdf000000)
			belong = false;
		else if ((adr >= 0xec000000) && (adr < 0xf0000000))
			// is it one of the shared libraries ?
			for (i=0; i<SHARED_LIB_COUNT; i++)
				if (strncmp(ainfo.name, short_lib_names[i], strlen(short_lib_names[i])) == 0) {
					for (j=0; j<31; j++)
						if (ainfo.name[j] == 0)
							break;
					if ((j < 6) || (j == 31))
						break;
					seg = ainfo.name[j-1]-'0';
					if ((seg < 0) || (seg > 7))
						break;
					strcpy(ainfo.name, real_lib_names[i]);
					j = strlen(ainfo.name);
					strcpy(ainfo.name+j, "_seg0");
					ainfo.name[j+4] += seg;
					if (seg == 0) {
						ainfo.area = i*1000+seg;
						register_area(&ainfo, libraries, true, type);
						belong = false;
					}
					else {
						if (B_PAGE_SIZE*ainfo.copy_count < ainfo.ram_size)
							ainfo.ram_size = B_PAGE_SIZE*ainfo.copy_count;
					}
				}
		switch (n->type) {
		case DESKBAR :
			if ((adr >= 0xea000000) && (adr < 0xebffffff)) {
				if (ainfo.copy_count == 0)
					belong = false;
				else {
					if (B_PAGE_SIZE*ainfo.copy_count < ainfo.ram_size)
						ainfo.ram_size = B_PAGE_SIZE*ainfo.copy_count;
				}
			}
			break;
		case APP_SERVER :
			len = strlen(ainfo.name);
			if (len > 6) {
				if (strcmp(ainfo.name+len-6, "RWHeap") == 0)
					belong = false;
				else if (strcmp(ainfo.name+len-6, "ROHeap") == 0)
					belong = false;
			}
			break;
		case KERNEL :
			if (strncmp(ainfo.name, "cached_", 7) == 0) {
				strcpy(ainfo.name, ainfo.name+7);
				register_area(&ainfo, loader_cache, false, type);
				belong = false;
			}
			else if (strncmp(ainfo.name, "physical_ram", 12) == 0)
				belong = false;
			else if ((strlen(ainfo.name) > 18) &&
				(ainfo.name[4] == '_') &&
				(ainfo.name[9] == '_') &&
				(ainfo.name[16] == ' '))
				belong = false;
			break;
		case NET_SERVER :
			if (strncmp(ainfo.name, "vulcan-mind-mel", 15) == 0)
				belong = false;
			break;
		}
		// record area (after filtering).
		if (strncmp(ainfo.name, "Stacks of ", 10) == 0) {
			ainfo.name[0] = '$';
			strncpy(ainfo.name+1, ainfo.name+10, 30);
			type = STACK_AREA;
		}
		if (belong)
			register_area(&ainfo, n, false, type);
	}
	// set the max to whatever is the highest
	if (n->max < old_max)
		n->max = old_max;
	// clean-up unused areas
	n->CleanUpAll(&n->areas);
}

TNode *update_one_team(team_info *tinfo) {
	int32		i;
	TNode		*n;
	
	// check if the team is alredady recorded
	n = root;
	while (n != NULL) {
		if (n->tid == tinfo->team)
			goto team_found;
		n = n->next;
	}
	// create a new entry
	n = new TNode();
	for (i=0; i<63; i++)
		if ((tinfo->args[i] == ' ') || (tinfo->args[i] == 0))
			break;
	for (; i>=0; i--)
		if (tinfo->args[i] == '/')
			break;
	strncpy(n->name, tinfo->args+i+1, 31);
	n->name[31] = 0;
	if (tinfo->team == FootPrint_team)
		return NULL;
	if (strncmp(n->name, "app_server", 10) == 0)
		n->type = APP_SERVER;
	else if (strncmp(n->name, "Deskbar", 7) == 0)
		n->type = DESKBAR;
	else if (strncmp(n->name, "net_server", 10) == 0)
		n->type = NET_SERVER;
	else if (tinfo->team == 1)
		n->type = KERNEL;
	else
		n->type = VOID;
	n->level = 0;
	n->tid = tinfo->team;
	n->max = 0;
	n->SetEnable(false);
	n->SetModified(false);
	n->SetTouched(false);
	n->prev = NULL;
	n->next = root;
	n->areas = NULL;
	if (root != NULL)
		root->prev = n;
	root = n;
	team_modified = true;
	
team_found:
	n->SetEnable(true);
	if (n->tid > 0) {
		update_areas(n);
		if (n->Sort(&root))
			team_modified = true;
	}
	return n;
}

void update_teams() {
	int32		cookie, i;
	int32		old_rsvd, old_used, old_copy, old_in, old_out;
	int32		old_loader_max, old_libraries_max;
	team_info	tinfo;
	TNode		*n;
	
	lock->Lock();
	// all team are removed by default...
	root->TouchAll();
	// check the fake libraries team.
	if (libraries)
		old_libraries_max = libraries->max;
	else
		old_libraries_max = 0;
	tinfo.team = -2;
	strcpy(tinfo.args, "Shared Libraries");
	libraries = update_one_team(&tinfo);
	if (libraries->areas != NULL)
		libraries->areas->TouchAll();
	// check the fake loader cache team.
	if (loader_cache)
		old_loader_max = loader_cache->max;
	else
		old_loader_max = 0;
	tinfo.team = -1;
	strcpy(tinfo.args, "Loader cache");
	loader_cache = update_one_team(&tinfo);
	loader_cache->used = 0;
	loader_cache->rsvd = 0;
	loader_cache->max = 0;
	loader_cache->swap = 0;
	loader_cache->copy = 0;
	loader_cache->in = 0;
	loader_cache->out = 0;
	if (loader_cache->areas != NULL)
		loader_cache->areas->TouchAll();
	// check the KERNEL_SPACE team;	
	tinfo.team = 1;
	strcpy(tinfo.args, "Kernel space");
	kernel = update_one_team(&tinfo);
	// then scan for all existing team.
	cookie = 0;
	while (get_next_team_info(&cookie, &tinfo) == B_OK)
		update_one_team(&tinfo);
	// remove unused teams...
	if (root->CleanUpAll(&root))
		team_modified = true;
	lock->Unlock();
	// update the stats of libraries
	old_used = libraries->used;
	old_rsvd = libraries->rsvd;
	old_copy = libraries->copy;
	old_in = libraries->in;
	old_out = libraries->out;
	libraries->used = 0;
	libraries->rsvd = 0;
	libraries->max = 0;
	libraries->swap = 0;
	libraries->copy = 0;
	libraries->in = 0;
	libraries->out = 0;
	n = libraries->areas;
	while (n) {
		libraries->rsvd += n->rsvd;
		libraries->used += n->used;
		libraries->max += n->max;
		libraries->swap += n->swap;
		libraries->copy += n->copy;
		libraries->in += n->in;
		libraries->out += n->out;
		n = n->next;
	}
	if (((libraries->rsvd ^ old_rsvd) | (libraries->used ^ old_used) |
		 (libraries->copy ^ old_copy) | (libraries->in ^ old_in) |
		 (libraries->out ^ old_out)) != 0)
		libraries->SetTouched(true);
	if (libraries->max < old_libraries_max)
		libraries->max = old_libraries_max;
	if (libraries->Sort(&root))
		team_modified = true;
	if (libraries->areas != NULL)
		libraries->areas->CleanUpAll(&libraries->areas);
	// Do the sorting and clean up for the loader cache
	if (loader_cache->max < old_loader_max)
		loader_cache->max = old_loader_max;
	if (loader_cache->Sort(&root))
		team_modified = true;
	if (loader_cache->areas != NULL)
		loader_cache->areas->CleanUpAll(&loader_cache->areas);
}

int32 watcher(void *data) {
	bool		*die;
	TWindow		*w;
	
	w = ((TApp*)be_app)->tw;
	die = (bool*)data;
	while (!*die) {
		if (UpdateCurrentState)
			if (w->Lock()) {
				update_teams();
				w->Redraw();
				if (w->auto_dump) {
					if (w->dump_delay_count == 0) {
						w->dump_delay_count = w->dump_delay;
						lock->Lock();
						w->DumpStates();
						lock->Unlock();
					}
					w->dump_delay_count--;
				}
				w->Unlock();
			}
		snooze(2000000);
	}
	return 0;
}

int main() {	
	bool		die = false;
	TApp		*app;
	int32		status, i;
	thread_id	th_id;
	thread_info	tinfo;

	sorting_index = 0;
	display_options[0] = true;
	display_options[1] = false;
	display_options[2] = true;
	display_options[3] = true;
	display_options[4] = false;
	display_options[5] = false;
	display_options[6] = false;
	display_options[7] = false;
	care_options[0] = true;
	care_options[1] = false;
	care_options[2] = true;
	care_options[3] = false;
	care_options[4] = false;
	care_options[5] = false;
	care_options[6] = false;

	PositiveState = -1;
	NegativeState = -1;
	UpdateCurrentState = true;
	for (i=0; i<MEMO_COUNT; i++) {
		memos[i] = NULL;
		memo_names[i][0] = 0;
	}
	
	app = new TApp();
	th_id = spawn_thread(watcher, "mem watch", B_DISPLAY_PRIORITY, (void*)&die);
	get_thread_info(th_id, &tinfo);
	FootPrint_team = tinfo.team;
	area_modified = false;
	team_modified = false;
	root = NULL;
	kernel = NULL;
	libraries = NULL;
	loader_cache = NULL;
	root_display = NULL;
	lock = new BLocker();
	
	resume_thread(th_id);
	app->Run();
	die = true;
	wait_for_thread(th_id, &status);
	delete app;
	return 0;
}

TApp::TApp() : BApplication("application/x-vnd.Be.AreaWatch") {
	tw = new TWindow(BRect(4000.0, 0.0, 3999.0+WIDTH, HEIGHT+2*BUTTON_HEIGHT-1.0+LINE_HEIGHT));
	tw->CheckVisibility();
	tw->Show();
}

TWindow::TWindow(BRect frame) : BWindow(frame, "AreaWatch", B_TITLED_WINDOW,
B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE) {
	BFont		font;
	BMessage	msg_top(BUTTON_TOP);
	BMessage	msg_bottom(BUTTON_BOTTOM);
	
	if (font.SetFamilyAndStyle("ProFontISOLatin1", "Regular") != B_OK)
		font.SetFamilyAndStyle(be_fixed_font->FamilyAndStyle());
	font.SetSize(9.0);

	mv = new BView(BRect(0, 0, WIDTH-1, 2*HEIGHT+2*BUTTON_HEIGHT+CTRL_HEIGHT-1+LINE_HEIGHT),
				   "mv", B_FOLLOW_LEFT_RIGHT,
				   B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	
	lnv = new LineView(BRect(0, CTRL_HEIGHT+BUTTON_HEIGHT,
							 WIDTH-1, CTRL_HEIGHT+BUTTON_HEIGHT+LINE_HEIGHT-1), &font);
	lnv->SetViewColor(222, 219, 222, 255);
	mv->AddChild(lnv);

	lv = new BListView(BRect(0, CTRL_HEIGHT+BUTTON_HEIGHT+LINE_HEIGHT,
							 WIDTH-15, HEIGHT-1+CTRL_HEIGHT+BUTTON_HEIGHT+LINE_HEIGHT),
							 "lv", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT_RIGHT,
							 B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	lv->SetSelectionMessage(new BMessage(TEAM_SELECTION));
	lv->SetFont(&font);
	lv->SetViewColor(222, 219, 222, 255);
	sv = new BScrollView("sv", lv, B_FOLLOW_LEFT_RIGHT,
						 B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE,
						 false, true, B_NO_BORDER);
	sv->SetViewColor(222, 219, 222, 255);
	mv->AddChild(sv);

	lv2 = new BListView(BRect(0, HEIGHT+2*BUTTON_HEIGHT+CTRL_HEIGHT+LINE_HEIGHT,
							  WIDTH-15, 2*HEIGHT+2*BUTTON_HEIGHT+CTRL_HEIGHT-1+LINE_HEIGHT),
							  "lv2", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT_RIGHT,
							  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	lv2->SetFont(&font);
	lv2->SetViewColor(222, 219, 222, 255);
	sv2 = new BScrollView("sv2", lv2, B_FOLLOW_LEFT_RIGHT,
						  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE,
						  false, true, B_NO_BORDER);
	sv2->SetViewColor(222, 219, 222, 255);
	mv->AddChild(sv2);

	b1 = new BarView(BRect(0, CTRL_HEIGHT,
						   WIDTH-1, BUTTON_HEIGHT+CTRL_HEIGHT-1),
					 ARROW_DOWN, &msg_top, B_FOLLOW_LEFT_RIGHT);
	mv->AddChild(b1);

	b2 = new BarView(BRect(0, HEIGHT+BUTTON_HEIGHT+CTRL_HEIGHT+LINE_HEIGHT,
						   WIDTH-1, HEIGHT+2*BUTTON_HEIGHT+CTRL_HEIGHT-1+LINE_HEIGHT),
					 ARROW_UP, &msg_bottom, B_FOLLOW_LEFT_RIGHT);
	mv->AddChild(b2);
	
	cv = new ControlView(BRect(0, 0, WIDTH-1, CTRL_HEIGHT-1), &font);
	mv->AddChild(cv);

	AddChild(mv);
	
	cv->Update();

	show_control = false;
	show_area = false;
	auto_dump = false;
	dump_save_all = false;
	dump_delay = 2;
	dump_index = 0;
	dump_delay_count = 0;

	UpdateWindow();
}

TWindow::~TWindow() {
	((TApp*)be_app)->tw = NULL;
}

void TWindow::WorkspaceActivated(int32 ws, bool state) {
	CheckVisibility();
}

void TWindow::CheckVisibility() {
	BRect		frame;
	BPoint		point;
	BScreen		screen;
	
	frame.right = screen.Frame().right-4.0;
	frame.bottom = screen.Frame().bottom-4.0;
	frame.left = 4.0;
	frame.top = 22.0;

	point.x = 0.0;
	point.y = 0.0;
	
	if (Frame().left < frame.left)
		point.x = frame.left-Frame().left;
	else if (Frame().right > frame.right)
		point.x = frame.right-Frame().right;
	if (Frame().top < frame.top)
		point.y = frame.top-Frame().top;
	else if (Frame().bottom > frame.bottom)
		point.y = frame.bottom-Frame().bottom;
		
	if ((point.x != 0.0) || (point.y != 0.0))
		MoveBy(point.x, point.y);
}

bool TWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
} 

static char sign_labels[6] = { '-', 0, ' ', 0, '+', 0 };

void TWindow::MessageReceived(BMessage *msg) {
	char		name[2];
	int32		i, index, index0, val;

	switch (msg->what) {
	case BUTTON_TOP :
		show_control = !show_control;
		UpdateWindow();
		break;
	case BUTTON_BOTTOM :
		show_area = !show_area;
		UpdateWindow();
		break;
	case STOP_BUTTON :
		UpdateCurrentState = 1-UpdateCurrentState;
		cv->Update();
		team_modified = true;
		Redraw();
		break;
	case RESET_BUTTON :
		if (NegativeState == -1)
			break;
		update_teams();
		lock->Lock();
		if (memos[NegativeState] != NULL) {
			memos[NegativeState]->TouchAll();
			memos[NegativeState]->CleanUpAll(&memos[NegativeState]);
		}
		memos[NegativeState] = root->Copy(&root);
		memos[NegativeState]->SortAll(&memos[NegativeState]);
		lock->Unlock();
		strcpy(memo_names[NegativeState], cv->tc->TextView()->Text());
		cv->tc->TextView()->SetText("");
		cv->Update();
		team_modified = true;
		Redraw();
		break;
	case ERASE_BUTTON :
		if ((NegativeState == -1) || (memos[NegativeState] == NULL))
			break;
		lock->Lock();
		if (memos[NegativeState] != NULL) {
			memos[NegativeState]->TouchAll();
			memos[NegativeState]->CleanUpAll(&memos[NegativeState]);
		}
		if (PositiveState > NegativeState)
			PositiveState--;
		for (i=NegativeState; i<MEMO_COUNT-1; i++) {
			memos[i] = memos[i+1];
			strcpy(memo_names[i], memo_names[i+1]);
		}
		memos[MEMO_COUNT-1] = NULL;
		memo_names[MEMO_COUNT-1][0] = 0;
		if (memos[NegativeState] == NULL)
			NegativeState--;
		lock->Unlock();
		cv->Update();
		team_modified = true;
		Redraw();
		break;
	case DUMP_BUTTON :
		lock->Lock();
		DumpStates();
		lock->Unlock();
		break;
	case AUTOSAVE_BUTTON :
		lock->Lock();
		auto_dump = true;
		dump_delay = 120;
		dump_save_all = true;
		lock->Unlock();
		cv->Update();
		Redraw();
		break;
	case SCROLL_BUTTON :
		if (PositiveState >= 0) {
			PositiveState--;
			cv->Update();
			team_modified = true;
			Redraw();
		}
		break;
	case SCROLL_BUTTON+1 :
		if ((PositiveState < (MEMO_COUNT+1)) && (memos[PositiveState+1] != NULL)) {
			PositiveState++;
			cv->Update();
			team_modified = true;
			Redraw();
		}
		break;
	case SCROLL_BUTTON+2 :
		if (NegativeState >= 0) {
			strcpy(memo_names[NegativeState], cv->tc->TextView()->Text());
			NegativeState--;
			cv->Update();
			team_modified = true;
			Redraw();
		}
		break;
	case SCROLL_BUTTON+3 :
		if (NegativeState == (MEMO_COUNT-1))
			break;
		if ((NegativeState >= 0) && (memos[NegativeState] == NULL))
			break;
		if (NegativeState >= 0)
			strcpy(memo_names[NegativeState], cv->tc->TextView()->Text());
		if (memos[NegativeState+1] != NULL) {
			NegativeState++;
			cv->Update();
			team_modified = true;
			Redraw();
		}
		else {
			NegativeState++;
			strcpy(memo_names[NegativeState], "< Please, enter a name >");
			cv->Update();
			cv->tc->TextView()->MakeFocus();
			cv->tc->TextView()->SelectAll();
			team_modified = true;
			Redraw();
		}
		break;
	case TEAM_SELECTION :
		area_modified = true;
		Redraw();
		break;
	case CHECK_SIZE :
		UpdateWindow();
		team_modified = true;
		mv->Invalidate();
		Redraw();
		break;
	case REDRAW :
		team_modified = true;
		lock->Lock();
		root->SortAll(&root);
		for (i=0; i<MEMO_COUNT; i++)
			if (memos[i] != NULL)
				memos[i]->SortAll(&memos[i]);
		lock->Unlock();
		mv->Invalidate();
		Redraw();
		break;
	/* external public messages */
	case 'sdpl' :
		{
		int32		pos_state, neg_state;
		
		if (msg->FindInt32("PosState", &pos_state) != B_OK)
			break;
		if (msg->FindInt32("NegState", &neg_state) != B_OK)
			break;
		if ((pos_state < -1) || (pos_state >= MEMO_COUNT) || (memos[pos_state] == NULL))
			break;
		if ((neg_state < -1) || (neg_state >= MEMO_COUNT) || (memos[neg_state] == NULL))
			break;
		if (NegativeState >= 0)
			strcpy(memo_names[NegativeState], cv->tc->TextView()->Text());
		PositiveState = pos_state;
		NegativeState = neg_state;
		cv->Update();
		team_modified = true;
		Redraw();
		}
		break;
	case 'smem' :
		{
		const char	*name;
		int32		mem_index;
		
		if (msg->FindInt32("MemIndex", &mem_index) != B_OK)
			break;
		if (msg->FindString("Name", &name) != B_OK)
			break;
		if ((mem_index < 0) || (mem_index >= MEMO_COUNT))
			break;
		update_teams();
		lock->Lock();
		if (memos[mem_index] != NULL) {
			memos[mem_index]->TouchAll();
			memos[mem_index]->CleanUpAll(&memos[mem_index]);
		}
		memos[mem_index] = root->Copy(&root);
		memos[mem_index]->SortAll(&memos[mem_index]);
		lock->Unlock();
		strcpy(memo_names[mem_index], name);
		cv->tc->TextView()->SetText("");
		cv->Update();
		team_modified = true;
		Redraw();
		}
		break;
	case 'swtc' :
		{
		bool		state;
		
		if (msg->FindBool("Active", &state) != B_OK)
			break;
		UpdateCurrentState = state;
		cv->Update();
		team_modified = true;
		Redraw();
		}
		break;
	case 'sadm' :
		{
		bool		state;
		bool		save_all;
		int32		delay;
		
		if (msg->FindBool("Active", &state) != B_OK)
			break;
		if (msg->FindBool("SaveAll", &save_all) != B_OK)
			break;
		if (msg->FindInt32("Delay", &delay) != B_OK)
			break;
		auto_dump = state;
		dump_delay = delay/2;
		if (dump_delay < 2)
			dump_delay = 2;
		dump_save_all = save_all;
		}
		break;
	}
}

void TWindow::UpdateWindow() {
	int32		i;
	float		height, width;

	BeginViewTransaction();
	if (show_control)
		mv->MoveTo(BPoint(0.0, 0));
	else
		mv->MoveTo(BPoint(0.0, -CTRL_HEIGHT));
	height = 2*BUTTON_HEIGHT + HEIGHT + LINE_HEIGHT;
	if (show_control)
		height += CTRL_HEIGHT;
	if (show_area)
		height += HEIGHT;
	width = WIDTH-3*45;
	if (display_fullname)
		width += 96;
	for (i=0; i<COLUMN_COUNT; i++)
		if (display_options[i])
			width += 45;
	if (show_control && (width < WIDTH))
		width = WIDTH;
	ResizeTo(width-1, height-1);
	b1->SetArrow(show_control?ARROW_UP:ARROW_DOWN);
	b2->SetArrow(show_area?ARROW_DOWN:ARROW_UP);
	EndViewTransaction();
}

void TWindow::Redraw() {
	bool		modified, touched;
	int32		i, selection;
	TNode		*n, *local_root, *old_root;
	
	lock->Lock();
	// Is it just a single display case ?
	if ((NegativeState == -1) || (memos[NegativeState] == NULL)) {
		if (PositiveState == -1)
			local_root = root;
		else
			local_root = memos[PositiveState];
		old_root = NULL;
		goto local_root_defined;
	}
	// It's a complex case. Is it a stable one ?
	if (root_display == NULL)
		goto rebuild_needed;
	if (team_modified)
		goto rebuild_needed;
	if (PositiveState == -1) {
		n = root;
		while (n) {
			if (n->Touched())
				goto rebuild_needed;
			n = n->next;
		}
	}
	local_root = root_display;
	old_root = NULL;
	goto local_root_defined;
	
	// reprocessing required
rebuild_needed :
	old_root = root_display;
	root_display = NULL;
	if (PositiveState == -1)
		root_display = root->Copy(&root, 1);
	else
		root_display = root->Copy(&memos[PositiveState], 1);
	memos[NegativeState]->Merge(&root_display, -1);
	root_display->SortAll(&root_display);
	team_modified = true;
	local_root = root_display;
	
local_root_defined :	
	lock->Unlock();
		
	BeginViewTransaction();
	DisableUpdates();
	
	modified = team_modified;
	if (modified) area_modified = true;
	team_modified = false;
	UpdateDisplay(true, local_root, modified, modified);

	selection = lv->CurrentSelection();
	if (selection < 0) {
		local_root = NULL;
		modified = area_modified;
		touched = modified;
	}
	else {
		n = ((TItem*)lv->ItemAt(selection))->Node();
		modified = area_modified | n->Modified();
		touched = modified | n->Touched();
		n->SetModified(false);
		local_root = n->areas;
	}
	area_modified = false;
	UpdateDisplay(false, local_root, modified, touched);

	n = root;
	while (n) {
		n->SetTouched(false);
		n = n->next;
	}

	EnableUpdates();
	EndViewTransaction();

	UpdateIfNeeded();

	if (old_root) {
		old_root->TouchAll();
		old_root->CleanUpAll(&old_root);
	}
}

void TWindow::DumpStates() {
	bool		visible;
	FILE		*fp;
	int32		index;
	time_t		t;
	TNode		*n, *m, *local_root;
	char		name[32] = "/boot/AreaWatchSnapshots/0000";
	
	// Open the right file.
	if (dump_save_all) {
		index = dump_index;
		sprintf(name+25, "%04d", index);
		dump_index++;
		if (dump_index == 10000) dump_index = 0;
	}
	fp = fopen(name, "w");
	if (fp == NULL) {
		mkdir("/boot/AreaWatchSnapshots", 0x777);
		fp = fopen(name, "w");
		if (fp == NULL)
			return;
	}
	time(&t);
	// Header.
	fprintf(fp, "######## AreaWatch Snapshot: %s\n", ctime(&t));
	// Team list
	fprintf(fp, "\n############## Team list\n");
	fprintf(fp, "<------------- Name ------------>< Phys >< Rsvd >< Used >< Swap >< Copy ><  In  ><  Out ><  Id  >\n");

		// Is it just a single display case ?
	if ((NegativeState == -1) || (memos[NegativeState] == NULL)) {
		if (PositiveState == -1)
			local_root = root;
		else
			local_root = memos[PositiveState];
	}
		// It's a complex case. Is it a stable one ?
	else {
		if (root_display == NULL)
			local_root = root;
		else
			local_root = root_display;
	}

	n = local_root;
	while (n != NULL) {
		visible = (care_options[USED_INDEX] && (n->used != 0)) ||
				  (care_options[RSVD_INDEX] && (n->rsvd != 0)) ||
				  (care_options[MAX_INDEX] && (n->max != 0)) ||
				  (care_options[COPY_INDEX] && (n->copy != 0)) ||
				  (care_options[IN_INDEX] && (n->in != 0)) ||
				  (care_options[OUT_INDEX] && (n->out != 0));
		n->SetVisible(visible);
		if (visible)
			fprintf(fp, "%33s %7d %7d %7d %7d %7d %7d %7d %7d\n",
					n->name, n->max/1024, n->rsvd/1024, n->used/1024, n->swap/1024, n->copy, n->in, n->out, n->tid);
		n = n->next;
	}
	n = local_root;
	while (n != NULL) {
		if (n->Visible()) {
			fprintf(fp, "\n############## Area list for team : %s\n", n->name);
			fprintf(fp, "<------------- Name ------------>< Phys >< Rsvd >< Used >< Swap >< Copy ><  In  ><  Out ><  Id  >\n");
			m = n->areas;
			while (m != NULL) {
				visible = (care_options[USED_INDEX] && (m->used != 0)) ||
						  (care_options[RSVD_INDEX] && (m->rsvd != 0)) ||
						  (care_options[MAX_INDEX] && (m->max != 0)) ||
						  (care_options[COPY_INDEX] && (m->copy != 0)) ||
						  (care_options[IN_INDEX] && (m->in != 0)) ||
						  (care_options[OUT_INDEX] && (m->out != 0));
				if (visible)
					fprintf(fp, "%33s %7d %7d %7d %7d %7d %7d %7d %7d\n",
							m->name, m->max/1024, m->rsvd/1024, m->used/1024, m->swap/1024, m->copy, m->in, m->out, m->aid);
				m = m->next;
			}
		}
		n = n->next;
	}
	fclose(fp);
}

void TWindow::UpdateDisplay(bool area, TNode *root, bool modified, bool touched) {
	bool		visible;
	int32		index, count, new_count;
	TNode		*n, *prev, *next, *selection;
	BListView	*local_lv;
	
	if (area)
		local_lv = lv;
	else
		local_lv = lv2;
	if (modified) {
		index = local_lv->CurrentSelection();
		if (index < 0)
			selection = NULL;
		else
			selection = ((TItem*)local_lv->ItemAt(index))->Node();
		count = local_lv->CountItems();
		new_count = 0;
		n = root;
		while (n) {
			visible = (care_options[USED_INDEX] && (n->used != 0)) ||
					  (care_options[RSVD_INDEX] && (n->rsvd != 0)) ||
					  (care_options[MAX_INDEX] && (n->max != 0)) ||
					  (care_options[COPY_INDEX] && (n->copy != 0)) ||
					  (care_options[IN_INDEX] && (n->in != 0)) ||
					  (care_options[OUT_INDEX] && (n->out != 0));
			n->SetVisible(visible);
			if (visible)
				new_count++;
			n = n->next;
		}
		for (index=count; index<new_count; index++)
			local_lv->AddItem(new TItem(NULL));
		if (new_count < count)
			local_lv->RemoveItems(new_count, 10000);
		n = root;
		index = 0;
		while (n != NULL) {
			if (n->Visible()) {
				((TItem*)local_lv->ItemAt(index))->SetNode(n);
				index++;
			}
			n = n->next;
		}
		if (selection) {
			for (index=0; index<new_count; index++)
				if (((TItem*)local_lv->ItemAt(index))->Node()->aid == selection->aid) {
					local_lv->Select(index);
					break;
				}
		}
	}
	if (touched)
		local_lv->Invalidate();
}


BarView::BarView(BRect frame, int32 arrow, BMessage *click_message, uint32 resize_mode) :
BView(frame, "bar", resize_mode,
	  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE) {
	msg = new BMessage(click_message);
	cached_arrow = arrow;
	pressed = false;
	click_session = false;
}

BarView::~BarView() {
	delete msg;
}

void BarView::SetArrow(int32 arrow) {
	if (cached_arrow == arrow)
		return;
	cached_arrow = arrow;
	Invalidate();
}

void BarView::Draw(BRect updateRect) {
	BRect		rect;
	int32		i, count;
	float		x, y, dx, dy;
	BRect		bounds = Bounds();
	
	if (pressed) {
		SetHighColor(156, 154, 156, 255);
		FillRect(bounds);
	}
	else {
		SetHighColor(255, 255, 255, 255);
		MovePenTo(bounds.left, bounds.bottom);
		StrokeLine(BPoint(bounds.left, bounds.top));
		StrokeLine(BPoint(bounds.right, bounds.top));
		SetHighColor(222, 219, 222);
		MovePenTo(bounds.left, bounds.bottom);
		StrokeLine(BPoint(bounds.left+1, bounds.bottom-1));
		MovePenTo(bounds.right, bounds.top);
		StrokeLine(BPoint(bounds.right-1, bounds.top+1));
		rect = bounds;
		rect.InsetBy(1.0, 1.0);
		FillRect(rect);
		SetHighColor(132, 130, 132, 255);
		MovePenTo(bounds.left, bounds.bottom);
		StrokeLine(BPoint(bounds.right, bounds.bottom));
		StrokeLine(BPoint(bounds.right, bounds.top));
	}
	if ((cached_arrow == ARROW_UP) || (cached_arrow == ARROW_DOWN)) {
		if (cached_arrow == ARROW_DOWN) {
			y = bounds.top;
			dy = 1.0;
		}
		else {
			y = bounds.bottom;
			dy = -1.0;
		}
		x = floor((bounds.left+bounds.right)*0.5);
		SetHighColor(0, 0, 0, 255);
		count = (int32)(bounds.bottom-bounds.top)-1;
		y += dy*((count-4)/2);
		for (i=0; i<4; i++) {
			y += dy;
			StrokeLine(BPoint(x-i, y), BPoint(x+1+i, y));
		}
	}
	else if ((cached_arrow == ARROW_LEFT) || (cached_arrow == ARROW_RIGHT)){
		if (cached_arrow == ARROW_LEFT) {
			x = bounds.left;
			dx = 1.0;
		}
		else {
			x = bounds.right;
			dx = -1.0;
		}
		y = floor((bounds.top+bounds.bottom)*0.5);
		SetHighColor(0, 0, 0, 255);
		count = (int32)(bounds.right-bounds.left)-1;
		x += dx*((count-4)/2);
		for (i=0; i<4; i++) {
			x += dx;
			StrokeLine(BPoint(x, y-i), BPoint(x, y+1+i));
		}
	}
}

void BarView::MouseDown(BPoint where) {
	if (cached_arrow == NO_ARROW)
		return;
	pressed = true;
	click_session = true;
	Draw(BRect(0, 0, 1000, 1000));
}

void BarView::MouseUp(BPoint where) {
	if (pressed)
		Window()->PostMessage(msg);
	pressed = false;
	click_session = false;
	Draw(BRect(0, 0, 1000, 1000));
}

void BarView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message) {
	switch (code) {
	case B_ENTERED_VIEW :
		if (click_session) {
			pressed = true;
			Draw(BRect(0, 0, 1000, 1000));
		}
		break;
	case B_EXITED_VIEW :
		if (pressed) {
			pressed = false;
			Draw(BRect(0, 0, 1000, 1000));
		}
		break;
	}
}

ControlView::ControlView(BRect frame, BFont *font) :
BView(frame, "control", B_FOLLOW_LEFT_RIGHT,
	  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE) {
	char		name[2];
	int32		i;
	BRect		rect, rect2;
	rgb_color	color;
	
	SetFont(font);
	
	color.red = color.blue = 0;
	color.green = 80;
	color.alpha = 255;
	rect.Set(5, 4, WIDTH-83, CTRL_HEIGHT/3);
	rect2.Set(2, 2, WIDTH-92, CTRL_HEIGHT/3-8);
	tv = new BTextView(rect, "tv", rect2,
					   B_FOLLOW_LEFT | B_FOLLOW_TOP,
					   B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	tv->MakeEditable(false);
	tv->MakeSelectable(false);
	AddChild(tv);
	
	color.red = 80;
	color.green = 0;
	rect.Set(2, CTRL_HEIGHT-(CTRL_HEIGHT/3)-3, WIDTH-83, CTRL_HEIGHT-3);
	tc = new BTextControl(rect, "tc", NULL, " ", NULL,
						  B_FOLLOW_LEFT | B_FOLLOW_TOP,
						  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(tc);
	tc->TextView()->SetMaxBytes(24);
	
	BMessage msg1(SCROLL_BUTTON);
	rect.Set(WIDTH-80, 3, WIDTH-60, CTRL_HEIGHT/6+2);
	scroll[0] = new BarView(rect, NO_ARROW, &msg1, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(scroll[0]);
	
	msg1.what++;
	rect.Set(WIDTH-80, CTRL_HEIGHT/6+3, WIDTH-60, CTRL_HEIGHT/3+2);
	scroll[1] = new BarView(rect, NO_ARROW, &msg1, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(scroll[1]);
	
	msg1.what++;
	rect.Set(WIDTH-80, CTRL_HEIGHT-(CTRL_HEIGHT/3)-3, WIDTH-60, CTRL_HEIGHT-(CTRL_HEIGHT/6)-4);
	scroll[2] = new BarView(rect, NO_ARROW, &msg1, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(scroll[2]);
	
	msg1.what++;
	rect.Set(WIDTH-80, CTRL_HEIGHT-(CTRL_HEIGHT/6)-3, WIDTH-60, CTRL_HEIGHT-4);
	scroll[3] = new BarView(rect, NO_ARROW, &msg1, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(scroll[3]);
	
	rect.Set(WIDTH-57, 1, WIDTH-3, CTRL_HEIGHT/3+4);
	stop = new BButton(rect, "stop", "Stop", new BMessage(STOP_BUTTON),
					   B_FOLLOW_LEFT | B_FOLLOW_TOP,
					   B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(stop);
	stop->ResizeBy(0.0, -7.0);

	rect.Set(WIDTH+3, 1, WIDTH+67, CTRL_HEIGHT/3+4);
	dump = new BButton(rect, "dump", "Dump", new BMessage(DUMP_BUTTON),
					   B_FOLLOW_LEFT | B_FOLLOW_TOP,
					   B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(dump);
	dump->ResizeBy(0.0, -7.0);

	rect.Set(WIDTH+3, CTRL_HEIGHT/3+5, WIDTH+67, 2*CTRL_HEIGHT/3+8);
	autosave = new BButton(rect, "autosave", "AutoSave", new BMessage(AUTOSAVE_BUTTON),
					    B_FOLLOW_LEFT | B_FOLLOW_TOP,
					    B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(autosave);
	autosave->ResizeBy(0.0, -7.0);

	rect.Set(WIDTH-57, CTRL_HEIGHT-2*(CTRL_HEIGHT/3)+4, WIDTH-3, CTRL_HEIGHT-(CTRL_HEIGHT/3)+1);
	erase = new BButton(rect, "erase", "Delete", new BMessage(ERASE_BUTTON),
					  B_FOLLOW_LEFT | B_FOLLOW_TOP,
					  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(erase);
	erase->ResizeBy(0.0, -7.0);

	rect.Set(WIDTH-57, CTRL_HEIGHT-(CTRL_HEIGHT/3)+1, WIDTH-3, CTRL_HEIGHT-2);
	reset = new BButton(rect, "reset", "Set Now", new BMessage(RESET_BUTTON),
					    B_FOLLOW_LEFT | B_FOLLOW_TOP,
					    B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	AddChild(reset);
	reset->ResizeBy(0.0, -7.0);

	SetViewColor(222, 219, 222, 255);
	SetLowColor(222, 219, 222, 255);
}

static char CurrentStateName[25] = "<<<< Current state >>>>";
static char FrozenStateName[24] = "<<<< Frozen state >>>>";
static char StartLabelText[6] = "Start";
static char StopLabelText[6] = "Stop";
static char EmptyName[1] = "";

void ControlView::Update() {
	bool		more_choice, modified;
	char		*text;
	int32		i;
	BFont		font;
	rgb_color	color;

	modified = false;
	// Check the Stop/Start button
	if (UpdateCurrentState)
		text = StopLabelText;
	else
		text = StartLabelText;
	if (strcmp(stop->Label(), text) != 0) {
		stop->SetLabel(text);
		modified = true;
	}
	// Check the text in the top TextView
	if (PositiveState == -1) {
		if (UpdateCurrentState)
			text = CurrentStateName;
		else
			text = FrozenStateName;
	}
	else 
		text = memo_names[PositiveState];
	if (strcmp(tv->Text(), text) != 0) {
		tv->SetText(text);
		modified = true;
	}
	// Check the scroller on the side
	scroll[0]->SetArrow((PositiveState >= 0)?ARROW_DOWN:NO_ARROW);
	more_choice = (PositiveState < (MEMO_COUNT-1)) && (memos[PositiveState+1] != NULL);
	scroll[1]->SetArrow(more_choice?ARROW_UP:NO_ARROW);
	
	// Check the text in the bottom TextControl
	if (NegativeState == -1)
		text = EmptyName;
	else
		text = memo_names[NegativeState];
	if (strcmp(tc->TextView()->Text(), text) != 0) {
		tc->TextView()->SetText(text);	
		tc->TextView()->MakeEditable((NegativeState != -1));
		tc->TextView()->MakeSelectable((NegativeState != -1));
		modified = true;
	}
	
	// Check the scroller on the side
	scroll[2]->SetArrow((NegativeState >= 0)?ARROW_DOWN:NO_ARROW);
	scroll[3]->SetArrow(((NegativeState == -1) || (memos[NegativeState]!=NULL))?ARROW_UP:NO_ARROW);
	
	// Set the color of the TextEdit
	if (modified) {
		GetFont(&font);
		color.green = 0;
		color.alpha = 255;
		color.red = 0;
		if ((PositiveState == -1) && ((NegativeState == -1) || (memos[NegativeState] == NULL)))
			color.blue = 0;
		else
			color.blue = 192;
		tv->SetFontAndColor(0, 24, &font, B_FONT_ALL, &color);
		color.blue = 0;
		if ((NegativeState == -1) || (memos[NegativeState] == NULL))
			color.red = 0;
		else
			color.red = 192;
		tc->TextView()->SetFontAndColor(0, 24, &font, B_FONT_ALL, &color);
	}
	
	// Check the erase button
	erase->SetEnabled((NegativeState != -1) && (memos[NegativeState]!=NULL));
	
	// Check the autosave button
	autosave->SetEnabled(!((TWindow*)Window())->auto_dump);
	
	// Check the reset button
	reset->SetEnabled(NegativeState != -1);
}

void ControlView::Draw(BRect updateRect) {
	MovePenTo(12, CTRL_HEIGHT/2+3);
	DrawString("compared to      (v1.0)");
}

LineView::LineView(BRect frame, BFont *font) :
BView(frame, "lnv", B_FOLLOW_LEFT_RIGHT,
	  B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE) {
	pressed = false;
	click_session = false;
	expanded = false;	
	SetFont(font);
	SetLowColor(222, 219, 222, 255);
	last_update.top = 0.0;
	last_update.left = 0.0;
	last_update.right = -1.0;
	last_update.bottom = -1.0;
}

void LineView::Draw(BRect updateRect) {
	bool		enable;
	float		x0, dx, x_start_bloc, x_start, x, ymin, ymax, y;
	int32		i, j, cur_bloc, len;
	BRect		bounds = Bounds();

	x_start_bloc = bounds.left;
	x = bounds.left;
	ymin = bounds.top;
	ymax = bounds.bottom;
	y = floor((ymin+ymax)*0.5);
	cur_bloc = 0;
	for (i=0; i<TAB_COUNT; i++) {
		if (TabBlocIndex[i] > cur_bloc) {
			// Draw the buttons...
			SetHighColor(255, 255, 255, 255);
			MovePenTo(x_start_bloc, ymax-1.0);
			StrokeLine(BPoint(x_start_bloc, ymin));
			StrokeLine(BPoint(x-2.0, ymin));
			SetHighColor(132, 130, 132, 255);
			MovePenTo(x_start_bloc+1.0, ymax);
			StrokeLine(BPoint(x-1.0, ymax));
			StrokeLine(BPoint(x-1.0, ymin+1.0));
			cur_bloc = TabBlocIndex[i];
			x_start_bloc = x;
		}
		x_start = x;
		x += NthTabWidth(i);
		// draw an item
		if (x > x_start) {
			// select the drawing color based on the selection
			if ((i == index_item) && pressed) {
				SetLowColor(190, 187, 190, 255);
				FillRect(BRect(x_start+1, ymin+1, x-1, ymax-1), B_SOLID_LOW);
			}
			else
				SetLowColor(222, 219, 222, 255);
			// draw the specific item, if any
			SetHighColor(0, 0, 0, 255);
			switch(TabType[i]) {
			case 0 :	// name
				len = strlen(TabNames[cur_bloc]);
				x0 = (x_start+x)*0.5-3.0*len;
				MovePenTo(x0, ymax-3.0);
				DrawString(TabNames[cur_bloc]);
				if (sorting_index == (i-USED_TAB)/3)
					StrokeLine(BPoint(x0, ymax-2.0), BPoint(x0+6.0*len-1.0, ymax-2.0));
				break;
			case 1 :	// fullname/shortname
				if (display_fullname) {
					x0 = x_start+2.0;
					dx = 1.0;
				}
				else {
					x0 = x_start+6.0;
					dx = -1.0;
				}
			draw_arrow:
				for (j=0; j<4; j++) {
					StrokeLine(BPoint(x0, y-j), BPoint(x0, y+1+j));
					x0 += dx;
				}
				break;
			case 2 :	// enable box
				enable = display[(i-USED_ENABLE_TAB)/3];
				MovePenTo(x_start+2.0, ymax-3.0);
				DrawString(enable?"X":"-");
				break;
			case 3 :	// care box
				enable = care_options[(i-USED_CARE_TAB)/3];
				MovePenTo(x_start+1.0, ymax-3.0);
				DrawString(enable?"?":".");
				break;
			case 4 :	// expand box
				if (expanded) {
					x0 = x_start+4.0;
					dx = 1.0;
				}
				else {
					x0 = x_start+7.0;
					dx = -1.0;
				}
				goto draw_arrow;
			}
		}
	}	
	if (x < bounds.right) {
		// Final part (if any space available)
		SetHighColor(255, 255, 255, 255);
		MovePenTo(x, ymax-1.0);
		StrokeLine(BPoint(x, ymin));
		StrokeLine(BPoint(bounds.right-1.0, ymin));
		SetHighColor(132, 130, 132, 255);
		MovePenTo(x+1.0, ymax);
		StrokeLine(BPoint(bounds.right, ymax));
		StrokeLine(BPoint(bounds.right, ymin+1.0));
	}
}

float LineView::NthTabWidth(int32 index) {
	switch (index) {
	case NAME_TAB :
		if (display_fullname)
			return 82.0+96.0;
		return 82.0;
	case NAME_FULL_TAB :
		return 10.0;
	case USED_ENABLE_TAB :
	case RSVD_ENABLE_TAB :
	case MAX_ENABLE_TAB :
	case SWAP_ENABLE_TAB :
	case COPY_ENABLE_TAB :
	case IN_ENABLE_TAB :
	case OUT_ENABLE_TAB :
	case ID_ENABLE_TAB :
		if (expanded)
			return 7.0;
		return 0.0;
	case USED_TAB :
	case RSVD_TAB :
	case MAX_TAB :
	case SWAP_TAB :
	case COPY_TAB :
	case IN_TAB :
	case OUT_TAB :
		if (display_options[(index-USED_TAB)/3]) {
			if (expanded)
				return 45.0-15.0;
			return 45.0-8.0;
		}
		return 0.0;
	case ID_TAB :
		if (display_options[ID_INDEX]) {
			if (expanded)
				return 45.0-7.0;
			return 45.0;
		}
		return 0.0;
	case USED_CARE_TAB :
	case RSVD_CARE_TAB :
	case MAX_CARE_TAB :
	case SWAP_CARE_TAB :
	case COPY_CARE_TAB :
	case IN_CARE_TAB :
	case OUT_CARE_TAB :
		if (display_options[(index-USED_CARE_TAB)/3])
			return 8.0;
		return 0.0;
	case EXPAND_TAB :
		return 13.0;
	default :
		return 0.0;
	}
}

int32 LineView::TabIndex(BPoint where, BRect *update) {
	float		x_start, x;
	int32		i;
	BRect		bounds = Bounds();

	if ((where.y < bounds.top+1.0) || (where.y > bounds.bottom-1.0))
		goto error;
	x = 0.0;
	for (i=0; i<TAB_COUNT-1; i++) {
		x_start = x;
		x += NthTabWidth(i);
		if ((where.x > x_start) && (where.x < x)) {
			if (TabSelectable[i]) {
				update->top = bounds.top+1.0;
				update->bottom = bounds.bottom-1.0;
				update->left = x_start+1.0;
				update->right = x-1.0;
				last_update = *update;
				return i;
			}
			else
				goto error;
		}
	}
error:
	*update = last_update;
	last_update.top = 0.0;
	last_update.left = 0.0;
	last_update.right = -1.0;
	last_update.bottom = -1.0;
	return -1;
}

void LineView::MouseDown(BPoint where) {
	int32		index;
	BRect		update;
	
	index = TabIndex(where, &update);
	if (index < 0)
		return;
	index_item = index;
	pressed = true;
	click_session = true;
	Invalidate(update);
}

void LineView::MouseUp(BPoint where) {
	int32		i, index;
	BRect		update;
	
	index = TabIndex(where, &update);
	if (pressed && (index == index_item) && (index >= 0))
		switch (index) {
		case NAME_FULL_TAB :
			display_fullname = 1-display_fullname;
			Window()->PostMessage(CHECK_SIZE);
			break;
		case USED_ENABLE_TAB :
		case RSVD_ENABLE_TAB :
		case MAX_ENABLE_TAB :
		case SWAP_ENABLE_TAB :
		case COPY_ENABLE_TAB :
		case IN_ENABLE_TAB :
		case OUT_ENABLE_TAB :
		case ID_ENABLE_TAB :
			index = (index-USED_ENABLE_TAB)/3;
			display[index] = 1-display[index];
			if ((display[0]|display[1]|display[2]|display[3]|display[4]|display[5]|display[6]) == 0)
				display[index] = true;
			Invalidate(update);
			break;
		case USED_CARE_TAB :
		case RSVD_CARE_TAB :
		case MAX_CARE_TAB :
		case SWAP_CARE_TAB :
		case COPY_CARE_TAB :
		case IN_CARE_TAB :
		case OUT_CARE_TAB :
			index = (index-USED_CARE_TAB)/3;
			care_options[index] = 1-care_options[index];
			Invalidate(update);
			Window()->PostMessage(REDRAW);
			break;
		case USED_TAB :
		case RSVD_TAB :
		case MAX_TAB :
		case SWAP_TAB :
		case COPY_TAB :
		case IN_TAB :
		case OUT_TAB :
		case ID_TAB :
			sorting_index = (index-USED_TAB)/3;
			Invalidate();
			Window()->PostMessage(REDRAW);
			break;
		case EXPAND_TAB :
			if (expanded) {
				for (i=0; i<COLUMN_COUNT; i++)
					display_options[i] = display[i];
				expanded = false;
			}
			else {
				for (i=0; i<COLUMN_COUNT; i++) {
					display[i] = display_options[i];
					display_options[i] = true;
				}
				expanded = true;
			}
			Window()->PostMessage(CHECK_SIZE);
			break;
		}
	pressed = false;
	click_session = false;
}

void LineView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message) {
	int32		index;
	BRect		update;
	
	if (!click_session)
		return;
	index = TabIndex(where, &update);
	
	if (index == index_item) {
		if (!pressed) {
			pressed = true;
			Invalidate(update);
		}
	}
	else if (pressed) {
		pressed = false;
		Invalidate(update);
	}
}

TItem::TItem(TNode *node) : BListItem() {
	n = node;
}

void TItem::ConvertToString(char *str, int32 val, int32 index) {
	char	sign;
	float	fval;
	
	if (index < 4) {
		if (val < 0) {
			sign = '-';
			val = -val;
		}
		else
			sign = '+';
		if (val < 102400000)
			sprintf(str, "%7d", val>>10);
		else {
			fval = (1.0/1048576.0)*(float)val;
			if (fval < 99.95)
				sprintf(str, "%7.2f", fval);
			else
				sprintf(str, "%7.1f", fval);	
		}
		while (str[1] == ' ') str++;
		*str = sign;
	}
	else if (index < 7) {
		if (val < 1000000)
			sprintf(str, "%7d", val);
		else {
			fval = (1.0/1000000)*(float)val;
			if (fval < 99.95)
				sprintf(str, "%7.2f", fval);
			else
				sprintf(str, "%7.1f", fval);	
		}
	}
	else
		sprintf(str, "%7d", val);
}

void TItem::DrawItem(BView *owner, BRect itemRect, bool complete) {
	bool		back_needed, selected;
	char		str[8];
	float		x, y;
	int32		i;

	// background	
	selected = IsSelected();
	back_needed = complete | selected;
	owner->SetDrawingMode(B_OP_COPY);
	if (selected)
		owner->SetLowColor(190, 187, 190, 255);
	else
		owner->SetLowColor(222, 219, 222, 255);
	if (back_needed)
		owner->FillRect(itemRect, B_SOLID_LOW);
	// set the drawing color
	if (n->level == 0)
		owner->SetHighColor(0, 0, 0, 255);
	else if (n->level > 0)
		owner->SetHighColor(0, 0, 192, 255);
	else
		owner->SetHighColor(192, 0, 0, 255);
	// draw items following the display_options
	x = itemRect.left+3;
	y = itemRect.bottom-2;
	owner->MovePenTo(itemRect.left+3, itemRect.bottom-2);
	owner->DrawString(n->name, (display_fullname)?31:15);
	x += 91;
	if (display_fullname)
		x += 96;
	for (i=0; i<COLUMN_COUNT; i++)
		if (display_options[i]) {
			owner->MovePenTo(x, y);
			ConvertToString(str, n->selector[i], i);
			owner->DrawString(str);
			x += 45;
		}
}

void TItem::Update(BView *owner, const BFont *font) {
	SetHeight(ITEM_HEIGHT-1);
	SetWidth(WIDTH+4*45);
}

void TNode::Remove(TNode **root) {
	TNode	*p, *n;

	p = prev;
	n = next;
	if (p == NULL)
		*root = n;
	else
		p->next = n;
	if (n != NULL)
		n->prev = p;
}

void TNode::TouchAll() {
	TNode	*n;

	n = this;
	while (n != NULL) {
		n->SetEnable(false);
		n = n->next;
	}
}

bool TNode::CleanUpAll(TNode **root) {
	bool		modified;
	TNode		*n, *n2, *a;

	modified = false;
	n = *root;
	while (n != NULL) {
		if (!n->Enable()) {
			a = n->areas;
			while (a) {
				n2 = a->next;
				delete a;
				a = n2;
			}
			modified = true;
			n2 = n->next;
			n->Remove(root);
			delete n;
			n = n2;
		}
		else
			n = n->next;
	}
	return modified;
}

bool TNode::Sort(TNode **root) {
	bool		modified;
	TNode		*n2;
	int32		sindex;
	
	sindex = sorting_index;
	modified = false;
	n2 = prev;
	if (n2 && (n2->selector[sindex] < selector[sindex])) {
		modified = true;
		Remove(root);
		while (n2->prev && (n2->prev->selector[sindex] < selector[sindex]))
			n2 = n2->prev;
		if (!n2->prev) {
			n2->prev = this;
			next = n2;
			prev = NULL;
			*root = this;
		}
		else {
			next = n2->next;
			prev = n2;
			n2->next = this;
			if (next)
				next->prev = this;
		}
	}
	n2 = next;
	if (n2 && (n2->selector[sindex] > selector[sindex])) {
		modified = true;
		Remove(root);
		while (n2->next && (n2->next->selector[sindex] > selector[sindex]))
			n2 = n2->next;
		prev = n2;
		next = n2->next;
		n2->next = this;
		if (next)
			next->prev = this;
	}
	return modified;
}

TNode *TNode::Copy(TNode **root, int32 sign) {
	TNode		*new_root, *cur, *n;
	
	if (*root == NULL)
		return NULL;
	new_root = NULL;

	n = *root;
	while (n) {
		cur = new TNode();
		memcpy(cur->name, n->name, 32);
		cur->prev = NULL;
		cur->next = new_root;
		if (new_root)
			new_root->prev = cur;
		new_root = cur;
		cur->used = n->used * sign;
		cur->rsvd = n->rsvd * sign;
		cur->max = n->max * sign;
		cur->swap = n->swap * sign;
		cur->copy = n->copy * sign;
		cur->in = n->in * sign;
		cur->out = n->out * sign;
		cur->type = n->type;
		cur->level = sign;
		cur->status = n->status;
		cur->aid = n->aid;
		cur->areas = n->Copy(&n->areas, sign);
		n = n->next;
	}
	return new_root;
}

void TNode::Merge(TNode **root, int32 sign) {
	TNode		*n, *n2;

	n2 = this;
	while (n2) {
		n = *root;
		while (n != NULL) {
			if (n->tid == n2->tid) {
				n->max += n2->max * sign;
				n->rsvd += n2->rsvd * sign;
				n->used += n2->used * sign;
				n->swap += n2->swap * sign;
				n->copy += n2->copy * sign;
				n->in += n2->in * sign;
				n->out += n2->out * sign;
				n->level += sign;
				if (n2->areas)
					n2->areas->Merge(&n->areas, sign);
				goto node_found;
			}
			n = n->next;
		}
		// duplicate the node...
		n = new TNode();
		memcpy(n->name, n2->name, 32);
		n->type = n2->type;
		n->tid = n2->tid;
		n->max = n2->max * sign;
		n->rsvd = n2->rsvd * sign;
		n->used = n2->used * sign;
		n->swap = n2->swap * sign;
		n->copy = n2->copy * sign;
		n->in = n2->in * sign;
		n->out = n2->out * sign;
		n->status = n2->status;
		n->level = sign;
		n->prev = NULL;
		n->next = *root;
		n->areas = n2->Copy(&n2->areas, sign);
		if (*root != NULL)
			(*root)->prev = n;
		*root = n;
		
	node_found:
		n2 = n2->next;
	}	
}

void TNode::SortAll(TNode **root) {
	TNode		*n, *n_min, *new_root;
	int32		sindex;

	sindex = sorting_index;
	new_root = NULL;
	while (*root != NULL) {
		n_min = *root;
		n = n_min->next;
		while (n) {
			if (n->selector[sindex] < n_min->selector[sindex])
				n_min = n;
			n = n->next;
		}
		n_min->Remove(root);
		n_min->prev = NULL;
		n_min->next = new_root;
		if (new_root != NULL)
			new_root->prev = n_min;
		new_root = n_min;
		if (n_min->areas)
			n_min->SortAll(&n_min->areas);
	}
	*root = new_root;
}

bool TNode::Enable() {
	return ((status & ITEM_ENABLE) != 0);
}

bool TNode::Modified() {
	return ((status & ITEM_MODIFIED) != 0);
}

bool TNode::Touched() {
	return ((status & ITEM_TOUCHED) != 0);
}

bool TNode::Visible() {
	return ((status & ITEM_VISIBLE) != 0);
}

void TNode::SetEnable(bool enable) {
	status = (status & ~ITEM_ENABLE);
	if (enable)
		status |= ITEM_ENABLE;
}

void TNode::SetModified(bool modified) {
	status = (status & ~ITEM_MODIFIED);
	if (modified)
		status |= ITEM_MODIFIED;
}

void TNode::SetTouched(bool touched) {
	status = (status & ~ITEM_TOUCHED);
	if (touched)
		status |= ITEM_TOUCHED;
}

void TNode::SetVisible(bool visible) {
	status = (status & ~ITEM_VISIBLE);
	if (visible)
		status |= ITEM_VISIBLE;
}
