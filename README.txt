本版本增添了fuction get_one_index从共享内存获取一条索引记录的判断条件：((read_pos + 1)%SHM_INDEX_NUM == write_pos)；
让存储模块的read_pos不可能赶上等于write_pos，目的是避免编码模块在出现read_pos赶上write_pos时，无法判断自己下一帧可写。
