#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 100

typedef char element[100];

typedef struct tree {
    char ch;
    int  num;
    struct tree* P;
    struct tree* R;
    struct tree* L;
} tree;

tree* find_node(tree* head, const char* path) {
    if (path == NULL || path[0] != '/') return NULL;
    if (head == NULL) return NULL;

    int i = 1;
    if (path[i] == '\0') return NULL;

    if (path[i] != head->ch) return NULL;
    tree* cur = head;
    i++;

    while (path[i] != '\0') {
        if (path[i] != '/') return NULL;
        i++;
        char c = path[i];
        if (c == '\0') return NULL;
        i++;

        tree* next = NULL;
        if (cur->L != NULL && cur->L->ch == c) next = cur->L;
        else if (cur->R != NULL && cur->R->ch == c) next = cur->R;
        if (next == NULL) return NULL;
        cur = next;
    }
    return cur;
}

tree* insert(tree* head, const char* parent_path, char child, char value) {
    if (!isupper((unsigned char)value)) {
        printf("오류: 새 데이터는 영문 대문자여야 합니다.\n");
        return head;
    }
    
    if (strcmp(parent_path, "/") == 0) {
        if (head != NULL) {
            printf("오류: 이미 루트 노드가 존재하여 새 루트를 만들 수 없습니다.\n");
            return head;
        }
        tree* new_node = (tree*)malloc(sizeof(tree));
        new_node->ch = value;
        new_node->num = 0;
        new_node->P = NULL;
        new_node->L = NULL;
        new_node->R = NULL;
        printf("루트 노드 %c 가 생성되었습니다.\n", value);
        return new_node;
    }

    if (child != 'L' && child != 'R') {
        printf("오류: child는 L(Left) 또는 R(Right)이어야 합니다.\n");
        return head;
    }

    tree* parent = find_node(head, parent_path);
    if (parent == NULL) {
        printf("오류: 부모 노드(%s)를 찾을 수 없습니다.\n", parent_path);
        return head;
    }
    if (parent->num >= 2) {
        printf("오류: 부모 노드의 자식이 이미 2개(꽉 참)입니다.\n");
        return head;
    }

    tree** slot = (child == 'L') ? &parent->L : &parent->R;
    tree* sibling = (child == 'L') ? parent->R : parent->L;

    if (*slot != NULL) {
        printf("오류: 해당 위치(%c)에 이미 자식 노드가 존재합니다.\n", child);
        return head;
    }
    if (sibling != NULL && sibling->ch == value) {
        printf("오류: 같은 부모의 형제 노드와 동일한 데이터를 가질 수 없습니다.\n");
        return head;
    }

    tree* new_node = (tree*)malloc(sizeof(tree));
    new_node->ch = value;
    new_node->num = 0;
    new_node->P = parent;
    new_node->L = NULL;
    new_node->R = NULL;

    *slot = new_node;
    parent->num++;

    printf("%s/%c 노드가 추가되었습니다.\n", parent_path, value);
    return head;
}

tree* delete_node(tree* head, const char* path) {
    tree* node = find_node(head, path);
    if (node == NULL) {
        printf("오류: 노드(%s)를 찾을 수 없습니다.\n", path);
        return head;
    }
    if (node->L != NULL || node->R != NULL) {
        printf("오류: 단말 노드가 아니므로 삭제할 수 없습니다.\n");
        return head;
    }

    tree* parent = node->P;
    if (parent == NULL) {
        free(node);
        printf("%s 노드가 삭제되었습니다.\n", path);
        return NULL;
    }
    if (parent->L == node) parent->L = NULL;
    else if (parent->R == node) parent->R = NULL;
    parent->num--;

    free(node);
    printf("%s 노드가 삭제되었습니다.\n", path);
    return head;
}

void update_node(tree* head, const char* path, char new_ch) {
    if (!isupper((unsigned char)new_ch)) {
        printf("오류: 데이터는 영문 대문자여야 합니다.\n");
        return;
    }
    tree* node = find_node(head, path);
    if (node == NULL) {
        printf("오류: 노드(%s)를 찾을 수 없습니다.\n", path);
        return;
    }
    tree* parent = node->P;
    if (parent != NULL) {
        tree* sibling = (parent->L == node) ? parent->R : parent->L;
        if (sibling != NULL && sibling->ch == new_ch) {
            printf("오류: 같은 부모의 형제 노드와 동일한 데이터를 가질 수 없습니다.\n");
            return;
        }
    }
    node->ch = new_ch;
    printf("노드의 데이터가 %c(으)로 변경되었습니다.\n", new_ch);
}

void read_node(tree* head, const char* path) {
    tree* node = find_node(head, path);
    if (node == NULL) {
        printf("오류: 노드(%s)를 찾을 수 없습니다.\n", path);
        return;
    }
    if (node->L == NULL && node->R == NULL) {
        printf("자식 노드가 없습니다.\n");
        return;
    }
    int first = 1;
    if (node->L != NULL) { printf("%c(L)", node->L->ch); first = 0; }
    if (node->R != NULL) { if (!first) printf(", "); printf("%c(R)", node->R->ch); }
    printf("\n");
}

