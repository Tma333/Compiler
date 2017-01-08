#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "Stack.h"
using namespace std;

// symbol table
struct STABLE
{
	string Lexeme;
	int MemLocation;
	string Type;
};
// symbol table of size 300
STABLE *Stable = new STABLE[300];
// symbol table used;
int STable_counter = 0;

// instruction table
struct ITABLE
{
	int Address;
	string Op;
	int Oprnd;
};
// counter for instruction table
int Itable_counter =1;
// instruction table size of 300	
ITABLE *Itable = new ITABLE[300];

// used for jumpstack pops and pushs
int jumpstack[100];
int jumpstackcounter =0;

// create linked list to hold tokens to be used for rdp
NODE *head=NULL;
NODE *tail=NULL;

// line counter for
int linecounter =0;

// memory address for identifers
int memory_address = 1000;

// used for display message toggles
bool displayfalse;
bool DisplayProduction;

bool isspecial(char x);
void lexer(string token);
void readfile(NODE * &list_one, NODE *&list_one_tail);
void go();

// production rules
bool Assign(NODE*&P);
bool Body(NODE*&P);
bool Compound(NODE*&P);
bool Condition(NODE*&P);
bool Declaration(NODE*&P);
bool DeclarationList(NODE*&P);
bool Empty(NODE*&P);
bool Expression(NODE*&P);
bool ExpressionPrime(NODE*&P);
bool Factor(NODE*&P);
bool Function(NODE*&P);
bool FunctionDefinitions(NODE*&P);
bool IDs(NODE*&P);
bool If(NODE*&P);
bool OptDeclarationList(NODE*&P);
bool OptFunctionDefinitions(NODE*&P);
bool OptParameterList(NODE*&P);
bool Parameter(NODE*&P);
bool ParameterList(NODE*&P);
bool Primary (NODE*&P);
bool Qualifier(NODE*&P);
bool Rat15s(NODE*&P);
bool Read(NODE*&P);
bool Relop(NODE*&P);
bool Return(NODE*&P);
bool Statement(NODE*&P);
bool StatementList(NODE*&P);
bool Term(NODE*&P);
bool TermPrime(NODE*&P);
bool While(NODE*&P);
bool Write(NODE*&P);

void DisplayStable();
void DisplayItable();
int ispresent(string checkword);
void gen_instr(string Op, int Oprnd);
void back_patch(int jump_addr);
int pop_jumpstack();
void push_jumpstack(int addr);



int main()
{
	// option for displaying return false productions used
	displayfalse= false;
	DisplayProduction= false;
	
	// toggle production rule display
	/*
	cout << "Turn off production rule output? (y/n) ";
	char ans;
	cin >> ans;
	ans = tolower(ans);
	if (ans != 'n' && ans != 'y')
	{
		cout << "Invalid selection [ " << ans << " ] production rules will be displayed" << endl;
	}
	else if (ans == 'n')
	{DisplayProduction = true;}
	else if (ans == 'y')
	{DisplayProduction= false;}
	cin.ignore();
	*/

	// list locations for someplace for readfile() to put individual tokens
	NODE *list_one = NULL;
	NODE *list_one_tail = NULL;

	// read text files and place into list_one
	readfile(list_one, list_one_tail);

	// call lexer on tokens to determine token type from list_one
	NODE *P;
	P = list_one;
	if (P != NULL)
	{
		lexer(P->tokentype);
		while (P->next != NULL)
		{
			P= P->next;
			lexer(P->tokentype);
		}
	}
	
	// start rats15 by calling go
	go();

	cout << "\nSymbol table and Assembly code listing has been created as \"Table_file.txt\"" << endl;
	DisplayStable();

	DisplayItable();

	system ("pause");
	return 0;
}

int pop_jumpstack()
{
	int temp = jumpstack[jumpstackcounter];
	jumpstackcounter--;
	return temp;
}
void push_jumpstack(int addr)
{
	jumpstackcounter++;
	jumpstack[jumpstackcounter] = addr;
}
void back_patch(int jump_addr)
{
	int addr = pop_jumpstack();
	Itable[addr].Oprnd= jump_addr;
}
void gen_instr(string Op, int Oprnd)
{
	Itable[Itable_counter].Op = Op;
	Itable[Itable_counter].Oprnd = Oprnd;
	Itable[Itable_counter].Address = Itable_counter;
	Itable_counter++;
}
//display symbol table
void DisplayStable()
{
	fstream itable_file;
	itable_file.open("Table_file.txt", ios::out);
	itable_file << "\nSymbol Table\n";
	itable_file << "Mem Location\tIdentifier\tType\n";
	cout << "\nSymbol Table\n";
	cout << "Mem Location\tIdentifier\tType\n";
	for (int i =0; i < STable_counter; i++)
	{
		itable_file << Stable[i].MemLocation << "\t\t" << Stable[i].Lexeme << "\t\t" <<  Stable[i].Type << endl;
		cout << Stable[i].MemLocation << "\t\t" << Stable[i].Lexeme << "\t\t" <<  Stable[i].Type << endl;
		if (Stable[i].Type == "Not Declared")
		{
			cout << "Error: identifer " << Stable[i].Lexeme << " has not been declared" << endl;
			system ("pause");
			exit(1);
		}

	}
}
// display instruction table
void DisplayItable()
{
	fstream itable_file;
	itable_file.open("Table_file.txt", ios::app);
	itable_file << "\nINSTRUCTION TABLE" << endl;
	cout << "\nINSTRUCTION TABLE" << endl;
	for (int i =1; i < Itable_counter; i++)
	{
		itable_file << Itable[i].Address << "\t" << Itable[i].Op<< '\t';
		cout << Itable[i].Address << "\t" << Itable[i].Op<< '\t';
		if (Itable[i].Oprnd != -999)
		{
			itable_file << Itable[i].Oprnd;
			cout << Itable[i].Oprnd;
		}
		itable_file << endl;
		cout << endl;
	}
}
// checks symbol table for existing identifiers.  returns -1 if not found.
int ispresent(string checkword)
{
	// loop through table to check if duplicate exisits
	for (int i =0; i <= STable_counter; i++)
	{
		string temp = Stable[i].Lexeme;
		if (temp == checkword) 
		{
			return Stable[i].MemLocation;
		}
	}
	return -1;
}

