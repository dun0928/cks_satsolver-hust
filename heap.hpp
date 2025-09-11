#include <stdio.h>
#include <stdlib.h>
#include "definition.h"
typedef struct {
    int length;  // 子句当前长度
    int clause_idx;  // 子句索引
} HeapNode;
HeapNode* min_heap;
int heap_size = 0;  // 堆中元素个数

// 堆下沉（维护最小堆性质）
void heapify(int i) {
    int left = 2 * i + 1;   // 左子节点索引
    int right = 2 * i + 2;  // 右子节点索引
    int smallest = i;       // 初始假设当前节点是最小的

    // 找到左/右子节点中比当前节点小的节点
    if (left < heap_size && min_heap[left].length < min_heap[smallest].length) {
        smallest = left;
    }
    if (right < heap_size && min_heap[right].length < min_heap[smallest].length) {
        smallest = right;
    }

    // 若最小节点不是当前节点，交换并递归调整
    if (smallest != i) {
        HeapNode temp = min_heap[i];
        min_heap[i] = min_heap[smallest];
        min_heap[smallest] = temp;
        heapify(smallest);
    }
}


/**
 * 弹出堆顶节点（最小长度的子句）
 */
HeapNode heap_pop() {
    if (heap_size == 0) {
        HeapNode empty = {0, -1};  // 空堆返回无效值
        return empty;
    }

    // 1. 保存堆顶节点，用堆尾节点覆盖堆顶
    HeapNode top = min_heap[0];
    min_heap[0] = min_heap[heap_size - 1];
    heap_size--;

    // 2. 下沉调整堆顶节点，恢复堆性质
    heapify(0);
    return top;
}

