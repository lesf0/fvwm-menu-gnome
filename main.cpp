#include <queue>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>
	
int main() {
	GMenuTree* tree=gmenu_tree_new("gnome-applications.menu", GMENU_TREE_FLAGS_NONE);
	
	std::queue<GMenuTreeDirectory*> qu;
	
	char lang[128];
	char* country=0;
	char* modifier=0;
	char* enc=0;
	strcpy(lang,getenv("LANG"));
	for(int i=0;lang[i];++i){
		switch(lang[i]){
			case '_': lang[i]=0; country=lang+i+1; break;
			case '@': lang[i]=0; modifier=lang+i+1; break;
			case '.': lang[i]=0; enc=lang+i+1; break;
		}
	}
	
	char locales[6][128]={"Name","Name","Name","Name","Name",0};
	int loc_i=0;
	if(*lang&&country&&modifier){
		strcpy(locales[loc_i++]+4,(std::string("[")+lang+"_"+country+"@"+modifier+"]").c_str());
	}
	if(*lang&&country){
		strcpy(locales[loc_i++]+4,(std::string("[")+lang+"_"+country+"]").c_str());
	}
	if(*lang&&modifier){
		strcpy(locales[loc_i++]+4,(std::string("[")+lang+"@"+modifier+"]").c_str());
	}
	if(*lang){
		strcpy(locales[loc_i++]+4,(std::string("[")+lang+"]").c_str());
	}
	
	while(!gmenu_tree_load_sync(tree,NULL)){
		usleep(5000);
	}
	qu.push(gmenu_tree_get_root_directory(tree));
	while(!qu.empty()){
		GMenuTreeDirectory*& dir=qu.front();
		const char* name=gmenu_tree_directory_get_menu_id(dir);
		printf("DestroyMenu \"FvwmMenu%s\"\n",name);
		printf("AddToMenu \"FvwmMenu%s\"\n",name);
		
		GMenuTreeIter* it=gmenu_tree_directory_iter(dir);
		GMenuTreeItemType next_type;
		while ((next_type = gmenu_tree_iter_next (it)) != GMENU_TREE_ITEM_INVALID){
			GMenuTreeDirectory* cur_d;
			GMenuTreeEntry* cur_e;
			switch (next_type){
				case GMENU_TREE_ITEM_DIRECTORY:
					cur_d=gmenu_tree_iter_get_directory(it);
					printf("+ \"%s\" Popup \"FvwmMenu%s\"\n",
						gmenu_tree_directory_get_name(cur_d),
						gmenu_tree_directory_get_menu_id(cur_d));
					qu.push(cur_d);
					break;
				case GMENU_TREE_ITEM_ENTRY:
					cur_e=gmenu_tree_iter_get_entry(it);
					GDesktopAppInfo* inf_e=gmenu_tree_entry_get_app_info(cur_e);
					std::string exec=g_desktop_app_info_get_string(inf_e,"Exec");
					int index;
					while((index=exec.find('%'))!=-1){
						exec.erase(index,2);
					}
					int l_i=0;
					char* name=0;
					do{
						name=g_desktop_app_info_get_string(inf_e,locales[l_i]);
						++l_i;
					}while(!name&&locales[l_i][0]);
					printf("+ \"%s\" Exec exec %s\n",name,exec.c_str());
					break;
			}
		}
		gmenu_tree_iter_unref(it);
		putchar('\n');
		
		qu.pop();
	}
	return 0;
}
