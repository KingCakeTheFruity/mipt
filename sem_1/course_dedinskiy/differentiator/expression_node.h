#ifndef EXPRESSION_NODE
#define EXPRESSION_NODE

#include <cmath>

#include "general/c/common.h"
#include "general/cpp/vector.hpp"

#include "expression_node_interface.h"
#include "expression_node_decender_interface.h"


template <typename T>
const T &min(const T &first, const T &second) {
	return first < second ? first : second;
}

template <typename T>
const T &max(const T &first, const T &second) {
	return second < first ? first : second;
}

//=============================================================================
// ExprNode ===================================================================

class ExprNode {
private:
// data =======================================================================
	ExprNode *L;
	ExprNode *R;

	char variable_presented;
//=============================================================================

	double evaluate_op(const double *var_table, const size_t var_table_len) {
		char op = val;
		double L_RES  = L ? L->evaluate(var_table, var_table_len) : 0;
		double R_RES = R ? R->evaluate(var_table, var_table_len) : 0;

		#define OPDEF(name, sign, arg_cnt, prior, calculation, ig1, ig2, ig3, ig4) {                                  \
            case #sign[0]: {                                                                                          \
            	if (!((arg_cnt == 2 && L && R) || (arg_cnt == 1 && !L && R) || (arg_cnt == 0 && !L && !R))) {         \
        			printf("[ERR]<exrp_eval>: Operation {%c} has bad number of args, need %d\n", #sign[0], arg_cnt);  \
        			return 0;                                                                                         \
            	}                                                                                                     \
                                                                                                                      \
            	return calculation;                                                                                   \
            }                                                                                                         \
		}

		switch (op) {

			#include "ops.dsl"

			default: {
				printf("[ERR]<expr_eval>: Invalid op {%c}\n", op);
				return 0;
			}
		}

		#undef OPDEF
	}

	double evaluate_variable(const double *var_table, const size_t var_table_len) {
		int var = val;
		if (var_table_len <= (size_t) var) {
			return (double) KCTF_POISON;
		} else  {
			return var_table[var];
		}
	}


public:
// data =======================================================================
	int prior;
	int low_prior;
	int high_prior;
	char type;
	double val;
	double complexity;
