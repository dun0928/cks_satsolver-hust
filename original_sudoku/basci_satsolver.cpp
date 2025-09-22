#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_VARS 100000
#define MAX_CLAUSES 1000000

// ��������
#define TRUE 1
#define FALSE 0
#define SAT 1
#define UNSAT 0
#define UNASSIGNED -1
#define ASSIGNED 1
#define SATISFIED 1
#define SHRUNK 0
#define POSITIVE 1
#define NEGATIVE 0

typedef struct {
    int count;
    int* clause_indices;
    int* literal_positions;
    int is_assigned;
    int in_unit_clause;
} LiteralInfo;

typedef struct {
    int* literals;
    int original_length;
    int current_length;
    int* assignment_status;
    int is_satisfied;
    int unit_literal;
} Clause;

typedef struct {
    int value;
} VariableResult;

typedef struct {
    int clause_index;
    int literal_position;
} ChangeRecord;

int num_vars;
int original_formula_length;
int current_formula_length;
int max_clause_length;

LiteralInfo literal_info[MAX_VARS + 1][2];
Clause* clauses;
VariableResult results[MAX_VARS + 1];

int unit_clause_stack[MAX_CLAUSES];
int unit_stack_size = 0;

ChangeRecord change_stack[MAX_CLAUSES * 10];
int change_stack_top = 0;

int change_counts[MAX_VARS * 2][2];
int depth = 0;

int contradiction_found = FALSE;
int conflicting_literal = 0;

int dpll_call_count = 0;

// ��������
void preprocess();
void assign_value(int literal);
void unassign_value(int literal);
int dpll();
int get_min_clause_length();
void get_literal_weight(int var, int clause_len, unsigned int* pos_weight, unsigned int* neg_weight);
int select_branching_variable();
void write_result(int result_value, double time_used, char* filename);
void free_memory();
int read_cnf_file(char* filename);

int main() {
    char filename[256];
    
    printf("������Ҫ��ȡ��cnf�ļ�: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("��ȡ����ʧ��\n");
        return 1;
    }

    filename[strcspn(filename, "\n")] = '\0';

    if (strlen(filename) == 0) {
        printf("�ļ�������Ϊ��\n");
        return 1;
    }
    
    clock_t start_time, end_time;
    double cpu_time_used;
    
    if (!read_cnf_file(filename)) {
        printf("��ȡCNF�ļ�ʧ��: %s\n", filename);
        return 1;
    }
    
    // ��ʼ���������
    for (int i = 1; i <= num_vars; i++) {
        results[i].value = UNASSIGNED;
    }
    
    // Ԥ����
    preprocess();
    
    // ��ʼ��ʱ
    start_time = clock();
    
    // ����DPLL�㷨
    int sat_result = dpll();
    
    // ������ʱ
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // ������
    write_result(sat_result, cpu_time_used, filename);
    
    // �ͷ��ڴ�
    free_memory();
    
    if (sat_result == SAT) {
        printf("������\n");
        printf("��ʱ: %.2f ��\n", cpu_time_used);
        printf("DPLL���ô���: %d\n", dpll_call_count);
    } else {
        printf("��������\n");
        printf("��ʱ: %.2f ��\n", cpu_time_used);
        printf("DPLL���ô���: %d\n", dpll_call_count);
    }
    
    return 0;
}

