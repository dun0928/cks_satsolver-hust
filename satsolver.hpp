#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include "definition.h"


int num_vars, original_formula_length, current_formula_length, max_clause_length;
Clause *clauses;

int unit_clause_stack[MAX_CLAUSES];
int unit_stack_size = 0;

ChangeRecord change_stack[MAX_CLAUSES * 10];
int change_stack_top = 0;
int change_counts[MAX_VARS * 2][2]; // [depth][SATISFIED/SHRUNK]
int depth = 0;

int contradiction_found = FALSE;
int conflicting_literal = 0;
int dpll_call_count = 0;

/* 扁平结构替代 literal_info */
LiteralOccurrenceList *pos_literals, *neg_literals;
uint8_t *in_unit_pos, *in_unit_neg; // 标记是否在单位子句堆中
VariableResult results[MAX_VARS + 1];

int satsolver(char *filename);
int  read_cnf_file(char *filename);
void preprocess();
void assign_value(int literal);
void unassign_value(int literal);
int  dpll();
int  select_branching_variable();
void write_result(int result_value, double time_used, char *filename);
void free_memory(void);


int satsolver(char *filename) {
    
    if (strlen(filename) == 0) { puts("文件名不能为空"); return 1; }

    if (!read_cnf_file(filename)) { printf("读取CNF文件失败\n"); return 1; }

    for (int i = 1; i <= num_vars; i++) results[i].value = UNASSIGNED;

    preprocess();
    clock_t start = clock();
    int sat = dpll();
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    write_result(sat, elapsed, filename);

    if(strcmp(filename,"sudoku.cnf")==0){
        free_memory();
        return 0;
    }
    if (sat == SAT)  printf("可满足\n用时: %.2fs\nDPLL调用: %d\n", elapsed, dpll_call_count);
    else             printf("不可满足\n用时: %.2fs\nDPLL调用: %d\n", elapsed, dpll_call_count);

    free_memory();
    return 0;
    
}
int read_cnf_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return 0; }

    char line[1024];
    int clause_count = 0, max_len = 0;

    /* 第一次扫描：统计变量数、子句数、最大子句长度 */
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') { sscanf(line, "p cnf %d %d", &num_vars, &original_formula_length); continue; }
        int len = 0, lit;
        for (char *p = strtok(line, " \t\n"); p; p = strtok(NULL, " \t\n"))
            if ((lit = atoi(p)) != 0) len++;
        if (len > max_len) max_len = len;
        clause_count++;
    }
    max_clause_length = max_len;
    current_formula_length = original_formula_length;

    clauses = (Clause *)calloc(original_formula_length, sizeof(Clause));
    if (!clauses) { perror("clauses"); exit(1); }

    /* 分配文字出现列表 */
    pos_literals = (LiteralOccurrenceList *)calloc(num_vars + 1, sizeof(LiteralOccurrenceList));
    neg_literals = (LiteralOccurrenceList *)calloc(num_vars + 1, sizeof(LiteralOccurrenceList));
    in_unit_pos    = (uint8_t *)calloc(num_vars + 1, sizeof(uint8_t));
    in_unit_neg    = (uint8_t *)calloc(num_vars + 1, sizeof(uint8_t));

    /* 第二次扫描：填充子句 & 构建出现列表 */
    fseek(fp, 0, SEEK_SET);
    int c = 0;
    while (fgets(line, sizeof(line), fp) && c < original_formula_length) {
        if (line[0] == 'c' || line[0] == 'p') continue;
        int lits[1000], len = 0;
        for (char *p = strtok(line, " \t\n"); p; p = strtok(NULL, " \t\n")) {
            int lit = atoi(p);
            if (lit == 0) break;
            lits[len++] = lit;
        }
        if (len == 0) continue;

        clauses[c].original_length = len;
        clauses[c].current_length  = len;
        clauses[c].literals        = (int *)malloc(len * sizeof(int));
        clauses[c].assignment_status = (int *)malloc(len * sizeof(int));
        clauses[c].is_satisfied    = FALSE;
        clauses[c].unit_literal    = 0;
        if (!clauses[c].literals || !clauses[c].assignment_status) { perror("malloc"); exit(1); }

        for (int i = 0; i < len; i++) {
            int lit = lits[i], var = abs(lit), pol = lit > 0 ? POSITIVE : NEGATIVE;
            clauses[c].literals[i] = lit;
            clauses[c].assignment_status[i] = UNASSIGNED;

            LiteralOccurrenceList *L = pol ? &pos_literals[var] : &neg_literals[var];
            if (L->count == L->capacity) {
                L->capacity = L->capacity ? L->capacity * 2 : 4;
                L->list = (LiteralOccurrence *)realloc(L->list, L->capacity * sizeof(LiteralOccurrence));
            }
            L->list[L->count++] = (LiteralOccurrence){c, i};
        }
        c++;
    }
    fclose(fp);
    return 1;
}