//=============================================================================

	ExprNode():
	L(nullptr),
	R(nullptr),
	variable_presented(0),
	type(0),
	val(0.0),
	complexity(0.0)
	{}

	~ExprNode() {}

	void update() {
		complexity = (L ? L->complexity : 0) + (R ? R->complexity : 0) + 1;
		variable_presented = (L ? L->variable_presented : 0) || (R ? R->variable_presented : 0) || (type == VARIABLE);
		low_prior  = min(min((L ? L->low_prior  : PRIOR_MAX), (R ? R->low_prior  : PRIOR_MAX)), prior);
		high_prior = max(max((L ? L->high_prior : PRIOR_MIN), (R ? R->high_prior : PRIOR_MIN)), prior);

		update_complexity();
	}

	void update_complexity() {
		if (type == VALUE) {
			complexity = 1 + fabs(val) * (GENERAL_EPS);
		} else if (type == VARIABLE) {
			complexity = val;
		} else {
			switch ((char) val) {
				case '-' :
				case '+' : {
					complexity = (L ? L->complexity : 0) + (L ? L->complexity : 0);
					break;
				}

				case '/' :
				case '*' : {
					complexity = (L ? L->complexity : 1) * (R ? R->complexity : 1);
					break;
				}

				case '^' : {
					complexity = (L ? L->complexity : 0);
					break;
				}

				default: {
					complexity = 5 + pow((L ? L->complexity : 2), (R ? R->complexity : 2));
				}
			}
		}
	}

	void ctor() {
		type = 0;
		val  = 0;
		
		L = nullptr;
		R = nullptr;

		complexity = 0;
		variable_presented = 0;
	}

	static ExprNode *NEW() {
		ExprNode *cake = (ExprNode*) calloc(1, sizeof(ExprNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void ctor(const char type_, const double val_, const int prior_ = PRIOR_VALUE, ExprNode *L_ = nullptr, ExprNode *R_ = nullptr) {
		type = type_;
		val  = val_;
		
		L = L_;
		R = R_;

		if (type == VARIABLE) {
			variable_presented = 1;
		}

		prior = prior_;

		update();
	}

	static ExprNode *NEW(const char type_, const double val_, const int prior_ = PRIOR_VALUE, ExprNode *L_ = nullptr, ExprNode *R_ = nullptr) {
		ExprNode *cake = (ExprNode*) calloc(1, sizeof(ExprNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, val_, prior_, L_, R_);
		return cake;
	}

	static ExprNode *NEW(const char type_, const double val_, const int prior_, ExprNode &R_) {
		return NEW(type_, val_, prior_, nullptr, &R_);
	}

	void dtor(const char recursive = 0) {
		type = 0;
		val  = 0;

		if (recursive) {
			if (L) L->dtor(recursive);
			if (R) R->dtor(recursive);
		}

		L = nullptr;
		R = nullptr;

		update();
	}

	static void DELETE(ExprNode *exprnode, const char recursive = 0) {
		if (!exprnode) {
			return;
		}

		if (recursive) {
			DELETE(exprnode->L, recursive);
			DELETE(exprnode->R, recursive);
		}

		exprnode->dtor();
		free(exprnode);
	}

//=============================================================================
// get-set ====================================================================
//=============================================================================

	ExprNode *get_L() const {
		return L;
	}

	ExprNode *get_R() const {
		return R;
	}

	ExprNode *get_base() {
		if (type != OPERATION) {
			return this;
		} else if (val == '^') {
			return L;
		} else if (val == '*') {
			return R;
		} else {
			return this;
		}
	}

	ExprNode *get_pow() {
		if (type != OPERATION || val != '^') {
			return NEW(VALUE, 1, PRIOR_VALUE);
		} else {
			return R;
		}
	}

	ExprNode *get_coef() {
		if (type != OPERATION || val != '*') {
			return NEW(VALUE, 1, PRIOR_VALUE);
		} else {
			return L;
		}
	}

	void set_pow(ExprNode *pow_) {
		if (type != OPERATION || val != '^') {
			ExprNode *left  = deep_copy();
			ExprNode *right = pow_;

			DELETE(L);
			DELETE(R);
			this->ctor(OPERATION, '^', PRIOR_POW, left, right);
		} else {
			set_R(pow_);
		}
	}

	void set_coef(ExprNode *coef_) {
		if (type != OPERATION || val != '*') {
			ExprNode *right  = deep_copy();
			ExprNode *left = coef_;

			DELETE(L);
			DELETE(R);
			this->ctor(OPERATION, '*', PRIOR_MUL, left, right);
		} else {
			set_L(coef_);
		}
	}

	void set_L(ExprNode *L_) {
		L = L_;
		update();
	}

	void set_R(ExprNode *R_) {
		R = R_;
		update();
	}

	bool is_variadic() {
		return variable_presented;
	}

	ExprNode *deep_copy() const {
		ExprNode *node = NEW(type, val, prior);
		if (L) node->set_L(L->deep_copy());
		if (R) node->set_R(R->deep_copy());

		node->update();
		return node;
	}

//=============================================================================
// Operations with nodes ======================================================
//=============================================================================
	
	ExprNode &operator+(ExprNode &other) {
		return *NEW(OPERATION, '+', PRIOR_ADD, this, &other);
	}

	ExprNode &operator-(ExprNode &other) {
		return *NEW(OPERATION, '-', PRIOR_SUB, this, &other);
	}

	ExprNode &operator*(ExprNode &other) {
		return *NEW(OPERATION, '*', PRIOR_MUL, this, &other);
	}

	ExprNode &operator/(ExprNode &other) {
		return *NEW(OPERATION, '/', PRIOR_DIV, this, &other);
	}

	ExprNode &operator^(ExprNode &other) {
		return *NEW(OPERATION, '^', PRIOR_POW, this, &other);
	}

	ExprNode &operator+(const double other) {
		return *NEW(OPERATION, '+', PRIOR_ADD, this, NEW(VALUE, other, PRIOR_VALUE));
	}

	ExprNode &operator-(const double other) {
		return *NEW(OPERATION, '-', PRIOR_SUB, this, NEW(VALUE, other, PRIOR_VALUE));
	}

	ExprNode &operator*(const double other) {
		return *NEW(OPERATION, '*', PRIOR_MUL, this, NEW(VALUE, other, PRIOR_VALUE));
	}

	ExprNode &operator/(const double other) {
		return *NEW(OPERATION, '/', PRIOR_DIV, this, NEW(VALUE, other, PRIOR_VALUE));
	}

	ExprNode &operator^(const double other) {
		return *NEW(OPERATION, '^', PRIOR_POW, this, NEW(VALUE, other, PRIOR_VALUE));
	}

//=============================================================================
// Maths ======================================================================
//=============================================================================

	double evaluate(const double *var_table = nullptr, const size_t var_table_len = 0) {
		switch(type) {
			case (OPERATION) : {
				return evaluate_op(var_table, var_table_len);
			}

			case (VARIABLE) : {
				return evaluate_variable (var_table, var_table_len);
			}

			case (VALUE) : {
				return val;
			}

			default: {
				printf("BAD EXPR\n");
				return 0.0;
			}
		}
	}

	#define OPDEF(name, sign, arg_cnt, prior, calculation, differed, ig1, ig2, ig3) {                             \
        case #sign[0]: {                                                                                          \
        	if (!((arg_cnt == 2 && L && R) || (arg_cnt == 1 && !L && R) || (arg_cnt == 0 && !L && !R))) {         \
    			printf("[ERR]<exrp_eval>: Operation {%c} has bad number of args, need %d\n", #sign[0], arg_cnt);  \
    			return 0;                                                                                         \
        	}                                                                                                     \
                                                                                                                  \
        	return &(differed);                                                                                   \
        }                                                                                                         \
	}

	ExprNode *differentiate() {
		if (type == VALUE) {
			return NEW(VALUE, 0, PRIOR_VALUE);
		} else if (type == VARIABLE) {
			return NEW(VALUE, 1, PRIOR_VALUE);
		} else {
			switch ((char) val) {

				#include "ops.dsl"

				default: {
					printf("[ERR]<expr_diff>: Invalid op {%c}\n", (char) val);
					return 0;
				}
			}
		}
	}

	#undef OPDEF

#define IS_VAL(N)  (N->type == VALUE)
#define IS_ZERO(N) (IS_VAL(N) && fabs(N->val)     < GENERAL_EPS)
#define IS_ONE(N)  (IS_VAL(N) && fabs(N->val - 1) < GENERAL_EPS)

#define IS_VAR(N) (N->type == VARIABLE)

#define IS_OP(N)  (N->type == OPERATION)
#define IS_ADD(N) (IS_OP(N) && N->val == '+')
#define IS_SUB(N) (IS_OP(N) && N->val == '-')
#define IS_MUL(N) (IS_OP(N) && N->val == '*')
#define IS_DIV(N) (IS_OP(N) && N->val == '/')
#define IS_POW(N) (IS_OP(N) && N->val == '^')

#define NEW_ZERO() NEW(VALUE, 0, PRIOR_VALUE)
#define NEW_ONE()  NEW(VALUE, 1, PRIOR_VALUE)

#define ADD(A, B)  NEW(OPERATION, '+', PRIOR_ADD, A, B)
#define SUB(A, B)  NEW(OPERATION, '-', PRIOR_SUB, A, B)
#define MUL(A, B)  NEW(OPERATION, '*', PRIOR_MUL, A, B)
#define DIV(A, B)  NEW(OPERATION, '/', PRIOR_DIV, A, B)

#define RETURN_ZERO()  DELETE(this, true); *success = SIMPLIFIED_ELEMENTARY; return NEW_ZERO();
#define RETURN_ONE()   DELETE(this, true); *success = SIMPLIFIED_ELEMENTARY; return NEW_ONE();
#define RETURN_LEFT()  ExprNode *left = L;  DELETE(R); DELETE(this); *success = SIMPLIFIED_ELEMENTARY; return left;
#define RETURN_RIGHT() ExprNode *right = R; DELETE(L); DELETE(this); *success = SIMPLIFIED_ELEMENTARY; return right;

#define RANDOM_CHANCE(x) rand() % x
#define RAND90() RANDOM_CHANCE(9)
#define RAND50() RANDOM_CHANCE(2)

	bool equivalent_absolute(const ExprNode *other) const {
		if (!other) {
			return false;
		}

		if (type != other->type) {
			return false;
		}

		if (type == VALUE || type == VARIABLE) {
			return fabs(val - other->val) < GENERAL_EPS;
		} else {
			if (val != other->val) {
				return false;
			} else {
				return (!L || L->equivalent_absolute(other->get_L())) && (!R || R->equivalent_absolute(other->get_R()));
			}
		}
	}

	bool commutative_reorder(char op, char *success, int order = 1) {
		if (type != OPERATION || val != op) {
			return *success;
		}

		L->commutative_reorder(op, success, order);
		R->commutative_reorder(op, success, order);

		if (L->type == OPERATION && L->val == op) {
			if (R->complexity * order < L->R->complexity * order) {
				ExprNode *lr = L->R;
				L->set_R(R);
				set_R(lr);

				*success = REORDERED_TREE;
				return *success;
			}
		} else {
			if (R->complexity * order < L->complexity * order) {
				ExprNode *l = L;
				set_L(R);
				set_R(l);

				*success = REORDERED_TREE;
				return *success;
			}
		}

		return *success;
	}

	bool commutative_linearize(char op, char *success) {
		if (type != OPERATION || val != op) {
			return *success;
		}

		L->commutative_linearize(op, success);
		R->commutative_linearize(op, success);

		while (R->type == OPERATION && R->val == op) {
			ExprNode *rr = R->R;
			ExprNode *rl = R->L;

			R->set_L(L);
			R->set_R(rl);
			set_L(R);
			set_R(rr);
			*success = LINEARIZED_TREE;
		}

		return *success;
	}

	bool add(ExprNode *other) {
		if (IS_VAL(this) && IS_VAL(other)) {
			val = val + other->val;
			other->val = 0;

			other->update();
			update();
			return true;
		}

		ExprNode *base_a = get_base();
		ExprNode *base_b = other->get_base();
		if (!base_a->equivalent_absolute(base_b)) {
			return false;
		}

		ExprNode *coef_a = get_coef();
		ExprNode *coef_b = other->get_coef();

		set_coef(NEW(OPERATION, '+', PRIOR_ADD, coef_a, coef_b));
		other->set_coef(NEW_ZERO());

		other->update();
		update();

		return true;
	}

	bool multiply(ExprNode *other) {
		if (IS_VAL(this) && IS_VAL(other)) {
			val = val * other->val;
			other->val = 1;

			other->update();
			update();
			return true;
		}

		ExprNode *base_a = get_base();
		ExprNode *base_b = other->get_base();
		if (!base_a->equivalent_absolute(base_b)) {
			return false;
		}

		ExprNode *pow_a = get_pow();
		ExprNode *pow_b = other->get_pow();

		set_pow(NEW(OPERATION, '+', PRIOR_ADD, pow_a, pow_b));
		other->set_pow(NEW_ZERO());

		other->update();
		update();

		return true;
	}

	// bool fold_addition(char *success) {
	// 	if (!IS_ADD(this)) {
	// 		return *success;
	// 	}
	// 	if (!IS_ADD(R)) {
	// 		if (L->add(R)) {
	// 			*success = FOLDED_OPERATION;
	// 		}

	// 		return *success;
	// 	}

	// 	if (R->L->add(L)) {
	// 		*success = FOLDED_OPERATION;
	// 	}

	// 	return R->fold_addition(success);
	// }

	bool fold_addition(char *success) {
		if (!IS_ADD(this)) {
			return *success;
		}

		ExprNodeDecender term_one;
		term_one.ctor(this, -1);

		while (term_one.next()) {
			ExprNodeDecender term_two  = {};
			term_two.ctor(term_one.get_op_node(), -1);

			while (term_two.next()) {
				if (term_one.get_elem_node() == term_two.get_elem_node()) {
					continue;
				}

				ExprNode *term_1 = term_one.get_elem_node();
				ExprNode *term_2 = term_two.get_elem_node();

				// printf("terms are |");
				// term_1->dump_space();
				// printf("| and |");
				// term_2->dump_space();
				// printf("|\n");

				if (IS_VAL(term_1) && IS_VAL(term_2)) {
					term_1->val += term_2->val;
					term_2->val = 0;
					term_1->update();
					term_2->update();

					*success = SIMPLIFIED_EVALUATIVE;
					return *success;
				} 

				if (!IS_MUL(term_1) && IS_VAR(term_1)) {
					term_one.set_operand(MUL(NEW_ONE(), term_1));
					term_1 = term_one.get_elem_node();
				}

				if (!IS_MUL(term_2) && IS_VAR(term_2)) {
					term_two.set_operand(MUL(NEW_ONE(), term_2));
					term_2 = term_two.get_elem_node();
				}

				if (term_1->val != term_2->val || !(IS_MUL(term_1) || IS_DIV(term_1))) {
					continue;
				}

				ExprNodeDecender factor_one = {};
				factor_one.ctor(term_1);

				while (factor_one.next()) {
					ExprNodeDecender factor_two = {};
					factor_two.ctor(term_2);

					while (factor_two.next()) {
						//=========================================================

						ExprNode *fact_1 = factor_one.get_elem_node();
						ExprNode *fact_2 = factor_two.get_elem_node();

						// printf("factos: |");
						// fact_1->dump_space();
						// printf("| vs |");
						// fact_2->dump_space();
						// printf("|\n");

						if (!fact_1->equivalent_absolute(fact_2) || IS_ONE(fact_1)) {
							continue;
						}

						DELETE(fact_2);
						factor_two.set_operand(NEW_ONE());
						factor_one.set_operand(NEW_ONE());

						term_two.set_operand(NEW_ZERO());

						if (IS_DIV(factor_one.get_op_node()) && fact_1 != factor_one.get_op_node()->get_L()) {
							term_one.set_operand(MUL(DIV(NEW_ONE(), fact_1), ADD(term_1, term_2)));
						} else {
							term_one.set_operand(MUL(fact_1, ADD(term_1, term_2)));
						}

						*success = PUT_OUT_OF_BRACKETS;
						return *success;

						//=========================================================
					}
				}
			}
		}

		return *success;
	}

	bool fold_multiplication(char *success) {
		if (!IS_MUL(this)) {
			return *success;
		}
		if (!IS_MUL(L)) {
			if (L->multiply(R)) {
				*success = FOLDED_OPERATION;
			}

			return *success;
		}

		if (L->R->multiply(R)) {
			*success = FOLDED_OPERATION;
		}

		return L->fold_multiplication(success);
	}

	ExprNode *simplify_evaluative(char *success) {
		if (L) set_L(L->simplify_evaluative(success));
		if (R) set_R(R->simplify_evaluative(success));
		if (!L || !R) {
			return this;
		}

		if (!is_variadic()) { //todo check for bad funcs, like log
			double res = evaluate();
			DELETE(this, true);

			*success = SIMPLIFIED_EVALUATIVE;
			return NEW(VALUE, res, PRIOR_VALUE);
		} else {
			return this;
		}
	}

	ExprNode *simplify_elementary(char *success) {
		if (L) set_L(L->simplify_elementary(success));
		if (R) set_R(R->simplify_elementary(success));
		if (!L || !R) {
			return this;
		}

		switch((char) val) {
			case '+' : {
				if (IS_ZERO(L)) {
					RETURN_RIGHT();
				}

				if (IS_ZERO(R)) {
					RETURN_LEFT();
				}
				break;
			}

			case '-': {
				if (IS_ZERO(R)) {
					RETURN_LEFT();
				}
				break;
			}

			case '*': {
				if (IS_ONE(L)) {
					RETURN_RIGHT();
				}

				if (IS_ONE(R)) {
					RETURN_LEFT();
				}

				if (IS_ZERO(L) || IS_ZERO(R)) {
					RETURN_ZERO();
				}
				break;
			}

			case '/': {
				if (IS_ONE(R)) {
					RETURN_LEFT();
				}
				break;
			}

			case '^' : {
				if (IS_ONE(R)) {
					RETURN_LEFT();
				}

				if (IS_ZERO(R)) {
					RETURN_ONE();
				}

				if (IS_ZERO(L) && IS_VAL(R)) {
					RETURN_ZERO();
				}
				break;
			}
		}

		return this;

	}

	ExprNode *simplify_structure(char *success) {
		if (L) set_L(L->simplify_structure(success));
		if (*success) {
			return this;
		}

		if (R) set_R(R->simplify_structure(success));
		if (*success) {
			return this;
		}

		if (!L || !R) {
			return this;
		}

		// ========================================================== op
		switch ((char) val) {
			case '+' : {
				if (commutative_linearize('+', success)) {
					break;
				} else if (commutative_reorder('+', success, -1)) {
					break;
				} else if (fold_addition(success)) {
					break;
				}
				break;
			}

			case '*' : {
				if (commutative_linearize('*', success)) {
					break;
				} else if (commutative_reorder('*', success)) {
					break;
				} else if (fold_multiplication(success)) {
					break;
				}
				break;
			}
		}

		if (success) {
			char ig = 0;
			return simplify_evaluative(&ig)->simplify_elementary(&ig);
		} else {
			return this;
		}
	}

	ExprNode *simplify_strange(char *success) {
		if (L) set_L(L->simplify_strange(success));
		if (*success) {
			return this;
		}

		if (R) set_R(R->simplify_strange(success));
		if (*success) {
			return this;
		}

		if (!L || !R) {
			return this;
		}

		switch ((char) val) {
			case '-' : {
				if (IS_ZERO(L)) {
					val = '*';
					prior = PRIOR_MUL;
					L->val = -1;

					*success = SIMPLIFIED_EVALUATIVE;
					return this;
				}
				break;
			}

			/*case '*' : {
				if (L->equivalent_absolute(R)) {
					DELETE(L);
					R = NEW(VALUE, 2, PRIOR_VALUE);

					this->ctor(OPERATION, '^', PRIOR_POW, L, R);

					*success = SIMPLIFIED_EVALUATIVE;
					return this;
				}
			}*/

			case '/' : {
				if (IS_OP(L) && L->val == '/') {
					ExprNode *prev_L = L;
					set_R(NEW(OPERATION, '*', PRIOR_MUL, R, L->R));
					set_L(L->L);
					DELETE(prev_L, false);

					*success = SIMPLIFIED_EVALUATIVE;
					return this;
				}
				break;
			}
		}

		return this;
	}

	#define RETURN_IF_SUCCESS(code) {if (ret = (code), *success) {return ret;}}

	ExprNode *simplify_step(char *success) {
		ExprNode *ret = this;
		RETURN_IF_SUCCESS(simplify_evaluative(success));
		RETURN_IF_SUCCESS(simplify_elementary(success));
		RETURN_IF_SUCCESS(simplify_strange   (success));
		RETURN_IF_SUCCESS(simplify_structure (success)); // <<<<<<<<<<<<<<<< SEGFAULT
		return this;
	}

//=============================================================================
// Dumps ======================================================================
//=============================================================================

	void dump(FILE *file_ptr = stdout) const {
		if (type == VALUE) {
			fprintf(file_ptr, "%03lg", val);
		} else {
			fprintf(file_ptr, "{%c}", (char) val);
		}
	}

	void dump_space(FILE *file_ptr = stdout) const {
		if (L && R) {
			printf("(");
		}
		if (L) {
			L->dump_space(file_ptr);
		}
		if (type == VALUE) {
			fprintf(file_ptr, " %lg ", val);
		} else {
			fprintf(file_ptr, " %c ", (char) val);
		}
		if (R) {
			R->dump_space(file_ptr);
		}
		if (L && R) {
			printf(")");
		}
	}

	void latex_dump_son(FILE *file = stdout, const ExprNode *son = nullptr) const {
		if (!son) return;

		if (son->prior < prior) {
			fprintf(file, "\\left(");
		}
		son->latex_dump(file);
		if (son->prior < prior) {
			fprintf(file, "\\right)");
		}
	}

	#define OPDEF(ig1, sign, ig3, ig4, ig5, ig6, before, between, after) {       \
        case #sign[0]: {							                             \
        	fprintf(file, "%s", before);                                         \
        	latex_dump_son(file, L);                                             \
        	fprintf(file, "%s", between);                                        \
        	latex_dump_son(file, R);                                             \
        	fprintf(file, "%s", after);                                          \
        	break;                                                               \
        }                                                                        \
	}

	void latex_dump(FILE *file = stdout) const {
		if (type == VALUE) {
			if (val < 0) {
				fprintf(file, "\\left(");
			}
			fprintf(file, "%lg", val);
			if (val < 0) {
				fprintf(file, "\\right)");
			}
		} else if (type == VARIABLE) {
			fprintf(file, "%c", (char) val);
		} else {
			switch ((char) val) {

				#include "ops.dsl"

			}
		}
	}

	#undef OPDEF

};


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================