static void print_rec(tree* node, int depth) {
    if (node == NULL) return;
    for (int i = 0; i < depth - 1; i++) printf("    ");
    printf("+---%c\n", node->ch);
    print_rec(node->L, depth + 1);
    print_rec(node->R, depth + 1);
}

void print_tree(tree* head) {
    if (head == NULL) {
        printf("트리가 비어 있습니다.\n");
        return;
    }
    printf("%c\n", head->ch);
    print_rec(head->L, 1);
    print_rec(head->R, 1);
}

void destroy_tree(tree* node) {
    if (node == NULL) return;
    destroy_tree(node->L);
    destroy_tree(node->R);
    free(node);
}

static char* next_token(char** cursor) {
    char* s = *cursor;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') { *cursor = s; return NULL; }

    char* start = s;
    while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n') s++;
    if (*s != '\0') { *s = '\0'; s++; }
    *cursor = s;
    return start;
}

static int cmd_match(const char* tok, const char* full, char abbr) {
    if (tok == NULL) return 0;
    if (tok[1] == '\0' && toupper((unsigned char)tok[0]) == abbr) return 1;
    if (strlen(tok) != strlen(full)) return 0;
    for (size_t i = 0; full[i]; i++)
        if (toupper((unsigned char)tok[i]) != toupper((unsigned char)full[i])) return 0;
    return 1;
}

int main(void) {
    tree* head = NULL;
    char line[MAX_LINE];

    printf("=== 이진트리 조작 프로그램 (포인터 기반) ===\n");
    printf("명령어: Insert(I) parent-node L|R value  (루트 생성: Insert / value)\n");
    printf("        Delete(D) leaf-node\n");
    printf("        Update(U) target-node value\n");
    printf("        Read(R) parent-node\n");
    printf("        Print(P)\n");
    printf("        Quit(Q)\n\n");

    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        char* cursor = line;
        char* cmd = next_token(&cursor);
        if (cmd == NULL) continue;

        if (cmd_match(cmd, "Insert", 'I')) {
            char* path = next_token(&cursor);
            if (path == NULL) {
                printf("오류: 인자가 부족합니다. (사용법: Insert parent-node L|R new-data)\n");
                continue;
            }
            if (strcmp(path, "/") == 0) {
                char* val = next_token(&cursor);
                char* extra = next_token(&cursor);
                if (val == NULL || extra != NULL || strlen(val) != 1) {
                    printf("오류: 인자 개수가 맞지 않습니다. (사용법: Insert / new-data)\n");
                    continue;
                }
                head = insert(head, "/", 0, val[0]);
            }
            else {
                char* childTok = next_token(&cursor);
                char* val = next_token(&cursor);
                char* extra = next_token(&cursor);
                if (childTok == NULL || val == NULL || extra != NULL || strlen(val) != 1) {
                    printf("오류: 인자 개수가 맞지 않습니다. (사용법: Insert parent-node L|R new-data)\n");
                    continue;
                }
                char child = (char)toupper((unsigned char)childTok[0]);
                if (strlen(childTok) != 1 || (child != 'L' && child != 'R')) {
                    printf("오류: child는 L(Left) 또는 R(Right)이어야 합니다.\n");
                    continue;
                }
                head = insert(head, path, child, val[0]);
            }

        }
        else if (cmd_match(cmd, "Delete", 'D')) {
            char* path = next_token(&cursor);
            char* extra = next_token(&cursor);
            if (path == NULL || extra != NULL) {
                printf("오류: 인자 개수가 맞지 않습니다. (사용법: Delete /path)\n");
                continue;
            }
            head = delete_node(head, path);

        }
        else if (cmd_match(cmd, "Update", 'U')) {
            char* path = next_token(&cursor);
            char* val = next_token(&cursor);
            char* extra = next_token(&cursor);
            if (path == NULL || val == NULL || extra != NULL || strlen(val) != 1) {
                printf("오류: 인자 개수가 맞지 않습니다. (사용법: Update /path X)\n");
                continue;
            }
            update_node(head, path, val[0]);

        }
        else if (cmd_match(cmd, "Read", 'R')) {
            char* path = next_token(&cursor);
            char* extra = next_token(&cursor);
            if (path == NULL || extra != NULL) {
                printf("오류: 인자 개수가 맞지 않습니다. (사용법: Read /path)\n");
                continue;
            }
            read_node(head, path);

        }
        else if (cmd_match(cmd, "Print", 'P')) {
            char* extra = next_token(&cursor);
            if (extra != NULL) {
                printf("오류: Print 명령은 추가 인자를 받지 않습니다.\n");
                continue;
            }
            print_tree(head);

        }
        else if (cmd_match(cmd, "Quit", 'Q') || cmd_match(cmd, "Exit", 'X')) {
            break;

        }
        else {
            printf("오류: 알 수 없는 명령어 '%s' 입니다.\n", cmd);
        }
    }

    destroy_tree(head);
    printf("프로그램을 종료합니다.\n");
    return 0;
}