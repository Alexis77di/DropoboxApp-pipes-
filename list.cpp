#include <cstring>
#include "list.h"




//-------------------------  Functions for our List  --------------------------//

list::list(){
	
	head = NULL; 

}


list::~list() {

	while(head) {              
    	node *deleteMe = head;
    	head = head->next;  
        delete [] deleteMe->id;   
    	delete deleteMe;      
	}
}

void list::insert(char* client_id){
	node *n = new node();
    n->id=new char [strlen(client_id+1)];
    strcpy(n->id,client_id);             
	n->next = head;       
                        	
	head = n;           
}

bool list::isempty(){
    return head == NULL;
}

char* list::pop(){
    node *n = head;
    head = head->next;
    char* value = n->id;
    delete n;
    return value;
    
}

bool list::find(char* client_id){
	node* current = head;
   	while(current != NULL){
        if(strcmp(current->id ,client_id)==0){ 
            return true;

        }
        current = current->next;
    }
    return false;
}