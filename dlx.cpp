#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include <unistd.h>

struct box;
struct linked_matrix;

linked_matrix *linked_matrix_from_boolean_rows(const std::vector<std::vector<int>>& rows, unsigned secondary = 0);
uint64_t solve(linked_matrix *lm);
uint64_t dlx(linked_matrix *lm, std::vector<int>& stack);
void cover_column(linked_matrix *lm, box *col);
void uncover_column(linked_matrix *lm, box *col);
void print_vector(const std::vector<int>& xs);
void print_solution(const std::vector<int>& rows);

bool opt_print_solutions;
bool opt_verbose;
std::vector<std::vector<int>> input_rows;

int main(int argc, char *argv[]) {
	for (;;) {
		switch (getopt(argc, argv, "pv")) {
			case -1:
				goto getopt_done;
			case 'p':
				opt_print_solutions = true;
				break;
			case 'v':
				opt_verbose = true;
				break;
		}
	}
	getopt_done:

	unsigned width = 0;
	unsigned secondary_columns = 0;
	std::cin >> width >> secondary_columns;
	while (std::cin) {
		std::vector<int> row(width);
		for (unsigned i = 0; i < width; ++i) {
			std::cin >> row[i];
		}
		input_rows.emplace_back(row);
	}
	linked_matrix *lm = linked_matrix_from_boolean_rows(input_rows, secondary_columns);

	std::cout << solve(lm) << std::endl;
}

struct box {
	int x, y, size;
	box *l, *r, *u, *d;
	box() {
		l = r = u = d = this;
	}
	void hide_lr() { l->r = r, r->l = l; }
	void show_lr() { l->r = r->l = this; }
	void hide_ud() { u->d = d, d->u = u; }
	void show_ud() { u->d = d->u = this; }
	void link_l(box *b) { b->r = this, b->l = l; l = l->r = b; }
	void link_u(box *b) { b->d = this, b->u = u; u = u->d = b; }
};

struct linked_matrix {
	box *root;
	std::vector<box *> cols;
};

uint64_t solve(linked_matrix *lm) {
	std::vector<int> stack;
	return dlx(lm, stack);
}

uint64_t dlx(linked_matrix *lm, std::vector<int>& stack) {
	static uint64_t counter;
	if (lm->root->r == lm->root) {
		if (opt_print_solutions) {
			print_solution(stack);
		}
		++counter;
		if ((counter & (counter - 1)) == 0) {
			std::cerr << "solutions found: " << counter << std::endl;
		}
		return 1;
	}
	box *col = lm->root->r;
	int min_size = col->size;
	for (box *c = col->r; c != lm->root; c = c->r) {
		if (c->size < min_size) {
			min_size = c->size;
			col = c;
		}
	}
	if (min_size < 1) {
		return 0;
	}
	uint64_t solutions = 0;
	cover_column(lm, col);
	for (box *row = col->d; row != col; row = row->d) {
		stack.push_back(row->y);
		for (box *cell = row->r; cell != row; cell = cell->r) {
			cover_column(lm, lm->cols[cell->x]);
		}
		solutions += dlx(lm, stack);
		for (box *cell = row->l; cell != row; cell = cell->l) {
			uncover_column(lm, lm->cols[cell->x]);
		}
		stack.pop_back();
	}
	uncover_column(lm, col);
	return solutions;
}

linked_matrix *linked_matrix_from_boolean_rows(const std::vector<std::vector<int>>& rows, unsigned secondary) {
	linked_matrix *lm = new linked_matrix;
	lm->root = new box;
	if (rows.empty()) {
		return lm;
	}
	unsigned width = rows[0].size();
	lm->cols.resize(width);
	for (unsigned i = 0; i < width; ++i) {
		box *col = new box;
		col->size = 0;
		col->x = i;
		lm->cols[i] = col;
		if (i >= secondary) {
			lm->root->link_l(col);
		}
	}
	for (unsigned i = 0; i < rows.size(); ++i) {
		auto& matrix_row = rows[i];
		if (matrix_row.size() != width) {
			return nullptr;
		}
		box *row = new box;
		for (unsigned j = 0; j < width; ++j) {
			if (matrix_row[j] == 0) {
				continue;
			}
			box *cell = new box;
			cell->x = j;
			cell->y = i;
			lm->cols[j]->link_u(cell);
			++lm->cols[j]->size;
			row->link_l(cell);
		}
		row->hide_lr();
		delete row;
	}
	return lm;
}

void dump(box *root) {
	std::cout << "dump" << std::endl;
	for (box *col = root->r; col != root; col = col->r) {
		for (box *cell = col->d; cell != col; cell = cell->d) {
			std::cout << " " << cell->y;
		}
		std::cout << " (" << col->size << ")";
		std::cout << std::endl;
	}
}

void print_vector(const std::vector<int>& xs) {
	bool first = true;
	for (int x : xs) {
		if (!first) {
			std::cout << " ";
		}
		first = false;
		std::cout << x;
	}
	std::cout << std::endl;
}

void print_solution(const std::vector<int>& row_indices) {
	if (opt_verbose) {
		for (int i : row_indices) {
			print_vector(input_rows[i]);
		}
		std::cout << std::endl;
	}
	else {
		print_vector(row_indices);
	}
}

void cover_column(linked_matrix *lm, box *col) {
	col->hide_lr();
	for (box *row = col->d; row != col; row = row->d) {
		for (box *cell = row->r; cell != row; cell = cell->r) {
			cell->hide_ud();
			--lm->cols[cell->x]->size;
		}
	}
}

void uncover_column(linked_matrix *lm, box *col) {
	for (box *row = col->u; row != col; row = row->u) {
		for (box *cell = row->l; cell != row; cell = cell->l) {
			cell->show_ud();
			++lm->cols[cell->x]->size;
		}
	}
	col->show_lr();
}
