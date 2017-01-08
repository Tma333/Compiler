// stack header used to store tokens to be used by RDP

#include <iostream>
#include <string>
using namespace std;

struct NODE
{
	string lexeme;
	string tokentype;
	NODE *next;
	NODE *prev;
};

// display contents of the list
void Display(NODE* p)
{
	cout << "LIST:";
	while (p!=NULL)
	{
		if(p->tokentype !="")
		{
			cout << "[" << p->tokentype << ":" << p->lexeme<< "] --> ";
		}
		p=p->next;
	}
	cout << "NULL\n";
}

// Queue push
void Push(string token, string lexeme, NODE* &head, NODE*&tail)
{
	NODE*p = new(NODE);
	p->tokentype = token;
	p->lexeme = lexeme;
	p->next = NULL;
	if (tail == NULL)
	{
		p->prev=NULL;
		head = p;
		tail =p;
	}
	else
	{
		p->prev = tail;
		tail->next = p;
		tail = p;
	}
}

void Push(string token, NODE* &head, NODE*&tail)
{
	NODE*p = new(NODE);
	p->tokentype = token;
	p->next = NULL;
	if (tail == NULL)
	{
		p->prev=NULL;
		head = p;
		tail =p;
	}
	else
	{
		p->prev = tail;
		tail->next = p;
		tail = p;
	}
}


