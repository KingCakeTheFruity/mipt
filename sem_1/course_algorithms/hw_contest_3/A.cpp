#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>

template <typename T>
class AVLNode;

template <typename T>
int AVLNode_get_balance(const AVLNode<T> *cake) {
	if (cake == nullptr) {
		return 0;
	} else {
		int r = cake->R == nullptr ? 0 : cake->R->height;
	  	int l = cake->L == nullptr ? 0 : cake->L->height;
	  	return r - l;
	}
}

template <typename T>
void AVLNode_dump(const AVLNode<T> *cake, const int depth, const int to_format_cnt) {
	if (!cake) {return;}

	AVLNode_dump(cake->R, depth + 1, to_format_cnt + 1);

	for (int i = 0; i < depth; ++i) {
		printf("    ");
		if (depth - to_format_cnt- 1 <= i) {
			printf("|");
		} else {
			printf(" ");
		}
	}

	printf("%03d>|\n", cake->key);
	AVLNode_dump(cake->L, depth + 1, to_format_cnt + 1);
}

// Реализуем само дерево в виде ноды, оставив в классе AVLTree только интерфейс к взаимодействию с нодами
// Так пользователь не будет ничего знать о самих нодах, т.к. и не должен
template <typename T>
class AVLNode {
public:
	T key;
	AVLNode<T> *L;
	AVLNode<T> *R;
	int height;
	int cnt;
	bool multinode;

	AVLNode<T>() {
		key = 0;
		int cnt = 0;
		L = nullptr;
		R = nullptr;
		height = 1;
		multinode = 0;
	}

	AVLNode<T>(const T &new_key) {
		key = new_key;
		cnt = 1;
		L = nullptr;
		R = nullptr;
		height = 1;
		multinode = 0;
	}

	~AVLNode<T>() {
		key = 0;
		cnt = 0;
		L = nullptr;
		R = nullptr;
		height = 0;
		multinode = 0;
	}

	AVLNode<T> *&operator[](int i) {
		switch(i % 2) {
			case 0:
				return L;
			case 1:
				return R;
			default:
				printf("wtf man...\n");
				return R;
		}
	}

	void update() {
		int r = R ? R->height : 0;
  		int l = L ? L->height : 0;
  		height = (r > l ? r : l) + 1;
	}

	AVLNode<T> *rotate_left() {
		AVLNode<T>* node = R;
		R = node->L;
		node->L = this;
		this->update();
		node->update();

		return node;
	}

	AVLNode<T> *rotate_right() {
		AVLNode<T>* node = L;
		L = node->R;
		node->R = this;
		this->update();
		node->update();

		return node;
	}

	AVLNode<T> *rotate_left_big() {
		R = R->rotate_right();
		return this->rotate_left();
	}

	AVLNode<T> *rotate_right_big() {
		L = L->rotate_left();
		return this->rotate_right();
	}

	AVLNode<T> *balance() {
		int balance = AVLNode_get_balance(this);
		if (balance == 2) {
			balance = AVLNode_get_balance(R);
			if (balance == 1 || balance == 0) {
				return rotate_left();
			} else {
				return rotate_left_big();
			}
		} else if (balance == -2) {
			balance = AVLNode_get_balance(L);
			if (balance == -1 || balance == 0) {
				return rotate_right();
			} else {
				return 	rotate_right_big();
			}
		} else {
			return this;
		}
	}

	AVLNode<T> *insert(const T val) {
		if (key == val) {
			++cnt;
			return this;
		}

		int flag = val > key;
		(*this)[flag] = (*this)[flag] ? (*this)[flag]->insert(val) : new AVLNode(val);
		
		update();
		return balance();
	}

	AVLNode<T> *dislink(const T val) {
		if (val == key) {
			return R ? R : L;
		} else {
			int flag = val > key;
			(*this)[flag] = (*this)[flag]->dislink(val);
			update();
			return balance();
		}
	}

