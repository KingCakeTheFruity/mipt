#ifndef DECISION_TREE
#define DECISION_TREE

#include "general/general_c/strings_and_files.h"
#include "general/general_cpp/string.h"

//=============================================================================
// Abstracts ==================================================================

class AbstractDecisionStatement {
public:
	virtual int state(char *end = (char*) "") = 0;
};

class DecisionAbstractQuestion : public AbstractDecisionStatement {
public:
	virtual int get_answer() = 0;
};

//=============================================================================
// Statement & Question =======================================================

class DecisionStatement : public AbstractDecisionStatement {
private:
	String statement;

public:
	DecisionStatement(const String &statement_);
	int state(char *end = (char*) "");
};

class DecisionQuestion : public DecisionStatement {
public:
	DecisionQuestion(const String statement_) : DecisionStatement(statement_) {};

	int state();
	int get_answer();
};

//=============================================================================
// Node =======================================================================

class DecisionTreeNode {
private:
	AbstractDecisionStatement *statement;
	DecisionTreeNode *node_true;
	DecisionTreeNode *node_false;

public:
	DecisionTreeNode();
	DecisionTreeNode(AbstractDecisionStatement *statement_, DecisionTreeNode* node_true_ = nullptr, DecisionTreeNode *node_false_  = nullptr);

	void set_true (DecisionTreeNode* node);
	void set_false(DecisionTreeNode* node);

	int state();

	DecisionTreeNode *proceed(const int answer);
};

//=============================================================================
// Tree =======================================================================

const char SYMB_OPEN_NODE = '[';
const char SYMB_CLOSE_NODE = ']';
const char SYMB_QUOTE = '"';

class DecisionTree {
private:
	DecisionTreeNode *root;

	DecisionTreeNode* read_node(File *file_ptr);
	DecisionTreeNode* find(const String &statement);
public:
	DecisionTree();

	int load(const char *file_name);
	int save(const char *file_name);

	int run_guess();
	int run_define(const String &statement);
	int run_difference(const String &first, const String &second);

	int run_interaction();
};

#endif // DECISION_TREE