#ifndef EXPR_NODE_DECENDER
#define EXPR_NODE_DECENDER

ExprNodeDecender::ExprNodeDecender():
	op_node(nullptr),
	elem_node(nullptr)
	{}

ExprNodeDecender::~ExprNodeDecender() {}

void ExprNodeDecender::ctor() {
	op_node   = nullptr;
	elem_node = nullptr;
}

ExprNodeDecender *ExprNodeDecender::NEW() {
	ExprNodeDecender *cake = (ExprNodeDecender*) calloc(1, sizeof(ExprNodeDecender));
	if (!cake) {
		return nullptr;
	}

	cake->ctor();
	return cake;
}

void ExprNodeDecender::ctor(ExprNode *op_node_, int order_) {
	op_node   = op_node_;
	elem_node = op_node_;
	order     = order_;
}

ExprNodeDecender *ExprNodeDecender::NEW(ExprNode *op_node_, int order_) {
	ExprNodeDecender *cake = (ExprNodeDecender*) calloc(1, sizeof(ExprNodeDecender));
	if (!cake) {
		return nullptr;
	}

	cake->ctor(op_node_, order_);
	return cake;
}

void ExprNodeDecender::dtor() {
	op_node = nullptr;
}

void ExprNodeDecender::DELETE(ExprNodeDecender *classname) {
	if (!classname) {
		return;
	}

	classname->dtor();
	free(classname);
}