int read_cnf_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("�޷����ļ�: %s\n", filename);
        return 0;
    }
    
    char line[1000];
    int clause_count = 0;
    int max_clause_size = 0;
    
    // ��һ�ζ�ȡ��ͳ����Ϣ
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &num_vars, &original_formula_length);
            clauses = (Clause*)malloc(original_formula_length * sizeof(Clause));
            if (clauses == NULL) {
                printf("�ڴ����ʧ��\n");
                fclose(file);
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
    
    max_clause_length = max_clause_size;
    current_formula_length = original_formula_length;
    
    // �����ļ�ָ��
    fseek(file, 0, SEEK_SET);
    clause_count = 0;
    
    // ��ʼ��literal_info
    for (int i = 1; i <= num_vars; i++) {
        for (int j = 0; j < 2; j++) {
            literal_info[i][j].count = 0;
            literal_info[i][j].clause_indices = NULL;
            literal_info[i][j].literal_positions = NULL;
            literal_info[i][j].is_assigned = FALSE;
            literal_info[i][j].in_unit_clause = FALSE;
        }
    }
    
    // �ڶ��ζ�ȡ��ʵ�ʶ�ȡ����
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c' || line[0] == 'p') continue;
        if (clause_count >= original_formula_length) break;
        
        char* token = strtok(line, " ");
        int literal_count = 0;
        int literals[1000];
        
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break;
            literals[literal_count++] = literal;
            token = strtok(NULL, " ");
        }
        
        if (literal_count == 0) continue;
        
        clauses[clause_count].original_length = literal_count;
        clauses[clause_count].current_length = literal_count;
        clauses[clause_count].literals = (int*)malloc(literal_count * sizeof(int));
        clauses[clause_count].assignment_status = (int*)malloc(literal_count * sizeof(int));
        clauses[clause_count].is_satisfied = FALSE;
        clauses[clause_count].unit_literal = 0;
        
        if (clauses[clause_count].literals == NULL || clauses[clause_count].assignment_status == NULL) {
            printf("�ڴ����ʧ��\n");
            fclose(file);
            return 0;
        }
        
        for (int i = 0; i < literal_count; i++) {
            int literal = literals[i];
            clauses[clause_count].literals[i] = literal;
            clauses[clause_count].assignment_status[i] = UNASSIGNED;
            
            int var = abs(literal);
            int polarity = (literal > 0) ? POSITIVE : NEGATIVE;

            if (var > num_vars || var < 1) {
                printf("��Ч�������: %d\n", var);
                continue;
            }
            
            literal_info[var][polarity].count++;
            literal_info[var][polarity].clause_indices = (int*)realloc(
                literal_info[var][polarity].clause_indices, 
                literal_info[var][polarity].count * sizeof(int));
            literal_info[var][polarity].literal_positions = (int*)realloc(
                literal_info[var][polarity].literal_positions, 
                literal_info[var][polarity].count * sizeof(int));
            
            literal_info[var][polarity].clause_indices[literal_info[var][polarity].count - 1] = clause_count;
            literal_info[var][polarity].literal_positions[literal_info[var][polarity].count - 1] = i;
        }
        
        clause_count++;
    }
    
    fclose(file);
    return 1;
}

void preprocess() {
    for (int i = 0; i < original_formula_length; i++) {
        if (clauses[i].original_length == 1) {
            unit_clause_stack[unit_stack_size] = clauses[i].literals[0];
            unit_stack_size++;
        }
    }
}

void assign_value(int literal) {
    int var = abs(literal);
    int polarity = (literal > 0) ? POSITIVE : NEGATIVE;
    
    // ������ͬ���Ե����֣������Ӿ䣩
    for (int i = 0; i < literal_info[var][polarity].count; i++) {
        int clause_idx = literal_info[var][polarity].clause_indices[i];
        if (clauses[clause_idx].is_satisfied) continue;
        
        clauses[clause_idx].is_satisfied = TRUE;
        current_formula_length--;
        change_stack[change_stack_top].clause_index = clause_idx;
        change_stack_top++;
        change_counts[depth][SATISFIED]++;
    }
    
    // �����෴���Ե����֣������Ӿ䣩
    int opposite_polarity = !polarity;
    for (int i = 0; i < literal_info[var][opposite_polarity].count; i++) {
        int clause_idx = literal_info[var][opposite_polarity].clause_indices[i];
        if (clauses[clause_idx].is_satisfied) continue;
        
        int literal_pos = literal_info[var][opposite_polarity].literal_positions[i];
        clauses[clause_idx].current_length--;
        clauses[clause_idx].assignment_status[literal_pos] = ASSIGNED;
        
        change_stack[change_stack_top].clause_index = clause_idx;
        change_stack[change_stack_top].literal_position = literal_pos;
        change_stack_top++;
        change_counts[depth][SHRUNK]++;
        
        // ����Ƿ��Ϊ���Ӿ�
        if (clauses[clause_idx].current_length == 1) {
            int unassigned_pos = -1;
            for (int j = 0; j < clauses[clause_idx].original_length; j++) {
                if (clauses[clause_idx].assignment_status[j] == UNASSIGNED) {
                    unassigned_pos = j;
                    break;
                }
            }
            
            if (unassigned_pos != -1) {
                int unit_literal = clauses[clause_idx].literals[unassigned_pos];
                int unit_var = abs(unit_literal);
                int unit_polarity = (unit_literal > 0) ? POSITIVE : NEGATIVE;
                
                if (literal_info[unit_var][!unit_polarity].in_unit_clause) {
                    contradiction_found = TRUE;
                    conflicting_literal = unit_literal;
                } else if (!literal_info[unit_var][unit_polarity].in_unit_clause) {
                    unit_clause_stack[unit_stack_size] = clauses[clause_idx].unit_literal = unit_literal;
                    literal_info[unit_var][unit_polarity].in_unit_clause = TRUE;
                    unit_stack_size++;
                }
            }
        }
    }
    
    literal_info[var][NEGATIVE].is_assigned = TRUE;
    literal_info[var][POSITIVE].is_assigned = TRUE;
    depth++;
}