// get data from text file and read into lexer break data into tokens;
void readfile(NODE *&list_one, NODE *&list_one_tail)
{
	string sentence;
	string token;

	// read text files for source code
	cout << "Please enter the name of the file to be read: ";
	string textfile ="test_case_three.txt";

	// get text file name to be read from
	getline(cin, textfile);

	fstream f;
	f.open (textfile, ios::in);

	fstream fileout;
	fileout.open("FileOut.txt",ios::out);
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	if (DisplayProduction == true)
	{
		fileout << "TOKEN\t\t\tlexeme\n\n";
		cout << "\nTOKEN\t\t\tlexeme\n\n";
	}
	fileout.close();

	// get data one line at a time
	while(true)
	{
		// keep track of number of lines read
		linecounter++;

		// read a line of text
		getline(f,sentence);

		//empty list for each line read
		head = NULL; tail = NULL;
		
		// no more lines to be read
		if(f.fail())break;
		
		//break sentence up by seperators
		// case for no special chars found 
		size_t found = sentence.find_first_of(":,;@(){}<>{}[]:=*-+/\"| ");
		if (found == -1)
		{
			//lexer(sentence);
			Push(sentence, list_one, list_one_tail);
		}
		
		// break up string into tokens
		else 
		{
			int i =0;
			int startcounter =0;
			while(sentence[i] != NULL)
			{
				//comment handling
				if (sentence[i] == '/' && sentence[i+1] == '*')
				{
					int temp = sentence.find("*/", i+1);
					// case for when */ is not found in string
					if (temp == -1)
					{ 
						while (temp == -1)
						{
							
							getline(f,sentence);
							
							if(f.fail())break;
							linecounter++;
							i = 0;
							temp = sentence.find("*/", i+1);
						}
						
						if (f.eof())
						{
						cout << "ERROR: */ expected for end of comment" << endl; 
						fout << "ERROR: */ expected for end of comment" << endl; 
						int len = sentence.length(); 
						temp = len-2;
						}
					}

					i = temp+1;
					startcounter = i+1;
					//cout << "check start counter position [" << sentence[startcounter] << "] [" << sentence[startcounter+1] << ']' << endl;
				}
				
				// break up string using recognized special characters
				else if (isspecial(sentence[i]))
				{
					// breaks the string into a substring, split at the special character
					token = sentence.substr(startcounter, i - startcounter);

					//call table look up for token
					//lexer(token);
					Push(token, list_one, list_one_tail);
					startcounter= i+1;
					
					// case for 3 char operators ::=
					if (sentence[i] == ':'&& sentence[i+1]==':'&&sentence[i+2]=='=')
					{
						token = sentence.substr(i,3);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+3;
						i +=2;
					}
					// case for 2 char operators ie !=, <=, =>, :=
					else if (sentence[i] == '!' && sentence[i+1] == '=')
					{
						token = sentence.substr(i,2);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+2;
						i +=1;
					}
					else if (sentence[i] == '<' && sentence[i+1] == '=')
					{
						token = sentence.substr(i,2);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+2;
						i +=1;
					}
					else if (sentence[i] == '=' && sentence[i+1] == '>')
					{
						token = sentence.substr(i,2);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+2;
						i +=1;
					}
					else if (sentence[i] == ':' && sentence[i+1] == '=')
					{
						token = sentence.substr(i,2);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+2;
						i +=1;
					}
					else if (sentence[i] == '@' && sentence[i+1] == '@')
					{
						token = sentence.substr(i,2);
						//lexer(token);
						Push(token, list_one, list_one_tail);
						startcounter = i+2;
						i +=1;
					}
					// case for 1 char operators
					else if (sentence[i] != ' ')
					{
						token = sentence[i];

						//call table look up for token
						//lexer(token);
						Push(token, list_one, list_one_tail);
					}
				}
				// increase counter for next char in the string
				i++;
			}
			// handle the remaining portion of the string that contain no special chars
			if (i != startcounter)
			{
				token = sentence.substr(startcounter, i-startcounter);
				//lexer(token);
				Push(token, list_one, list_one_tail);
			}
		}

		fout.close();
	}	

	// close file and terminate program
	f.close();
	if (DisplayProduction == true)
	{
		cout << endl << "FileOut.txt file has now been created with the output. \nProgram terminating." << endl;
	}
}
// table lookup function
void lexer (string token)
{
	// 1 accepting state for int and 3 for real.  Col 0 = integer, col 1 = '.'
	int fsmtable[5][2] = {
		{1,4},
		{1,2}, // 1 accepting state for int
		{3,4},
		{3,4}, // 2 accepting state for real
		{4,4}};

	// fsm table for identifier. col 0 = letter, col 1 = digit,  col 2 = '_'
	int fsmid[7][3] = {
		{1,6,6}, 
		{2,3,4}, // 1 accepting state for idenifier
		{2,3,5}, // 2 accepting state for idenifier
		{2,3,5}, // 3 accepting state for idenifier
		{2,3,5},
		{2,3,5},
		{6,6,6}};

	// set counter states to zero
	int statecounter =0;
	int idstatecounter =0;
	int acceptstate = 0;
	int col =0;
	int idaccepsate =0;
	int idcol =0;

	//state movement
	int i=0;
	while (token[i]!= NULL)
	{
		// if it digit check, check digit collum for next state
		if (isdigit(token[i]))
		{col = 0;idcol = 1;}
		// if it is '.' char, check '.' collum for next state
		else if (token[i] == '.')
		{col = 1;idstatecounter = 6;}
		// if it is '_' char, check '_' collum for next state
		else if (token[i] == '_')
		{idcol = 2;statecounter = 4;}
		// if it is a letter, check letter collum for next state
		else if (isalpha(token[i]))
		{idcol = 0;statecounter = 4;}
		// if it not a valid input, move to invalid input state
		else
		{statecounter = 4;idstatecounter = 6;}
		statecounter = fsmtable[statecounter][col];
		idstatecounter = fsmid[idstatecounter][idcol];
		++i;
	}
	
	//finished checking the states for the token now calling statetoken which is the display and file output function
	
	fstream fout;
	fout.open("FileOut.txt",ios::app);
	
	//display for identifiers and keywords
	if (idstatecounter == 1 || idstatecounter == 2 || idstatecounter == 3)
	{
		// check for keywords which is a subset of identifiers; list of keywords
		if (token == "while"|| token == "else" || token == "if" || token == "int" || token == "boolean" || token == "real" || token == "endif" || token == "read" || token == "write" || token == "return" || token == "true" || token == "false" || token == "function")
		{
			Push("keyword", token, head, tail);
			if (DisplayProduction == false)
			{
				//cout << "keyword\t\t" << token << endl;
				//fout << "keyword\t\t" << token << endl;
			}
		}
		// if not a keyword then it is an identifiers
		else
		{
			Push("identifier", token, head, tail);
			
			//check to see if identifier is already in symbol table and place into symbol table
			int temp = ispresent(token);
			if (temp == -1)
			{
				// place into symbol table
				Stable[STable_counter].Lexeme = token;
				Stable[STable_counter].MemLocation = memory_address;
				Stable[STable_counter].Type = "Not Declared";
				// increase counters
				STable_counter++;
				memory_address++;
				if (DisplayProduction == false)
				{
					//cout << "identifier\t" << token << endl;
					//fout << "identifier\t" << token << endl;
				}
			}
		}
	}
	
	//unknown and seperator display
	else if (idstatecounter != 1 && idstatecounter != 2 && idstatecounter != 3 && statecounter== 4)
	{
		// display for seperators
		if(token==","||token == "(" ||token == ")"||token == "{"||token == "}"||token=="["||token=="]"||token=="@@"||token==","||token==";"||token==":"||token=="\t")
		{
			if (DisplayProduction == false)
			{
				//cout << "seperator\t" << token << endl;
				//fout << "seperator\t" << token << endl;
			}
			Push("seperator", token,head, tail);

		}

		// display for operators
		else if (token=="="||token == "<"||token == ">"||token == "<="||token == "=>"||token=="=="||token=="!="||token ==":="||token == "::=" || token == "|"||token=="+"||token=="-"||token=="*"||token=="/")
		{
			if (DisplayProduction == false)
			{
				//cout << "operator\t" << token << endl;
				//fout << "operator\t" << token << endl;
			}
			Push("operator",token,head, tail);
		}

		// display for unknowns
		else
		{
			if (DisplayProduction == false)
			{
				//cout << "unknown\t\t" << token << endl;
				//fout << "unknown\t\t" << token << endl;
			}
			cout << "ERROR: Unknown token \"" << token << "\" at line number " << linecounter << endl;
			fout << "ERROR: Unknown token \"" << token << "\" at line number " << linecounter << endl;
			Push("unknown", token,head, tail);
		}
	}
	
	//display for integer and real
	if (statecounter == 1)
	{
		Push("integer",token, head, tail);
		if (DisplayProduction == false)
		{
			//cout << "integer\t\t" << token << endl;
			//fout << "integer\t\t" << token << endl;
		}
	}
	else if (statecounter==3)
	{
		Push("real", token,head, tail);
		if (DisplayProduction == false)
		{
			//cout << "real\t\t" << token << endl;
			//fout << "real\t\t" << token << endl;
		}
	}

	// close file
	fout.close();
	
}
// speical char list used to markers to break up string
bool isspecial(char x)
{
	if (isspace(x)||x=='('||x==')'||x=='\0'||x =='<'||x=='>'||x==':'||x=='|'||x=='{'||x=='}'||x=='['||x==']'||x=='*'||x=='+'||x=='-'||x=='/'||x=='!'||x=='='||x=='@'||x==';'||x==',')
	{return true;}

	else 
	{return false;}
}
// being rdp with Rat15s()
void go ()
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE* P = head;
	if (P!=NULL)
	{
		bool parsed = Rat15s(P);

		// messages depending on result parsed line
		if (parsed == false)
		{
			fout << "ERROR PARSING LINE " << linecounter << endl;
			cout << "ERROR PARSING LINE " << linecounter << endl;
		}
		else
		{
			fout << linecounter << " line(s) parsed" << endl;
			cout << linecounter << " line(s) parsed" << endl;
		}
	}
	fout.close();
}
	
