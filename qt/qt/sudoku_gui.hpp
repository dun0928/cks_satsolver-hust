#pragma once
// 给纯 C 环境用的“C”接口
#ifdef __cplusplus
extern "C" {
#endif
void runInteractiveGui(int grid[9][9], int playerGrid[9][9], int init[9][9]);

#ifdef __cplusplus
}
#endif