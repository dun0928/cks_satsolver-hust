#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define MAX_VARS 100000
#define MAX_CLAUSES 1000000

// 常量定义
#define YES 1
#define NO 0
#define TRUE 1
#define FALSE 0
#define SAT 1
#define UNSAT 0
#define UNASSIGNED -1
#define ASSIGNED 1
#define SATISFIED 1//子句为真的时的标志，这个是在回溯中的标志点，决策层中的标志便于回溯，子句满足
#define SHRUNK 0//文字为假时的使得子句长度减少的标志，文字缩减
#define POSITIVE 1
#define NEGATIVE 0

typedef struct {
    int n_number;
    int* literal_clause;
    int* literal_clause_pos;
    int is_assigned;
    int is_in_unit_clause;
} LiteralInfo;

typedef struct {
    int* literals;
    int length_original;
    int length_current;
    int* literals_is_assigned;
    int is_clause_satisfied;
    int unit_cluase_literal;
} Clause;

typedef struct {
    int value;
} VariableResult;

typedef struct {
    int index_of_clause;// 受影响的子句索引
    int index_of_literal;// 受影响的文字在子句中的位置索引
} ChangeRecord;


int n_vars;
int original_formula_length;
int current_formula_length;
int max_clause_len;

LiteralInfo literal_info[MAX_VARS + 1][2];
Clause* clauses;
VariableResult result[MAX_VARS + 1];

int unit_clause_stack[MAX_CLAUSES];//全局单子句栈
int n_unit_clause_stack = 0;

ChangeRecord changes_stack[MAX_CLAUSES * 10];//记录变更时被影响的子句和文字
int changes_stack_index = 0;//栈顶

int n_changes[MAX_VARS * 2][2];// 各层的变更计数，0：第depth层产生的"子句满足"变更数量 1：第depth层产生的"文字移除"变更数量
int depth = 0;

int is_contradicted = FALSE;
int conflicting_literal = 0;

int dpll_called = 0;

// 函数声明
void Preprocesser();
void Value(int v);
void UnValue(int v);
int dpll();
int get_length_of();
void get_weight(int x, int k, unsigned int* s, unsigned int* t);
int GetMaxLenOfLiteral();
void PrintToRes(int value, double time, char* filename);
void Free();
int read_cnf_file(char* filename);

int main() {
    char filename[256];
    
    printf("请选择要读取的cnf文件: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("读取输入失败\n");
        return 1;
    }

    filename[strcspn(filename, "\n")] = '\0';

    if (strlen(filename) == 0) {
        printf("文件名不能为空\n");
        return 1;
    }
    
    clock_t start, end;
    double cpu_time_used;
    
    if (!read_cnf_file(filename)) {
        printf("Failed to read CNF file: %s\n", filename);
        return 1;
    }
    
    // 初始化结果数组
    for (int i = 1; i <= n_vars; i++) {
        result[i].value = UNASSIGNED;
    }
    
    // 预处理
    Preprocesser();
    
    // 开始计时
    start = clock();
    
    // 运行DPLL算法
    int sat_result = dpll();
    
    // 结束计时
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // 输出结果
    PrintToRes(sat_result, cpu_time_used, filename);
    
    // 释放内存
    Free();
    
    if (sat_result == SAT) {
        printf("SATISFIABLE\n");
        printf("Time used: %.2f seconds\n", cpu_time_used);
        printf("DPLL called: %d times\n", dpll_called);
    } else {
        printf("UNSATISFIABLE\n");
        printf("Time used: %.2f seconds\n", cpu_time_used);
        printf("DPLL called: %d times\n", dpll_called);
    }
    
    return 0;
}

