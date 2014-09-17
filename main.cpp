#include <queue>

#include <stdio.h>
#include <unistd.h>

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>
	
int main() {
	GMenuTree* tree=gmenu_tree_new("gnome-applications.menu", GMENU_TREE_FLAGS_NONE);
	while(!gmenu_tree_load_sync(tree,NULL)){
		usleep(100000);
	}
	std::queue<GMenuTreeDirectory*> qu;
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
					//puts(g_desktop_app_info_get_generic_name(inf_e));
					printf("+ \"%s\" Exec exec %s\n",g_desktop_app_info_get_string(inf_e,"Name"),g_desktop_app_info_get_string(inf_e,"Exec"));
					break;
			}
		}
		gmenu_tree_iter_unref(it);
		putchar('\n');
		
		qu.pop();
	}
	return 0;
}
