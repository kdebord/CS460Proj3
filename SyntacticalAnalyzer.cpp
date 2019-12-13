/*******************************************************************************
* File name:                                                                   *
* Project: CS 460 Project 2 Fall 2019                                          *
* Author(s):                                                                   *
* Date:                                                                        *
* Description: This file contains                                              *
*******************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include "SyntacticalAnalyzer.h"

using namespace std;

#define lexeme lex->GetTokenName(token)

SyntacticalAnalyzer::SyntacticalAnalyzer (char * filename)
{

	// Trim filename in order to add correct extensions for output streams
	string fNameStr = string(filename);
	size_t periodIdx = fNameStr.find('.');
	string noExtension = fNameStr.substr(0, periodIdx);

	// cout << "Filename with no extension: " << noExtension << endl;
	// p2 file (.p2)
	string p2FName = noExtension + ".p2";
	p2.open(p2FName, ios::out);
	if(!p2.is_open()){
		cout << "ERROR: could not create file" << p2FName << ". Program will now terminate." << endl;
		exit(1);
	}

	lex = new LexicalAnalyzer (filename);
	cg = new CodeGen (filename);
	deque<string> tempobj;
	token_type t;
	plus_helper = false;
	mult_helper = false;
	div_helper = false;
	minus_helper = false;
	mult_param = false;
	is_nested = false;
	in_plus = 0;
	in_minus = 0;
	in_mult = 0;
	in_div = 0;
	int totalErrors = program ();
}

SyntacticalAnalyzer::~SyntacticalAnalyzer ()
{
	delete lex;
	delete cg;
}

// -------------------------------------------------------------------------------------------------------------

int SyntacticalAnalyzer::program () {
	
	int errors = 0;
	token = lex->GetToken();

	// cout << "Entering Program function; current token is " << lex->GetTokenName(token) << ", lexeme: " << endl; //lex->GetLexeme() << endl;
	p2 << "Using rule 1\n"; // here

	if(token == LPAREN_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("( expected");
	}

	errors += define();

	if(token == LPAREN_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("( expected");
	}

	errors += more_defines();

	if(token == EOF_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("EOF_T expected");
	}

	

	return errors;  	
}

int SyntacticalAnalyzer::define () {
	p2 << "Using rule 4\n"; // here since we enter <define> from <program>
	int errors = 0;
	if(token == DEFINE_T){
	// <define> -> DEFINE_T LPAREN_T IDENT_T <param_list> RPAREN_T <stmt> <stmt_list> RPAREN_T
		// p2 << "Using rule 4\n"; //where?
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("define expected");
	}
	
	if(token == LPAREN_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("( expected");
	}
	
	// if(token == IDENT_T) {
	//	token = lex->GetToken();
	//} else {
	//	errors++;
	//	lex->ReportError("IDENT_T expected");
	//}
	bool is_main;
	if(token == IDENT_T) {
		if(lex->GetLexeme() == "main")
		{
			cg->WriteCode(0 ,"int main()\n{\n");
			is_main = true;
		}
		else
		{
			cg->WriteCode(0, "Object ");
			cg->WriteCode(0, lex->GetLexeme());
			cg->WriteCode(0, " ()\n{");
			cg->WriteCode(1, "Object __RetVal;\n");
			cg->WriteCode(1, "__RetVal = ");
		}

		token = lex->GetToken();
		
	} else {
		errors++;
		lex->ReportError("IDENT_T expected");
	}

	errors += param_list();
	
	if(token == RPAREN_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("\')\' expected");
	}
	
	errors += stmt();

	errors += stmt_list();
	
	if(token == RPAREN_T) {
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError("\')\' expected");
	}
	if(is_main == true)
		cg->WriteCode(1, "return 0;\n");
	else
		cg->WriteCode(1, "return __RetVal;\n");
	cg->WriteCode(0,"}\n\n");	
	return errors;
}

int SyntacticalAnalyzer::action () {
	int errors = 0;
	if(token == IF_T){
		//<action> -> IF_T <stmt> <stmt> <else_part>
		p2 << "Using rule 24\n";
		token = lex->GetToken();
		errors += stmt();
		errors += stmt();
		errors += else_part();
	}
	else if(token == COND_T){
		// <action> -> COND_T LPAREN_T <stmt_pair_body>
		p2 << "Using rule 25\n";
		token = lex->GetToken();
		if(token == LPAREN_T){
			token = lex->GetToken();
			// errors += stmt_pair_body();
		} else {

			lex->ReportError("( expected");
		}
		errors += stmt_pair_body();
	}
	else if(token == LISTOP1_T){
		//<action> -> LISTOP1_T <stmt>
		p2 << "Using rule 26\n";
		cg->WriteCode(0, "listop (\"" + lex->GetLexeme() + "\", ");
		token = lex->GetToken();
		errors += stmt();
		cg->WriteCode(0, " )");
	}
	else if(token == LISTOP2_T){
		// <action> -> LISTOP2_T <stmt> <stmt>
		p2 << "Using rule 27\n";
		cg->WriteCode(0, "listop (\"" + lex->GetLexeme() + "\", ");
		token = lex->GetToken();
		errors += stmt();
		cg->WriteCode(0, ", ");
		errors += stmt();
		cg->WriteCode(0, ");\n");
	}
	else if(token == AND_T){
		// <action> -> AND_T <stmt_list>
		p2 << "Using rule 28\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == OR_T){
		// <action> -> OR_T <stmt_list>
		p2 << "Using rule 29\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == NOT_T){
		// <action> -> NOT_T <stmt>
		p2 << "Using rule 30\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == NUMBERP_T){
		// <action> -> NUMBERP_T <stmt>
		p2 << "Using rule 31\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == LISTP_T){
		// <action> -> LISTP_T <stmt>
		p2 << "Using rule 32\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == ZEROP_T){
		// <action> -> ZEROP_T <stmt>
		p2 << "Using rule 33\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == NULLP_T){
		// <action> -> NULLP_T <stmt>
		p2 << "Using rule 34\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == STRINGP_T){
		// <action> -> STRINGP_T <stmt>
		p2 << "Using rule 35\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == PLUS_T){
		// <action> -> PLUS_T 
		p2 << "Using rule 36\n";
		token = lex->GetToken();

		in_plus++;

		plus_helper = true;
		minus_helper = false;
		mult_helper = false;
		div_helper = false;
		mult_param = true;

		cg->WriteCode(0, "(");


		errors += stmt_list();



		plus_helper = false;
		cg->WriteCode(0, ")");
		if(in_plus > 1) {
			cg->WriteCode(0, " + ");
			in_plus--;
		}
		in_plus--;




	}
	else if(token == MINUS_T){
		// <action> -> MINUS_T <stmt>
		p2 << "Using rule 37\n";
		token = lex->GetToken();

		in_minus++;

		minus_helper = true;
		mult_helper = false;
		div_helper = false;
		plus_helper = false;
		mult_param = true;

		cg->WriteCode(0, "(");

		errors += stmt();
		errors += stmt_list();
		
		cg->WriteCode(0, ")");
		if(in_minus > 1) {
			cg->WriteCode(0, " - ");
			in_minus--;
		}
		in_minus--;
		minus_helper = false;

	}
	else if(token == DIV_T){
		// <action> -> DIV_T <stmt>
		p2 << "Using rule 38\n";
		token = lex->GetToken();

		in_div++;

		div_helper = true;
		minus_helper = false;
		mult_helper = false;
		plus_helper = false;
		mult_param = true;

		cg->WriteCode(0, "(");
		errors += stmt();
		errors += stmt_list();

		cg->WriteCode(0, ")");

		if(in_div > 1)
		{
			cg->WriteCode(0, " / ");
			in_div--;
		}
		in_div--;




		div_helper = false;
	}
	else if(token == MULT_T){
		// <action> -> MULT_T
		p2 << "Using rule 39\n";
		token = lex->GetToken();

		in_mult++;

		mult_helper = true;
		plus_helper = false;
		minus_helper = false;
		div_helper = false;
		mult_param = true;

		cg->WriteCode(0, "(");

		errors += stmt_list();



		cg->WriteCode(0, ")");

		if(in_mult > 1)
		{
			cg->WriteCode(0, " * ");
			in_mult--;
		}
		in_mult--;

		mult_helper = false;
	}
	else if(token == MODULO_T){
		// <action> -> MODULO_T <stmt> <stmt>
		p2 << "Using rule 40\n";
		token = lex->GetToken();
		errors += stmt();
		errors += stmt();
	}
	else if(token == ROUND_T){
		// <action> -> ROUND_T <stmt>
		p2 << "Using rule 41\n";
		token = lex->GetToken();
		errors += stmt();
	}
	else if(token == EQUALTO_T){
		// <action> -> EQUALTO_T
		p2 << "Using rule 42\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == GT_T){
		// <action> -> GT_T
		p2 << "Using rule 43\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == LT_T){
		// <action> -> LT_T
		p2 << "Using rule 44\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == GTE_T){
		// <action> -> GTE_T
		p2 << "Using rule 45\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == LTE_T){
		// <action> -> LTE_T
		p2 << "Using rule 46\n";
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == IDENT_T){
		// <action> -> LTE_T
		p2 << "Using rule 47\n";
		cg->WriteCode(0,lex->GetLexeme() + "();\n");
		token = lex->GetToken();
		errors += stmt_list();
	}
	else if(token == DISPLAY_T){
		// <action> -> DISPLAY_T <stmt>
		p2 << "Using rule 48\n";
		//print a cout for display
		cg->WriteCode(1,"cout << ");
		token = lex->GetToken();
		errors += stmt();
		cg->WriteCode(0, ";\n");
		in_plus = 0;
		in_minus = 0;
		in_div = 0;
		in_mult = 0;

	}
	else if(token == NEWLINE_T){
		// <action> -> NEWLINE_T
		p2 << "Using rule 49\n";
		cg->WriteCode(1,"cout << endl;\n");
		token = lex->GetToken();
	} else {
		errors++;
		lex->ReportError(lexeme + " unexpected");
	}
	
	return errors;
}

int SyntacticalAnalyzer::param_list () {
	int errors = 0;

	if(token == IDENT_T){
	// <param_list> -> IDENT_T <param_list>
		p2 << "Using rule 16\n";
		token = lex->GetToken();
		errors += param_list();
	} 
	else if (token == RPAREN_T){
		// <param_list> -> { }
		p2 << "Using rule 17\n";
	} 
	else {
		errors++;
		lex->ReportError(lexeme + " unexpected");
	}

	return errors;
}

int SyntacticalAnalyzer::any_other_token(){
  int errors = 0;
  if(token == LPAREN_T){
    // <any_other_token> -> LPAREN_T <more_tokens> RPAREN_T
    p2 << "Using rule 50\n";
    token = lex->GetToken();
    errors += more_tokens();
    if(token == RPAREN_T)
      token = lex->GetToken();
    else{
      errors++;
      lex->ReportError("( expected");
    }
  }
  else if(token == IDENT_T){
    // <any_other_token> -> IDENT_T
    p2 << "Using rule 51\n";
    token = lex->GetToken();
  }
  else if(token == NUMLIT_T){
    // <any_other_token> -> NUMLIT_T
    p2 << "Using rule 52\n";
    token = lex->GetToken();
  }
  else if(token == STRLIT_T){
    // <any_other_token> -> STRLIT_T
    p2 << "Using rule 53\n";
    token = lex->GetToken();
  }
  else if(token == LISTOP2_T){
    // <any_other_token> -> LISTOP2_T
    p2 << "Using rule 54\n";
    token = lex->GetToken();
  }
  else if(token == IF_T){
    // <any_other_token> -> IF_T
    p2 << "Using rule 55\n";
    token = lex->GetToken();
  }
  else if(token == DISPLAY_T){
    // <any_other_token> -> DISPLAY_T
    p2 << "Using rule 56\n";
    token = lex->GetToken();
  }
  else if(token == NEWLINE_T){
    // <any_other_token> -> NEWLINE_T
    p2 << "Using rule 57\n";
    token = lex->GetToken();
  }
  else if(token == LISTOP1_T){
    // <any_other_token> -> LISTOP1_T
    p2 << "Using rule 58\n";
    token = lex->GetToken();
  }
  else if(token == AND_T){
    // <any_other_token> -> AND_T
    p2 << "Using rule 59\n";
    token = lex->GetToken();
  }
  else if(token == OR_T){
    // <any_other_token> -> OR_T
    p2 << "Using rule 60\n";
    token = lex->GetToken();
  }
  else if(token == NOT_T){
    // <any_other_token> -> NOT_T
    p2 << "Using rule 61\n";
    token = lex->GetToken();
  }
  else if(token == DEFINE_T){
    // <any_other_token> -> DEFINE_T
    p2 << "Using rule 62\n";
    token = lex->GetToken();
  }
  else if(token == NUMBERP_T){
    // <any_other_token> -> NUMBERP_T
    p2 << "Using rule 63\n";
    token = lex->GetToken();
  }
  else if(token == LISTP_T){
    // <any_other_token> -> LISTP_T
    p2 << "Using rule 64\n";
    token = lex->GetToken();
  }
  else if(token == ZEROP_T){
    // <any_other_token> -> ZEROP_T
    p2 << "Using rule 65\n";
    token = lex->GetToken();
  }
  else if(token == NULLP_T){
    // <any_other_token> -> NULLP_T
    p2 << "Using rule 66\n";
    token = lex->GetToken();
  }
  else if(token == STRINGP_T){
    // <any_other_token> -> STRINGP_T
    p2 << "Using rule 67\n";
    token = lex->GetToken();
  }
  else if(token == PLUS_T){
    // <any_other_token> -> PLUS_T
    p2 << "Using rule 68\n";
    token = lex->GetToken();
  }
  else if(token == MINUS_T){
    // <any_other_token> -> MINUS_T
    p2 << "Using rule 69\n";
    token = lex->GetToken();
  }
  else if(token == DIV_T){
    // <any_other_token> -> DIV_T
    p2 << "Using rule 70\n";
    token = lex->GetToken();
  }
  else if(token == MULT_T){
    // <any_other_token> -> MULT_T
    p2 << "Using rule 71\n";
    token = lex->GetToken();
  }
  else if(token == MODULO_T){
    // <any_other_token> -> MODULO_T
    p2 << "Using rule 72\n";
    token = lex->GetToken();
  }
  else if(token == ROUND_T){
    // <any_other_token> -> ROUND_T
    p2 << "Using rule 73\n";
    token = lex->GetToken();
  }
  else if(token == EQUALTO_T){
    // <any_other_token> -> EQUALTO_T
    p2 << "Using rule 74\n";
    token = lex->GetToken();
  }
  else if(token == GT_T){
    // <any_other_token> -> GT_T
    p2 << "Using rule 75\n";
    token = lex->GetToken();
  }
  else if(token == LT_T){
    // <any_other_token> -> LT_T
    p2 << "Using rule 76\n";
    token = lex->GetToken();
  }
  else if(token == GTE_T){
    // <any_other_token> -> GTE_T
    p2 << "Using rule 77\n";
    token = lex->GetToken();
  }
  else if(token == LTE_T){
    // <any_other_token> -> LTE_T
    p2 << "Using rule 78\n";
    token = lex->GetToken();
  }
  else if(token == SQUOTE_T){
    // <any_other_token> -> SQUOTE <any_other_token>
    p2 << "Using rule 79\n";
    token = lex->GetToken();
    errors += any_other_token();
  }
  else if(token == COND_T){
    // <any_other_token> -> COND_T
    p2 << "Using rule 80\n";
    token = lex->GetToken();
  }
  else if(token == ELSE_T){
    // <any_other_token> -> ELSE_T
    p2 << "Using rule 81\n";
    token = lex->GetToken();
  }
  else{
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }

  return errors;
}

int SyntacticalAnalyzer::stmt_pair(){
  int errors = 0;

  if(token == LPAREN_T){
    // <stmt_pair> -> LPAREN_T <stmt_pair_body>
    p2 << "Using rule 20\n";
    token = lex->GetToken();
    errors += stmt_pair_body();
  }
  else if(token == RPAREN_T){
    // <stmt_pair> -> { }
    p2 << "Using rule 21\n";
  }
  else{
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }
  
  return errors;
}

int SyntacticalAnalyzer::literal(){
    int errors = 0;
    if(token == NUMLIT_T){
      // <literal> -> NUMLIT_T
      p2 << "Using rule 10\n";



	  cg->WriteCode(0,"Object(");
	  cg->WriteCode(0,lex->GetLexeme());
	  cg->WriteCode(0,")");
	
      token = lex->GetToken();
	  if(token == RPAREN_T)
	  {
		  mult_param = false;
	  }
	  if(plus_helper == true && mult_param == true) {
		cg->WriteCode(0, " + ");
	  }
	  else if(minus_helper == true && mult_param == true) {
		cg->WriteCode(0, " - ");
	  }
	  else if(mult_helper == true && mult_param == true) {
		cg->WriteCode(0, " * ");
	  }
	  else if(div_helper == true && mult_param == true) {
		cg->WriteCode(0, " / ");
	  }

      //token = lex->GetToken();
    }
    else if(token == STRLIT_T){
      // <literal> -> STRLIT_T
      p2 << "Using rule 11\n";
	  cg->WriteCode(0,"Object(");
	  cg->WriteCode(0,lex->GetLexeme());
	  cg->WriteCode(0,")");
	
      token = lex->GetToken();
	  if(token == RPAREN_T)
	  {
		  mult_param = false;
	  }
	  if(mult_param == true) {
		  
		cg->WriteCode(0, " + ");
		plus_helper = false;
	  }
	  if(minus_helper == true) {
		cg->WriteCode(0, " - ");
		minus_helper = false;
	  }
	  if(mult_helper == true) {
		cg->WriteCode(0, " * ");
		mult_helper = false;
	  }
	  if(div_helper == true) {
		cg->WriteCode(0, " / ");
		div_helper = false;
	  }


    }
    else if(token == SQUOTE_T){
      // <literal> -> SQUOTE_T <quoted_lit>
      p2 << "Using rule 12\n";
      token = lex-> GetToken();
	  cg->WriteCode(0, "Object(\"(");
      errors += quoted_lit();
	  cg->WriteCode(0, ")\")");
    }
    else{
      errors++;
      lex->ReportError(lexeme + " unexpected");
    }
    return errors;
}

int SyntacticalAnalyzer::quoted_lit(){
  int errors = 0;

  // <quoted_lit> -> <any_other_token>
  p2 << "Using rule 13\n";
  errors += any_other_token();
  
  return errors;
}

int SyntacticalAnalyzer::stmt_list (){
  int errors = 0;

  if(token == IDENT_T || token == LPAREN_T || token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T){
    // <stmt_list> -> <stmt> <stmt_list>
    p2 << "Using rule 5\n";
	errors += stmt();
    errors += stmt_list();
  }
  else if(token == RPAREN_T){
    // <stmt_list> -> {}
	p2 << "Using rule 6\n";
  }
  else{
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }

  return errors;
}

int SyntacticalAnalyzer::stmt (){
  int errors = 0;
  if(token == IDENT_T){
    // <stmt> -> IDENT_T
    p2 << "Using rule 8\n";
    token = lex -> GetToken();
  }
  else if (token == LPAREN_T){
    // <stmt> -> LPAREN_T <action> RPAREN_T
    p2 << "Using rule 9\n";
    token = lex -> GetToken();

    errors += action();

    if(token == RPAREN_T)
	{
      token = lex -> GetToken();

	}
    else {
      errors++;
      lex->ReportError(") expected");
    }
  }
  else if(token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T){
    // <stmt> -> <literal>
    p2 << "Using rule 7\n";
    errors += literal();
  }
  else {
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }

  return errors;
}

int SyntacticalAnalyzer::else_part (){
  int errors = 0;

  if(token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T || token == IDENT_T || token == LPAREN_T){
    // <else_part> -> <stmt>
    p2 << "Using rule 18\n";

    errors += stmt();
  }
  else if(token == RPAREN_T){
  // <else_part> -> { }
     p2 << "Using rule 19\n";
  }
  else {
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }

  return errors;
}

int SyntacticalAnalyzer::more_defines(){
  int errors = 0;

  if(token == DEFINE_T){
    // <more_defines> -> <define> LPAREN_T <more_defines>
    p2 << "Using rule 2\n";
    errors += define();

    if(token == LPAREN_T)
      token = lex -> GetToken();
    else{
      errors++;
      lex->ReportError("( expected");
    }

    errors += more_defines();
  }
  else if( token == IDENT_T){
    // <more_defines> -> IDENT_T <stmt_list> RPAREN_T
    p2 << "Using rule 3\n";
    token = lex -> GetToken();

    errors += stmt_list();

    if(token == RPAREN_T) {
      token = lex->GetToken();
    } else {
      errors++;
      lex->ReportError("\')\' expected");
    }
  }
  else{
    errors++;
    lex->ReportError(lexeme + " unexpected");
  }

  return errors;
}



// -------------------------------------------------------------------------------------------------------------------------------

int SyntacticalAnalyzer::more_tokens ()
{
	int errors = 0;
	if(token == IF_T || token == COND_T || token ==  LISTOP1_T || token ==  LISTOP2_T || token == AND_T || token ==  OR_T || token == NOT_T 
		|| token ==  NUMBERP_T || token == LISTP_T || token ==  ZEROP_T || token ==  NULLP_T || token ==  STRINGP_T || token ==  PLUS_T 
		|| token ==  MINUS_T || token ==  DIV_T || token == MULT_T || token ==  MODULO_T || token ==  ROUND_T || token ==  EQUALTO_T 
		|| token ==  GT_T || token ==  LT_T || token ==  GTE_T || token ==  LTE_T || token == IDENT_T || token ==  DISPLAY_T || token == NEWLINE_T
		|| token ==  NUMLIT_T || token == STRLIT_T || token ==  LPAREN_T || token ==  DEFINE_T || token == SQUOTE_T || token ==  ELSE_T)
	{
		// <more_tokens> -> <any_other_token> <more_tokens>
		p2 << "Using rule 14\n";
		cg->WriteCode(0, lex->GetLexeme() + " ");
		errors += any_other_token();
		errors += more_tokens();
	} else if(token == RPAREN_T){
		// <more_tokens> -> {}
		p2 << "Using rule 15\n";
	} else {
		errors++;
		lex->ReportError(lexeme + " unexpected");
	}
	return errors;
}

int SyntacticalAnalyzer::stmt_pair_body ()
{
	int errors = 0;
	if(token == ELSE_T){
		// <stmt_pair_body> -> ELSE_T <stmt> RPAREN_T
		p2 << "Using rule 23\n";
		token = lex->GetToken();
		errors += stmt();
		if(token == RPAREN_T){
			token = lex->GetToken();
		} else {
			errors++;
			lex->ReportError("\')\' expected");
		}
	} else if(token == IDENT_T || token == LPAREN_T || token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T){
		// <stmt_pair_body> -> <stmt> <stmt> RPAREN_T <stmt_pair>
		p2 << "Using rule 22\n";
		errors += stmt();
		errors += stmt();
		if(token == RPAREN_T){
			token = lex->GetToken();
		} else {
			errors++;
			lex->ReportError("\')\' expected");
		}
		errors += stmt_pair();
	} else {
		errors++;
		lex->ReportError(lexeme + " unexpected");
	}
	return errors;
}