	AVLNode<T> *erase(T val) {
		if (val < key) {
			if (L) {
				L = L->erase(val);
				update();
				return balance();
			} else {
				return this;
			}
		} else if (val > key) {
			if (R) {
				R = R->erase(val);
				update();
				return balance();
			} else {
				return this;
			}
		} else {
			if (multinode) {
				--cnt;
				if (cnt > 0) {
					return this;
				}
			}

			if (!R) {
				AVLNode<T> *l = L;
				delete this;
				return l;
			}

			AVLNode<T> *r_min = R->min();
			R = R->dislink(r_min->key);
			r_min->R = R;
			r_min->L = L;

			delete this;

			r_min->update();
			return r_min->balance();;
		}
	}

	AVLNode<T> *min() {
		return L ? L->min() : this;
	}

	AVLNode<T> *max() {
		return R ? R->max() : this;
	}

	AVLNode<T> *find(const T val) {
		if (key == val) {
			return this;
		}

		int flag = key <= val;
		return (*this)[flag] ? (*this)[flag]->find(val) : nullptr;
	}

	AVLNode<T> *next(const T val) {
		if (key == val) {
			return R ? R->min() : nullptr;
		} else if (key < val) {
			AVLNode<T> *ret = R ? R->next(val) : nullptr;
			return ret ? ret : nullptr;
		} else {
			AVLNode<T> *ret = L ? L->next(val) : nullptr;
			return ret ? ret : this;
		}
	}

	AVLNode<T> *prev(const T val) {
		if (key == val) {
			return L ? L->max() : nullptr;
		} else if (key > val) {
			AVLNode<T> *ret = L ? L->prev(val) : nullptr;
			return ret ? ret : nullptr;
		} else {
			AVLNode<T> *ret = R ? R->prev(val) : nullptr;
			return ret ? ret : this;
		}
	}

	void dump(int depth = 0) {
		AVLNode_dump(this, depth, depth);
	}
};

template <typename T>
class AVLTree {
private:
	AVLNode<T> *root;
	size_t size;
public:
	AVLTree<T>() {
		root = nullptr;
		size = 0;
	}

	~AVLTree<T>() {
		recursive_node_delete(root);
	}

	void recursive_node_delete(AVLNode<T> *node) {
		if (!node) {return;}
		recursive_node_delete(node->L);
		recursive_node_delete(node->R);
		delete node;
	}

	void insert(const T val) {
		if (!root) {
			root = new AVLNode<T>(val);
		} else {
			root = root->insert(val);
		}
		++size;
	}

	void erase(const T val) {
		if (root) {
			if (root->find(val)) {
				root = root->erase(val);
			}
		}
		--size;
	}

	T *find(const T val) {
		if (!root) {
			return nullptr;
		}

		AVLNode<T> *ret = root->find(val);
		return ret ? &ret->key : nullptr;
	}

	T *min() {
		return root ? root->min()->key : (T) 0;
	}

	T *max() {
		return root ? root->max()->key : (T) 0;
	}

	T *next(const T val) {
		if (!root) {
			return nullptr;
		}

		AVLNode<T> *node = root->next(val);
		if (!node) {
			return nullptr;
		} else {
			return &node->key;
		}
	}

	T *prev(const T val) {
		if (!root) {
			return nullptr;
		}

		AVLNode<T> *node = root->prev(val);
		if (!node) {
			return nullptr;
		} else {
			return &node->key;
		}
	}

	void dump() {
		if (root) {
			root->dump();
		}
	}
};

// Решение задачи по написанию авл-дерево сведено к задаче написания авл-дерева
int main() {
	AVLTree<int> tree;

	char str[10] = {};
    while (scanf("%6s", str) == 1) {
        int x;
        scanf("%d", &x);
        if (       str[0] == 'i') {
            tree.insert(x);
        } else if (str[0] == 'd') {
            tree.erase(x);
        } else if (str[0] == 'e') {
        	printf("%s\n", tree.find(x) == nullptr ? "false" : "true");            
        } else if (str[0] == 'n') {
        	int *ret = tree.next(x);
        	if (ret) {
        		printf("%d\n", *ret);
        	} else {
        		printf("none\n");
        	}
        } else if(str[0] == 'p') {
        	int *ret = tree.prev(x);
        	if (ret) {
        		printf("%d\n", *ret);
        	} else {
        		printf("none\n");
        	}
        } else if (str[0] == 'd') {
            
        }
        //tree.dump();
    }

	return 0;
}

// AVL-дерево даровало нам свой логарифм для O(nlogn)