//=============================================================================

bool ExprNodeDecender::is_end() {
	return !op_node;
}

bool ExprNodeDecender::decend() {
	if (!op_node) {
		return false;
	}

	char op = op_node->val;

	ExprNode *L = op_node->get_L();

	if (!L) {
		dtor();
		return false;
	}

	if (L->type != OPERATION || (L->val != op && ! ((op == '*' && L->val == '/') || (op == '/' && L->val == '*')))) { //todo with /
		dtor();
		return false;
	}

	op_node   = L;
	elem_node = L->get_R();
	return true;
}

bool ExprNodeDecender::next() {
	if (!op_node) {
		return false;
	} else if (op_node->type != OPERATION) {
		return false;
	} else if (elem_node == nullptr) {
		return false;
	}

	ExprNode *L = op_node->get_L();
	ExprNode *R = op_node->get_R();

	if (order < 0) {
		if (elem_node == L) {
			elem_node = R;
			return true;
		}

		if (elem_node == R) {
			return decend();
		}

		elem_node = L;
		return true;
	} else {
		if (elem_node == R) {
			elem_node = L;
			return true;
		}

		if (elem_node == L) {
			return decend();
		}

		elem_node = R;
		return true;
	}
}

void ExprNodeDecender::set_operand(ExprNode *new_elem) {
	if (!op_node) {
		return;
	}

	if (elem_node == op_node->get_L()) {
		op_node->set_L(new_elem);
	}

	if (elem_node == op_node->get_R()) {
		op_node->set_R(new_elem);
	}

	elem_node = new_elem;
}

ExprNode *ExprNodeDecender::get_op_node() {
	return op_node;
}

ExprNode *ExprNodeDecender::get_elem_node() {
	return elem_node;
}

#endif // EXPR_NODE_DECENDER

#endif // EXPRESSION_NODE