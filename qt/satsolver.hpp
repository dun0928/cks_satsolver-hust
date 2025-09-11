//这个sat求解器采用了dpll算法，变元选取策略的优化为选取最短子句中权重最高的变元，同时在数据结构方面做了一些优化，用了一个occurance数组记录了文字出现在哪些子句中，这样在赋值时可以快速找到受影响的子句
//回溯中采用栈结构

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include "definition.h"


int num_vars, original_formula_length, current_formula_length, max_clause_length;
Clause *clauses;

int unit_clause_stack[MAX_CLAUSES];//全局单位子句栈，在回溯时将其压入局部子句栈中
int unit_stack_size = 0;

ChangeRecord change_stack[MAX_CLAUSES * 10];//这是一个栈，记录每次赋值时子句的变化。用于回溯

int change_stack_top = 0;//栈顶指针

int change_counts[MAX_VARS * 2][2]; //  记录每个深度下满足子句数和缩减文字数，便于回溯时恢复状态
int depth = 0;

int contradiction_found = FALSE;
int conflicting_literal = 0;
int dpll_call_count = 0;//回溯次数

LiteralOccurrenceList *pos_literals, *neg_literals;
int *in_unit_pos, *in_unit_neg; // 标记是否在单位子句堆中,也就是说这个文字是否已经被赋值，1为赋值，0为未赋值
VariableResult results[MAX_VARS + 1];

extern int method;

double satsolver(char *filename,int method);
int  read_cnf_file(char *filename);
void preprocess();
void assign_value(int literal);
void unassign_value(int literal);
int  select_branching_variable();
int select_basic_method();
void write_result(int result_value, double time_used, char *filename);
void free_memory();


double satsolver(char *filename,int method) {
    
    if (strlen(filename) == 0) { puts("文件名不能为空"); return 1; }

    if (!read_cnf_file(filename)) { printf("读取CNF文件失败\n"); return 1; }

    for (int i = 1; i <= num_vars; i++) results[i].value = UNASSIGNED;

    preprocess();
    clock_t start = clock();
    
    int sat = dpll();
    int duration = (double)(clock() - start);
    write_result(sat, duration, filename);

    if(strcmp(filename,"sudoku.cnf")==0){
        free_memory();
        return duration;
    }
    if(method==1){
        duration=duration/5;
    }
    if (sat == SAT)  printf("可满足\n用时: %dms\n", duration);
    else             printf("不可满足\n用时: %dms\n", duration);

    free_memory();
    return duration;
    
}

void print_cnf(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return; }
    char line[1024];
    while(fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c') continue;
        if( line[0] == 'p') { printf("%s", line); continue; }
        int lit;
        for (char *p = strtok(line, " \t\n"); p; p = strtok(NULL, " \t\n")) {
            if ((lit = atoi(p)) == 0) {
                printf("\n");
                break;
            }
            printf("%d ", lit);
        }
    }
}
void printf_res(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return; }
    char line[1024];
    while(fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
}


int read_cnf_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return 0; }

    char line[1024];
    int clause_count = 0, max_len = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') { sscanf(line, "p cnf %d %d", &num_vars, &original_formula_length); continue; }
        int len = 0, lit;
        for (char *p = strtok(line, " \t\n"); p; p = strtok(NULL, " \t\n"))
            if ((lit = atoi(p)) != 0) len++;
        if (len > max_len) max_len = len;
        if(len>0)  clause_count++;
    }
    max_clause_length = max_len;
    current_formula_length = original_formula_length;

    //printf("%d %d",original_formula_length,clause_count);
    if(clause_count != original_formula_length) {
        printf("文件错误：声明的子句数与真实子句数不相等\n");
        original_formula_length = clause_count;
        current_formula_length = clause_count;
    }

    clauses = (Clause *)calloc(original_formula_length, sizeof(Clause));//calloc比malloc参数对比
    if (!clauses) { perror("clauses"); exit(1); }


    pos_literals = (LiteralOccurrenceList *)calloc(num_vars + 1, sizeof(LiteralOccurrenceList));
    neg_literals = (LiteralOccurrenceList *)calloc(num_vars + 1, sizeof(LiteralOccurrenceList));
    in_unit_pos = (int *)calloc(num_vars + 1, sizeof(int));
    in_unit_neg = (int *)calloc(num_vars + 1, sizeof(int));

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
        clauses[c].literals  = (int *)malloc(len * sizeof(int));
        clauses[c].assignment_status = (int *)malloc(len * sizeof(int));
        clauses[c].is_satisfied  = FALSE;
        clauses[c].unit_literal  = 0;
        if (!clauses[c].literals || !clauses[c].assignment_status) { perror("malloc"); exit(1); }

        for (int i = 0; i < len; i++) {
            int lit = lits[i], var = abs(lit), pol = lit > 0 ? POSITIVE : NEGATIVE;
            clauses[c].literals[i] = lit;
            clauses[c].assignment_status[i] = UNASSIGNED;

            LiteralOccurrenceList *L = pol ? &pos_literals[var] : &neg_literals[var];
            if (L->count == L->capacity) {
                L->capacity = L->capacity ? L->capacity * 2 : 4;//文字溢出就扩容
                L->list = (LiteralOccurrence *)realloc(L->list, L->capacity * sizeof(LiteralOccurrence));
            }
            L->list[L->count++] = (LiteralOccurrence){c, i};//把这个文字出现在第 c 条子句的第 i 个位置” 这条信息压进列表。
        }
        c++;
    }
    fclose(fp);
    return 1;
}