int read_cnf_file(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file: %s\n", filename);
        return 0;
    }
    
    char line[1000];
    int clause_count = 0;
    int max_clause_size = 0;
    
    // 第一次读取，统计子句数量和最大子句长度,看懂了但不知第目的是什么，接着看
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &n_vars, &original_formula_length);
            clauses = (Clause*)malloc(original_formula_length * sizeof(Clause));
            if (clauses == NULL) {
                printf("Memory allocation failed\n");
                fclose(fp);
                return 0;
            }
            continue;
        }
        
        if (clause_count >= original_formula_length) break;
        
        char* token = strtok(line, " ");
        int literal_count = 0;
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break;
            literal_count++;
            token = strtok(NULL, " ");
        }
        
        if (literal_count > max_clause_size) {
            max_clause_size = literal_count;
        }
        clause_count++;
    }
    
    max_clause_len = max_clause_size;
    current_formula_length = original_formula_length;
    
    // 重置文件指针
    fseek(fp, 0, SEEK_SET);
    clause_count = 0;
    
    // 初始化literal_info
    for (int i = 1; i <= n_vars; i++) {
        for (int j = 0; j < 2; j++) {
            literal_info[i][j].n_number = 0;
            literal_info[i][j].literal_clause = NULL;
            literal_info[i][j].literal_clause_pos = NULL;
            literal_info[i][j].is_assigned = NO;
            literal_info[i][j].is_in_unit_clause = NO;
        }
    }
    
    // 第二次读取，实际读取数据
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == 'p') continue;
        if (clause_count >= original_formula_length) break;
        
        char* token = strtok(line, " ");
        int literal_count = 0;
        int literals[1000]; // 假设子句最多1000个文字
        
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break;
            literals[literal_count++] = literal;
            token = strtok(NULL, " ");
        }
        
        if (literal_count == 0) continue;
        
        clauses[clause_count].length_original = literal_count;
        clauses[clause_count].length_current = literal_count;
        clauses[clause_count].literals = (int*)malloc(literal_count * sizeof(int));
        clauses[clause_count].literals_is_assigned = (int*)malloc(literal_count * sizeof(int));
        clauses[clause_count].is_clause_satisfied = NO;
        clauses[clause_count].unit_cluase_literal = 0;
        
        if (clauses[clause_count].literals == NULL || clauses[clause_count].literals_is_assigned == NULL) {
            printf("Memory allocation failed\n");
            fclose(fp);
            return 0;
        }
        
        for (int i = 0; i < literal_count; i++) {
            clauses[clause_count].literals[i] = literals[i];
            clauses[clause_count].literals_is_assigned[i] = UNASSIGNED;
            
            int var = abs(literals[i]);

            int type = (literals[i] > 0) ? POSITIVE : NEGATIVE;

            if (var > n_vars || var < 1) {
                printf("Invalid variable number: %d\n", var);
                continue;
            }
            
            // literal_info数组
            literal_info[var][type].n_number++;
            literal_info[var][type].literal_clause = (int*)realloc(literal_info[var][type].literal_clause, 
                literal_info[var][type].n_number * sizeof(int));
            literal_info[var][type].literal_clause_pos = (int*)realloc(literal_info[var][type].literal_clause_pos, 
                literal_info[var][type].n_number * sizeof(int));
            
            literal_info[var][type].literal_clause[literal_info[var][type].n_number - 1] = clause_count;
            literal_info[var][type].literal_clause_pos[literal_info[var][type].n_number - 1] = i;
        }
        
        clause_count++;
    }
    
    fclose(fp);
    return 1;
}

//预处理，将单子句加入全局变量
void Preprocesser() {
    for (int i = 0; i < original_formula_length; i++) {
        if (clauses[i].length_original == 1) {
            unit_clause_stack[n_unit_clause_stack] = clauses[i].literals[0];
            ++n_unit_clause_stack;
        }
    }
}