void unassign_value(int literal) {
    int var = abs(literal);
    depth--;
    
    // �ָ��������Ӿ�
    while (change_counts[depth][SHRUNK] > 0) {
        change_counts[depth][SHRUNK]--;
        ChangeRecord change = change_stack[--change_stack_top];
        int clause_idx = change.clause_index;
        int literal_pos = change.literal_position;
        
        clauses[clause_idx].current_length++;
        if (clauses[clause_idx].current_length == 2) {
            int unit_lit = clauses[clause_idx].unit_literal;
            int unit_var = abs(unit_lit);
            int unit_pol = (unit_lit > 0) ? POSITIVE : NEGATIVE;
            
            if (unit_var >= 1 && unit_var <= num_vars) {
                literal_info[unit_var][unit_pol].in_unit_clause = FALSE;
            }
            clauses[clause_idx].unit_literal = 0;
        }
        clauses[clause_idx].assignment_status[literal_pos] = UNASSIGNED;
    }
    
    // �ָ�������Ӿ�
    while (change_counts[depth][SATISFIED] > 0) {
        change_counts[depth][SATISFIED]--;
        int clause_idx = change_stack[--change_stack_top].clause_index;
        clauses[clause_idx].is_satisfied = FALSE;
        current_formula_length++;
    }
    
    literal_info[var][POSITIVE].is_assigned = FALSE;
    literal_info[var][NEGATIVE].is_assigned = FALSE;
}

int dpll() {
    dpll_call_count++;
    if (dpll_call_count % 1000000 == 0) {
        printf("%d  ", dpll_call_count / 1000000);
    }
    
    int* local_assignments = NULL;
    unsigned int local_assign_count = 0;
    
    while (TRUE) {
        if (contradiction_found) {
            // ���ݴ���ì��
            while (local_assign_count > 0) {
                int literal = local_assignments[--local_assign_count];
                unassign_value(literal);
                int var = abs(literal);
                if (var >= 1 && var <= num_vars) {
                    results[var].value = UNASSIGNED;
                }
            }
            free(local_assignments);
            contradiction_found = FALSE;
            unit_stack_size = 0;
            return UNSAT;
        } else if (unit_stack_size > 0) {
            // �����Ӿ�
            local_assignments = (int*)realloc(local_assignments, (local_assign_count + 1) * sizeof(int));
            if (local_assignments == NULL) {
                printf("DPLL�ڴ����ʧ��\n");
                exit(1);
            }
            
            int unit_literal = unit_clause_stack[--unit_stack_size];
            local_assignments[local_assign_count++] = unit_literal;
            int var = abs(unit_literal);
            
            if (var >= 1 && var <= num_vars) {
                results[var].value = (unit_literal > 0) ? TRUE : FALSE;
            }
            assign_value(unit_literal);
        } else {
            break;
        }
    }
    
    if (current_formula_length == 0) return SAT;
    
    // ѡ���֧����
    int branch_literal = select_branching_variable();
    int branch_var = abs(branch_literal);
    
    if (branch_var >= 1 && branch_var <= num_vars) {
        results[branch_var].value = (branch_literal > 0) ? TRUE : FALSE;
    }
    
    // ��������ֵ
    assign_value(branch_literal);
    if (dpll()) return SAT;
    unassign_value(branch_literal);
    
    // ���Ը���ֵ
    if (branch_var >= 1 && branch_var <= num_vars) {
        results[branch_var].value = !results[branch_var].value;
    }
    assign_value(-branch_literal);
    if (dpll()) return SAT;
    unassign_value(-branch_literal);
    
    // �ָ�״̬
    if (branch_var >= 1 && branch_var <= num_vars) {
        results[branch_var].value = UNASSIGNED;
    }
    
    while (local_assign_count > 0) {
        int literal = local_assignments[--local_assign_count];
        unassign_value(literal);
        int var = abs(literal);
        if (var >= 1 && var <= num_vars) {
            results[var].value = UNASSIGNED;
        }
    }
    
    free(local_assignments);
    contradiction_found = FALSE;
    return UNSAT;
}