/* PRODUCTION RULES SECTION */
// <Assign> -> <Identifier> := <Expression> 
bool Assign(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	NODE*S = P;
	if(T->next!=NULL)
	{T=T->next;}
	if (P->tokentype== "identifier" && T->lexeme ==":=")
	{
		string Save = P->lexeme;
		if(T->next!=NULL)
		{T=T->next;}
		bool expressionbool = Expression(T);
		if (expressionbool == true)
		{
			int addr = ispresent(Save);
			gen_instr("POPM", addr);

			if (T->lexeme== ";")
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					S = S->next;
					cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Assign> -> <Identifier> := <Expression> ;" << endl;
					fout << "\t<Assign> -> <Identifier> := <Expression> ;" << endl;
				}
				return true; 
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Assign() false;" << endl;} return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Assign() false;" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Assign() false;" << endl;}return false;}
	fout.close();
}
// <Body> -> { <Statement List> }
bool Body(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T=P;
	NODE*S=P;
	if (T->lexeme== "{")
	{
		if (T->next !=NULL)
		{T = T->next;}
		bool statementlistbool = StatementList(T);
		if (statementlistbool == true)
		{
			if (T->lexeme == "}")
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Body> -> { <Statement List> }" << endl;
					fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << "\t<Body> -> { <Statement List> }" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Body() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Body() false;" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Body() false;" << endl;}return false;}
	fout.close();
}
// <Compound> -> { <Statement List> }
bool Compound(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*S = P;
	NODE*T = P;
	if (P->lexeme == "{")
	{
		if (T->next!=NULL)
		{T=T->next;}
		bool statementlistbool = StatementList(T);
		if (statementlistbool== true)
		{
			if (T->lexeme== "}")
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Compound> -> { <Statement List> }" << endl;
					fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << "\t<Compound> -> { <Statement List> }" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Compound() false:" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Compound() false:" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Compound() false:" << endl;}return false;}
	fout.close();
}
// <Condition> -> <Expression> <Relop> <Expression>
bool Condition(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	bool expressionbool = Expression(T);
	if (expressionbool == true)
	{
		bool relopbool = Relop(T);
		if (relopbool == true)
		{
			string Op = T->lexeme;
			if (T->next !=NULL)
			{T=T->next;}
			bool expressionbool2 = Expression(T);
			if(expressionbool2 == true)
			{
				if (Op == "<")
				{
					gen_instr("LES", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				else if (Op == ">")
				{
					gen_instr("GRT", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				else if (Op == "=")
				{
					gen_instr("EQU", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				else if (Op == "!=")
				{
					gen_instr("GRT", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);

					gen_instr("LES", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				else if (Op == "=>")
				{
					gen_instr("EQU", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);

					gen_instr("GRT", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				else if (Op == "<=")
				{
					gen_instr("LES", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);

					gen_instr("EQU", -999);
					push_jumpstack(Itable_counter);
					gen_instr("JUMPZ", -999);
				}
				P=T;
				if (DisplayProduction == true)
				{
					cout << "\t<Condition> -> <Expression> <Relop> <Expression>" << endl;
					fout << "\t<Condition> -> <Expression> <Relop> <Expression>" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Condition() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Condition() false;" << endl;}return false;}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Condition() false;" << endl;}return false;}
	fout.close();
}
// <Declaration> -> <Qualifier> <IDs>
bool Declaration(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	bool qualifierbool = Qualifier(P);
	if (P->next == NULL)
	{return false;}
	else
	{
		NODE *T = P->next;
		bool idbool = IDs(T);
		if(qualifierbool==true&&idbool==true)
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout <<"\t<Declaration> -> <Qualifier> <IDs>"<< endl;
				fout <<"\t<Declaration> -> <Qualifier> <IDs>"<< endl;
			}
			return true;
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Declaration() false;" << endl;}return false;}
	}
	fout.close();
}
// <Declaration List> -> <Declaration> ; <Declaration List> | <Declaration> ;
bool DeclarationList(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	bool declarationbool = Declaration(T);
	if (declarationbool==true)
	{
		if (T->next!=NULL)
		{T=T->next;}
		if (T->lexeme==";")
		{
			if (T->next != NULL)
			{T=T->next;}
			bool declist = DeclarationList(T);
			if (declist == true)
			{
				P= T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
					cout << "\t<Declaration List> -> <Declaration> ; <Declaration List>" << endl;
					fout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
					fout << "\t<Declaration List> -> <Declaration> ; <Declaration List>" << endl;
				}
				return true;
			}
			else
			{
				P=T->prev;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Declaration List> -> <Declaration> ;" << endl;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << "\t<Declaration List> -> <Declaration> ;" << endl;
				}
				return true;
			}
		}
		else
		{
			if (displayfalse == true) 
			{
				cout << "FALSE: Declaration List() false;" << endl;
			}
			return false;
		}
	}
	else
	{
		if (displayfalse == true) 
		{
			cout << "FALSE: Declaration List() false;" << endl;
		}
		return false;
	}
	fout.close();
}
// <Empty> -> E
bool Empty(NODE *&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	if (DisplayProduction == true)
	{
		cout << "\t<Empty> -> E" << endl;
		fout << "\t<Empty> -> E" << endl;
	}
	return true;
	fout.close();
}
// <Expression> -> <Term> <ExpressionPrime>
bool Expression(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE *T=P;
	bool Termbool = Term(T);
	if (Termbool == true)
	{
		bool Expressionbool = ExpressionPrime(T);
		if (Expressionbool == true)
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << "\t<Expression> -> <Term> <ExpressionPrime>" << endl;
				fout << "\t<Expression> -> <Term> <ExpressionPrime>" << endl;
			}

			return true;
		}
		else
		{
			if (DisplayProduction == true)
			{
				cout << "\t<Expression> -> <Term> <ExpressionPrime>" << endl;
				fout << "\t<Expression> -> <Term> <ExpressionPrime>" << endl;
			}
			return true;
		}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Expression() false;" << endl;}return false;}
	fout.close();
}
//<Expression Prime> -> + <Term> <Expression Prime> | - <Term> <Expression Prime> | E
bool ExpressionPrime(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	if (T->lexeme == "+")
	{
		if(T->next!=NULL)
		{T=T->next;}
		bool termbool = Term(T);
		if (termbool== true)
		{
			bool expressionbool= ExpressionPrime(T);
			if (termbool == true)
			{
				// instruction table add
				gen_instr("ADD", -999);

				P=T;
				if (DisplayProduction == true)
				{
					NODE *S = T;
					S = S->prev;
					cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << "+" << endl;
					cout << "\t<Expression Prime> -> + <Term> <Expression Prime>" << endl;
					fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << "+" << endl;;
					fout << "\t<Expression Prime> -> + <Term> <Expression Prime>" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Expression Prime() false;" << endl;}return false;}
		} 
		else 
		{if (displayfalse == true) {cout << "FALSE: Expression Prime() false;" << endl;}return false;}

	}
	else if (T->lexeme == "-")
	{
		if(T->next!=NULL)
		{T=T->next;}
		bool termbool = Term(T);
		if (termbool== true)
		{
			bool expressionbool= ExpressionPrime(T);
			if (termbool == true)
			{
				// instruction table sub
				gen_instr("SUB", -999);

				P=T;
				if (DisplayProduction == true)
				{
					NODE *S = T;
					S = S->prev;
					cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << "-" << endl;
					cout << "\t<Expression Prime> -> - <Term> <Expression Prime>" << endl;
					fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << "-" << endl;
					fout << "\t<Expression Prime> -> - <Term> <Expression Prime>" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Expression Prime() false;" << endl;}return false;}
		} 
		else 
		{if (displayfalse == true) {cout << "FALSE: Expression Prime() false;" << endl;}return false;}

	}
	else
	{
		if (DisplayProduction == true)
		{
			cout << "\t<TermPrime> -> E " << endl;
			fout << "\t<TermPrime> -> E " << endl;
		}
		return true;
	}
	fout.close();
}
// <Factor> -> - <Primary> | <Primary>
bool Factor(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	if (P->lexeme == "-")
	{
		if(P->next==NULL)
		{return false;}
		else
		{
			NODE*T = P->next;
			bool primarybool = Primary(T);
			if (primarybool == true)
			{
				// instruction table pushm
				int addr;
				if (P->tokentype == "integer")
				{
					addr = atoi(P->lexeme.c_str());
					gen_instr("PUSHI", addr);
				}
				else
				{
					addr = ispresent(P->lexeme);
					gen_instr("PUSHM", addr);
				}
				

				NODE*S = P;
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Factor> -> - <Primary>" << endl;
					fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << "\t<Factor> -> - <Primary>" << endl;
				}
				return true;
			}
			else 
			{if (displayfalse == true) {cout << "FALSE: Factor() false;" << endl;}return false;}
		}
	}
	else
	{
		bool primarybool = Primary(P);
		if (primarybool == true)
		{
			// instruction table pushm
			int addr;
			if (P->tokentype == "integer")
			{
				addr = atoi(P->lexeme.c_str());
				gen_instr("PUSHI", addr);

			}
			else
			{
				addr = ispresent(P->lexeme);
				gen_instr("PUSHM", addr);
			}

			if (DisplayProduction == true)
			{
				cout << "\t<Factor> -> <Primary>" << endl;
				fout << "\t<Factor> -> <Primary>" << endl;
			}
			return true;
		}
		else 
		{if (displayfalse == true) {cout << "FALSE: Factor() false;" << endl;}return false;}
	}
	fout.close();
}
// <Function> -> function <Identifier> [ <Opt Parameter List> ] <Opt Declaration List> <Body>
bool Function(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*S = P;
	NODE*T = P;
	NODE*U = T;
	if(T->next!=NULL)
	{T=T->next;}
	if(T->next!=NULL)
	{U=T->next;}
	if (P->lexeme=="function"&&T->tokentype=="identifier"&&U->lexeme=="[")
	{
		if(U->next!=NULL)
		{U=U->next;}
		bool optparalistbool = OptParameterList(U);
		if (optparalistbool == true)
		{
			if (U->lexeme == "]")
			{
				if(U->next!=NULL)
				{U=U->next;}
				bool optdeclartionlistbool = OptDeclarationList(U);
				if(optdeclartionlistbool==true)
				{
					if(U->next!=NULL)
					{U=U->next;}
					bool bodybool = Body(U);
					if(bodybool==true)
					{
						P=U;
						if (DisplayProduction == true)
						{
							cout << endl << "Token: " << "Separator" << "\tlexeme: " << "[";
							cout << endl << "Token: " << "Separator" << "\tlexeme: " << "]";
							cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme << endl;;
							cout << "\t<Function> -> function <Identifier> [ <Opt Parameter List> ] <Opt Declaration List> <Body>" << endl;
							fout << endl << "Token: " << "Separator" << "\tlexeme: " << "[";
							fout << endl << "Token: " << "Separator" << "\tlexeme: " << "]";
							fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme << endl;;
							fout << "\t<Function> -> function <Identifier> [ <Opt Parameter List> ] <Opt Declaration List> <Body>" << endl;
						}
						return true;
					}
					else
					{if (displayfalse == true) {cout << "FALSE: Function() 5 false" << endl;}return false;}
				}
				else
				{if (displayfalse == true) {cout << "FALSE: Function() 4 false" << endl;}return false;}
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Function() 3  false" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Function() 2 false" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Function() 1 false" << endl;}return false;}
	fout.close();
}
// <Function Definitions> -> <Function> <Function Definition> | <Function> 
bool FunctionDefinitions(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	bool functionbool = Function(T);
	if (functionbool== true)
	{
		if (T->next != NULL)
		{T=T->next;}
		if (T->lexeme== "function")
		{
			bool functiondefbool = FunctionDefinitions(T);
			if (functiondefbool== true)
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << "\t<Function Definitions> -> <Function> <Function Definition>" << endl;
					fout << "\t<Function Definitions> -> <Function> <Function Definition>" << endl;
				}
				return true;
			}
		}
		else
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << "\t<Function Definitions> -> <Function>" << endl;
				fout << "\t<Function Definitions> -> <Function>" << endl;
			}
			return true;
		}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Function Definitions() false;"<< endl;}return false;}
	fout.close();
}
// <IDs> -> <Identifier> | <IDs> -> <Identifier> , <IDs>
bool IDs(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	if(T->next != NULL)
	{T= T->next;}
	if (P->tokentype=="identifier" && T->lexeme!=",")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<IDs> -> <Identifier>" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<IDs> -> <Identifier>" << endl;
		}
		return true;
	}
	else if (P->tokentype=="identifier" && T->lexeme==",")
	{
		if (T->next == NULL)
		{return false;}
		else
		{
			T=T->next;
			bool idbool = IDs(T);
			if(idbool == true)
			{
				NODE*S = P;
				P=T;

				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
					S=S->next;
					cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme<< endl;
					fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme<< endl;
					cout << "\t<IDs> -> <Identifer> , <IDs>" << endl;
					fout << "\t<IDs> -> <Identifer> , <IDs>" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: IDs() false;" << endl;}return false;}
		}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: IDs() false;" << endl;}return false;}
	fout.close();
}
// <If> -> if ( <Condition> ) <Statement> endif | if ( <Condition> ) <Statement> else <Statement> endif
bool If(NODE*&P)
{
	fstream fout; 
	fout.open("FileOut.txt",ios::app);

	NODE*T=P;
	NODE*S =P;
	if(T->next!=NULL)
	{T=T->next;}
	if (P->lexeme== "if" && T->lexeme== "(")
	{
		if(T->next!=NULL)
		{T=T->next;}
		bool conditionbool = Condition(T);
		if (conditionbool == true)
		{

			if (T->lexeme== ")")
			{
				if(T->next!=NULL)
				{T=T->next;}
				bool statementbool = Statement(T);
				if (statementbool==true)
				{

					// SA call backpatch
					int addr = Itable_counter;


					if(T->next!=NULL)
					{T=T->next;}
					if (T->lexeme=="endif")
					{
						back_patch(addr);
						back_patch(addr);

						P=T;
						if (DisplayProduction == true)
						{
							cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
							cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
							cout << "\t<If> -> if ( <Condition> ) <Statement> endif" << endl;
							fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
							fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
							fout << "\t<If> -> if ( <Condition> ) <Statement> endif" << endl;
						}
						return true;
					}

					else if (T->lexeme == "else")
					{
						//SA instruction 
						int addr2 = Itable_counter; 
						gen_instr("JUMP", -999);


						if(T->next!=NULL)
						{T=T->next;}
						statementbool = Statement(T);
						if (statementbool == true)
						{
							if(T->next!=NULL)
							{T=T->next;}

							// SA call backpatch
							back_patch(addr+1);
							back_patch(addr+1);
							gen_instr("LABEL", -999);

							if (T->lexeme == "endif")
							{
								P=T;
								if (DisplayProduction == true)
								{
									cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
									cout << "\t<If> -> if ( <Condition> ) <Statement> else <Statement> endif" << endl;
									fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
									fout << "\t<If> -> if ( <Condition> ) <Statement> else <Statement> endif" << endl;
								}
								return true;
							}
							else
							{if (displayfalse == true) {cout << "1FALSE: IF() false;" << endl;}return false;}
						}
						else 
						{if (displayfalse == true) {cout << "2FALSE: IF() false;" << endl;}return false;}
					}
					else
					{if (displayfalse == true) {cout << "3FALSE: IF() false;" << endl;}return false;}
				}
				else 
				{if (displayfalse == true) {cout << "4FALSE: IF() false;" << endl;}return false;}
			}
			else
			{if (displayfalse == true) {cout << "5FALSE: IF() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "6FALSE: IF() false;" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: IF() false;" << endl;}return false;}
	fout.close();
}
// <Opt Declaration List> -> <Declaration List> | <Empty>
bool OptDeclarationList(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	bool declarationbool = DeclarationList(P);
	if (declarationbool == true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Opt Declaration List> -> <Declaration List>" << endl;
			fout << "\t<Opt Declaration List> -> <Declaration List>" << endl;
		}
		return true;
	}
	else 
	{
		if (displayfalse == true) {cout << "FALSE: Opt Declaration List() false;"<< endl;}
		return false;
	}
	fout.close();
}
// <Opt Function Definition> -> <Function Definitions> | <Function>
bool OptFunctionDefinitions(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	bool functiondefbool = FunctionDefinitions(P);
	if (functiondefbool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Opt Function Definition> -> <Function Definitions>" << endl;
			fout << "\t<Opt Function Definition> -> <Function Definitions>" << endl;
		}
		return true;
	}
	else
	{
		if (displayfalse == true) {cout << "FALSE: Opt Function List() false;"<< endl;}
		return false;
	}
	fout.close();
}
// <Opt Parameter List> -> <Parameter List> | <Empty>
bool OptParameterList(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	bool parameterbool = ParameterList(P);
	if (parameterbool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Opt Parameter List> -> <Parameter List>" << endl;
			fout << "\t<Opt Parameter List> -> <Parameter List>" << endl;
		}
		return true;
	}
	else
	{
		bool emptybool = Empty(P);
		if(emptybool== true)
		{
			if (DisplayProduction == true)
			{
				cout << "\t<Opt Parameter List> -> <Empty>" << endl;
				fout << "\t<Opt Parameter List> -> <Empty>" << endl;
			}

			return true;
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Opt Parameter List() false;"<< endl;}return false;}
	}
	fout.close();
}
// <Parameter> -> <IDs> : <Qualifier>
bool Parameter(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T =P;
	bool idbool = IDs(T);
	if (idbool==true)
	{
		if(T->next!=NULL)
		{T=T->next;}
		if(T->lexeme == ":")
		{
			if(T->next!=NULL)
			{T=T->next;}
			bool qualiferbool = Qualifier(T);
			if (qualiferbool== true)
			{
				P=T;
				if (DisplayProduction == true)
				{
					T = T->prev;
					NODE *S = T->prev;
					cout << endl << "Token: " << T->tokentype << "\tlexeme: " << T->lexeme << endl;
					cout << "\t<Parameter> -> <IDs> : <Qualifier>" << endl;
					fout << endl << "Token: " << T->tokentype << "\tlexeme: " << T->lexeme << endl;
					fout << "\t<Parameter> -> <IDs> : <Qualifier>" << endl;
				}
				return true;
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Parameter() false"<< endl;}return false;}
		}
		else 
		{if (displayfalse == true) {cout << "FALSE: Parameter() false"<< endl;}return false;}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Parameter() false"<< endl;}return false;}
	fout.close();
}
// <Parameter List> -> <Parameter> , <Parameter List> | <Parameter>
bool ParameterList(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T =P;
	bool parameterbool = Parameter(T);
	if (parameterbool==true)
	{
		if(T->next!=NULL)
		{T=T->next;}
		if(T->lexeme==",")
		{
			if(T->next!=NULL)
			{T=T->next;}
			bool parameterlistbool = ParameterList(T);
			if(parameterlistbool== true)
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << "Separator" << "\tlexeme: " << "," << endl;
					cout << "\t<Parameter List> -> <Parameter> , <Parameter List>" << endl;
					fout << endl << "Token: " << "Separator" << "\tlexeme: " << "," << endl;
					fout << "\t<Parameter List> -> <Parameter> , <Parameter List>" << endl;
				}
				return true;
			}
		}
		else if (T->lexeme!= ",")
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << "\t<Parameter List> -> <Parameter>" << endl;
				fout << "\t<Parameter List> -> <Parameter>" << endl;
			}
			return true;
		}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Parameter List() false;"<< endl;}return false;}
	fout.close();
}
// <Primary> -> <Identifier> [ <IDs> ] | <Identifier> | <Integer> | ( <Expression> ) | <Real> | true | false
bool Primary (NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	if (T->next!=NULL)
	{T = T->next;}
	if (P->tokentype == "identifier"&&T->lexeme=="[")
	{
		if (T->next==NULL)
		{return false;}
		else
		{
			T=T->next;
			bool idbool = IDs(T);
			if (idbool== true)
			{
				if (T->next==NULL)
				{return false;}
				else
				{
					T=T->next;
					if (T->lexeme=="]")
					{
						NODE*S = P;
						P=T;
						if (DisplayProduction == true)
						{
							cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
							fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
							S = S->next;
							cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme << endl;
							fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme << endl;
							cout << "\t<Primary> -> <Identifier> [ <IDs> ]" << endl;
							fout << "\t<Primary> -> <Identifier> [ <IDs> ]" << endl;
						}
						return true;
					}
					else
					{if (displayfalse == true) {cout << "FALSE: Primary() false;" << endl;}return false;}
				}
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Primary() false;" << endl;}return false;}
		}
	}
	else if (P->lexeme =="(")
	{
		if (P->next!=NULL)
		{T=P->next;}
		bool expressionbool = Expression(T);
		if (expressionbool==true)
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
				cout << "\t<Primary> -> ( <Expression> )" << endl;
				fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
				fout << "\t<Primary> -> ( <Expression> )" << endl;
			}
			return true;
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Primary() false;" << endl;}return false;}
	}
	else if (P->tokentype=="identifier"&&T->lexeme!="[")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Primary> -> <Identifier>" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Primary> -> <Identifier>" << endl;
		}
		return true;
	}
	else if (P->tokentype == "integer")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Primary> -> <Integer>" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Primary> -> <Integer>" << endl;
		}
		return true;
	}
	else if (P->tokentype == "real")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Primary> -> <Real>" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Primary> -> <Real>" << endl;
		}
		return true;
	}
	else if (P->lexeme == "true")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Primary> -> ture" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Primary> -> ture" << endl;
		}
		return true;
	}
	else if (P->lexeme == "false")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Primary> -> false" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Primary> -> false" << endl;
		}
		return true;
	}
	else
	{if (displayfalse == true) {cout << "Primary() false;" << endl;}return false;}
	fout.close();
}
// <Qualifer> -> boolean | int | real
bool Qualifier(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	if(P->lexeme == "int")
	{
		NODE *T = P;
		// filling type for symbol table
		while (T->lexeme != ";")
		{
			if (T->next!=NULL)
			{T = T->next;}
			int tempint = ispresent(T->lexeme);
			if (tempint >0 )
			{
				tempint -= 1000;
				if (Stable[tempint].Type == "Not Declared")
				{
					Stable[tempint].Type= "integer";
				}
				else if (Stable[tempint].Type != "integer")
				{
					cout << "Error: identifer " << T->lexeme << " already declared as " << Stable[tempint].Type << endl;
					system("pause");
					exit(1);
				}
			}
		}
			
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			cout << "\t<Qualifier> -> int" << endl;
			fout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			fout << "\t<Qualifier> -> int" << endl;
		}
		return true;
	}
	else if (P->lexeme=="boolean")
	{
		NODE *T = P;
		while (T->lexeme != ";")
		{
			if (T->next!=NULL)
			{T = T->next;}
			int tempint = ispresent(T->lexeme);
			if (tempint >0 )
			{
				tempint -= 1000;
				if (Stable[tempint].Type == "Not Declared")
				{
					Stable[tempint].Type= "Boolean";
				}
				else if (Stable[tempint].Type != "Boolean")
				{
					cout << "Error: identifer " << T->lexeme << " already declared as " << Stable[tempint].Type << endl;
					system("pause");
					exit(1);
				}
			}
		}
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			cout << "\t<Qualifer> -> boolean" << endl;
			fout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			fout << "\t<Qualifer> -> boolean" << endl;
		}
		return true;
	}
	else if (P->lexeme=="real")
	{
		NODE *T = P;
		while (T->lexeme != ";")
		{
			if (T->next!=NULL)
			{T = T->next;}
			int tempint = ispresent(T->lexeme);
			if (tempint >0 )
			{
				tempint -= 1000;
				if (Stable[tempint].Type == "Not Declared")
				{
					Stable[tempint].Type= "real";
				}
				else if (Stable[tempint].Type != "real")
				{
					cout << "Error: identifer " << T->lexeme << " already declared as " << Stable[tempint].Type << endl;
					system("pause");
					exit(1);
				}
			}
		}
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			cout << "\t<Qualifier> -> real" << endl;
			fout << endl << "Token: " << P->tokentype << "\t\tlexeme: " << P->lexeme << endl;
			fout << "\t<Qualifier> -> real" << endl;
		}
		return true;
	}
	else
	{if (displayfalse == true) {cout << "Qualifer() false;" << endl;}return false;}
	fout.close();
}
// <Rat15s> -> <Opt Function Definitions> | <Opt Declaration List> | <Statment List>
bool Rat15s(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*A = P;
	NODE*B = P;
	NODE*C = P;


	//remove <Function Definitions> for project 3
	/*
	bool optfundefbool = OptFunctionDefinitions(A);
	if (optfundefbool==true)
	{
	*/
		if (A->lexeme == "@@")
		{
			A= A->next;
			if (A->lexeme == "@@")
			{
				A=A->next;
				bool statementlistbool =StatementList(A);
				if (statementlistbool==true)
				{
					P=A;
					return true;
				}
				else
				{
					if (displayfalse == true) 
					{cout << "FALSE: Rat15s() false;" << endl;}
					return false;
				}
			}
			bool optdeclarationlistbool = OptDeclarationList(A);
			if(optdeclarationlistbool==true)
			{
				A=A->next;
				if (A->lexeme == "@@")
				{
					A=A->next;
					bool statementlistbool =StatementList(A);
					if (statementlistbool==true)
					{
						P=A;
						return true;
					}
					else
					{
						if (displayfalse == true) 
						{cout << "FALSE: Rat15s() false;" << endl;}
						return false;
					}
				}
				else
				{
					if (displayfalse == true) 
					{cout << "FALSE: Rat15s() false;" << endl;}
					return false;
				}
			}
			else
			{
				if (displayfalse == true) 
				{cout << "FALSE: Rat15s() false;" << endl;}
				return false;
			}
		}
		else
		{
			if (displayfalse == true) 
			{cout << "FALSE: Rat15s() false;" << endl;}
			return false;
		}
	
	//remove <Function Definitions> for project 3
	/*	
	}
	else
	{
		if (displayfalse == true) 
		{cout << "FALSE: Rat15s() false;" << endl;}
		return false;
	}
	*/

	fout.close();
}
// <Read> -> read ( <IDs> ) ;
bool Read (NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	NODE*S = P;
	if (T->next!=NULL)
	{T=T->next;}
	if (P->lexeme == "read" && T->lexeme== "(")
	{
		if (T->next!=NULL)
		{T=T->next;}
		bool idbool = IDs(T);
		if (idbool == true)
		{
			// gen instruction
			gen_instr("STDIN", -999);
			int addr = ispresent(T->lexeme);
			gen_instr("POPM", addr);

			if (T->next!=NULL)
			{T=T->next;}
			if (T->lexeme== ")")
			{
				if (T->next!=NULL)
				{T=T->next;}
				if(T->lexeme==";")
				{
					P=T;
					if (DisplayProduction == true)
					{
						cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
						S=S->next;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						S=P->prev;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
						fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
						cout << "\t<Read> -> read ( <IDs> ) ;" << endl;
						fout << "\t<Read> -> read ( <IDs> ) ;" << endl;
					}
					return true;
				}
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Read() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Read() false;" << endl;}return false;}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Read() false;" << endl;}return false;}
	fout.close();
}
// <Relop> -> = | != | > | < | => | <=
bool Relop(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	if (P->lexeme == "=")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> =" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> =" << endl;
		}
		return true;
	}
	else if (P->lexeme== "!=")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> !=" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> !=" << endl;
		}
		return true;
	}
	else if (P->lexeme== ">")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> >" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> >" << endl;
		}
		return true;
	}
	else if (P->lexeme== "<")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> <" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> <" << endl;
		}
		return true;
	}
	else if (P->lexeme== "=>")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> =>" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> =>" << endl;
		}
		return true;
	}
	else if (P->lexeme== "<=")
	{
		if (DisplayProduction == true)
		{
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Relop> -> <=" << endl;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Relop> -> <=" << endl;
		}
		return true;
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Relop() false;" << endl;}return false;}
	fout.close();
}
// <Return> -> return ; | return <Expression> ;
bool Return(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T=P;
	NODE*S=P;
	if(T->next!=NULL)
	{T=T->next;}
	if (P->lexeme == "return" && T->lexeme== ";")
	{
		P=T;
		if (DisplayProduction == true)
		{
			T=T->prev;
			cout << endl << "Token: " << T->tokentype << "\t\tlexeme: " << T->lexeme;
			cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			cout << "\t<Return> -> return ;" << endl;
			fout << endl << "Token: " << T->tokentype << "\t\tlexeme: " << T->lexeme;
			fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
			fout << "\t<Return> -> return ;" << endl;
		}
		return true;
	}
	else if (P->lexeme == "return" && T->lexeme!=";")
	{
		bool expressionbool = Expression(T);
		if (expressionbool== true)
		{
			if(T->next!=NULL)
			{T=T->next;}
			if (T->lexeme== ";")
			{
				P=T;
				if (DisplayProduction == true)
				{
					cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					cout << "\t<Return> -> return <Expression> ;" << endl;
					fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
					fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
					fout << "\t<Return> -> return <Expression> ;" << endl;
				}
				return true;
			}
			else 
			{if (displayfalse == true) {cout << "FALSE: Return() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Return() false;" << endl;}return false;}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Return() false;" << endl;}return false;}
	fout.close();
}
// <Statement> -> <Compound> | <Assign> | <If> | <Return> | <Write> | <Read> | <While>
bool Statement(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);
	bool compoundbool = Compound(P);
	if (compoundbool== true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <Compound>" << endl;
			fout << "\t<Statement> -> <Compound>" << endl;
		}
		return true;
	}
	bool assignbool = Assign(P);
	if (assignbool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <Assign>" << endl;
			fout << "\t<Statement> -> <Assign>" << endl;
		}
		return true;
	}
	bool ifbool = If(P);
	if (ifbool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <If>" << endl;
			fout << "\t<Statement> -> <If>" << endl;
		}
		return true;
	}
	bool returnbool = Return(P);
	if (returnbool == true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <Return>" << endl;
			fout << "\t<Statement> -> <Return>" << endl;
		}
		return true;
	}
	bool writebool = Write(P);
	if (writebool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <Write>" << endl;
			fout << "\t<Statement> -> <Write>" << endl;
		}
		return true;
	}

	bool readbool = Read(P);
	if (readbool==true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <Read>" << endl;
			fout << "\t<Statement> -> <Read>" << endl;
		}
		return true;
	}

	bool whilebool = While(P);
	if(whilebool== true)
	{
		if (DisplayProduction == true)
		{
			cout << "\t<Statement> -> <While>" << endl;
			fout << "\t<Statement> -> <While>" << endl;
		}
		return true;
	}
		else
		{
			if (displayfalse == true) 
			{cout << "FALSE: Statement() false;" << endl;}
			return false;
		}
	
	fout.close();
}
// <Statement List> -> <Statement> <Statement List> | <Statement>
bool StatementList(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T =P;
	bool statementbool = Statement(T);
	if (statementbool==true)
	{
		if(T->next!=NULL)
		{T=T->next;}
		bool statementlistbool = StatementList(T);
		if (statementlistbool == true)
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << "\t<Statement List> -> <Statement> <Statement List>" << endl;
				fout << "\t<Statement List> -> <Statement> <Statement List>" << endl;
			}
			return true;
		}
		else
		{
			P=T;
			if (DisplayProduction == true)
			{
				cout << "\t<Statement List> -> <Statement>" << endl;
				fout << "\t<Statement List> -> <Statement>" << endl;
			}
			return true;
		}
	}
	else
	{
		if (displayfalse == true) 
		{
			cout << "FALSE: Statement List() false;" << endl;
		}
		return false;
	}
	fout.close();
}
// <Term> -> <Factor> <TermPrime>
bool Term(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE *T=P;
	bool factorbool = Factor(T);
	if (factorbool == true)
	{
		if (P->next!=NULL)
		{T=T->next;}
		bool termbool = TermPrime(T);
		if (termbool == true)
		{
			P = T;
			if (DisplayProduction == true)
			{
				cout << "\t<Term> -> <Factor> <TermPrime>" << endl;
				fout << "\t<Term> -> <Factor> <TermPrime>" << endl;
			}
			return true;
		}
		else
		{
			T=T->prev;
			if (DisplayProduction == true)
			{
				cout << "\t<Term> -> <Factor> <TermPrime>" << endl;
				fout << "\t<Term> -> <Factor> <TermPrime>" << endl;
			}
			return true;
		}
	}
	else 
	{if (displayfalse == true) {cout << "FALSE: Term() false;" << endl;}return false;}
	fout.close();
}
// <TermPrime> -> * <Factor> <TermPrime> | / <<Factor> <TermPrime> | E
bool TermPrime(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	if (P->lexeme == "*")
	{
		if(P->next==NULL)
		{return false;}
		else 
		{
			T=T->next;
			bool factorbool = Factor(T);
			if (factorbool == true)
			{
				if(T->next!=NULL)
				{T=T->next;}
				bool termbool = TermPrime(T);
				if (termbool == true)
				{
					// instruction table mul
					gen_instr("MUL", -999);

					P=T;
					if (DisplayProduction == true)
					{
						NODE*S = T;
						S = S->prev;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << "*" << endl;;
						cout << "\t<TermPrime> -> * <Factor> <TermPrime>" << endl;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << "*" << endl;;
						fout << "\t<TermPrime> -> * <Factor> <TermPrime>" << endl;
					}
					return true;
				}
				else
				{if (displayfalse == true) {cout << "FALSE: Term Prime() false;" << endl;}return false;}
			} 
			else 
			{if (displayfalse == true) {cout << "FALSE: Term Prime() false;" << endl;}return false;}
		}
	}
	else if (P->lexeme == "/")
	{
		if(P->next==NULL)
		{return false;}
		else 
		{
			T=T->next;
			bool factorbool = Factor(T);
			if (factorbool == true)
			{
				if(T->next!=NULL)
				{T=T->next;}
				bool termbool = TermPrime(T);
				if (termbool == true)
				{
					// instruction table div
					gen_instr("DIV", -999);

					P=T;
					if (DisplayProduction == true)
					{
						NODE*S = T;
						S = S->prev;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << "/" << endl;;
						cout << "\t<TermPrime> -> / <Factor> <TermPrime>" << endl;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << "/" << endl;;
						fout << "\t<TermPrime> -> / <Factor> <TermPrime>" << endl;
					}
					return true;
				}
				else
				{if (displayfalse == true) {cout << "FALSE: Term Prime() false;" << endl;}return false;}
			} 
			else 
			{if (displayfalse == true) {cout << "FALSE: Term Prime() false;" << endl;}return false;}
		}
	}
	else
	{
		if (DisplayProduction == true)
		{
			cout << "\t<TermPrime> -> E " << endl;
			fout << "\t<TermPrime> -> E " << endl;
		}
		return true;
	}
	fout.close();
}
// <While> -> while ( <Condition> ) <Statement>
bool While(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	if (T->next !=NULL)
	{T=T->next;}
	if (P->lexeme== "while" && T->lexeme== "(")
	{
		int addr = Itable_counter;
		gen_instr("LABEL", -999);

		if(T->next !=NULL)
		{T=T->next;}
		bool conditionbool = Condition(T);
		if (conditionbool == true)
		{
			if (T->lexeme == ")")
			{
				if(T->next !=NULL)
				{T=T->next;}

				bool statementbool = Statement(T);

				if (statementbool == true)
				{
					gen_instr("JUMP", addr);
					back_patch(Itable_counter);
					back_patch(Itable_counter);

					P= T;
					if (DisplayProduction == true)
					{
						cout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
						cout << "\t<While> -> while ( <Condition> ) <Statement>" << endl;
						fout << endl << "Token: " << P->tokentype << "\tlexeme: " << P->lexeme << endl;
						fout << "\t<While> -> while ( <Condition> ) <Statement>" << endl;
					}
					return true;
				}
				else 
				{if (displayfalse == true) {cout << "FALSE: While() false;" << endl;}return false;}
			}
			else
			{if (displayfalse == true) {cout << "FALSE: While() false;" << endl;}return false;}
		}
		else 
		{if (displayfalse == true) {cout << "FALSE: While() false;" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: While() false;" << endl;}return false;}
	fout.close();
}
// <Write> -> write ( <Expression> ) ;
bool Write(NODE*&P)
{
	fstream fout;
	fout.open("FileOut.txt",ios::app);

	NODE*T = P;
	NODE*S = P;
	if (T->next !=NULL)
	{T=T->next;}
	if (P->lexeme == "write" && T->lexeme =="(")
	{
		if (T->next !=NULL)
		{T=T->next;}
		bool expressionbool = Expression(T);
		if (expressionbool == true)
		{
			gen_instr("STDOUT", -999);
			if (T->lexeme == ")")
			{
				if (T->next !=NULL)
				{T=T->next;}
				if (T->lexeme== ";")
				{
					P=T;
					if (DisplayProduction == true)
					{
						cout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\t\tlexeme: " << S->lexeme;
						S=S->next;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						S=P->prev;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme;
						S=P;
						cout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme << endl;
						fout << endl << "Token: " << S->tokentype << "\tlexeme: " << S->lexeme << endl;
						cout << "\t<Write> -> write ( <Expression> ) ;" << endl;
						fout << "\t<Write> -> write ( <Expression> ) ;" << endl;
					}
					return true;
				}
				else
				{if (displayfalse == true) {cout << "FALSE: Write() false;" << endl;}return false;}
			}
			else
			{if (displayfalse == true) {cout << "FALSE: Write() false;" << endl;}return false;}
		}
		else
		{if (displayfalse == true) {cout << "FALSE: Write() false;" << endl;}return false;}
	}
	else
	{if (displayfalse == true) {cout << "FALSE: Write() false;" << endl;}return false;}
	fout.close();
}