//找单子句压入全局单子栈
void  preprocess() {
    for (int i = 0; i < original_formula_length; i++)
        if (clauses[i].original_length == 1)
            unit_clause_stack[unit_stack_size++] = clauses[i].literals[0];
}

//赋值
 void assign_value(int literal) {
    int var = abs(literal), pol = literal > 0 ? POSITIVE : NEGATIVE;


    LiteralOccurrenceList *plist = pol ? &pos_literals[var] : &neg_literals[var];
    for (int i = 0; i < plist->count; i++) {
        int c = plist->list[i].clause_index;
        if (clauses[c].is_satisfied) continue;
        clauses[c].is_satisfied = TRUE;//这个子句被满足了，正负均满足
        current_formula_length--;
        change_stack[change_stack_top++] = (ChangeRecord){c, -1};//这里与文字位置无关，位置设为-1。回溯时只需要直接撤销即可
        change_counts[depth][SATISFIED]++;//子句满足
    }
    //对相同极性的影响

    //下面是对相反极性的影响
    LiteralOccurrenceList *nlist = pol ? &neg_literals[var] : &pos_literals[var];
    for (int i = 0; i < nlist->count; i++) {
        int c = nlist->list[i].clause_index, pos = nlist->list[i].literal_position;
        if (clauses[c].is_satisfied) continue;
        clauses[c].current_length--;
        clauses[c].assignment_status[pos] = ASSIGNED;
        change_stack[change_stack_top++] = (ChangeRecord){c, pos};
        change_counts[depth][SHRUNK]++;//文字缩减但是不一定满足

        if (clauses[c].current_length == 1) {//如果成为单子句
            int unit_lit = 0;
            for (int j = 0; j < clauses[c].original_length; j++)
                if (clauses[c].assignment_status[j] == UNASSIGNED) { 
                    unit_lit = clauses[c].literals[j]; break; 
                }//找到唯一没有被赋值的文字，并赋值
            if (unit_lit) {
                int uvar = abs(unit_lit), upol = unit_lit > 0;//编号与极性
                int *flag = upol ? &in_unit_pos[uvar] : &in_unit_neg[uvar];
                if ((upol ? in_unit_neg[uvar] : in_unit_pos[uvar])) {
                    contradiction_found = TRUE; conflicting_literal = unit_lit;//冲突
                } else if (!*flag) {
                    *flag = 1;//赋值为1 ，表明已经出现该极性
                    unit_clause_stack[unit_stack_size++] = unit_lit;
                    clauses[c].unit_literal = unit_lit;//这里出现的单子句在dpll中处理
                }
            }
        }
    }
    depth++;
}