void preprocess(void) {
    for (int i = 0; i < original_formula_length; i++)
        if (clauses[i].original_length == 1)
            unit_clause_stack[unit_stack_size++] = clauses[i].literals[0];
}

void assign_value(int literal) {
    int var = abs(literal), pol = literal > 0 ? POSITIVE : NEGATIVE;

    /* 满足子句 */
    LiteralOccurrenceList *plist = pol ? &pos_literals[var] : &neg_literals[var];
    for (int i = 0; i < plist->count; i++) {
        int c = plist->list[i].clause_index;
        if (clauses[c].is_satisfied) continue;
        clauses[c].is_satisfied = TRUE;
        current_formula_length--;
        change_stack[change_stack_top++] = (ChangeRecord){c, -1};
        change_counts[depth][SATISFIED]++;
    }

    /* 缩减相反极性 */
    LiteralOccurrenceList *nlist = pol ? &neg_literals[var] : &pos_literals[var];
    for (int i = 0; i < nlist->count; i++) {
        int c = nlist->list[i].clause_index, pos = nlist->list[i].literal_position;
        if (clauses[c].is_satisfied) continue;
        clauses[c].current_length--;
        clauses[c].assignment_status[pos] = ASSIGNED;
        change_stack[change_stack_top++] = (ChangeRecord){c, pos};
        change_counts[depth][SHRUNK]++;

        if (clauses[c].current_length == 1) {
            int unit_lit = 0;
            for (int j = 0; j < clauses[c].original_length; j++)
                if (clauses[c].assignment_status[j] == UNASSIGNED) { unit_lit = clauses[c].literals[j]; break; }
            if (unit_lit) {
                int uvar = abs(unit_lit), upol = unit_lit > 0;
                uint8_t *flag = upol ? &in_unit_pos[uvar] : &in_unit_neg[uvar];
                if ((upol ? in_unit_neg[uvar] : in_unit_pos[uvar])) {
                    contradiction_found = TRUE; conflicting_literal = unit_lit;
                } else if (!*flag) {
                    *flag = 1;
                    unit_clause_stack[unit_stack_size++] = unit_lit;
                    clauses[c].unit_literal = unit_lit;
                }
            }
        }
    }
    depth++;
}


void unassign_value(int literal) {
    int var = abs(literal);
    depth--;

    while (change_counts[depth][SHRUNK] > 0) {
        change_counts[depth][SHRUNK]--;
        ChangeRecord rec = change_stack[--change_stack_top];
        int c = rec.clause_index, pos = rec.literal_position;
        clauses[c].current_length++;
        if (clauses[c].current_length == 2 && clauses[c].unit_literal) {
            int uvar = abs(clauses[c].unit_literal), upol = clauses[c].unit_literal > 0;
            (upol ? in_unit_pos[uvar] : in_unit_neg[uvar]) = 0;
            clauses[c].unit_literal = 0;
        }
        clauses[c].assignment_status[pos] = UNASSIGNED;
    }

    while (change_counts[depth][SATISFIED] > 0) {
        change_counts[depth][SATISFIED]--;
        int c = change_stack[--change_stack_top].clause_index;
        clauses[c].is_satisfied = FALSE;
        current_formula_length++;
    }
}


