#include <queue>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define GMENU_I_KNOW_THIS_IS_UNSTABLE //and i dont care
#include <gmenu-tree.h>

#define locale_size 64 //max size of locale string
#define sync_wait_time 5000 //sleep time while gmenu tree loads
#define menu_name "gnome-applications.menu" //xdg menu filename

std::queue<GMenuTreeDirectory*> qu; //directory queue
	
char locales[6][locale_size]={"Name","Name","Name","Name","Name","Name"}; //parameter name, locales adds at position 4

int main() {
	auto tree=gmenu_tree_new(menu_name, GMENU_TREE_FLAGS_NONE); //no need flags, defaults is good enough
	
	{
		char lang[locale_size]; //here we get system locale string
		strcpy(lang,getenv("LANG"));
		char* country=0; //and parse it to 3 strings - lang, country, modifier
		char* modifier=0;
		//char* enc=0; //it could be useful somewhere else. let it be commented
		for(int i=0;lang[i];++i){
			switch(lang[i]){
				case '_':
					lang[i]=0;
					country=lang+i+1;
					break;
				case '@':
					lang[i]=0;
					modifier=lang+i+1;
					break;
				case '.':
					lang[i]=0;
					//enc=lang+i+1;
					break;
			}
		}

		{
			int loc_i=0; //current locale index
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
			locales[loc_i+1][0]=0; //barier to stop in case of buggy .desktop entry
		}
	}
	
	while(!gmenu_tree_load_sync(tree,NULL)){ //wait for load
		usleep(sync_wait_time);
	}
	
	qu.push(gmenu_tree_get_root_directory(tree)); //push root menu dir in queue
	while(!qu.empty()){ //while queue not empty, create menu for first dir
		auto& dir=qu.front();
		
		{
			const char* name=gmenu_tree_directory_get_menu_id(dir); //menu id used as unique fvwm menu suffix
			printf("DestroyMenu \"FvwmMenu%s\"\n",name);
			printf("AddToMenu \"FvwmMenu%s\"\n",name);
		}
		
		auto it=gmenu_tree_directory_iter(dir); //iterator
		decltype(gmenu_tree_iter_next(it)) next_type; //iterator type
		while ((next_type=gmenu_tree_iter_next(it))!=GMENU_TREE_ITEM_INVALID){
			switch (next_type){ 
				case GMENU_TREE_ITEM_DIRECTORY:{ //add directory function popup and queue it
					auto cur=gmenu_tree_iter_get_directory(it);
					printf("+ \"%s\" Popup \"FvwmMenu%s\"\n",
						gmenu_tree_directory_get_name(cur),
						gmenu_tree_directory_get_menu_id(cur));
					qu.push(cur);
					break;
				}
				case GMENU_TREE_ITEM_ENTRY:{
					auto cur=gmenu_tree_iter_get_entry(it);
					auto inf=gmenu_tree_entry_get_app_info(cur);
					std::string exec=g_desktop_app_info_get_string(inf,"Exec"); //get exec command
					int index;
					while((index=exec.find('%'))!=-1){ //delete all % params
						exec.erase(index,2);
					}
					int loc_i=0;
					char* name=0;
					do{ //find localised name in standard specified order
						name=g_desktop_app_info_get_string(inf,locales[loc_i]);
						++loc_i;
					}while(!name&&locales[loc_i][0]);
					printf("+ \"%s\" Exec exec %s\n",name,exec.c_str()); //add menu entry
					break;
				}
				case GMENU_TREE_ITEM_SEPARATOR:{ //add empty menu entry
					printf("+ \"\" Nop");
				}
				default:{ //TODO: add other types if needed
					
				}
			}
		}
		gmenu_tree_iter_unref(it); //unref iterator (dunno why, but why not?)
		printf("\n"); //just for better readability
		
		qu.pop();
	}
	return 0;
}