//设置子句
void Value(int v) {
    int i;
    int p = abs(v), q = (v > 0) ? POSITIVE : NEGATIVE;//q在info的第二维中表示正负
    for (i = 0; i < literal_info[p][q].n_number; ++i) {
        int j = literal_info[p][q].literal_clause[i];
        if (clauses[j].is_clause_satisfied) continue;
        clauses[j].is_clause_satisfied = YES;//无论如何都满足的
        --current_formula_length;
        changes_stack[changes_stack_index++].index_of_clause = j;
        n_changes[depth][SATISFIED]++;
    }//这个如果要回溯，直接改为yes
    q = !q;//这一步骤处理相反属性的文字，就是去缩短子句长度

    for (i = 0; i < literal_info[p][q].n_number; ++i) {
        int j = literal_info[p][q].literal_clause[i];
        if (clauses[j].is_clause_satisfied) continue;
        int k = literal_info[p][q].literal_clause_pos[i];//找到出现的位置
        --clauses[j].length_current;//缩短子句长度
        clauses[j].literals_is_assigned[k] = ASSIGNED;
        changes_stack[changes_stack_index].index_of_clause = j;
        changes_stack[changes_stack_index++].index_of_literal = k;//这里与上面不同，因为回溯得找到位置再回溯的
        n_changes[depth][SHRUNK]++;
        if (clauses[j].length_current == 1) {//单子句
            int location = -1;
            for (int i_literal = 0; i_literal < clauses[j].length_original; i_literal++) {
                if (clauses[j].literals_is_assigned[i_literal] == UNASSIGNED) {
                    location = i_literal;
                    break;
                }
            }
            //if (location == -1) continue;
            
            int w = clauses[j].literals[location];
            int s = abs(w), t = (w > 0) ? POSITIVE : NEGATIVE;
            if (literal_info[s][(!t)].is_in_unit_clause == YES) {
                is_contradicted = TRUE;
                conflicting_literal = w;
            }//产生矛盾，既要又要
            else if (literal_info[s][t].is_in_unit_clause == NO) {
                unit_clause_stack[n_unit_clause_stack] = clauses[j].unit_cluase_literal = w;
                literal_info[s][t].is_in_unit_clause = YES;
                ++n_unit_clause_stack;
            }
        }
    }
    literal_info[p][NEGATIVE].is_assigned = YES;
    literal_info[p][POSITIVE].is_assigned = YES;//被赋值
    ++depth;
}

//取消设置
void UnValue(int v){
    int i;
    int p = abs(v), q = (v > 0) ? SATISFIED : SHRUNK;
    --depth;
    while (n_changes[depth][SHRUNK]){
        --n_changes[depth][SHRUNK];
        int j = changes_stack[--changes_stack_index].index_of_clause;
        int k = changes_stack[changes_stack_index].index_of_literal;
        ++clauses[j].length_current;
        if (clauses[j].length_current == 2) {
            int s = abs(clauses[j].unit_cluase_literal);
            int t = (clauses[j].unit_cluase_literal > 0) ? SATISFIED : SHRUNK;
            if (s >= 1 && s <= n_vars) {
                literal_info[s][t].is_in_unit_clause = NO;
            }
            clauses[j].unit_cluase_literal = 0;
        }
        clauses[j].literals_is_assigned[k] = UNASSIGNED;
    }
    while (n_changes[depth][SATISFIED])	{
        --n_changes[depth][SATISFIED];
        int j = changes_stack[--changes_stack_index].index_of_clause;
        clauses[j].is_clause_satisfied = NO;
        ++current_formula_length;
    }
    literal_info[p][SATISFIED].is_assigned = NO;
    literal_info[p][SHRUNK].is_assigned = NO;
}


int dpll() {
    ++dpll_called;
    if(dpll_called%10000==0){
        printf("%d",dpll_called/10000);
    }
    int* local_stack = NULL;
    unsigned int n_local_stack = 0;
    while (1) {
        if (is_contradicted) {
            int cl = abs(conflicting_literal);
            while (n_local_stack) {
                UnValue(local_stack[--n_local_stack]);
                int s = abs(local_stack[n_local_stack]);
                if (s >= 1 && s <= n_vars) {
                    result[s].value = UNASSIGNED;
                }
            }
            is_contradicted = FALSE;
            free(local_stack);
            n_unit_clause_stack = 0;
            return UNSAT;
        }
        else if (n_unit_clause_stack) {//这个是全局的单子句哈
            local_stack = (int*)realloc(local_stack, (n_local_stack + 1) * sizeof(int));
            if (local_stack == NULL) {
                printf("Memory allocation failed in dpll\n");
                exit(1);
            }
            int implied_lit = unit_clause_stack[--n_unit_clause_stack];
            local_stack[n_local_stack++] = implied_lit;//这个单子句正负不确定，是个文字
            int var = abs(implied_lit);
            if (var >= 1 && var <= n_vars) {
                result[var].value = implied_lit > 0 ? TRUE : FALSE;
            }
            Value(implied_lit);//单子句确定赋值
        }
        else break;
    }
    if (!current_formula_length) return SAT;//递归结束

    
    int v = GetMaxLenOfLiteral();//变元选取策略，与上面的不同，这里是选择一个比较合适的赋值，如果是单子句，直接赋值value
    int var_v = abs(v);
    if (var_v >= 1 && var_v <= n_vars) {
        result[var_v].value = v > 0 ? TRUE : FALSE;
    }
    Value(v);
    if (dpll()) return SAT;
    UnValue(v);
    if (var_v >= 1 && var_v <= n_vars) {
        result[var_v].value = !result[var_v].value;
    }
    Value(-v);
    if (dpll()) return SAT;
    UnValue(-v);
    if (var_v >= 1 && var_v <= n_vars) {
        result[var_v].value = UNASSIGNED;
    }
    while (n_local_stack) {
        int z = local_stack[--n_local_stack];
        UnValue(z);
        int var_z = abs(z);
        if (var_z >= 1 && var_z <= n_vars) {
            result[var_z].value = UNASSIGNED;
        }
    }
    free(local_stack);
    is_contradicted = FALSE;
    return UNSAT;
}

