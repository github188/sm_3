本版本为存储模块和编码模块初步联合测试版本改进版，对进一步测试中出现有视频段信息输出不完整纠正bug；
具体体现在函数output_search_video_info的判断语句：
if ((atoi(timeseg[i].end_time)-atoi(timeseg[i].start_time)) >= (atoi(update_timeseg->end_time)-atoi(update_timeseg->start_time)))有问题
替换成如下：
if ((atoi(timeseg[i].end_time)-atoi(update_timeseg->end_time)) >= 0)
此外，本版本还改善了终端交互界面显示效果，采用不同颜色来显示不同数据信息。