#ifndef LIST_H
#define LIST_H


struct node {
    char* id;
    node *next;
};


class list{

	private:

    	node *head;

	public:

    	list();
    	~list();

    	void insert(char* client_id);
        bool isempty();
    	char* pop();
    	bool find(char* client_id);
 
};


#endif