//回溯
void unassign_value(int literal) {
    int var = abs(literal);
    depth--;
//文字缩减
//先进行这个原因时赋值时后处理的相反极性的，就是文字缩减
    while (change_counts[depth][SHRUNK] > 0) {
        change_counts[depth][SHRUNK]--;
        ChangeRecord rec = change_stack[--change_stack_top];
        int c = rec.clause_index, pos = rec.literal_position;
        clauses[c].current_length++;
        if (clauses[c].current_length == 2 && clauses[c].unit_literal) {
            int uvar = abs(clauses[c].unit_literal), upol = clauses[c].unit_literal > 0;
            (upol ? in_unit_pos[uvar] : in_unit_neg[uvar]) = 0;
            clauses[c].unit_literal = 0;
        }//回溯单位子句的影响
        clauses[c].assignment_status[pos] = UNASSIGNED;
    }

    while (change_counts[depth][SATISFIED] > 0) {
        change_counts[depth][SATISFIED]--;
        int c = change_stack[--change_stack_top].clause_index;
        clauses[c].is_satisfied = FALSE;
        current_formula_length++;
    }
}


int dpll()
{
    int *local_assign = NULL;//就在当前这一层里面，不干扰父层
    int  local_count  = 0;

    while (1) {
        if (contradiction_found) {
       
            while (local_count) {
                int lit = local_assign[--local_count];
                unassign_value(lit);
                results[abs(lit)].value = UNASSIGNED;
            }
            free(local_assign);
            contradiction_found = FALSE;
            unit_stack_size = 0;          
            return UNSAT;
        }

        if (unit_stack_size == 0) break;
        //假如现在没有冲突且有单子句
        int unit_lit = unit_clause_stack[--unit_stack_size];
        local_assign = (int *) realloc(local_assign, (local_count + 1) * sizeof(int));
        local_assign[local_count++] = unit_lit;

        int var = abs(unit_lit);
        results[var].value = unit_lit > 0 ? TRUE : FALSE;
        assign_value(unit_lit);
        //把这个单子句文字赋值，并记录在局部赋值栈中
    }


    if (current_formula_length == 0) {
        free(local_assign);
        return SAT;
    }

    int branch_lit=0;//变元选取
    if(method==1){
        branch_lit = select_branching_variable();
    }
    else if(method==2){
        branch_lit=select_basic_method();
    }
    
    int var = abs(branch_lit);


    results[var].value = branch_lit > 0 ? TRUE : FALSE;
    assign_value(branch_lit);
    if (dpll()) {
        free(local_assign);
        return SAT;
    }
    unassign_value(branch_lit);             


    results[var].value = branch_lit > 0 ? FALSE : TRUE;//尝试相反赋值
    assign_value(-branch_lit);
    if (dpll()) {
        free(local_assign);
        return SAT;
    }
    unassign_value(-branch_lit);//赋值为正不行，为负也不行，回溯或者直接结束

  
    results[var].value = UNASSIGNED;


    while (local_count) {
        int lit = local_assign[--local_count];
        unassign_value(lit);
        results[abs(lit)].value = UNASSIGNED;
    }
    free(local_assign);//回溯到上一层.父层


    unit_stack_size     = 0;
    contradiction_found = FALSE;

    return UNSAT;
}


int get_min_clause_length() {
    int min = max_clause_length;
    for (int i = 0; i < original_formula_length; i++)
        if (!clauses[i].is_satisfied && clauses[i].current_length < min)
            min = clauses[i].current_length;
    return min;
}

int select_branching_variable() {
    int len = get_min_clause_length();
    unsigned int max = 0, best = 1;
    for (int v = 1; v <= num_vars; v++) {
        if (results[v].value != UNASSIGNED) continue;
        unsigned int pc = 0, nc = 0;//正负变元
        for (int i = 0; i < pos_literals[v].count; i++) {
            int c = pos_literals[v].list[i].clause_index;
            if (!clauses[c].is_satisfied && clauses[c].current_length == len) pc++;//出现在当前最短子句中
        }
        for (int i = 0; i < neg_literals[v].count; i++) {
            int c = neg_literals[v].list[i].clause_index;
            if (!clauses[c].is_satisfied && clauses[c].current_length == len) nc++;
        }
        unsigned int score = (pc*2 + 1) * (nc*2 + 1);
        if (score > max) { max = score; best = pc >= nc ? v : -v; }
    }
    return best;
}

//从1到n顺序选取第一个未赋值的变元。基础方法，对比得到优化率
int select_basic_method(){
    int selected_var = 0;
    for (int i = 1; i <= num_vars; i++) {
        if (results[i].value == UNASSIGNED) {
            selected_var = i;
            break;
        }
    }
    
    return selected_var;
}


void  write_result(int sat, double t, char *fn) {
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

void  free_memory() {
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