int get_length_of() {
    int min = max_clause_len;  
    if (min == 2) return 2;
    
    // 直接遍历所有子句
    for (int j = 0; j < original_formula_length; j++) {
        if (clauses[j].is_clause_satisfied) {
            continue;
        }
        if (clauses[j].length_current < min) {
            min = clauses[j].length_current;
            if (min == 2) {
                return 2;
            }
        }
    }
    
    return min;
}

void get_weight(int x, int k, unsigned int* s, unsigned int* t) {
    int j, c;
    *s = *t = 0;
    for (j = 0; j < literal_info[x][SATISFIED].n_number; ++j) {
        c = literal_info[x][SATISFIED].literal_clause[j];//获取包含正文字x的子句编号
        if (clauses[c].length_current == k)
            *s += 1 - clauses[c].is_clause_satisfied;
    }
    for (j = 0; j < literal_info[x][SHRUNK].n_number; ++j) {//获取包含负文字?x的子句编号
        c = literal_info[x][SHRUNK].literal_clause[j];
        if (clauses[c].length_current == k)
            *t += 1 - clauses[c].is_clause_satisfied;
    }
}

int GetMaxLenOfLiteral() {
    unsigned int i, k;
    unsigned int max = 0, r, s, t;
    int u = 0;
    k = get_length_of();//获取最短子句长度
    for (i = 1; i <= n_vars; ++i) {
        if (result[i].value == UNASSIGNED) {//选择没有被赋值的变元
            get_weight(i, k, &s, &t);//i在k子句中的权重
            r = (s + 1) * (t + 1);
            if (r > max) {
                max = r;
                if (s >= t) u = i;
                else u = -(int)i;
            }
        }
    }
    return u;
}

//将求解结果输出到res文件
void PrintToRes(int value, double time, char* filename) {
    char res_filename[256];
    strcpy(res_filename, filename);
    int len = strlen(res_filename);
    if (len > 3) {
        res_filename[len - 3] = 'r';
        res_filename[len - 2] = 'e';
        res_filename[len - 1] = 's';
    } else {
        strcat(res_filename, ".res");
    }
    
    FILE* fp = fopen(res_filename, "w");
    if (fp == NULL) {
        printf("打开文件%s失败\n", res_filename);
        return;
    }
    
    if (value == SAT) {
        fprintf(fp, "s 1\n");
        fprintf(fp, "v");
        for (int i = 1; i <= n_vars; i++) {
            if (result[i].value == TRUE) {
                fprintf(fp, " %d", i);
            } else {
                fprintf(fp, " -%d", i);
            }
        }
        fprintf(fp, "\n");
        fprintf(fp, "t %d\n", time*1000);
    } else if (value == UNSAT) {
        fprintf(fp, "s 0\n");
        fprintf(fp, "t %d\n", time*1000);
    }
    fclose(fp);
}

//释放分配的内存
void Free() {
    for (int i = 1; i <= n_vars; i++) {
        for (int j = 0; j < 2; j++) {
            if (literal_info[i][j].literal_clause) {
                free(literal_info[i][j].literal_clause);
                literal_info[i][j].literal_clause = NULL;
            }
            if (literal_info[i][j].literal_clause_pos) {
                free(literal_info[i][j].literal_clause_pos);
                literal_info[i][j].literal_clause_pos = NULL;
            }
        }
    }
    
    if (clauses) {
        for (int i = 0; i < original_formula_length; i++) {
            if (clauses[i].literals) {
                free(clauses[i].literals);
                clauses[i].literals = NULL;
            }
            if (clauses[i].literals_is_assigned) {
                free(clauses[i].literals_is_assigned);
                clauses[i].literals_is_assigned = NULL;
            }
        }
        free(clauses);
        clauses = NULL;
    }
}