int dpll(void)
{
    dpll_call_count++;

    /* ---------- 单位传播 ---------- */
    int *local_assign = NULL;
    int  local_count  = 0;

    while (1) {
        if (contradiction_found) {
            /* 回滚本次传播产生的所有赋值 */
            while (local_count) {
                int lit = local_assign[--local_count];
                unassign_value(lit);
                results[abs(lit)].value = UNASSIGNED;
            }
            free(local_assign);
            contradiction_found = FALSE;
            unit_stack_size     = 0;          /* 关键：清空单位堆 */
            return UNSAT;
        }

        if (unit_stack_size == 0) break;

        int unit_lit = unit_clause_stack[--unit_stack_size];
        local_assign = (int *) realloc(local_assign, (local_count + 1) * sizeof(int));
        local_assign[local_count++] = unit_lit;

        int var = abs(unit_lit);
        results[var].value = unit_lit > 0 ? TRUE : FALSE;
        assign_value(unit_lit);
    }

    /* ---------- 公式已满足 ---------- */
    if (current_formula_length == 0) {
        free(local_assign);
        return SAT;
    }

    /* ---------- 选择分支文字 ---------- */
    int branch_lit = select_branching_variable();
    int var        = abs(branch_lit);

    /* 尝试正文字 */
    results[var].value = branch_lit > 0 ? TRUE : FALSE;
    assign_value(branch_lit);
    if (dpll()) {
        free(local_assign);
        return SAT;
    }
    unassign_value(branch_lit);               /* 回溯 */

    /* 尝试负文字 */
    results[var].value = branch_lit > 0 ? FALSE : TRUE;
    assign_value(-branch_lit);
    if (dpll()) {
        free(local_assign);
        return SAT;
    }
    unassign_value(-branch_lit);

    /* ---------- 恢复变量状态 ---------- */
    results[var].value = UNASSIGNED;

    /* 回滚本次传播产生的赋值 */
    while (local_count) {
        int lit = local_assign[--local_count];
        unassign_value(lit);
        results[abs(lit)].value = UNASSIGNED;
    }
    free(local_assign);

    /* 清空单位堆 & 矛盾标志 */
    unit_stack_size     = 0;
    contradiction_found = FALSE;

    return UNSAT;
}


int get_min_clause_length(void) {
    int min = max_clause_length;
    for (int i = 0; i < original_formula_length; i++)
        if (!clauses[i].is_satisfied && clauses[i].current_length < min)
            min = clauses[i].current_length;
    return min;
}

int select_branching_variable(void) {
    int len = get_min_clause_length();
    unsigned int max = 0, best = 1;
    for (int v = 1; v <= num_vars; v++) {
        if (results[v].value != UNASSIGNED) continue;
        unsigned int pc = 0, nc = 0;
        for (int i = 0; i < pos_literals[v].count; i++) {
            int c = pos_literals[v].list[i].clause_index;
            if (!clauses[c].is_satisfied && clauses[c].current_length == len) pc++;
        }
        for (int i = 0; i < neg_literals[v].count; i++) {
            int c = neg_literals[v].list[i].clause_index;
            if (!clauses[c].is_satisfied && clauses[c].current_length == len) nc++;
        }
        unsigned int score = (pc + 1) * (nc + 1);
        if (score > max) { max = score; best = pc >= nc ? v : -v; }
    }
    return best;
}


void write_result(int sat, double t, char *fn) {
    char out[512];
    strcpy(out, fn);
    int L = strlen(out);
    if (L > 3) strcpy(out + L - 3, "res");
    else strcat(out, ".res");
    FILE *fp = fopen(out, "w");
    if (!fp) { perror("write"); return; }
    if (sat == SAT) {
        fprintf(fp, "s 1\nv");
        for (int i = 1; i <= num_vars; i++) fprintf(fp, " %d", results[i].value == TRUE ? i : -i);
        fprintf(fp, "\nt %.0f\n", t * 1000);
    } else fprintf(fp, "s 0\nt %.0f\n", t * 1000);
    fclose(fp);
}

void free_memory(void) {
    for (int i = 1; i <= num_vars; i++) {
        free(pos_literals[i].list);
        free(neg_literals[i].list);
    }
    free(pos_literals); free(neg_literals);
    free(in_unit_pos); free(in_unit_neg);
    for (int i = 0; i < original_formula_length; i++) {
        free(clauses[i].literals);
        free(clauses[i].assignment_status);
    }
    free(clauses);
}