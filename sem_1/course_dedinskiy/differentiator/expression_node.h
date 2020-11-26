#include <cmath>

#include "general/c/common.h"
#include "general/cpp/vector.hpp"

template <typename T>
const T &min(const T &first, const T &second) {
	return first < second ? first : second;
}

template <typename T>
const T &max(const T &first, const T &second) {
	return second < first ? first : second;
}

enum EXPRNODE_TYPES {
	OPERATION = 1,
	VALUE     = 2,
	VARIABLE  = 3,
};

enum PRIORITIES {
	PRIOR_MAX   = 999,
	PRIOR_MIN   = 0,
	PRIOR_VALUE = 15,

	PRIOR_ADD  = 5,
	PRIOR_SUB  = 5,
	PRIOR_MUL  = 7,
	PRIOR_DIV  = 7,

	PRIOR_POW  = 9,
	PRIOR_SIN  = 15,
	PRIOR_COS  = 15,
	PRIOR_LOG  = 10,
};

enum SIMPLIFICATIONS_RESULTS {
	SIMPLIFIED_EVALUATIVE = 1,
	SIMPLIFIED_ELEMENTARY = 2,
	REORDERED_TREE        = 3,
	LINEARIZED_TREE       = 4,
	FOLDED_OPERATION      = 5,
};

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
			complexity = 1 + fabs(val);
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
#define IS_MUL(N) (IS_OP(N) && N->val == '*')
#define IS_POW(N) (IS_OP(N) && N->val == '^')

#define NEW_ZERO() NEW(VALUE, 0, PRIOR_VALUE)
#define NEW_ONE()  NEW(VALUE, 1, PRIOR_VALUE)

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

		if (R->type == OPERATION && R->val == op) {
			if (L->complexity > R->L->complexity) {
				ExprNode *rl = R->L;
				R->set_L(L);
				set_L(rl);

				*success = REORDERED_TREE;
				return *success;
			}
		} else {
			if (L->complexity > R->complexity) {
				ExprNode *r = R;
				set_R(L);
				set_L(r);

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

		while (L->type == OPERATION && R->type == OPERATION && L->val == op && R->val == op) {
			ExprNode *ll = L->L;
			ExprNode *lr = L->R;

			L->set_R(R);
			L->set_L(lr);
			set_R(L);
			set_L(ll);

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

	bool fold_addition(char *success) {
		if (!IS_ADD(this)) {
			return *success;
		}
		if (!IS_ADD(R)) {
			if (L->add(R)) {
				*success = FOLDED_OPERATION;
			}

			return *success;
		}

		if (R->L->add(L)) {
			*success = FOLDED_OPERATION;
		}

		return R->fold_addition(success);
	}

	bool fold_multiplication(char *success) {
		if (!IS_MUL(this)) {
			return *success;
		}
		if (!IS_MUL(R)) {
			if (R->multiply(L)) {
				*success = FOLDED_OPERATION;
			}

			return *success;
		}

		if (R->L->multiply(L)) {
			*success = FOLDED_OPERATION;
		}

		return R->fold_multiplication(success);
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

		//commutative_linearize('*', success);
		//commutative_reorder('*', success);
		//fold_multiplication(success);

		// ========================================================== op
		switch ((char) val) {
			case '+' : {
				if (commutative_linearize('+', success)) {
					break;
				} else if (commutative_reorder('+', success)) {
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
				} else {
					commutative_linearize('*', success);
					commutative_reorder('*', success);
					fold_multiplication(success);
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

			case '+' : {
				if (L->equivalent_absolute(R)) {
					DELETE(L);
					L = NEW(VALUE, 2, PRIOR_VALUE);

					this->ctor(OPERATION, '*', PRIOR_MUL, L, R);

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
		if (L) {
			printf("(");
			L->dump_space(file_ptr);
			printf(")");
		}
		if (type == VALUE) {
			fprintf(file_ptr, "%03lg", val);
		} else {
			fprintf(file_ptr, "{%c}", (char) val);
		}
		if (R) {
			printf("(");
			R->dump_space(file_ptr);
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