int get_min_clause_length() {
    int min_length = max_clause_length;
    
    for (int i = 0; i < original_formula_length; i++) {
        if (clauses[i].is_satisfied) continue;
        
        if (clauses[i].current_length < min_length) {
            min_length = clauses[i].current_length;
            if (min_length == 2) return 2;
        }
    }
    
    return min_length;
}

void get_literal_weight(int var, int clause_len, unsigned int* pos_weight, unsigned int* neg_weight) {
    *pos_weight = *neg_weight = 0;
    
    for (int i = 0; i < literal_info[var][POSITIVE].count; i++) {
        int clause_idx = literal_info[var][POSITIVE].clause_indices[i];
        if (clauses[clause_idx].current_length == clause_len && !clauses[clause_idx].is_satisfied) {
            (*pos_weight)++;
        }
    }
    
    for (int i = 0; i < literal_info[var][NEGATIVE].count; i++) {
        int clause_idx = literal_info[var][NEGATIVE].clause_indices[i];
        if (clauses[clause_idx].current_length == clause_len && !clauses[clause_idx].is_satisfied) {
            (*neg_weight)++;
        }
    }
}

int select_branching_variable() {
    // unsigned int max_score = 0;
    // unsigned int pos_score, neg_score, total_score;
    // int selected_var = 0;
    
    // int min_clause_len = get_min_clause_length();
    
    // for (int i = 1; i <= num_vars; i++) {
    //     if (results[i].value == UNASSIGNED) {
    //         get_literal_weight(i, min_clause_len, &pos_score, &neg_score);
    //         total_score = (pos_score + 1) * (neg_score + 1);
            
    //         if (total_score > max_score) {
    //             max_score = total_score;
    //             selected_var = (pos_score >= neg_score) ? i : -i;
    //         }
    //     }
    // }
    int selected_var = 0;
    for (int i = 1; i <= num_vars; i++) {
        if (results[i].value == UNASSIGNED) {
            selected_var = i;
            break;
        }
    }
    
    return selected_var;
}

void write_result(int result_value, double time_used, char* filename) {
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
    
    FILE* file = fopen(res_filename, "w");
    if (file == NULL) {
        printf("�޷���������ļ�: %s\n", res_filename);
        return;
    }
    
    if (result_value == SAT) {
        fprintf(file, "s 1\n");
        fprintf(file, "v");
        for (int i = 1; i <= num_vars; i++) {
            if (results[i].value == TRUE) {
                fprintf(file, " %d", i);
            } else {
                fprintf(file, " -%d", i);
            }
        }
        fprintf(file, "\n");
        fprintf(file, "t %.0f\n", time_used * 1000);
    } else {
        fprintf(file, "s 0\n");
        fprintf(file, "t %.0f\n", time_used * 1000);
    }
    
    fclose(file);
}

void free_memory() {
    for (int i = 1; i <= num_vars; i++) {
        for (int j = 0; j < 2; j++) {
            if (literal_info[i][j].clause_indices) {
                free(literal_info[i][j].clause_indices);
            }
            if (literal_info[i][j].literal_positions) {
                free(literal_info[i][j].literal_positions);
            }
        }
    }
    
    if (clauses) {
        for (int i = 0; i < original_formula_length; i++) {
            if (clauses[i].literals) {
                free(clauses[i].literals);
            }
            if (clauses[i].assignment_status) {
                free(clauses[i].assignment_status);
            }
        }
        free(clauses);
    }
}